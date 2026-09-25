//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
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

// Six white series on black, one per data unit along X. With 40 px per unit, data (x, y)
// maps to pixel (40 * x, 40 * (1 - y)). Antialiasing is off so sampled pixels are fully covered.
constexpr auto kPixelsPerUnit = 40;
constexpr auto kWindowWidth = 6 * kPixelsPerUnit;
constexpr auto kWindowHeight = kPixelsPerUnit;

constexpr auto kScene = R"(
import QtQuick
import QAccelPlot

Item {
    id: root
    width: 240
    height: 40

    property real seriesOpacity: 1
    property real groupOpacity: 1

    Axis { id: xAxis; orientation: Axis.Horizontal; viewportMin: 0; viewportMax: 6 }
    Axis { id: yAxis; orientation: Axis.Vertical; viewportMin: 0; viewportMax: 1 }

    Item {
        anchors.fill: parent
        opacity: root.groupOpacity

        // Two overlapping squares spanning pixels 4-28 and 12-36.
        PointCloud {
            anchors.fill: parent
            opacity: root.seriesOpacity
            xAxis: xAxis
            yAxis: yAxis
            color: "white"
            antialiasingEnabled: false
            marker.shape: PointCloud.Square
            marker.size: 12
            Component.onCompleted: setData([Qt.point(0.4, 0.5), Qt.point(0.6, 0.5)])
        }

        LineCurve {
            anchors.fill: parent
            opacity: root.seriesOpacity
            xAxis: xAxis
            yAxis: yAxis
            color: "white"
            lineWidth: 8
            antialiasingEnabled: false
            Component.onCompleted: setData([Qt.point(1.1, 0.5), Qt.point(1.9, 0.5)])
        }

        LineCurve {
            anchors.fill: parent
            opacity: root.seriesOpacity
            xAxis: xAxis
            yAxis: yAxis
            color: "white"
            antialiasingEnabled: false
            lineStyle: NoLine {}
            marker.shape: LineCurve.Square
            marker.size: 12
            Component.onCompleted: setData([Qt.point(2.5, 0.5)])
        }

        LineCurve {
            anchors.fill: parent
            opacity: root.seriesOpacity
            xAxis: xAxis
            yAxis: yAxis
            lineWidth: 8
            antialiasingEnabled: false
            effects: GradientStroke {
                gradient: Gradient {
                    GradientStop { position: 0; color: "white" }
                    GradientStop { position: 1; color: "white" }
                }
            }
            Component.onCompleted: setData([Qt.point(3.1, 0.5), Qt.point(3.9, 0.5)])
        }

        LineCurve {
            anchors.fill: parent
            opacity: root.seriesOpacity
            xAxis: xAxis
            yAxis: yAxis
            color: "white"
            effects: GradientFill {
                opacity: 1
                gradient: Gradient {
                    GradientStop { position: 0; color: "white" }
                    GradientStop { position: 1; color: "white" }
                }
            }
            Component.onCompleted: setData([Qt.point(4.1, 0.9), Qt.point(4.9, 0.9)])
        }

        RectangleList {
            anchors.fill: parent
            opacity: root.seriesOpacity
            // RectangleList draws only inside a non-empty plotRect, which PlotView normally sets.
            plotRect: Qt.rect(0, 0, width, height)
            xAxis: xAxis
            yAxis: yAxis
            color: "white"
            Component.onCompleted: setData([{ x1: 5.1, y1: 0.1, x2: 5.9, y2: 0.9 }])
        }
    }
}
)";

struct Scene {
    std::unique_ptr<QQuickWindow> window;
    std::unique_ptr<QQmlEngine> engine;
    std::unique_ptr<QObject> root;
};

int grayAt(const QImage& image, const QPoint& pixel)
{
    return qGray(image.pixel(pixel));
}

} // namespace

class SeriesOpacityTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void seriesOpacityBlendsWithBackground_data();
    void seriesOpacityBlendsWithBackground();
    void overlappingMarkersBlendPerFragment();

private:
    QImage render(qreal seriesOpacity, qreal groupOpacity);

    Scene scene_;
};

void SeriesOpacityTest::initTestCase()
{
    scene_.window = std::make_unique<QQuickWindow>();
    if (scene_.window->rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom materials do not render with the software scene graph backend");
    }
    scene_.window->setColor(Qt::black);
    scene_.window->resize(kWindowWidth, kWindowHeight);

    scene_.engine = std::make_unique<QQmlEngine>();
    auto component = QQmlComponent{scene_.engine.get()};
    component.setData(kScene, QUrl{});
    scene_.root.reset(component.create());
    QVERIFY2(scene_.root, qPrintable(component.errorString()));
    auto* rootItem = qobject_cast<QQuickItem*>(scene_.root.get());
    QVERIFY(rootItem);
    rootItem->setParentItem(scene_.window->contentItem());

    scene_.window->show();
    QVERIFY(QTest::qWaitForWindowExposed(scene_.window.get()));
}

QImage SeriesOpacityTest::render(const qreal seriesOpacity, const qreal groupOpacity)
{
    scene_.root->setProperty("seriesOpacity", seriesOpacity);
    scene_.root->setProperty("groupOpacity", groupOpacity);
    return scene_.window->grabWindow();
}

void SeriesOpacityTest::seriesOpacityBlendsWithBackground_data()
{
    QTest::addColumn<QPoint>("pixel");

    QTest::newRow("PointCloud") << QPoint{6, 20};
    QTest::newRow("LineCurve line") << QPoint{60, 20};
    QTest::newRow("LineCurve markers") << QPoint{100, 20};
    QTest::newRow("GradientStroke") << QPoint{140, 20};
    QTest::newRow("GradientFill") << QPoint{180, 28};
    QTest::newRow("RectangleList") << QPoint{220, 20};
}

void SeriesOpacityTest::seriesOpacityBlendsWithBackground()
{
    QFETCH(QPoint, pixel);

    constexpr auto kOpaque = 255;
    constexpr auto kHalf = 128;
    constexpr auto kTolerance = 8;

    const auto opaque = render(1.0, 1.0);
    QVERIFY2(std::abs(grayAt(opaque, pixel) - kOpaque) <= kTolerance, qPrintable(QString::number(grayAt(opaque, pixel))));

    const auto own = render(0.5, 1.0);
    QVERIFY2(std::abs(grayAt(own, pixel) - kHalf) <= kTolerance, qPrintable(QString::number(grayAt(own, pixel))));

    const auto inherited = render(1.0, 0.5);
    QVERIFY2(std::abs(grayAt(inherited, pixel) - kHalf) <= kTolerance, qPrintable(QString::number(grayAt(inherited, pixel))));

    // Inherited opacity multiplies with the item's own opacity.
    const auto combined = render(0.5, 0.5);
    QVERIFY2(std::abs(grayAt(combined, pixel) - 64) <= kTolerance, qPrintable(QString::number(grayAt(combined, pixel))));
}

void SeriesOpacityTest::overlappingMarkersBlendPerFragment()
{
    constexpr auto kTolerance = 8;
    const auto image = render(0.5, 1.0);

    // Each marker blends on its own, so the overlap is 0.5 + 0.5 * 0.5 = 0.75 bright.
    QVERIFY2(std::abs(grayAt(image, {6, 20}) - 128) <= kTolerance, qPrintable(QString::number(grayAt(image, {6, 20}))));
    QVERIFY2(std::abs(grayAt(image, {20, 20}) - 191) <= kTolerance, qPrintable(QString::number(grayAt(image, {20, 20}))));
    QVERIFY2(std::abs(grayAt(image, {34, 20}) - 128) <= kTolerance, qPrintable(QString::number(grayAt(image, {34, 20}))));
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Pixel lookups use logical coordinates, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::SeriesOpacityTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_series_opacity.moc"
