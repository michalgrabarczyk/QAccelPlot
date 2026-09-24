//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <memory>

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
