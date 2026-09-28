//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/series/PointCloud.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <limits>
#include <memory>
#include <vector>

namespace QAccelPlot {

namespace {

constexpr auto kWindowWidth = 400;
constexpr auto kWindowHeight = 300;

// Renders every GPU material: a gradient-stroked curve with a gradient fill and a NaN gap,
// a dashed curve, rectangles, and a point cloud, on primary and secondary axes with titles.
constexpr auto kScene = R"(
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

    property alias fillStop: fillStop

    xAxis: Axis { label: "X"; viewportMin: 0; viewportMax: 10 }
    yAxis: Axis { label: "Y"; viewportMin: 0; viewportMax: 10 }
    x2Axis: Axis { label: "X2"; viewportMin: 0; viewportMax: 10 }
    y2Axis: Axis { label: "Y2"; viewportMin: 0; viewportMax: 10 }

    LineCurve {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        lineWidth: 2
        effects: [
            GradientStroke {
                gradient: Gradient {
                    GradientStop { position: 0; color: "red" }
                    GradientStop { position: 1; color: "yellow" }
                }
            },
            GradientFill {
                direction: GradientDirection.Vertical
                gradientValueMinSource: GradientValueSource.Fixed
                gradientValueMin: 0
                gradientValueMaxSource: GradientValueSource.Fixed
                gradientValueMax: 10
                opacity: 1
                gradient: Gradient {
                    GradientStop { id: fillStop; position: 0; color: "lime" }
                    GradientStop { position: 1; color: "lime" }
                }
            }
        ]
        Component.onCompleted: setData([Qt.point(0, 5), Qt.point(2, 5), Qt.point(4, 5), Qt.point(5, NaN), Qt.point(6, 5), Qt.point(7, 5)])
    }

    LineCurve {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "white"
        lineStyle: DashLine { pattern: [6, 4] }
        Component.onCompleted: setData([Qt.point(0, 9), Qt.point(10, 9)])
    }

    RectangleList {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "blue"
        Component.onCompleted: setData([{ x1: 8, y1: 1, x2: 10, y2: 3 }])
    }

    PointCloud {
        xAxis: plot.x2Axis
        yAxis: plot.y2Axis
        marker.shape: PointCloud.Circle
        marker.size: 12
        colormap: Colormap {}
        Component.onCompleted: {
            setData([Qt.point(9, 7), Qt.point(8, 7)]);
            setValues([0, 1]);
        }
    }
}
)";

// Two point clouds, one with per-point values (stride 3) and one without, each drawn in a separate
// draw call from its own vertex buffer.
constexpr auto kPointCloudScene = R"(
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

    property alias redCloud: redCloud
    property alias cyanCloud: cyanCloud

    xAxis: Axis { viewportMin: 0; viewportMax: 10 }
    yAxis: Axis { viewportMin: 0; viewportMax: 10 }

    PointCloud {
        id: redCloud
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        marker.shape: PointCloud.Square
        marker.size: 6
        antialiasingEnabled: false
        Component.onCompleted: setData([Qt.point(1, 2), Qt.point(3, 2), Qt.point(5, 2), Qt.point(7, 2)])
    }

    PointCloud {
        id: cyanCloud
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "cyan"
        marker.shape: PointCloud.Square
        marker.size: 6
        antialiasingEnabled: false
        Component.onCompleted: {
            setData([Qt.point(2, 8), Qt.point(4, 8), Qt.point(6, 8), Qt.point(8, 8)]);
            setValues([1, 2, 3, 4]);
        }
    }
}
)";

bool isColor(const QColor& pixel, const QColor& expected)
{
    constexpr auto kTolerance = 60;
    return std::abs(pixel.red() - expected.red()) < kTolerance && std::abs(pixel.green() - expected.green()) < kTolerance
        && std::abs(pixel.blue() - expected.blue()) < kTolerance;
}

} // namespace

class PlotRenderingTest : public QObject {
    Q_OBJECT

private slots:
    void rendersFillStrokeRectanglesAndPoints();
    void rendersEveryPointOfSeveralPointClouds();
    void rendersPointsBeyondFirstTextureRows();
};

void PlotRenderingTest::rendersFillStrokeRectanglesAndPoints()
{
    auto window = QQuickWindow{};
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom curve materials require a hardware scene graph backend");
    }
    window.setColor(Qt::black);
    window.resize(kWindowWidth, kWindowHeight);

    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(kScene, QUrl{});
    const auto root = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(root, qPrintable(component.errorString()));
    auto* plot = qobject_cast<QAccelPlot*>(root.get());
    QVERIFY(plot);
    plot->setParentItem(window.contentItem());

    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const auto pixelAt
        = [plot](const QImage& image, const qreal x, const qreal y) { return image.pixelColor(qRound(plot->dataToPixelX(x)), qRound(plot->dataToPixelY(y))); };
    auto image = window.grabWindow();
    QVERIFY(isColor(pixelAt(image, 2.0, 2.5), Qt::green));
    QVERIFY(isColor(pixelAt(image, 5.0, 2.5), Qt::black));
    QVERIFY(isColor(pixelAt(image, 2.0, 7.0), Qt::black));
    QVERIFY(isColor(pixelAt(image, 9.0, 2.0), Qt::blue));

    // Editing a gradient stop in place must re-render the fill.
    auto* fillStop = root->property("fillStop").value<QObject*>();
    QVERIFY(fillStop);
    fillStop->setProperty("color", QColor{Qt::magenta});
    image = window.grabWindow();
    QVERIFY(!isColor(pixelAt(image, 2.0, 2.5), Qt::green));
    QVERIFY(isColor(pixelAt(image, 5.0, 2.5), Qt::black));
}

void PlotRenderingTest::rendersEveryPointOfSeveralPointClouds()
{
    auto window = QQuickWindow{};
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom curve materials require a hardware scene graph backend");
    }
    window.setColor(Qt::black);
    window.resize(kWindowWidth, kWindowHeight);

    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(kPointCloudScene, QUrl{});
    const auto root = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(root, qPrintable(component.errorString()));
    auto* plot = qobject_cast<QAccelPlot*>(root.get());
    QVERIFY(plot);
    plot->setParentItem(window.contentItem());

    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const auto pixelAt
        = [plot](const QImage& image, const qreal x, const qreal y) { return image.pixelColor(qRound(plot->dataToPixelX(x)), qRound(plot->dataToPixelY(y))); };
    auto image = window.grabWindow();
    for (const auto x : {1.0, 3.0, 5.0, 7.0}) {
        QVERIFY2(isColor(pixelAt(image, x, 2.0), Qt::red), qPrintable(QStringLiteral("red point at x=%1").arg(x)));
        QVERIFY(isColor(pixelAt(image, x + 1.0, 2.0), Qt::black));
    }
    for (const auto x : {2.0, 4.0, 6.0, 8.0}) {
        QVERIFY2(isColor(pixelAt(image, x, 8.0), Qt::cyan), qPrintable(QStringLiteral("cyan point at x=%1").arg(x)));
        QVERIFY(isColor(pixelAt(image, x - 1.0, 8.0), Qt::black));
    }

    // New point counts reallocate the vertex buffers. Float data is not origin-shifted, so a
    // point index offset would leave the first point undrawn instead of hiding it at the origin.
    auto* redCloud = root->property("redCloud").value<PointCloud*>();
    auto* cyanCloud = root->property("cyanCloud").value<PointCloud*>();
    QVERIFY(redCloud);
    QVERIFY(cyanCloud);
    constexpr auto kRedCount = 9;
    constexpr auto kCyanCount = 3;
    auto redXy = std::vector<float>{};
    for (auto i = 0; i < kRedCount; ++i) {
        redXy.insert(redXy.end(), {static_cast<float>(i + 1), 5.0f});
    }
    redCloud->setDataF(std::move(redXy), kRedCount);
    cyanCloud->setDataF(std::vector<float>{3.0f, 8.0f, 5.0f, 8.0f, 7.0f, 8.0f}, std::vector<float>{1.0f, 2.0f, 3.0f}, kCyanCount);
    image = window.grabWindow();
    for (auto i = 0; i < kRedCount; ++i) {
        QVERIFY2(isColor(pixelAt(image, i + 1.0, 5.0), Qt::red), qPrintable(QStringLiteral("red point at x=%1").arg(i + 1)));
    }
    for (const auto x : {3.0, 5.0, 7.0}) {
        QVERIFY2(isColor(pixelAt(image, x, 8.0), Qt::cyan), qPrintable(QStringLiteral("cyan point at x=%1").arg(x)));
    }
    QVERIFY(isColor(pixelAt(image, 1.0, 2.0), Qt::black));
    QVERIFY(isColor(pixelAt(image, 8.0, 8.0), Qt::black));
}

void PlotRenderingTest::rendersPointsBeyondFirstTextureRows()
{
    auto window = QQuickWindow{};
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom curve materials require a hardware scene graph backend");
    }
    window.setColor(Qt::black);
    window.resize(kWindowWidth, kWindowHeight);

    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(kPointCloudScene, QUrl{});
    const auto root = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(root, qPrintable(component.errorString()));
    auto* plot = qobject_cast<QAccelPlot*>(root.get());
    QVERIFY(plot);
    plot->setParentItem(window.contentItem());
    auto* redCloud = root->property("redCloud").value<PointCloud*>();
    auto* cyanCloud = root->property("cyanCloud").value<PointCloud*>();
    QVERIFY(redCloud);
    QVERIFY(cyanCloud);

    // NaN points are not drawn. With values, points 700 and 2800 start at data texture floats
    // 2100 and 8400: rows 0 and 1 of the 8192-wide texture, but rows 1 and 4 of a 2048-wide one,
    // so a width mismatch between C++ and the shader loses them.
    constexpr auto kCount = 3000;
    constexpr auto kNaN = std::numeric_limits<float>::quiet_NaN();
    auto xy = std::vector<float>(2 * kCount, kNaN);
    xy[2 * 700] = 3.0f;
    xy[2 * 700 + 1] = 5.0f;
    xy[2 * 2800] = 7.0f;
    xy[2 * 2800 + 1] = 5.0f;
    cyanCloud->setDataF(std::move(xy), std::vector<float>(kCount, 1.0f), kCount);
    redCloud->clearData();

    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const auto pixelAt
        = [plot](const QImage& image, const qreal x, const qreal y) { return image.pixelColor(qRound(plot->dataToPixelX(x)), qRound(plot->dataToPixelY(y))); };
    const auto image = window.grabWindow();
    QVERIFY(isColor(pixelAt(image, 3.0, 5.0), Qt::cyan));
    QVERIFY(isColor(pixelAt(image, 7.0, 5.0), Qt::cyan));
    QVERIFY(isColor(pixelAt(image, 5.0, 5.0), Qt::black));
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Pixel lookups use logical coordinates, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::PlotRenderingTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_plot_rendering.moc"
