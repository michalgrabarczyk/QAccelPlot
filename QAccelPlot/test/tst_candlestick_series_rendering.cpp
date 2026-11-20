//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/PlotView.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"
#include "WindowPlacement.hpp"

#include <QImage>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <memory>

namespace QAccelPlot {

namespace {

constexpr auto kScenePrefix = R"(
import QtQuick
import QAccelPlot

PlotView {
    id: plot
    width: 400
    height: 300
    plotAreaColor: "black"
    axesAreaColor: "#202020"
    grid.gridVisible: false
    grid.subGridVisible: false

    xAxis: Axis { viewportMin: 0; viewportMax: 10 }
    yAxis: Axis { viewportMin: 0; viewportMax: 10 }
)";

// Candles 1.6 data units wide at x 2, 4, 6, and 8: an up candle, a down candle, a doji, and an up
// candle without a high whose low is infinite.
constexpr auto kCandleScene = R"(
    CandlestickSeries {
        objectName: "candles"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        upColor: "lime"
        downColor: "red"
        lineWidth: 3
        Component.onCompleted: setData([2, 4, 6, 8], [4, 6, 5, 4], [8, 7, NaN, NaN], [2, 3, NaN, -Infinity], [6, 4, 5, 6])
    }
}
)";

// An OHLC bar at x 5: a line from 2 to 8, the open tick at 4 to the left, and the close tick at 6 to the right.
constexpr auto kOhlcScene = R"(
    CandlestickSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        style: CandlestickSeries.Ohlc
        upColor: "blue"
        lineWidth: 3
        spacing: 2
        Component.onCompleted: setData([5], [4], [8], [2], [6])
    }
}
)";

// An OHLC bar at x 5 on a reversed x axis.
constexpr auto kReversedOhlcScene = R"(
    CandlestickSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        style: CandlestickSeries.Ohlc
        upColor: "blue"
        lineWidth: 3
        spacing: 2
        Component.onCompleted: {
            plot.xAxis.viewportMin = 10;
            plot.xAxis.viewportMax = 0;
            setData([5], [4], [8], [2], [6]);
        }
    }
}
)";

// A candle at x 5 a twentieth of a data unit wide, widened to 30 px.
constexpr auto kNarrowScene = R"(
    CandlestickSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        upColor: "lime"
        lineWidth: 0
        spacing: 1
        candleWidth: 0.05
        minimumWidth: 30
        Component.onCompleted: setData([5], [4], [8], [2], [6])
    }
}
)";

constexpr auto kHoverScene = R"(
    CandlestickSeries {
        objectName: "candles"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        upColor: "lime"
        lineWidth: 3
        spacing: 2
        Component.onCompleted: setData([5], [4], [8], [2], [6])
    }
}
)";

// Candles at x 1, 10, and 100 on a log x axis; the spacing of 9 puts the first candle's left
// edge below 0.
constexpr auto kLogScene = R"(
    CandlestickSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        upColor: "lime"
        lineWidth: 0
        Component.onCompleted: {
            plot.xAxis.viewportMin = 0.1;
            plot.xAxis.viewportMax = 1000;
            plot.xAxis.logScale = true;
            setData([1, 10, 100], [4, 4, 4], [8, 8, 8], [2, 2, 2], [6, 6, 6]);
        }
    }
}
)";

// A translucent candlestick at x 3 and a translucent OHLC bar at x 7, each from 2 to 8 with an
// open of 4 and a close of 6.
constexpr auto kTranslucentScene = R"(
    CandlestickSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        upColor: Qt.rgba(0, 1, 0, 0.5)
        lineWidth: 5
        spacing: 2
        Component.onCompleted: setData([3], [4], [8], [2], [6])
    }

    CandlestickSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        style: CandlestickSeries.Ohlc
        upColor: Qt.rgba(0, 0, 1, 0.5)
        lineWidth: 5
        spacing: 2
        Component.onCompleted: setData([7], [4], [8], [2], [6])
    }
}
)";

constexpr auto kLegendScene = R"(
    CandlestickSeries {
        id: candles
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        upColor: "lime"
        downColor: "red"
        name: "Prices"
    }

    Legend {
        objectName: "legend"
        x: 20
        y: 20
        model: LegendModel { series: [candles] }
    }
}
)";

// Shows a PlotView built from kScenePrefix and \a sceneBody in a 400 × 300 window.
class SceneWindow {
public:
    explicit SceneWindow(const char* sceneBody)
    {
        window_.setColor(Qt::black);
        window_.resize(400, 300);
        component_.setData(QByteArray{kScenePrefix} + sceneBody, QUrl{});
        root_.reset(component_.create());
        plot_ = qobject_cast<PlotView*>(root_.get());
        if (plot_) {
            plot_->setParentItem(window_.contentItem());
            QAccelPlotTest::moveAwayFromCursor(window_);
            window_.show();
        }
    }

    bool isSoftware() const
    {
        return window_.rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
    }

    QString error() const
    {
        return component_.errorString();
    }

    QQuickWindow& window()
    {
        return window_;
    }

    PlotView* plot() const
    {
        return plot_;
    }

    QImage grab()
    {
        return window_.grabWindow();
    }

    QPoint pixel(const qreal x, const qreal y) const
    {
        return {qRound(plot_->dataToPixelX(x)), qRound(plot_->dataToPixelY(y))};
    }

private:
    QQuickWindow window_;
    QQmlEngine engine_;
    QQmlComponent component_{&engine_};
    std::unique_ptr<QObject> root_;
    PlotView* plot_{nullptr};
};

bool isColor(const QColor& pixel, const QColor& expected)
{
    constexpr auto kTolerance = 60;
    return std::abs(pixel.red() - expected.red()) < kTolerance && std::abs(pixel.green() - expected.green()) < kTolerance
        && std::abs(pixel.blue() - expected.blue()) < kTolerance;
}

// Whether \a image shows \a color somewhere in \a rect.
bool showsColorIn(const QImage& image, const QRect& rect, const QColor& color)
{
    for (auto y = rect.top(); y <= rect.bottom(); ++y) {
        for (auto x = rect.left(); x <= rect.right(); ++x) {
            if (isColor(image.pixelColor(x, y), color)) {
                return true;
            }
        }
    }
    return false;
}

} // namespace

class CandlestickSeriesRenderingTest : public QObject {
    Q_OBJECT

private slots:
    void candlesticksDrawBodiesAndWicks();
    void ohlcBarsDrawLinesAndTicks();
    void ohlcTicksKeepTheirScreenSidesOnAReversedAxis();
    void narrowCandlesKeepMinimumWidth();
    void logAxisClampsNonPositiveCandleEdgesToX();
    void hoverColorDrawsTheHoveredCandle();
    void translucentPartsDoNotOverlap();
    void legendShowsUpAndDownCandles();
};

void CandlestickSeriesRenderingTest::candlesticksDrawBodiesAndWicks()
{
    auto scene = SceneWindow{kCandleScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto image = scene.grab();
    const auto at = [&](const qreal x, const qreal y) { return image.pixelColor(scene.pixel(x, y)); };
    // The up candle: a body 1.6 wide from 4 to 6 over a wick from 2 to 8.
    QVERIFY(isColor(at(2, 5), Qt::green));
    QVERIFY(isColor(at(2.7, 5), Qt::green));
    QVERIFY(isColor(at(2.95, 5), Qt::black));
    QVERIFY(isColor(at(2, 7.5), Qt::green));
    QVERIFY(isColor(at(2.4, 7.5), Qt::black));
    QVERIFY(isColor(at(2, 2.5), Qt::green));
    QVERIFY(isColor(at(2, 8.5), Qt::black));

    // The down candle.
    QVERIFY(isColor(at(4, 5), Qt::red));
    QVERIFY(isColor(at(4, 6.5), Qt::red));
    QVERIFY(isColor(at(4, 7.5), Qt::black));

    // The doji is a body one line width tall.
    QVERIFY(isColor(at(6.5, 5), Qt::green));
    QVERIFY(isColor(at(6.5, 5.5), Qt::black));

    // Without a high the wick ends at the body; the infinite low reaches the plot edge.
    QVERIFY(isColor(at(8, 7), Qt::black));
    QVERIFY(isColor(at(8, 0.1), Qt::green));
}

void CandlestickSeriesRenderingTest::ohlcBarsDrawLinesAndTicks()
{
    auto scene = SceneWindow{kOhlcScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto image = scene.grab();
    const auto at = [&](const qreal x, const qreal y) { return image.pixelColor(scene.pixel(x, y)); };
    QVERIFY(isColor(at(5, 7.5), Qt::blue));
    QVERIFY(isColor(at(5, 2.5), Qt::blue));
    QVERIFY(isColor(at(5, 8.5), Qt::black));
    QVERIFY(isColor(at(5.4, 5), Qt::black));
    QVERIFY(isColor(at(4.4, 4), Qt::blue));
    QVERIFY(isColor(at(5.6, 4), Qt::black));
    QVERIFY(isColor(at(5.6, 6), Qt::blue));
    QVERIFY(isColor(at(4.4, 6), Qt::black));
    QVERIFY(isColor(at(4.0, 4), Qt::black));
}

void CandlestickSeriesRenderingTest::ohlcTicksKeepTheirScreenSidesOnAReversedAxis()
{
    auto scene = SceneWindow{kReversedOhlcScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto image = scene.grab();
    const auto at = [&](const qreal x, const qreal y) { return image.pixelColor(scene.pixel(x, y)); };
    // Larger x lies to the left, where the open tick still points.
    QVERIFY(isColor(at(5.6, 4), Qt::blue));
    QVERIFY(isColor(at(4.4, 4), Qt::black));
    QVERIFY(isColor(at(4.4, 6), Qt::blue));
    QVERIFY(isColor(at(5.6, 6), Qt::black));
}

void CandlestickSeriesRenderingTest::narrowCandlesKeepMinimumWidth()
{
    auto scene = SceneWindow{kNarrowScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto image = scene.grab();
    const auto center = scene.pixel(5, 5);
    const auto at = [&](const int dx) { return image.pixelColor(center + QPoint{dx, 0}); };
    QVERIFY(isColor(at(-12), Qt::green));
    QVERIFY(isColor(at(12), Qt::green));
    QVERIFY(isColor(at(-18), Qt::black));
    QVERIFY(isColor(at(18), Qt::black));
}

void CandlestickSeriesRenderingTest::logAxisClampsNonPositiveCandleEdgesToX()
{
    auto scene = SceneWindow{kLogScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto image = scene.grab();
    const auto at = [&](const qreal x, const qreal y) { return image.pixelColor(scene.pixel(x, y)); };
    // The body of the first candle starts at its x instead of the plot edge.
    QVERIFY(isColor(at(2, 5), Qt::green));
    QVERIFY(isColor(at(0.5, 5), Qt::black));
    QVERIFY(isColor(at(0.12, 5), Qt::black));
}

void CandlestickSeriesRenderingTest::hoverColorDrawsTheHoveredCandle()
{
    auto scene = SceneWindow{kHoverScene};
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));
    auto* candles = plot->findChild<QQuickItem*>(QStringLiteral("candles"));
    QVERIFY(candles);

    QTest::mouseMove(&scene.window(), scene.pixel(5.0, 9.5));
    QTest::mouseMove(&scene.window(), scene.pixel(5.0, 5.0));
    QTRY_COMPARE(candles->property("hoveredIndex").toInt(), 0);
    if (scene.isSoftware()) {
        return;
    }
    // Without a hover color the hovered candle keeps its own.
    QTRY_VERIFY(isColor(scene.grab().pixelColor(scene.pixel(5.5, 5.0)), Qt::green));
    candles->setProperty("hoverColor", QColor{Qt::white});
    QTRY_VERIFY(isColor(scene.grab().pixelColor(scene.pixel(5.5, 5.0)), Qt::white));
}

void CandlestickSeriesRenderingTest::translucentPartsDoNotOverlap()
{
    auto scene = SceneWindow{kTranslucentScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto image = scene.grab();
    const auto at = [&](const qreal x, const qreal y) { return image.pixelColor(scene.pixel(x, y)); };
    const auto sameColor = [](const QColor& a, const QColor& b) {
        constexpr auto kTolerance = 3;
        return std::abs(a.red() - b.red()) <= kTolerance && std::abs(a.green() - b.green()) <= kTolerance && std::abs(a.blue() - b.blue()) <= kTolerance;
    };
    // The body is as translucent where the wick passes as beside it, and so is the wick.
    const auto body = at(3.5, 5);
    QVERIFY(body.green() > 60 && body.green() < 200);
    QVERIFY(sameColor(at(3, 5), body));
    QVERIFY(sameColor(at(3, 7), body));
    QVERIFY(sameColor(at(3, 3), body));

    // The OHLC line is as translucent where the ticks meet it.
    const auto line = at(7, 3);
    QVERIFY(line.blue() > 60 && line.blue() < 200);
    QVERIFY(sameColor(at(7, 4), line));
    QVERIFY(sameColor(at(7, 6), line));
    QVERIFY(sameColor(at(6.4, 4), line));
    QVERIFY(sameColor(at(7.6, 6), line));
}

void CandlestickSeriesRenderingTest::legendShowsUpAndDownCandles()
{
    auto scene = SceneWindow{kLegendScene};
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));
    auto* legend = plot->findChild<QQuickItem*>(QStringLiteral("legend"));
    QVERIFY(legend);
    QTRY_VERIFY(legend->width() > 0);

    const auto image = scene.grab();
    const auto rect = legend->mapRectToScene(QRectF{0, 0, legend->width(), legend->height()}).toAlignedRect().intersected(image.rect());
    QVERIFY(showsColorIn(image, rect, Qt::green));
    QVERIFY(showsColorIn(image, rect, Qt::red));
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::CandlestickSeriesRenderingTest)
#include "tst_candlestick_series_rendering.moc"
