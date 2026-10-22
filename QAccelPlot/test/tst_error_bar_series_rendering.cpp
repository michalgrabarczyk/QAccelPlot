//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"
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

// A red vertical error bar at x 5 from y 2 to 8, a blue horizontal one at y 9 from x 2 to 8, and a
// green one at x 1 whose high bound is infinite.
constexpr auto kErrorBarScene = R"(
    ErrorBarSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        lineWidth: 3
        capLength: 20
        Component.onCompleted: setData([5], [5], [2], [8])
    }

    ErrorBarSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        orientation: Qt.Horizontal
        color: "blue"
        lineWidth: 3
        capLength: 20
        Component.onCompleted: setErrorData([5], [9], [3])
    }

    ErrorBarSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "lime"
        lineWidth: 3
        capLength: 20
        Component.onCompleted: setData([1], [5], [3], [Infinity])
    }
}
)";

// A red error bar under a blue point at its value.
constexpr auto kHoverScene = R"(
    ErrorBarSeries {
        objectName: "errorBars"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        hoverColor: "white"
        lineWidth: 3
        Component.onCompleted: setData([5], [5], [2], [8])
    }

    PointCloud {
        objectName: "points"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "blue"
        Component.onCompleted: setData([Qt.point(5, 5)])
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

class ErrorBarSeriesRenderingTest : public QObject {
    Q_OBJECT

private slots:
    void errorBarsDrawLinesAndCaps();
    void hoverReachesErrorBarsBeneathMarkers();
};

void ErrorBarSeriesRenderingTest::errorBarsDrawLinesAndCaps()
{
    auto scene = SceneWindow{kErrorBarScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto image = scene.grab();
    const auto at = [&](const qreal x, const qreal y, const QPoint& shift = {}) { return image.pixelColor(scene.pixel(x, y) + shift); };
    // The vertical line and its caps, 20 px long.
    QVERIFY(isColor(at(5, 5), Qt::red));
    QVERIFY(isColor(at(5, 2.1), Qt::red));
    QVERIFY(isColor(at(5, 2, {8, 0}), Qt::red));
    QVERIFY(isColor(at(5, 8, {-8, 0}), Qt::red));
    QVERIFY(isColor(at(5, 2, {14, 0}), Qt::black));
    QVERIFY(isColor(at(5, 5, {6, 0}), Qt::black));
    QVERIFY(isColor(at(5, 1.5), Qt::black));
    QVERIFY(isColor(at(5, 8.5), Qt::black));

    // The horizontal line from the errors around x, and its caps.
    QVERIFY(isColor(at(3, 9), Qt::blue));
    QVERIFY(isColor(at(2, 9, {0, 8}), Qt::blue));
    QVERIFY(isColor(at(8, 9, {0, -8}), Qt::blue));
    QVERIFY(isColor(at(1.5, 9), Qt::black));

    // The infinite bound reaches the plot edge.
    QVERIFY(isColor(at(1, 9.9), Qt::green));
    QVERIFY(isColor(at(1, 2.5), Qt::black));
}

void ErrorBarSeriesRenderingTest::hoverReachesErrorBarsBeneathMarkers()
{
    auto scene = SceneWindow{kHoverScene};
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));
    auto* errorBars = plot->findChild<QQuickItem*>(QStringLiteral("errorBars"));
    auto* points = plot->findChild<QQuickItem*>(QStringLiteral("points"));
    QVERIFY(errorBars && points);
    const auto hoveredAt = [&](const qreal y) {
        QTest::mouseMove(&scene.window(), scene.pixel(5.0, 9.5));
        QTest::mouseMove(&scene.window(), scene.pixel(5.0, y));
        return std::pair{errorBars->property("hoveredIndex").toInt(), points->property("hoveredIndex").toInt()};
    };

    // The point drawn on top takes hover at the value; the error bar beneath it elsewhere.
    QCOMPARE(hoveredAt(5.0), (std::pair{-1, 0}));
    QCOMPARE(hoveredAt(3.0), (std::pair{0, -1}));
    QCOMPARE(hoveredAt(9.0), (std::pair{-1, -1}));

    // A point with hover turned off leaves it to the error bar.
    points->setProperty("hoverEnabled", false);
    QCOMPARE(hoveredAt(5.0), (std::pair{0, -1}));

    if (scene.isSoftware()) {
        return;
    }
    QTRY_VERIFY(isColor(scene.grab().pixelColor(scene.pixel(5.0, 3.0)), Qt::white));
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::ErrorBarSeriesRenderingTest)
#include "tst_error_bar_series_rendering.moc"
