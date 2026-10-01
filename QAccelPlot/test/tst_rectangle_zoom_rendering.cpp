//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"

#include <QImage>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <cmath>
#include <memory>

namespace {

QColor pixel(const QImage& image, const QQuickWindow& window, const QPointF& pos)
{
    const auto scale = static_cast<qreal>(image.width()) / window.width();
    return image.pixelColor(static_cast<int>(std::floor(pos.x() * scale)), static_cast<int>(std::floor(pos.y() * scale)));
}

bool nearColor(const QColor& actual, const QColor& expected)
{
    constexpr auto kTolerance = 12;
    return std::abs(actual.red() - expected.red()) <= kTolerance && std::abs(actual.green() - expected.green()) <= kTolerance
        && std::abs(actual.blue() - expected.blue()) <= kTolerance;
}

void sendMouse(QQuickItem* plot, const QEvent::Type type, const QPointF& pos)
{
    const auto button = type == QEvent::MouseMove ? Qt::NoButton : Qt::LeftButton;
    const auto buttons = type == QEvent::MouseButtonRelease ? Qt::NoButton : Qt::LeftButton;
    auto event = QMouseEvent{type, pos, pos, pos, button, buttons, Qt::ShiftModifier};
    QCoreApplication::sendEvent(plot, &event);
}

const auto kScene = R"(
import QtQuick
import QAccelPlot as QAccelPlot

QAccelPlot.%1 {
    id: plot
    width: 400
    height: 300
    axesAreaColor: "#202020"
    plotAreaColor: "black"
    grid.gridVisible: false
    grid.subGridVisible: false
    rectangleZoom.enabled: true
    rectangleZoom.fillColor: "#8000ff00"
    rectangleZoom.borderColor: "red"

    xAxis: QAccelPlot.Axis { viewportMin: 0; viewportMax: 100 }
    yAxis: QAccelPlot.Axis { viewportMin: -10; viewportMax: 10 }

    QAccelPlot.RectangleSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "blue"
        Component.onCompleted: setData([{ x1: 0, y1: -10, x2: 100, y2: 10 }])
    }

    %2
}
)";

} // namespace

class TestRectangleZoomRendering : public QObject {
    Q_OBJECT

private slots:
    void overlayAboveSeriesBelowLegend_data();
    void overlayAboveSeriesBelowLegend();
};

void TestRectangleZoomRendering::overlayAboveSeriesBelowLegend_data()
{
    QTest::addColumn<QString>("type");
    QTest::addColumn<QString>("legend");
    QTest::newRow("PlotView") << QStringLiteral("PlotView") << QStringLiteral(R"(
        Rectangle {
            z: 1
            x: plot.plotRect.x + plot.plotRect.width - 68
            y: plot.plotRect.y + 8
            width: 60
            height: 30
            color: "yellow"
        })");
    QTest::newRow("Plot") << QStringLiteral("Plot") << QStringLiteral(R"(
        legend: Rectangle { width: 60; height: 30; color: "yellow" }
    )");
}

void TestRectangleZoomRendering::overlayAboveSeriesBelowLegend()
{
    QFETCH(QString, type);
    QFETCH(QString, legend);
    auto window = QQuickWindow{};
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("RectangleSeries materials require a hardware scene graph backend");
    }
    window.resize(400, 300);
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(QString::fromLatin1(kScene).arg(type, legend).toUtf8(), QUrl{});
    const auto root = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(root, qPrintable(component.errorString()));
    auto* plot = qobject_cast<QAccelPlot::QAccelPlot*>(root.get());
    QVERIFY(plot);
    plot->setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const auto rect = plot->plotRect();
    const auto center = rect.center();
    const auto legendPoint = QPointF{rect.right() - 38, rect.top() + 23};
    const auto start = rect.topLeft() + QPointF{10, 3};
    const auto end = rect.bottomRight() + QPointF{20, 20};
    auto image = window.grabWindow();
    QVERIFY(nearColor(pixel(image, window, center), Qt::blue));
    QVERIFY(nearColor(pixel(image, window, legendPoint), Qt::yellow));

    sendMouse(plot, QEvent::MouseButtonPress, start);
    sendMouse(plot, QEvent::MouseMove, end);
    QVERIFY(plot->rectangleZoom()->active());
    image = window.grabWindow();
    QVERIFY(nearColor(pixel(image, window, center), QColor(0, 128, 127)));
    QVERIFY(nearColor(pixel(image, window, legendPoint), Qt::yellow));
    QVERIFY(nearColor(pixel(image, window, {start.x() + 0.4, center.y()}), Qt::red));
    QVERIFY(nearColor(pixel(image, window, {rect.right() - 0.4, center.y()}), Qt::red));
    QVERIFY(nearColor(pixel(image, window, {rect.left() + 3, center.y()}), Qt::blue));
    QVERIFY(nearColor(pixel(image, window, {rect.right() + 5, center.y()}), QColor(32, 32, 32)));

    plot->rectangleZoom()->setFillColor(QColor(255, 0, 0, 128));
    plot->rectangleZoom()->setBorderColor(Qt::yellow);
    image = window.grabWindow();
    QVERIFY(nearColor(pixel(image, window, center), QColor(128, 0, 127)));
    QVERIFY(nearColor(pixel(image, window, {start.x() + 0.4, center.y()}), Qt::yellow));

    auto escape = QKeyEvent{QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier};
    QCoreApplication::sendEvent(plot, &escape);
    QVERIFY(!plot->rectangleZoom()->active());
    image = window.grabWindow();
    QVERIFY(nearColor(pixel(image, window, center), Qt::blue));

    sendMouse(plot, QEvent::MouseButtonPress, start);
    sendMouse(plot, QEvent::MouseMove, end);
    sendMouse(plot, QEvent::MouseButtonRelease, end);
    QVERIFY(!plot->rectangleZoom()->active());
    image = window.grabWindow();
    QVERIFY(nearColor(pixel(image, window, plot->plotRect().center()), Qt::blue));

    const auto newRect = plot->plotRect();
    sendMouse(plot, QEvent::MouseButtonPress, newRect.topLeft() + QPointF{10, 10});
    sendMouse(plot, QEvent::MouseMove, newRect.center());
    QVERIFY(plot->rectangleZoom()->active());
    plot->setWidth(420);
    QVERIFY(!plot->rectangleZoom()->active());
    image = window.grabWindow();
    QVERIFY(nearColor(pixel(image, window, plot->plotRect().center()), Qt::blue));
}

QTEST_MAIN(TestRectangleZoomRendering)
#include "tst_rectangle_zoom_rendering.moc"
