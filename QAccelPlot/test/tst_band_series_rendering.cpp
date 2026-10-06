//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/linestyles/LineStyle.hpp"
#include "WindowPlacement.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <array>
#include <memory>
#include <tuple>

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

// A red band from y 2 to 6, and a blue band whose low and high values are swapped.
constexpr auto kFillScene = R"(
    BandSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        Component.onCompleted: setData([1, 9], [2, 2], [6, 6])
    }

    BandSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "blue"
        Component.onCompleted: setData([1, 9], [8.5, 8.5], [7.5, 7.5])
    }
}
)";

// A red band from y 2 to 6 with an invalid sample at x = 5.
constexpr auto kGapScene = R"(
    BandSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        Component.onCompleted: setData([1, 2, 3, 4, 5, 6, 7, 8, 9], [2, 2, 2, 2, NaN, 2, 2, 2, 2], [6, 6, 6, 6, 6, 6, 6, 6, 6])
    }
}
)";

// A transparent band with white edge lines at y 2 and y 6.
constexpr auto kEdgeScene = R"(
    BandSeries {
        objectName: "band"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "transparent"
        edges.width: 4
        edges.color: "white"
        Component.onCompleted: setData([1, 9], [2, 2], [6, 6])
    }

    DashLine {
        objectName: "dash"
        pattern: [10, 10]
    }
}
)";

// White edge lines on two transparent bands. The first band's low and high values cross at x = 5,
// so its edges follow the drawn bounds at y 2 and 8. The second band has an invalid low at x = 5.
constexpr auto kEdgeBoundsScene = R"(
    BandSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "transparent"
        edges.width: 4
        edges.color: "white"
        Component.onCompleted: setData([1, 9], [2, 8], [8, 2])
    }

    BandSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "transparent"
        edges.width: 4
        edges.color: "white"
        Component.onCompleted: setData([1, 3, 5, 7, 9], [3, 3, NaN, 3, 3], [6, 6, 6, 6, 6])
    }
}
)";

// A red band from y 10 to 100 on a logarithmic Y axis.
constexpr auto kLogScene = R"(
    BandSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        Component.onCompleted: {
            plot.yAxis.viewportMin = 1;
            plot.yAxis.viewportMax = 1000;
            plot.yAxis.logScale = true;
            setData([1, 9], [10, 10], [100, 100]);
        }
    }
}
)";

// 20,000 samples fill 8 rows of the data texture. The band steps from y 2..4 to y 6..8 at x = 5.
constexpr auto kLargeScene = R"(
    BandSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        Component.onCompleted: {
            const count = 20000;
            const xs = [], lows = [], highs = [];
            for (let i = 0; i < count; ++i) {
                const x = 10 * i / (count - 1);
                xs.push(x);
                lows.push(x < 5 ? 2 : 6);
                highs.push(x < 5 ? 4 : 8);
            }
            setData(xs, lows, highs);
        }
    }
}
)";

// A red band at epoch-millisecond X values, extended with appendData() in the test.
constexpr auto kAppendScene = R"(
    BandSeries {
        objectName: "band"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        Component.onCompleted: {
            plot.xAxis.viewportMin = 1789032600000;
            plot.xAxis.viewportMax = 1789032600010;
            setData([1789032600000, 1789032600005], [2, 2], [4, 4]);
        }
    }
}
)";

// Nested bands, y 2..8 and 4..6, under a line at y 5, like a forecast with two intervals.
constexpr auto kStackedHoverScene = R"(
    BandSeries {
        objectName: "outer"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        Component.onCompleted: setData([1, 9], [2, 2], [8, 8])
    }

    BandSeries {
        objectName: "inner"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        Component.onCompleted: setData([1, 9], [4, 4], [6, 6])
    }

    LineCurve {
        objectName: "line"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        Component.onCompleted: setData([Qt.point(1, 5), Qt.point(9, 5)])
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
        plot_ = qobject_cast<QAccelPlot*>(root_.get());
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

    QAccelPlot* plot() const
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
    QAccelPlot* plot_{nullptr};
};

bool isColor(const QColor& pixel, const QColor& expected)
{
    constexpr auto kTolerance = 60;
    return std::abs(pixel.red() - expected.red()) < kTolerance && std::abs(pixel.green() - expected.green()) < kTolerance
        && std::abs(pixel.blue() - expected.blue()) < kTolerance;
}

} // namespace

class BandSeriesRenderingTest : public QObject {
    Q_OBJECT

private slots:
    void fillSpansFromLowToHigh();
    void invalidSamplesLeaveGaps();
    void edgeLinesFollowBothBounds();
    void edgeLinesFollowTheDrawnBoundsAndGaps();
    void logScaleBandIsDrawn();
    void samplesBeyondFirstTextureRowsAreDrawn();
    void appendedEpochSamplesAreDrawnInPlace();
    void hoverReachesStackedBands();
};

void BandSeriesRenderingTest::fillSpansFromLowToHigh()
{
    auto scene = SceneWindow{kFillScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto image = scene.grab();
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 4.0)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene.pixel(1.5, 2.5)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 1.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 7.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(0.5, 4.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(9.5, 4.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 8.0)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 9.5)), Qt::black));
}

void BandSeriesRenderingTest::invalidSamplesLeaveGaps()
{
    auto scene = SceneWindow{kGapScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto image = scene.grab();
    QVERIFY(isColor(image.pixelColor(scene.pixel(3.5, 4.0)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene.pixel(4.5, 4.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.5, 4.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(6.5, 4.0)), Qt::red));
}

void BandSeriesRenderingTest::edgeLinesFollowBothBounds()
{
    auto scene = SceneWindow{kEdgeScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto image = scene.grab();
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 2.0)), Qt::white));
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 6.0)), Qt::white));
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 4.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 8.0)), Qt::black));

    auto* band = plot->findChild<QQuickItem*>(QStringLiteral("band"));
    auto* dash = plot->findChild<LineStyle*>(QStringLiteral("dash"));
    QVERIFY(band);
    QVERIFY(dash);
    band->property("edges").value<QObject*>()->setProperty("lineStyle", QVariant::fromValue(dash));
    // Counts the white and black pixels along the upper edge line.
    const auto countUpperEdge = [&scene](const QImage& grabbed) {
        auto counts = std::pair{0, 0};
        const auto upper = scene.pixel(0.0, 6.0).y();
        for (auto x = scene.pixel(2.0, 0.0).x(); x < scene.pixel(8.0, 0.0).x(); ++x) {
            const auto color = grabbed.pixelColor(x, upper);
            counts.first += isColor(color, Qt::white) ? 1 : 0;
            counts.second += isColor(color, Qt::black) ? 1 : 0;
        }
        return counts;
    };
    const auto [white, black] = countUpperEdge(scene.grab());
    QVERIFY2(white > 50 && black > 50, qPrintable(QStringLiteral("%1 white, %2 black").arg(white).arg(black)));

    // Editing the assigned style repaints the edges: long dashes leave few gaps.
    dash->setProperty("pattern", QVariant::fromValue(QList<qreal>{40, 2}));
    const auto [longWhite, longBlack] = countUpperEdge(scene.grab());
    QVERIFY2(longBlack < black / 2, qPrintable(QStringLiteral("%1 white, %2 black").arg(longWhite).arg(longBlack)));
}

void BandSeriesRenderingTest::edgeLinesFollowTheDrawnBoundsAndGaps()
{
    auto scene = SceneWindow{kEdgeBoundsScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto image = scene.grab();
    // Crossing values: straight edges at the bounds instead of two lines crossing at y = 5.
    QVERIFY(isColor(image.pixelColor(scene.pixel(4.0, 2.0)), Qt::white));
    QVERIFY(isColor(image.pixelColor(scene.pixel(4.0, 8.0)), Qt::white));
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 5.0)), Qt::black));
    // An invalid low breaks both edge lines, like the fill.
    QVERIFY(isColor(image.pixelColor(scene.pixel(2.0, 6.0)), Qt::white));
    QVERIFY(isColor(image.pixelColor(scene.pixel(4.5, 6.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.5, 6.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(8.0, 6.0)), Qt::white));
}

void BandSeriesRenderingTest::logScaleBandIsDrawn()
{
    auto scene = SceneWindow{kLogScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto image = scene.grab();
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 30.0)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 3.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 300.0)), Qt::black));
}

void BandSeriesRenderingTest::samplesBeyondFirstTextureRowsAreDrawn()
{
    auto scene = SceneWindow{kLargeScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto image = scene.grab();
    QVERIFY(isColor(image.pixelColor(scene.pixel(2.5, 3.0)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene.pixel(7.5, 7.0)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene.pixel(7.5, 3.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(2.5, 7.0)), Qt::black));
}

void BandSeriesRenderingTest::appendedEpochSamplesAreDrawnInPlace()
{
    // Static, because MSVC v142 refuses to use a local constexpr inside a lambda without a capture.
    constexpr static auto kEpoch = 1789032600000.0;
    auto scene = SceneWindow{kAppendScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));
    auto* band = plot->findChild<QQuickItem*>(QStringLiteral("band"));
    QVERIFY(band);
    const auto pixel = [&scene](const qreal offset, const qreal y) { return scene.pixel(kEpoch + offset, y); };
    QVERIFY(isColor(scene.grab().pixelColor(pixel(2.5, 3.0)), Qt::red));

    // After the first frame, appends extend the origin-relative render data in place. A step from
    // y 2..4 to y 6..8 after offset 6 shows whether the appended samples land where they belong.
    for (const auto [offset, low, high] : {std::tuple{6.0, 6.0, 8.0}, std::tuple{9.0, 6.0, 8.0}}) {
        QVERIFY(QMetaObject::invokeMethod(band, "appendData", Q_ARG(qreal, kEpoch + offset), Q_ARG(qreal, low), Q_ARG(qreal, high)));
    }
    const auto image = scene.grab();
    QVERIFY(isColor(image.pixelColor(pixel(2.5, 3.0)), Qt::red));
    QVERIFY(isColor(image.pixelColor(pixel(7.5, 7.0)), Qt::red));
    QVERIFY(isColor(image.pixelColor(pixel(7.5, 3.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(pixel(9.5, 7.0)), Qt::black));
}

void BandSeriesRenderingTest::hoverReachesStackedBands()
{
    auto scene = SceneWindow{kStackedHoverScene};
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));
    auto* outer = plot->findChild<QQuickItem*>(QStringLiteral("outer"));
    auto* inner = plot->findChild<QQuickItem*>(QStringLiteral("inner"));
    auto* line = plot->findChild<QQuickItem*>(QStringLiteral("line"));
    QVERIFY(outer && inner && line);
    // Returns which of outer, inner, and line are hovered with the mouse at data (5, y).
    const auto hoveredAt = [&](const qreal y) {
        QTest::mouseMove(&scene.window(), scene.pixel(5.0, y));
        return std::array<bool, 3>{outer->property("hovered").toBool(), inner->property("hovered").toBool(), line->property("hovered").toBool()};
    };

    // Only the topmost series under the cursor is hovered.
    QCOMPARE(hoveredAt(3.0), (std::array<bool, 3>{true, false, false}));
    QCOMPARE(hoveredAt(4.5), (std::array<bool, 3>{false, true, false}));
    QCOMPARE(hoveredAt(5.0), (std::array<bool, 3>{false, false, true}));
    QCOMPARE(hoveredAt(7.0), (std::array<bool, 3>{true, false, false}));
    QCOMPARE(hoveredAt(9.0), (std::array<bool, 3>{false, false, false}));

    // With hoverRadius 0 the line leaves hover to the band beneath it.
    line->setProperty("hoverRadius", 0.0);
    QCOMPARE(hoveredAt(5.0), (std::array<bool, 3>{false, true, false}));
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::BandSeriesRenderingTest)
#include "tst_band_series_rendering.moc"
