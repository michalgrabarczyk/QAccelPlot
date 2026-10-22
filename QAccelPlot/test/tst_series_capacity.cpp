//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/series/BarSeries.hpp"
#include "QAccelPlot/series/HeatmapSeries.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/series/RectangleSeries.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQuickWindow>
#include <QRegularExpression>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <atomic>
#include <memory>
#include <vector>

namespace QAccelPlot {

namespace {

constexpr auto kSize = 100;
// With QACCELPLOT_MAX_TEXTURE_SIZE=1 the data texture holds 8192 floats.
constexpr auto kTextureFloats = 8192;

// A 100 px window whose series map the data range [0, 1] onto it.
struct Scene {
    QQuickWindow window;
    Axis xAxis;
    Axis yAxis;

    Scene()
    {
        window.setColor(Qt::black);
        window.resize(kSize, kSize);
        xAxis.setOrientation(Qt::Horizontal);
        yAxis.setOrientation(Qt::Vertical);
    }

    bool isSoftware() const
    {
        return window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
    }

    void add(PlotSeries& series)
    {
        series.setParentItem(window.contentItem());
        series.setXAxis(&xAxis);
        series.setYAxis(&yAxis);
        series.setPlotRect({0, 0, kSize, kSize});
    }
};

bool isLit(const QImage& image, const int x, const int y)
{
    return qGray(image.pixel(x, y)) > 200;
}

bool isDark(const QImage& image, const int x, const int y)
{
    return qGray(image.pixel(x, y)) < 50;
}

// Counts the capacity warnings while it is alive and passes other messages on.
class CapacityWarningCounter {
public:
    CapacityWarningCounter()
    {
        count_ = 0;
        previousHandler_ = qInstallMessageHandler(handle);
    }

    ~CapacityWarningCounter()
    {
        qInstallMessageHandler(previousHandler_);
    }

    CapacityWarningCounter(const CapacityWarningCounter&) = delete;
    CapacityWarningCounter& operator=(const CapacityWarningCounter&) = delete;

    int count() const
    {
        return count_;
    }

private:
    static void handle(const QtMsgType type, const QMessageLogContext& context, const QString& message)
    {
        if (type == QtWarningMsg && message.contains(QStringLiteral("but draws at most"))) {
            ++count_;
            return;
        }
        previousHandler_(type, context, message);
    }

    static inline std::atomic<int> count_{0};
    static inline QtMessageHandler previousHandler_{nullptr};
};

// \a pointCount points along a horizontal line through the middle of the plot.
std::vector<float> horizontalLine(const int pointCount)
{
    auto data = std::vector<float>{};
    for (auto index = 0; index < pointCount; ++index) {
        data.push_back(static_cast<float>(index) / static_cast<float>(pointCount - 1));
        data.push_back(0.5f);
    }
    return data;
}

// \a rectCount copies of one rectangle in the middle of the plot.
std::vector<float> stackedRectangles(const int rectCount)
{
    auto data = std::vector<float>{};
    for (auto index = 0; index < rectCount; ++index) {
        data.insert(data.end(), {0.25f, 0.25f, 0.75f, 0.75f});
    }
    return data;
}

} // namespace

// A series draws and hovers the records its GPU buffers hold and warns about the rest each time its data outgrows them.
class SeriesCapacityTest : public QObject {
    Q_OBJECT

private slots:
    void linePointsBeyondCapacityAreNotDrawnOrHovered();
    void rectanglesBeyondCapacityAreNotDrawnOrHovered();
    void barsBeyondCapacityAreNotDrawnOrHovered();
    void heatmapBeyondCapacityIsNotDrawn();
    void warnsEachTimeDataOutgrowsCapacity();
    void eachSeriesWarnsAboutItsOwnData();
};

void SeriesCapacityTest::linePointsBeyondCapacityAreNotDrawnOrHovered()
{
    auto scene = Scene{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    constexpr auto kCapacity = kTextureFloats / 2;
    constexpr auto kPointCount = 6000;

    // Points within the capacity run along the upper line, the ones beyond it along the lower line.
    auto data = std::vector<float>{};
    for (auto index = 0; index < kPointCount; ++index) {
        const auto upper = index < kCapacity;
        const auto first = upper ? 0 : kCapacity;
        const auto last = upper ? kCapacity - 1 : kPointCount - 1;
        data.push_back(static_cast<float>(index - first) / static_cast<float>(last - first));
        data.push_back(upper ? 0.75f : 0.25f);
    }
    auto curve = LineCurve{};
    scene.add(curve);
    curve.setColor(Qt::white);
    curve.setLineWidth(6.0);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve has 6000 points but draws at most 4096.*"));
    curve.setDataF(std::move(data), kPointCount);

    scene.window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window));
    const auto image = scene.window.grabWindow();
    QVERIFY(isLit(image, 50, 25));
    QVERIFY(isDark(image, 50, 75));

    const auto& item = static_cast<const QQuickItem&>(curve);
    QVERIFY(item.contains({50, 25}));
    QVERIFY(!item.contains({50, 75}));

    curve.marker()->setShape(PlotSeries::MarkerShape::Circle);
    curve.setHoverRadius(0.0);
    QVERIFY(item.contains({50, 25}));
    QVERIFY(!item.contains({50, 75}));
}

void SeriesCapacityTest::rectanglesBeyondCapacityAreNotDrawnOrHovered()
{
    auto scene = Scene{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    constexpr auto kCapacity = kTextureFloats / 4;
    constexpr auto kRectangleCount = 3000;

    // Rectangles within the capacity cover the left, the ones beyond it the right.
    auto data = std::vector<float>{};
    for (auto index = 0; index < kRectangleCount; ++index) {
        const auto left = index < kCapacity ? 0.1f : 0.6f;
        data.insert(data.end(), {left, 0.1f, left + 0.3f, 0.9f});
    }
    auto rectangles = RectangleSeries{};
    scene.add(rectangles);
    rectangles.setColor(Qt::white);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleSeries has 3000 rectangles but draws at most 2048.*"));
    rectangles.setDataF(std::move(data), kRectangleCount);

    scene.window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window));
    const auto image = scene.window.grabWindow();
    QVERIFY(isLit(image, 25, 50));
    QVERIFY(isDark(image, 75, 50));

    const auto drawn = rectangles.rectangleIndexAt({25, 50});
    QVERIFY(drawn >= 0 && drawn < kCapacity);
    QCOMPARE(rectangles.rectangleIndexAt({75, 50}), -1);
}

void SeriesCapacityTest::barsBeyondCapacityAreNotDrawnOrHovered()
{
    auto scene = Scene{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    constexpr auto kCapacity = kTextureFloats / 2;
    constexpr auto kBarCount = 6000;

    // Bars within the capacity stand at the left, the ones beyond it at the right.
    auto data = std::vector<float>{};
    for (auto index = 0; index < kBarCount; ++index) {
        data.insert(data.end(), {index < kCapacity ? 0.25f : 0.75f, 0.8f});
    }
    auto bars = BarSeries{};
    scene.add(bars);
    bars.setColor(Qt::white);
    bars.setBarWidth(0.3);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BarSeries has 6000 bars but draws at most 4096.*"));
    bars.setDataF(std::move(data), kBarCount);

    scene.window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window));
    const auto image = scene.window.grabWindow();
    QVERIFY(isLit(image, 25, 50));
    QVERIFY(isDark(image, 75, 50));

    const auto drawn = bars.barIndexAt({25, 50});
    QVERIFY(drawn >= 0 && drawn < kCapacity);
    QCOMPARE(bars.barIndexAt({75, 50}), -1);
}

void SeriesCapacityTest::heatmapBeyondCapacityIsNotDrawn()
{
    auto scene = Scene{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    // One float per cell: the data texture holds 8192 cells.
    constexpr auto kOverSide = 100;
    constexpr auto kUnderSide = 50;
    auto heatmap = HeatmapSeries{};
    scene.add(heatmap);
    heatmap.setXMin(0.0);
    heatmap.setXMax(1.0);
    heatmap.setYMin(0.0);
    heatmap.setYMax(1.0);
    scene.window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window));

    QTest::ignoreMessage(QtWarningMsg, "HeatmapSeries has 10000 cells but draws at most 8192 ; the series is not drawn.");
    heatmap.setValues(std::vector<float>(kOverSide * kOverSide, 1.0f), kOverSide, kOverSide);
    QCOMPARE(scene.window.grabWindow().pixel(50, 50), qRgb(0, 0, 0));

    heatmap.setValues(std::vector<float>(kUnderSide * kUnderSide, 1.0f), kUnderSide, kUnderSide);
    QVERIFY(scene.window.grabWindow().pixel(50, 50) != qRgb(0, 0, 0));
}

void SeriesCapacityTest::warnsEachTimeDataOutgrowsCapacity()
{
    auto scene = Scene{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    constexpr auto kOverCapacity = 5000;
    constexpr auto kUnderCapacity = 3000;
    auto curve = LineCurve{};
    scene.add(curve);
    scene.window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window));
    const auto counter = CapacityWarningCounter{};
    const auto render = [&](const int pointCount) {
        curve.setDataF(horizontalLine(pointCount), pointCount);
        scene.window.grabWindow();
    };

    render(kOverCapacity);
    QCOMPARE(counter.count(), 1);
    // Streaming data that stays over capacity warns only once.
    render(kOverCapacity + 1);
    render(kOverCapacity + 2);
    QCOMPARE(counter.count(), 1);
    render(kUnderCapacity);
    QCOMPARE(counter.count(), 1);
    render(kOverCapacity);
    QCOMPARE(counter.count(), 2);
    curve.setDataF(std::vector<float>{}, 0);
    scene.window.grabWindow();
    render(kOverCapacity);
    QCOMPARE(counter.count(), 3);
}

void SeriesCapacityTest::eachSeriesWarnsAboutItsOwnData()
{
    auto scene = Scene{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    constexpr auto kOverCapacity = 3000;
    auto first = RectangleSeries{};
    auto second = RectangleSeries{};
    scene.add(first);
    scene.add(second);
    scene.window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window));
    const auto counter = CapacityWarningCounter{};

    first.setDataF(stackedRectangles(kOverCapacity), kOverCapacity);
    scene.window.grabWindow();
    QCOMPARE(counter.count(), 1);
    second.setDataF(stackedRectangles(kOverCapacity), kOverCapacity);
    scene.window.grabWindow();
    QCOMPARE(counter.count(), 2);
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    qputenv("QACCELPLOT_MAX_TEXTURE_SIZE", "1");
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::SeriesCapacityTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_series_capacity.moc"
