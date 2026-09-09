//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "BenchmarkRunner.hpp"
#include "BenchmarkWorkload.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QMetaObject>
#include <QOpenGLFunctions>
#include <QQuickRenderTarget>
#include <QSGRendererInterface>
#include <QSysInfo>
#include <QThread>

#include <cmath>

namespace QAccelPlot {

BenchmarkRunner::BenchmarkRunner(QObject* parent)
    : QObject(parent)
    , reporter_("0.1.0")
    , availableScenarios_(BenchmarkScenario::defaultScenarios())
{
    // Filter out CPU-only scenarios — those have no rendering and shouldn't appear in the GUI
    availableScenarios_.erase(std::remove_if(availableScenarios_.begin(), availableScenarios_.end(),
                                  [](const BenchmarkScenario& s) {
                                      return s.name().startsWith(QStringLiteral("data_ingestion")) || s.name().startsWith(QStringLiteral("vertex_cache"));
                                  }),
        availableScenarios_.end());

    QQuickWindow::setDefaultAlphaBuffer(true);

    renderControl_ = new QQuickRenderControl(this);
    window_ = new QQuickWindow(renderControl_);
    window_->setGeometry(0, 0, 1920, 1080);
    window_->contentItem()->setSize(QSizeF(1920, 1080));

    plot_ = new QAccelPlot(window_->contentItem());
    plot_->setSize(QSizeF(1920, 1080));

    xAxis_ = new Axis(plot_);
    xAxis_->setSide(Axis::Bottom);
    yAxis_ = new Axis(plot_);
    yAxis_->setSide(Axis::Left);

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

    fbo_ = new QOpenGLFramebufferObject(window_->size(), QOpenGLFramebufferObject::CombinedDepthStencil);
    window_->setRenderTarget(QQuickRenderTarget::fromOpenGLTexture(fbo_->texture(), window_->size()));

    connect(renderControl_, &QQuickRenderControl::renderRequested, this, &BenchmarkRunner::requestRender);
    connect(renderControl_, &QQuickRenderControl::sceneChanged, this, &BenchmarkRunner::requestRender);

    auto apiStr = QString{"OpenGL"};
    if (context_->isValid()) {
        context_->makeCurrent(offscreenSurface_);
        auto* funcs = context_->functions();
        if (funcs) {
            const auto* versionStr = reinterpret_cast<const char*>(funcs->glGetString(GL_VERSION));
            if (versionStr) {
                apiStr = QStringLiteral("OpenGL %1").arg(versionStr);
            }
        }
        context_->doneCurrent();
    }
    reporter_.setGraphicsApi(apiStr);
}

BenchmarkRunner::~BenchmarkRunner()
{
    context_->makeCurrent(offscreenSurface_);
    delete fbo_;
    renderControl_->invalidate();
    context_->doneCurrent();
    delete window_;
}

bool BenchmarkRunner::isRunning() const
{
    return running_;
}

double BenchmarkRunner::liveFps() const
{
    return liveFps_;
}

double BenchmarkRunner::liveFrameTimeMs() const
{
    return liveFrameTimeMs_;
}

double BenchmarkRunner::progress() const
{
    return progress_;
}

QString BenchmarkRunner::currentScenarioName() const
{
    return currentScenarioName_;
}

QString BenchmarkRunner::currentScenarioLabel() const
{
    return currentScenarioLabel_;
}

QVariantList BenchmarkRunner::results() const
{
    return results_;
}

QVariantList BenchmarkRunner::availableScenarios() const
{
    QVariantList list;
    for (const auto& s : availableScenarios_) {
        QVariantMap map;
        map["name"] = s.name();
        map["label"] = s.label();
        map["desc"] = s.description();
        list.append(map);
    }
    return list;
}

QString BenchmarkRunner::systemInfo() const
{
    return QStringLiteral("%1  ·  %2 logical cores  ·  Qt %3").arg(QSysInfo::prettyProductName()).arg(QThread::idealThreadCount()).arg(qVersion());
}

void BenchmarkRunner::runAll()
{
    if (running_) {
        return;
    }
    scenariosToRun_ = availableScenarios_;
    if (scenariosToRun_.empty()) {
        return;
    }

    currentScenarioIndex_ = 0;
    results_ = {};
    emit resultsChanged();

    setRunning(true);
    QMetaObject::invokeMethod(this, "runNextScenario", Qt::QueuedConnection);
}

void BenchmarkRunner::runScenario(const QString& name)
{
    if (running_) {
        return;
    }
    scenariosToRun_.clear();
    for (const auto& s : availableScenarios_) {
        if (s.name() == name) {
            scenariosToRun_.push_back(s);
            break;
        }
    }
    if (scenariosToRun_.empty()) {
        qWarning() << "BenchmarkRunner: scenario not found:" << name;
        return;
    }
    currentScenarioIndex_ = 0;
    results_ = {};
    emit resultsChanged();
    setRunning(true);
    QMetaObject::invokeMethod(this, "runNextScenario", Qt::QueuedConnection);
}

void BenchmarkRunner::addCustomScenario(
    const QString& name, const QString& label, const QString& desc, int pointCount, int curveCount, int updateMode, int durationSeconds, int warmupSeconds)
{
    availableScenarios_.emplace_back(
        name, label, desc, pointCount, curveCount, static_cast<BenchmarkScenario::UpdateMode>(updateMode), durationSeconds, warmupSeconds);
    emit availableScenariosChanged();
}

bool BenchmarkRunner::exportJson(const QString& path)
{
    return reporter_.writeJson(path);
}

void BenchmarkRunner::stop()
{
    scenarioRunning_ = false;
    renderRequested_ = false;
    dataBufferPool_.reset();
    clearCurves();
    setRunning(false);
    setCurrentScenarioName({});
    setLiveFps(0.0);
    setLiveFrameTimeMs(0.0);
    setProgress(0.0);
}

// ── private slots ──────────────────────────────────────────────────────────

void BenchmarkRunner::requestRender()
{
    if (!scenarioRunning_ || renderRequested_) {
        return;
    }

    renderRequested_ = true;
    QMetaObject::invokeMethod(this, "render", Qt::QueuedConnection);
}

void BenchmarkRunner::render()
{
    if (!renderRequested_) {
        return;
    }
    renderRequested_ = false;
    if (!scenarioRunning_) {
        return;
    }

    metrics_.beginFrame();

    if (currentUpdateMode_ == BenchmarkScenario::UpdateMode::Live || currentUpdateMode_ == BenchmarkScenario::UpdateMode::MaxRate) {
        updateData();
    } else {
        const auto viewport = staticPanViewport(currentPointCount_, scenarioTimer_.elapsed());
        xAxis_->setViewportMin(viewport.minimum);
        xAxis_->setViewportMax(viewport.maximum);
        for (auto* curve : curves_) {
            curve->update();
        }
    }

    context_->makeCurrent(offscreenSurface_);
    renderControl_->polishItems();
    renderControl_->beginFrame();
    renderControl_->sync();

    renderControl_->render();
    renderControl_->endFrame();

    metrics_.endFrame(static_cast<int64_t>(currentPointCount_) * static_cast<int64_t>(curves_.size()));
    context_->doneCurrent();

    // Publish live metrics
    const auto frameMs = metrics_.lastFrameTimeMs();
    const auto fps = metrics_.rollingFps(30);
    setLiveFps(fps);
    setLiveFrameTimeMs(frameMs);

    const auto elapsed = scenarioTimer_.elapsed();
    const auto p = std::min(1.0, elapsed / currentTargetDurationMs_);
    const auto scenarioFraction = 1.0 / static_cast<double>(scenariosToRun_.size());
    setProgress((currentScenarioIndex_ * scenarioFraction) + p * scenarioFraction);

    if (elapsed > currentTargetDurationMs_) {
        finishScenario();
    } else {
        requestRender();
    }
}

void BenchmarkRunner::runNextScenario()
{
    if (currentScenarioIndex_ >= scenariosToRun_.size()) {
        scenarioRunning_ = false;
        renderRequested_ = false;
        reporter_.printConsoleTable();
        setProgress(1.0);
        setLiveFps(0.0);
        setLiveFrameTimeMs(0.0);
        setCurrentScenarioName({});
        setCurrentScenarioLabel({});
        setRunning(false);
        emit allCompleted();
        return;
    }

    const auto& scenario = scenariosToRun_[currentScenarioIndex_];
    setCurrentScenarioName(scenario.name());
    setCurrentScenarioLabel(scenario.label());
    qDebug() << "BenchmarkRunner: running" << scenario.name();

    setupScene(scenario);

    metrics_ = BenchmarkMetrics{};
    metrics_.setWarmupDurationMs(scenario.warmupSeconds() * 1000);
    metrics_.start();

    currentTargetDurationMs_ = (scenario.warmupSeconds() + scenario.durationSeconds()) * 1000.0;

    scenarioRunning_ = true;
    scenarioTimer_.start();

    requestRender();
}

void BenchmarkRunner::updateData()
{
    if (!dataBufferPool_) {
        return;
    }
    ++currentFrameIndex_;
    for (size_t i = 0; i < curves_.size(); ++i) {
        const auto* dataPtr = dataBufferPool_->curveBufferData(static_cast<int>(i), currentFrameIndex_);
        curves_[i]->setDataFNoRange(dataPtr, currentPointCount_);
    }
    requestRender();
}

void BenchmarkRunner::finishScenario()
{
    scenarioRunning_ = false;
    // Scene updates may have queued another render while the final frame was
    // being processed. Do not let that stale request block the next scenario's
    // explicit initial render request.
    renderRequested_ = false;

    const auto& scenario = scenariosToRun_[currentScenarioIndex_];
    const auto res = metrics_.results();
    reporter_.addResult(scenario, res);

    auto entry = QVariantMap{};
    entry[QStringLiteral("name")] = scenario.name();
    entry[QStringLiteral("label")] = scenario.label();
    entry[QStringLiteral("description")] = scenario.description();
    entry[QStringLiteral("pointCount")] = scenario.pointCount();
    entry[QStringLiteral("curveCount")] = scenario.curveCount();
    entry[QStringLiteral("fpsAvg")] = res.fpsAvg;
    entry[QStringLiteral("fpsMin")] = res.fpsMin;
    entry[QStringLiteral("frameTimeP50")] = res.frameTimeP50Ms;
    entry[QStringLiteral("frameTimeP95")] = res.frameTimeP95Ms;
    entry[QStringLiteral("frameTimeP99")] = res.frameTimeP99Ms;
    entry[QStringLiteral("frameIntervalP50")] = res.frameIntervalP50Ms;
    entry[QStringLiteral("frameIntervalP95")] = res.frameIntervalP95Ms;
    entry[QStringLiteral("frameIntervalP99")] = res.frameIntervalP99Ms;
    entry[QStringLiteral("totalFrames")] = static_cast<qint64>(res.totalFrames);
    entry[QStringLiteral("throughputMpps")] = res.dataThroughputMpps;
    results_.append(entry);
    emit resultsChanged();
    emit scenarioCompleted(scenario.name());

    currentScenarioIndex_++;
    QMetaObject::invokeMethod(this, "runNextScenario", Qt::QueuedConnection);
}

// ── private helpers ────────────────────────────────────────────────────────

void BenchmarkRunner::setupScene(const BenchmarkScenario& scenario)
{
    clearCurves();
    currentPointCount_ = scenario.pointCount();
    currentUpdateMode_ = scenario.updateMode();
    currentFrameIndex_ = 0;

    for (auto i = 0; i < scenario.curveCount(); ++i) {
        auto* curve = new LineCurve(plot_);
        curve->setXAxis(xAxis_);
        curve->setYAxis(yAxis_);
        curves_.push_back(curve);
    }

    xAxis_->setViewportMin(0);
    xAxis_->setViewportMax(currentPointCount_);
    yAxis_->setViewportMin(-2.0);
    yAxis_->setViewportMax(2.0);

    dataBufferPool_.reset();
    dataBufferPool_ = std::make_unique<ExistingDataBufferPool>(currentPointCount_, scenario.curveCount(), sourceBufferCount(currentUpdateMode_));
    for (size_t i = 0; i < curves_.size(); ++i) {
        const auto* dataPtr = dataBufferPool_->curveBufferData(static_cast<int>(i), 0);
        curves_[i]->setDataFNoRangeWithCache(dataPtr, currentPointCount_, std::vector<char>(dataBufferPool_->vertexCache()));
    }
    dataBufferPool_->discardVertexCache();
}

void BenchmarkRunner::clearCurves()
{
    for (auto* curve : curves_) {
        delete curve;
    }
    curves_.clear();
}

void BenchmarkRunner::setRunning(bool v)
{
    if (running_ != v) {
        running_ = v;
        emit runningChanged();
    }
}

void BenchmarkRunner::setLiveFps(double v)
{
    if (liveFps_ != v) {
        liveFps_ = v;
        emit liveFpsChanged();
    }
}

void BenchmarkRunner::setLiveFrameTimeMs(double v)
{
    if (liveFrameTimeMs_ != v) {
        liveFrameTimeMs_ = v;
        emit liveFrameTimeMsChanged();
    }
}

void BenchmarkRunner::setProgress(double v)
{
    if (progress_ != v) {
        progress_ = v;
        emit progressChanged();
    }
}

void BenchmarkRunner::setCurrentScenarioName(const QString& name)
{
    if (currentScenarioName_ != name) {
        currentScenarioName_ = name;
        emit currentScenarioNameChanged();
    }
}

void BenchmarkRunner::setCurrentScenarioLabel(const QString& label)
{
    if (currentScenarioLabel_ != label) {
        currentScenarioLabel_ = label;
        emit currentScenarioLabelChanged();
    }
}
} // namespace QAccelPlot
