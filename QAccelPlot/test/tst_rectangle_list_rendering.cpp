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

// A red full-height span at x 1..2, a red rectangle with a NaN edge at x 6..8, and a blue
// full-width band at y 4..5 drawn on top.
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

    xAxis: Axis { viewportMin: 0; viewportMax: 10 }
    yAxis: Axis { viewportMin: 0; viewportMax: 10 }

    RectangleList {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        Component.onCompleted: setData([{ x1: 1, x2: 2 }, { x1: 6, y1: 6, x2: 8, y2: NaN }])
    }

    RectangleList {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "blue"
        Component.onCompleted: setData([{ y1: 4, y2: 5 }])
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

class RectangleListRenderingTest : public QObject {
    Q_OBJECT

private slots:
    void unboundedEdgesReachThePlotEdges();
};

void RectangleListRenderingTest::unboundedEdgesReachThePlotEdges()
{
    auto window = QQuickWindow{};
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    window.setColor(Qt::black);
    window.resize(400, 300);

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

    const auto area = plot->plotRect().toAlignedRect();
    const auto top = area.top() + 2;
    const auto bottom = area.bottom() - 2;
    const auto left = area.left() + 2;
    const auto right = area.right() - 2;
    const auto pixelX = [plot](const qreal x) { return qRound(plot->dataToPixelX(x)); };
    const auto pixelY = [plot](const qreal y) { return qRound(plot->dataToPixelY(y)); };

    auto image = window.grabWindow();
    QVERIFY(isColor(image.pixelColor(pixelX(1.5), top), Qt::red));
    QVERIFY(isColor(image.pixelColor(pixelX(1.5), bottom), Qt::red));
    QVERIFY(isColor(image.pixelColor(left, pixelY(4.5)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(right, pixelY(4.5)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(pixelX(7.0), pixelY(7.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(pixelX(5.0), pixelY(8.0)), Qt::black));

    // Far from the data, the span still fills the plot height and the band leaves the view.
    plot->yAxis()->setViewportMin(1.0e6);
    plot->yAxis()->setViewportMax(1.0e6 + 10.0);
    image = window.grabWindow();
    QVERIFY(isColor(image.pixelColor(pixelX(1.5), top), Qt::red));
    QVERIFY(isColor(image.pixelColor(pixelX(1.5), bottom), Qt::red));
    QVERIFY(isColor(image.pixelColor(pixelX(5.0), (top + bottom) / 2), Qt::black));

    plot->yAxis()->setViewportMin(1.0);
    plot->yAxis()->setViewportMax(1000.0);
    plot->yAxis()->setLogScale(true);
    image = window.grabWindow();
    QVERIFY(isColor(image.pixelColor(pixelX(1.5), top), Qt::red));
    QVERIFY(isColor(image.pixelColor(pixelX(1.5), bottom), Qt::red));
    QVERIFY(isColor(image.pixelColor(left, pixelY(4.5)), Qt::blue));

    plot->xAxis()->setViewportMin(1.0e9);
    plot->xAxis()->setViewportMax(1.0e9 + 10.0);
    image = window.grabWindow();
    QVERIFY(isColor(image.pixelColor(left, pixelY(4.5)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(right, pixelY(4.5)), Qt::blue));
    QVERIFY(isColor(image.pixelColor((left + right) / 2, top), Qt::black));
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Pixel lookups use logical coordinates, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::RectangleListRenderingTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_rectangle_list_rendering.moc"
