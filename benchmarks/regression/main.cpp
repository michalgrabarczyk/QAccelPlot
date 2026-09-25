//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "BenchmarkMetrics.hpp"
#include "BenchmarkReporter.hpp"
#include "BenchmarkScenario.hpp"
#include "BenchmarkWorkload.hpp"
#include "ExistingDataBufferPool.hpp"

#include <QAccelPlot/QAccelPlot.hpp>
#include <QAccelPlot/series/LineCurve.hpp>
#include <QAccelPlot/series/PointCloud.hpp>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQuickRenderControl>
#include <QQuickRenderTarget>
#include <QQuickWindow>
#include <memory>
#include <vector>

class HeadlessBenchmarkRunner : public QObject {
    Q_OBJECT
public:
    HeadlessBenchmarkRunner(QObject* parent = nullptr)
        : QObject(parent)
    {
        // Initialize Qt Quick offscreen rendering
        QQuickWindow::setDefaultAlphaBuffer(true);

        renderControl_ = new QQuickRenderControl(this);
        window_ = new QQuickWindow(renderControl_);
        window_->setGeometry(0, 0, 1920, 1080);
        window_->contentItem()->setSize(QSizeF(1920, 1080));

        // Create plotting components
        plot_ = new QAccelPlot::QAccelPlot(window_->contentItem());
        plot_->setSize(QSizeF(1920, 1080));

        xAxis_ = new QAccelPlot::Axis(plot_);
        xAxis_->setSide(QAccelPlot::Axis::Bottom);
        yAxis_ = new QAccelPlot::Axis(plot_);
        yAxis_->setSide(QAccelPlot::Axis::Left);

        plot_->setXAxis(xAxis_);
        plot_->setYAxis(yAxis_);

        context_ = new QOpenGLContext(this);
        context_->setFormat(window_->requestedFormat());
        context_->create();

        offscreenSurface_ = new QOffscreenSurface(nullptr, this);
        offscreenSurface_->setFormat(context_->format());
        offscreenSurface_->create();

        context_->makeCurrent(offscreenSurface_);
        renderControl_->initialize();

        // Create FBO for rendering
        fbo_ = new QOpenGLFramebufferObject(window_->size(), QOpenGLFramebufferObject::CombinedDepthStencil);
        window_->setRenderTarget(QQuickRenderTarget::fromOpenGLTexture(fbo_->texture(), window_->size()));

        connect(renderControl_, &QQuickRenderControl::renderRequested, this, &HeadlessBenchmarkRunner::requestRender);
        connect(renderControl_, &QQuickRenderControl::sceneChanged, this, &HeadlessBenchmarkRunner::requestRender);

        reporter_.setGraphicsApi(QQuickWindow::graphicsApi() == QSGRendererInterface::OpenGL ? "OpenGL" : "Software");
        if (context_->isValid() && context_->hasExtension("GL_VERSION")) {
            reporter_.setGraphicsApi(QString("OpenGL %1").arg(reinterpret_cast<const char*>(context_->functions()->glGetString(GL_VERSION))));
        }
    }

    ~HeadlessBenchmarkRunner()
    {
        context_->makeCurrent(offscreenSurface_);
        delete fbo_;
        renderControl_->invalidate();
        context_->doneCurrent();
        delete window_; // Deletes plot_ and curves_
    }

    void setScenarios(const std::vector<QAccelPlot::BenchmarkScenario>& scenarios)
    {
        scenarios_ = scenarios;
        currentScenarioIndex_ = 0;
    }

    void setOutput(const QString& path)
    {
        outputPath_ = path;
    }

    void setThresholdFps(double fps)
    {
        thresholdFps_ = fps;
    }

public slots:

    void start()
    {
        if (scenarios_.empty()) {
            QCoreApplication::quit();
            return;
        }
        runNextScenario();
    }

private slots:

    void requestRender()
    {
        if (!scenarioRunning_ || renderRequested_) {
            return;
        }

        renderRequested_ = true;
        QMetaObject::invokeMethod(this, "render", Qt::QueuedConnection);
    }

    void render()
    {
        if (!renderRequested_) {
            return;
        }
        renderRequested_ = false;
        if (!scenarioRunning_) {
            return;
        }

        metrics_.beginFrame();

        if (currentUpdateMode_ == QAccelPlot::BenchmarkScenario::UpdateMode::Live || currentUpdateMode_ == QAccelPlot::BenchmarkScenario::UpdateMode::MaxRate) {
            updateData();
        } else {
            const auto viewport = QAccelPlot::staticPanViewport(currentPointCount_, scenarioTimer_.elapsed());
            xAxis_->setViewportMin(viewport.minimum);
            xAxis_->setViewportMax(viewport.maximum);
            for (auto* series : series_) {
                series->update();
            }
        }

        context_->makeCurrent(offscreenSurface_);

        renderControl_->polishItems();

        renderControl_->beginFrame();
        renderControl_->sync();

        renderControl_->render();
        renderControl_->endFrame();

        metrics_.endFrame(static_cast<int64_t>(currentPointCount_) * static_cast<int64_t>(series_.size()));

        context_->doneCurrent();

        if (scenarioTimer_.elapsed() > currentTargetDurationMs_) {
            finishScenario();
        } else {
            requestRender();
        }
    }

    void runNextScenario()
    {
        if (currentScenarioIndex_ >= scenarios_.size()) {
            scenarioRunning_ = false;
            renderRequested_ = false;
            reporter_.printConsoleTable();
            if (!outputPath_.isEmpty()) {
                if (reporter_.writeJson(outputPath_)) {
                    qDebug() << "Results written to" << outputPath_;
                } else {
                    qWarning() << "Failed to write results to" << outputPath_;
                }
            }
            if (failedThreshold_) {
                qWarning() << "One or more scenarios failed to meet the FPS threshold.";
                QCoreApplication::exit(1);
            } else {
                QCoreApplication::quit();
            }
            return;
        }

        const auto& scenario = scenarios_[currentScenarioIndex_];

        // Skip non-rendering scenarios
        if (scenario.isCpuOnly()) {
            currentScenarioIndex_++;
            QMetaObject::invokeMethod(this, "runNextScenario", Qt::QueuedConnection);
            return;
        }

        qDebug() << "Running scenario:" << scenario.name();

        setupScenario(scenario);

        metrics_ = QAccelPlot::BenchmarkMetrics();
        metrics_.setWarmupDurationMs(scenario.warmupSeconds() * 1000);
        metrics_.start();

        currentTargetDurationMs_ = (scenario.warmupSeconds() + scenario.durationSeconds()) * 1000.0;

        scenarioRunning_ = true;
        scenarioTimer_.start();

        // Initial render trigger
        requestRender();
    }

    void setupScenario(const QAccelPlot::BenchmarkScenario& scenario)
    {
        for (auto* series : series_) {
            delete series;
        }
        series_.clear();

        currentPointCount_ = scenario.pointCount();
        currentUpdateMode_ = scenario.updateMode();
        currentSeriesType_ = scenario.seriesType();
        currentFrameIndex_ = 0;

        for (auto i = 0; i < scenario.curveCount(); ++i) {
            auto* series = createSeries();
            series->setXAxis(xAxis_);
            series->setYAxis(yAxis_);
            series_.push_back(series);
        }

        xAxis_->setViewportMin(0);
        xAxis_->setViewportMax(currentPointCount_);
        yAxis_->setViewportMin(-2.0);
        yAxis_->setViewportMax(2.0);

        dataBufferPool_.reset();
        dataBufferPool_ = std::make_unique<QAccelPlot::ExistingDataBufferPool>(
            currentPointCount_, scenario.curveCount(), QAccelPlot::sourceBufferCount(currentUpdateMode_));
        pointValues_.clear();
        if (currentSeriesType_ == QAccelPlot::BenchmarkScenario::SeriesType::PointCloud
            && currentUpdateMode_ != QAccelPlot::BenchmarkScenario::UpdateMode::Static) {
            pointValues_.resize(static_cast<std::size_t>(currentPointCount_));
            for (auto i = std::size_t{0}; i < pointValues_.size(); ++i) {
                pointValues_[i] = static_cast<float>(i % 256) / 255.0f;
            }
        }
        for (size_t i = 0; i < series_.size(); ++i) {
            const auto* dataPtr = dataBufferPool_->curveBufferData(static_cast<int>(i), 0);
            if (auto* curve = qobject_cast<QAccelPlot::LineCurve*>(series_[i])) {
                curve->setDataFNoRangeWithCache(dataPtr, currentPointCount_, std::vector<char>(dataBufferPool_->vertexCache()));
            } else {
                setPointCloudData(static_cast<QAccelPlot::PointCloud*>(series_[i]), dataPtr);
            }
        }
        dataBufferPool_->discardVertexCache();
    }

    QAccelPlot::PlotSeries* createSeries()
    {
        if (currentSeriesType_ == QAccelPlot::BenchmarkScenario::SeriesType::PointCloud) {
            auto* cloud = new QAccelPlot::PointCloud(plot_);
            cloud->marker()->setSize(1.5);
            return cloud;
        }
        return new QAccelPlot::LineCurve(plot_);
    }

    void setPointCloudData(QAccelPlot::PointCloud* cloud, const float* dataPtr)
    {
        // Copying from an existing buffer mirrors LineCurve::setDataFNoRange(const float*, int).
        auto xy = std::vector<float>(dataPtr, dataPtr + static_cast<std::size_t>(currentPointCount_) * 2);
        cloud->setDataFNoRange(std::move(xy), std::vector<float>(pointValues_), currentPointCount_);
    }

    void updateData()
    {
        if (!dataBufferPool_) {
            return;
        }
        ++currentFrameIndex_;
        for (size_t i = 0; i < series_.size(); ++i) {
            const auto* dataPtr = dataBufferPool_->curveBufferData(static_cast<int>(i), currentFrameIndex_);
            if (auto* curve = qobject_cast<QAccelPlot::LineCurve*>(series_[i])) {
                curve->setDataFNoRange(dataPtr, currentPointCount_);
            } else {
                setPointCloudData(static_cast<QAccelPlot::PointCloud*>(series_[i]), dataPtr);
            }
        }
        requestRender();
    }

    void finishScenario()
    {
        scenarioRunning_ = false;
        // Discard any scene-change request queued by the final frame so the
        // next scenario can always schedule its own first render.
        renderRequested_ = false;

        const auto& scenario = scenarios_[currentScenarioIndex_];
        const auto results = metrics_.results();
        reporter_.addResult(scenario, results);

        if (thresholdFps_ > 0.0 && results.fpsAvg < thresholdFps_) {
            failedThreshold_ = true;
            qWarning() << "Scenario" << scenario.name() << "failed threshold: Avg FPS" << results.fpsAvg << "<" << thresholdFps_;
        }

        currentScenarioIndex_++;
        QMetaObject::invokeMethod(this, "runNextScenario", Qt::QueuedConnection);
    }

private:
    QQuickRenderControl* renderControl_{nullptr};
    QQuickWindow* window_{nullptr};
    QOpenGLContext* context_{nullptr};
    QOffscreenSurface* offscreenSurface_{nullptr};
    QOpenGLFramebufferObject* fbo_{nullptr};

    QAccelPlot::QAccelPlot* plot_{nullptr};
    QAccelPlot::Axis* xAxis_{nullptr};
    QAccelPlot::Axis* yAxis_{nullptr};
    std::vector<QAccelPlot::PlotSeries*> series_;
    std::vector<float> pointValues_;

    std::vector<QAccelPlot::BenchmarkScenario> scenarios_;
    size_t currentScenarioIndex_{0};

    std::unique_ptr<QAccelPlot::ExistingDataBufferPool> dataBufferPool_;

    QAccelPlot::BenchmarkReporter reporter_;
    QAccelPlot::BenchmarkMetrics metrics_;
    QElapsedTimer scenarioTimer_;
    double currentTargetDurationMs_{0};
    int currentPointCount_{0};
    int currentFrameIndex_{0};
    QAccelPlot::BenchmarkScenario::UpdateMode currentUpdateMode_;
    QAccelPlot::BenchmarkScenario::SeriesType currentSeriesType_{QAccelPlot::BenchmarkScenario::SeriesType::LineCurve};

    bool renderRequested_{false};
    bool scenarioRunning_{false};
    QString outputPath_;
    double thresholdFps_{0.0};
    bool failedThreshold_{false};
};

int main(int argc, char* argv[])
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    // Need a GUI application for QQuickRenderControl
    auto app = QGuiApplication{argc, argv};
    QCoreApplication::setApplicationName("QAccelPlotBenchmarkRegression");
    QCoreApplication::setApplicationVersion("0.3.0");

    auto parser = QCommandLineParser{};
    parser.setApplicationDescription("QAccelPlot headless regression benchmarks");
    parser.addHelpOption();
    parser.addVersionOption();

    auto outputOption = QCommandLineOption{QStringList() << "o" << "output", "Output JSON results to <file>.", "file"};
    parser.addOption(outputOption);

    auto scenarioOption = QCommandLineOption{QStringList() << "s" << "scenario", "Run specific scenario.", "name"};
    parser.addOption(scenarioOption);

    auto thresholdOption = QCommandLineOption{QStringList() << "t" << "threshold-fps", "Fail if average FPS is below this value.", "fps"};
    parser.addOption(thresholdOption);

    parser.process(app);

    const auto allScenarios = QAccelPlot::BenchmarkScenario::defaultScenarios();
    auto scenariosToRun = std::vector<QAccelPlot::BenchmarkScenario>{};

    if (parser.isSet(scenarioOption)) {
        const auto scenarioName = parser.value(scenarioOption);
        for (const auto& s : allScenarios) {
            if (s.name() == scenarioName) {
                scenariosToRun.push_back(s);
                break;
            }
        }
        if (scenariosToRun.empty()) {
            qWarning() << "Scenario not found:" << scenarioName;
            return 1;
        }
    } else {
        scenariosToRun = allScenarios;
    }

    auto runner = HeadlessBenchmarkRunner{};
    runner.setScenarios(scenariosToRun);
    if (parser.isSet(outputOption)) {
        runner.setOutput(parser.value(outputOption));
    }
    if (parser.isSet(thresholdOption)) {
        runner.setThresholdFps(parser.value(thresholdOption).toDouble());
    }

    QMetaObject::invokeMethod(&runner, "start", Qt::QueuedConnection);

    return app.exec();
}

#include "main.moc"
