//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/axis/Axis.hpp"

#include <QGuiApplication>
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

// Red bars at x = 2 (0..6) and x = 5 (from the baseline 2 down to 1), and a blue bar at x = 8
// that starts at the plot bottom.
constexpr auto kVerticalScene = R"(
    BarSeries {
        objectName: "bars"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        barWidth: 1
        Component.onCompleted: setData([{ position: 2, value: 6 }, { position: 5, value: 1 }])
    }

    BarSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "blue"
        barWidth: 1
        baselineValue: -Infinity
        Component.onCompleted: setData([{ position: 8, value: 3 }])
    }
}
)";

// A red horizontal bar at y = 2 from 0 to 6, and a NaN bar at y = 7 that is not drawn.
constexpr auto kHorizontalScene = R"(
    BarSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        orientation: Qt.Horizontal
        color: "red"
        barWidth: 1
        Component.onCompleted: setData([{ position: 2, value: 6 }, { position: 7, value: NaN }])
    }
}
)";

// A red bar far narrower than a pixel at x = 5, and blue and lime bars that pick category colors.
constexpr auto kNarrowAndCategoryScene = R"(
    BarSeries {
        objectName: "narrow"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        barWidth: 0.0001
        minimumWidth: 8
        Component.onCompleted: setData([{ position: 5, value: 8 }])
    }

    BarSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "blue"
        categoryColors: ["lime"]
        barWidth: 1
        Component.onCompleted: setData([{ position: 2, value: 4 }, { position: 8, value: 4, category: 0 }])
    }
}
)";

// Two red bars with a 3 px white outline that turn yellow while hovered.
constexpr auto kBorderAndHoverScene = R"(
    BarSeries {
        objectName: "hoverable"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        hoverColor: "yellow"
        border.width: 3
        border.color: "white"
        barWidth: 2
        Component.onCompleted: setData([{ position: 3, value: 8 }, { position: 7, value: 8 }])
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

class BarSeriesRenderingTest : public QObject {
    Q_OBJECT

private slots:
    void verticalBarsSpanFromBaselineToValue();
    void geometryChangesMoveTheBars();
    void logValueAxisStartsBarsAtThePlotEdge();
    void horizontalBarsGrowAlongX();
    void narrowBarsKeepMinimumWidthAndCategoriesSelectColors();
    void borderAndHoverColorApplyToBars();
};

void BarSeriesRenderingTest::verticalBarsSpanFromBaselineToValue()
{
    auto scene = SceneWindow{kVerticalScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    auto image = scene.grab();
    QVERIFY(isColor(image.pixelColor(scene.pixel(2.0, 3.0)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene.pixel(2.0, 7.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(2.7, 3.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 0.5)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 1.5)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(8.0, 2.0)), Qt::blue));

    // Scrolled up, the -Infinity baseline still reaches the plot bottom; the red bar starts below the view.
    plot->yAxis()->setViewportMin(1.0);
    image = scene.grab();
    const auto bottom = plot->plotRect().toAlignedRect().bottom() - 2;
    QVERIFY(isColor(image.pixelColor(scene.pixel(8.0, 0.0).x(), bottom), Qt::blue));
    QVERIFY(isColor(image.pixelColor(scene.pixel(2.0, 0.0).x(), bottom), Qt::red));
}

void BarSeriesRenderingTest::geometryChangesMoveTheBars()
{
    auto scene = SceneWindow{kVerticalScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));
    auto* bars = plot->findChild<QQuickItem*>(QStringLiteral("bars"));
    QVERIFY(bars);

    bars->setProperty("barOffset", 1.0);
    bars->setProperty("barWidth", 0.4);
    bars->setProperty("baselineValue", 4.0);
    const auto image = scene.grab();
    QVERIFY(isColor(image.pixelColor(scene.pixel(3.0, 5.0)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene.pixel(3.0, 3.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(2.0, 5.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(3.3, 5.0)), Qt::black));
    // The bar at x = 5 now spans from the baseline 4 down to its value 1.
    QVERIFY(isColor(image.pixelColor(scene.pixel(6.0, 2.0)), Qt::red));
}

void BarSeriesRenderingTest::logValueAxisStartsBarsAtThePlotEdge()
{
    auto scene = SceneWindow{kVerticalScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    plot->yAxis()->setViewportMin(0.1);
    plot->yAxis()->setViewportMax(100.0);
    plot->yAxis()->setLogScale(true);
    const auto image = scene.grab();
    const auto bottom = plot->plotRect().toAlignedRect().bottom() - 2;
    QVERIFY(isColor(image.pixelColor(scene.pixel(2.0, 0.0).x(), bottom), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene.pixel(2.0, 3.0)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene.pixel(2.0, 20.0)), Qt::black));
}

void BarSeriesRenderingTest::horizontalBarsGrowAlongX()
{
    auto scene = SceneWindow{kHorizontalScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto image = scene.grab();
    QVERIFY(isColor(image.pixelColor(scene.pixel(3.0, 2.0)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene.pixel(0.5, 2.0)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene.pixel(7.0, 2.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(3.0, 2.7)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(3.0, 7.0)), Qt::black));
}

void BarSeriesRenderingTest::narrowBarsKeepMinimumWidthAndCategoriesSelectColors()
{
    auto scene = SceneWindow{kNarrowAndCategoryScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto narrow = scene.pixel(5.0, 4.0);
    auto image = scene.grab();
    for (const auto offset : {-3, 0, 3}) {
        QVERIFY(isColor(image.pixelColor(narrow.x() + offset, narrow.y()), Qt::red));
    }
    for (const auto offset : {-7, 7}) {
        QVERIFY(isColor(image.pixelColor(narrow.x() + offset, narrow.y()), Qt::black));
    }
    QVERIFY(isColor(image.pixelColor(scene.pixel(2.0, 2.0)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(scene.pixel(8.0, 2.0)), Qt::green));

    // The minimum applies across the bar only, so the bar keeps its length.
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 8.5)), Qt::black));

    plot->findChild<QQuickItem*>(QStringLiteral("narrow"))->setProperty("minimumWidth", 0.0);
    image = scene.grab();
    auto redPixels = 0;
    for (auto offset = -3; offset <= 3; ++offset) {
        redPixels += isColor(image.pixelColor(narrow.x() + offset, narrow.y()), Qt::red) ? 1 : 0;
    }
    QVERIFY(redPixels <= 1);
}

void BarSeriesRenderingTest::borderAndHoverColorApplyToBars()
{
    auto scene = SceneWindow{kBorderAndHoverScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));
    auto* bars = plot->findChild<QQuickItem*>(QStringLiteral("hoverable"));
    QVERIFY(bars);

    const auto first = scene.pixel(3.0, 4.0);
    const auto second = scene.pixel(7.0, 4.0);
    const auto leftEdge = scene.pixel(2.0, 4.0);
    const auto topEdge = scene.pixel(3.0, 8.0);
    auto image = scene.grab();
    QVERIFY(isColor(image.pixelColor(first), Qt::red));
    QVERIFY(isColor(image.pixelColor(leftEdge.x() + 1, leftEdge.y()), Qt::white));
    QVERIFY(isColor(image.pixelColor(topEdge.x(), topEdge.y() + 1), Qt::white));

    const auto hover = [bars](const QEvent::Type type, const QPoint& windowPosition) {
        const auto position = bars->mapFromScene(windowPosition);
        auto event = QHoverEvent{type, position, position, position};
        QCoreApplication::sendEvent(bars, &event);
    };
    hover(QEvent::HoverEnter, second);
    QCOMPARE(bars->property("hoveredIndex").toInt(), 1);
    image = scene.grab();
    QVERIFY(isColor(image.pixelColor(first), Qt::red));
    QVERIFY(isColor(image.pixelColor(second), Qt::yellow));
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Pixel lookups use logical coordinates, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::BarSeriesRenderingTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_bar_series_rendering.moc"
