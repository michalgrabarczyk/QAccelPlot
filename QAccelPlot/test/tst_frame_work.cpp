//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Checks how much work a whole plot does per frame, by counting instead of timing: full scene
// graph rebuilds and vertex bytes from Qt Quick's batch renderer log (QSG_RENDERER_DEBUG), and
// data texture uploads from the library. Counts don't depend on the machine, so CI can check them.
//
#include "WindowPlacement.hpp"

#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/materials/internal/DataTextureUploadTotals.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"

#include <QGuiApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QRegularExpression>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <cmath>
#include <cstdint>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

namespace QAccelPlot {

namespace {

constexpr auto kWindowWidth = 640;
constexpr auto kWindowHeight = 400;
constexpr auto kWarmupFrames = 3;
constexpr auto kMeasuredFrames = 4;
// Counts are compared at a record count and at four times as many records.
constexpr auto kSmallCount = 2000;
constexpr auto kLargeCount = kSmallCount * 4;
constexpr auto kHeatmapColumns = 100;

// The work Qt Quick and the library did over some frames.
struct FrameWork {
    int frames{0};
    int fullRebuilds{0};
    std::int64_t vertexBytes{0};
    std::int64_t dataTextureUploads{0};
    std::int64_t dataTextureBytes{0};
};

// Collects the batch renderer's debug output, enabled with QSG_RENDERER_DEBUG=build,upload.
class RendererLog {
public:
    RendererLog()
    {
        instance_ = this;
        previous_ = qInstallMessageHandler(&RendererLog::handle);
    }

    ~RendererLog()
    {
        qInstallMessageHandler(previous_);
        instance_ = nullptr;
    }

    RendererLog(const RendererLog&) = delete;
    RendererLog& operator=(const RendererLog&) = delete;

    // Returns the work since the last call and starts counting from zero.
    FrameWork take()
    {
        const auto locker = QMutexLocker{&mutex_};
        auto work = std::exchange(work_, FrameWork{});
        const auto uploads = Internal::dataTextureUploadTotals();
        Internal::resetDataTextureUploadTotals();
        work.dataTextureUploads = uploads.count;
        work.dataTextureBytes = uploads.bytes;
        return work;
    }

private:
    static void handle(const QtMsgType type, const QMessageLogContext& context, const QString& message)
    {
        // Qt Quick logs the renderer output as plain debug messages; everything else goes on to Qt Test.
        auto* const log = instance_;
        if (!log) {
            return;
        }
        const auto isRendererOutput = type == QtDebugMsg && (!context.category || qstrcmp(context.category, "default") == 0);
        if (isRendererOutput) {
            log->record(message);
        } else if (log->previous_) {
            log->previous_(type, context, message);
        }
    }

    void record(const QString& message)
    {
        static const auto kBatchUpload = QRegularExpression{QStringLiteral(R"(^ - batch .* vbo: .*:\s*(\d+)\s*$)")};
        const auto locker = QMutexLocker{&mutex_};
        if (message.startsWith(QLatin1String("Renderer::render()"))) {
            ++work_.frames;
            if (message.contains(QLatin1String("rebuild: full"))) {
                ++work_.fullRebuilds;
            }
            return;
        }
        const auto match = kBatchUpload.match(message);
        if (match.hasMatch()) {
            work_.vertexBytes += match.captured(1).toLongLong();
        }
    }

    static inline RendererLog* instance_{nullptr};
    QtMessageHandler previous_{nullptr};
    QMutex mutex_;
    FrameWork work_;
};

// The data record layouts of the series types, as PlotSeries::setDataF() takes them.
enum class Layout { Value, XY, XLowHigh, XYLowHigh, Rectangle };

int valuesPerRecord(const Layout layout)
{
    switch (layout) {
    case Layout::Value:
        return 1;
    case Layout::XY:
        return 2;
    case Layout::XLowHigh:
        return 3;
    case Layout::XYLowHigh:
    case Layout::Rectangle:
        return 4;
    }
    return 0;
}

// A series type in the test scene, and the layout of its data records.
struct SeriesKind {
    QString type;
    QString properties;
    Layout layout{Layout::XY};
};

void appendRecord(std::vector<float>& data, const Layout layout, const float x, const float y, const int count)
{
    switch (layout) {
    case Layout::Value:
        data.push_back(y);
        break;
    case Layout::XY:
        data.insert(data.end(), {x, y});
        break;
    case Layout::XLowHigh:
        data.insert(data.end(), {x, y - 0.2F, y + 0.2F});
        break;
    case Layout::XYLowHigh:
        data.insert(data.end(), {x, y, y - 0.2F, y + 0.2F});
        break;
    case Layout::Rectangle: {
        const auto halfWidth = 0.25F / static_cast<float>(count);
        data.insert(data.end(), {x - halfWidth, y - 0.2F, x + halfWidth, y + 0.2F});
        break;
    }
    }
}

// Returns \a count records spread over x in [0, 1], shifted by \a phase.
std::vector<float> records(const Layout layout, const int count, const float phase)
{
    auto data = std::vector<float>{};
    data.reserve(static_cast<std::size_t>(count) * static_cast<std::size_t>(valuesPerRecord(layout)));
    for (auto i = 0; i < count; ++i) {
        const auto x = static_cast<float>(i) / static_cast<float>(count - 1);
        const auto y = std::sin(x * 31.4F + phase);
        appendRecord(data, layout, x, y, count);
    }
    return data;
}

QString sceneQml(const SeriesKind& kind, const int seriesCount)
{
    auto series = QString{};
    for (auto i = 0; i < seriesCount; ++i) {
        series += QStringLiteral("    %1 { xAxis: plot.xAxis; yAxis: plot.yAxis; %2 }\n").arg(kind.type, kind.properties);
    }
    return QStringLiteral(R"(
import QtQuick
import QAccelPlot

PlotView {
    id: plot
    width: %1
    height: %2
    xAxis: Axis { label: "X"; viewportMin: 0; viewportMax: 1 }
    yAxis: Axis { label: "Y"; viewportMin: -2; viewportMax: 2 }
%3}
)")
        .arg(kWindowWidth)
        .arg(kWindowHeight)
        .arg(series);
}

// A shown window with one plot, whose series all hold the same data.
class PlotScene {
public:
    PlotScene(const SeriesKind& kind, const int seriesCount, const int recordCount)
        : kind_(kind)
        , recordCount_(recordCount)
    {
        window_.resize(kWindowWidth, kWindowHeight);
        QAccelPlotTest::moveAwayFromCursor(window_);
        auto component = QQmlComponent{&engine_};
        component.setData(sceneQml(kind, seriesCount).toUtf8(), QUrl{});
        root_.reset(component.create());
        if (!root_) {
            errors_ = component.errorString();
            return;
        }
        plot_ = qobject_cast<QAccelPlot*>(root_.get());
        plot_->setParentItem(window_.contentItem());
        for (auto* child : plot_->childItems()) {
            if (auto* series = qobject_cast<PlotSeries*>(child)) {
                series_.push_back(series);
            }
        }
    }

    // Fills the series and shows the window. Aborts the test run when that fails, because the
    // measurements return values and can't use QVERIFY.
    void show()
    {
        if (!plot_ || series_.empty()) {
            qFatal("Invalid scene: %s", qPrintable(errors_));
        }
        setData(0.0F);
        window_.show();
        if (!QTest::qWaitForWindowExposed(&window_)) {
            qFatal("Window not exposed");
        }
    }

    void setData(const float phase)
    {
        const auto data = records(kind_.layout, recordCount_, phase);
        for (auto* series : series_) {
            series->setDataF(data.data(), recordCount_);
        }
    }

    // Hands each series a buffer from a worker thread and waits until the series has it.
    void postData(const float phase)
    {
        auto revisions = std::vector<quint64>{};
        for (auto* series : series_) {
            revisions.push_back(series->dataRevision());
        }
        auto worker = std::thread{[this, phase] {
            for (auto* series : series_) {
                series->postData(records(kind_.layout, recordCount_, phase), recordCount_);
            }
        }};
        worker.join();
        for (auto i = std::size_t{0}; i < series_.size(); ++i) {
            QTRY_VERIFY(series_[i]->dataRevision() != revisions[i]);
        }
    }

    void pan(const double offset)
    {
        plot_->xAxis()->setViewportMin(offset);
        plot_->xAxis()->setViewportMax(offset + 0.8);
    }

    void renderFrame()
    {
        (void)window_.grabWindow();
    }

private:
    SeriesKind kind_;
    int recordCount_;
    QQuickWindow window_;
    QQmlEngine engine_;
    std::unique_ptr<QObject> root_;
    QAccelPlot* plot_{nullptr};
    std::vector<PlotSeries*> series_;
    QString errors_;
};

// Data rows for every series type. Dash lines recompute screen-space arc lengths for every vertex
// whenever the view or the data changes, so their vertex uploads grow with the record count.
void addSeriesRows()
{
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("properties");
    QTest::addColumn<int>("layout");
    QTest::addColumn<bool>("vertexDataFollowsRecords");

    const auto row = [](const char* name, const char* type, const QString& properties, const Layout layout, const bool vertexDataFollowsRecords) {
        QTest::newRow(name) << QString::fromLatin1(type) << properties << static_cast<int>(layout) << vertexDataFollowsRecords;
    };
    row("LineCurve", "LineCurve", {}, Layout::XY, false);
    row("LineCurve steps", "LineCurve", QStringLiteral("stepMode: StepMode.After"), Layout::XY, false);
    row("LineCurve markers", "LineCurve", QStringLiteral("marker.shape: LineCurve.Circle"), Layout::XY, false);
    row("LineCurve dash", "LineCurve", QStringLiteral("lineStyle: DashLine { pattern: [6, 4] }"), Layout::XY, true);
    row("PointCloud", "PointCloud", {}, Layout::XY, false);
    row("RectangleSeries", "RectangleSeries", {}, Layout::Rectangle, false);
    row("BarSeries", "BarSeries", QStringLiteral("barWidth: 0.0001"), Layout::XY, false);
    row("BandSeries", "BandSeries", {}, Layout::XLowHigh, false);
    row("ErrorBarSeries", "ErrorBarSeries", {}, Layout::XYLowHigh, false);
    row("HeatmapSeries", "HeatmapSeries", QStringLiteral("columns: %1; xMin: 0; xMax: 1; yMin: -2; yMax: 2").arg(kHeatmapColumns), Layout::Value, false);
}

SeriesKind fetchSeriesKind()
{
    QFETCH(QString, type);
    QFETCH(QString, properties);
    QFETCH(int, layout);
    return {type, properties, static_cast<Layout>(layout)};
}

} // namespace

class FrameWorkTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void panningUploadsNoSeriesData_data();
    void panningUploadsNoSeriesData();
    void replacingDataUploadsItOnce_data();
    void replacingDataUploadsItOnce();
    void postedDataUploadsItOnce_data();
    void postedDataUploadsItOnce();
    void replacingDataOfSeveralCurvesUploadsEachOnce();

private:
    enum class DataApi { Copied, Posted };

    // Renders kWarmupFrames, then measures kMeasuredFrames that each call step(frame).
    template <typename Step> FrameWork measure(PlotScene& scene, const Step& step);
    // Measures panning over unchanged data at \a recordCount records.
    FrameWork measurePanning(const SeriesKind& kind, int recordCount);
    // Measures replacing the data of \a seriesCount series every frame at \a recordCount records.
    FrameWork measureDataUpdates(const SeriesKind& kind, int seriesCount, int recordCount, DataApi api);
    void checkDataUpdates(DataApi api);

    std::unique_ptr<RendererLog> log_;
};

void FrameWorkTest::initTestCase()
{
    auto window = QQuickWindow{};
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom series materials require a hardware scene graph backend");
    }
    log_ = std::make_unique<RendererLog>();
}

template <typename Step> FrameWork FrameWorkTest::measure(PlotScene& scene, const Step& step)
{
    for (auto frame = 0; frame < kWarmupFrames; ++frame) {
        scene.renderFrame();
    }
    // The first frames rebuild the whole scene graph; without them the renderer log is missing.
    const auto warmup = log_->take();
    if (warmup.frames == 0 || warmup.fullRebuilds == 0) {
        qFatal(
            "Qt Quick logged %d frames and %d full rebuilds while warming up; QSG_RENDERER_DEBUG output may have changed.", warmup.frames, warmup.fullRebuilds);
    }
    for (auto frame = 0; frame < kMeasuredFrames; ++frame) {
        step(frame);
        scene.renderFrame();
    }
    return log_->take();
}

FrameWork FrameWorkTest::measurePanning(const SeriesKind& kind, const int recordCount)
{
    auto scene = PlotScene{kind, 1, recordCount};
    scene.show();
    return measure(scene, [&scene](const int frame) { scene.pan(0.02 * (frame + 1)); });
}

FrameWork FrameWorkTest::measureDataUpdates(const SeriesKind& kind, const int seriesCount, const int recordCount, const DataApi api)
{
    auto scene = PlotScene{kind, seriesCount, recordCount};
    scene.show();
    return measure(scene, [&scene, api](const int frame) {
        const auto phase = 0.1F * static_cast<float>(frame + 1);
        if (api == DataApi::Posted) {
            scene.postData(phase);
        } else {
            scene.setData(phase);
        }
    });
}

void FrameWorkTest::panningUploadsNoSeriesData_data()
{
    addSeriesRows();
}

// Panning only changes uniforms and the grid. Adding or removing scene graph nodes would make
// Qt Quick rebuild every batch and upload all series vertex data again.
void FrameWorkTest::panningUploadsNoSeriesData()
{
    const auto kind = fetchSeriesKind();
    QFETCH(bool, vertexDataFollowsRecords);

    const auto small = measurePanning(kind, kSmallCount);
    const auto large = measurePanning(kind, kLargeCount);

    QCOMPARE(small.fullRebuilds, 0);
    QCOMPARE(large.fullRebuilds, 0);
    QCOMPARE(small.dataTextureUploads, 0);
    QCOMPARE(large.dataTextureUploads, 0);
    if (!vertexDataFollowsRecords) {
        QCOMPARE(large.vertexBytes, small.vertexBytes);
    }
}

void FrameWorkTest::checkDataUpdates(const DataApi api)
{
    const auto kind = fetchSeriesKind();
    QFETCH(bool, vertexDataFollowsRecords);

    const auto small = measureDataUpdates(kind, 1, kSmallCount, api);
    const auto large = measureDataUpdates(kind, 1, kLargeCount, api);

    QCOMPARE(small.fullRebuilds, 0);
    QCOMPARE(large.fullRebuilds, 0);
    QCOMPARE(small.dataTextureUploads, kMeasuredFrames);
    QCOMPARE(large.dataTextureUploads, kMeasuredFrames);
    QVERIFY(small.dataTextureBytes > 0);
    QCOMPARE(large.dataTextureBytes, small.dataTextureBytes * (kLargeCount / kSmallCount));
    if (!vertexDataFollowsRecords) {
        QCOMPARE(large.vertexBytes, small.vertexBytes);
    }
}

void FrameWorkTest::replacingDataUploadsItOnce_data()
{
    addSeriesRows();
}

// Same-sized data reuses the scene graph nodes and the vertex buffers, and uploads the data
// texture once per frame.
void FrameWorkTest::replacingDataUploadsItOnce()
{
    checkDataUpdates(DataApi::Copied);
}

void FrameWorkTest::postedDataUploadsItOnce_data()
{
    addSeriesRows();
}

void FrameWorkTest::postedDataUploadsItOnce()
{
    checkDataUpdates(DataApi::Posted);
}

void FrameWorkTest::replacingDataOfSeveralCurvesUploadsEachOnce()
{
    constexpr auto kCurveCount = 8;
    const auto kind = SeriesKind{QStringLiteral("LineCurve"), QStringLiteral("hoverEnabled: false"), Layout::XY};

    const auto work = measureDataUpdates(kind, kCurveCount, kSmallCount, DataApi::Copied);

    QCOMPARE(work.fullRebuilds, 0);
    QCOMPARE(work.dataTextureUploads, kCurveCount * kMeasuredFrames);
    QCOMPARE(work.dataTextureBytes, std::int64_t{kCurveCount} * kMeasuredFrames * kSmallCount * 2 * std::int64_t{sizeof(float)});
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Read once, when Qt Quick renders its first frame.
    qputenv("QSG_RENDERER_DEBUG", "build,upload");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::FrameWorkTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_frame_work.moc"
