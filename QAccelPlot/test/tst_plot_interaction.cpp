//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/PlotMouseEvent.hpp"
#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/axis/Axis.hpp"

#include <QHoverEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QWheelEvent>
#include <QtTest/QtTest>

#include <cmath>

namespace {

class TestablePlot final : public QAccelPlot::QAccelPlot {
public:
    using QAccelPlot::QAccelPlot::hoverEnterEvent;
    using QAccelPlot::QAccelPlot::hoverLeaveEvent;
    using QAccelPlot::QAccelPlot::hoverMoveEvent;
    using QAccelPlot::QAccelPlot::keyPressEvent;
    using QAccelPlot::QAccelPlot::mouseDoubleClickEvent;
    using QAccelPlot::QAccelPlot::mouseMoveEvent;
    using QAccelPlot::QAccelPlot::mousePressEvent;
    using QAccelPlot::QAccelPlot::mouseReleaseEvent;
    using QAccelPlot::QAccelPlot::wheelEvent;
};

class TestableAxis final : public QAccelPlot::Axis {
public:
    using Axis::Axis;
    using Axis::keyPressEvent;
    using Axis::mouseDoubleClickEvent;
    using Axis::mouseMoveEvent;
    using Axis::mousePressEvent;
    using Axis::mouseReleaseEvent;
};

constexpr auto kWheelStep = 120;
constexpr auto kTolerance = 1e-9;

QMouseEvent mouseEvent(const QEvent::Type type, const QPointF& pos, const Qt::MouseButton button, const Qt::MouseButtons buttons)
{
    return QMouseEvent{type, pos, pos, pos, button, buttons, Qt::NoModifier};
}

void wheel(TestablePlot& plot, const QPointF& pos, const int deltaY)
{
    auto event = QWheelEvent{pos, pos, QPoint{}, QPoint{0, deltaY}, Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false};
    plot.wheelEvent(&event);
}

void drag(TestablePlot& plot, const QPointF& from, const QPointF& to)
{
    auto press = mouseEvent(QEvent::MouseButtonPress, from, Qt::LeftButton, Qt::LeftButton);
    plot.mousePressEvent(&press);
    auto move = mouseEvent(QEvent::MouseMove, to, Qt::NoButton, Qt::LeftButton);
    plot.mouseMoveEvent(&move);
    auto release = mouseEvent(QEvent::MouseButtonRelease, to, Qt::LeftButton, Qt::NoButton);
    plot.mouseReleaseEvent(&release);
}

void drag(TestableAxis& axis, const QPointF& from, const QPointF& to)
{
    auto press = mouseEvent(QEvent::MouseButtonPress, from, Qt::LeftButton, Qt::LeftButton);
    axis.mousePressEvent(&press);
    auto move = mouseEvent(QEvent::MouseMove, to, Qt::NoButton, Qt::LeftButton);
    axis.mouseMoveEvent(&move);
    auto release = mouseEvent(QEvent::MouseButtonRelease, to, Qt::LeftButton, Qt::NoButton);
    axis.mouseReleaseEvent(&release);
}

void setViewport(QAccelPlot::Axis* axis, const qreal min, const qreal max)
{
    axis->setViewportMin(min);
    axis->setViewportMax(max);
}

bool fuzzyEqual(const qreal actual, const qreal expected)
{
    return std::abs(actual - expected) <= kTolerance * std::max(1.0, std::abs(expected));
}

} // namespace

#define QCOMPARE_NEAR(actual, expected)                                                                                                                        \
    QVERIFY2(fuzzyEqual((actual), (expected)), qPrintable(QStringLiteral("actual %1, expected %2").arg(actual, 0, 'g', 17).arg(expected, 0, 'g', 17)))

class TestPlotInteraction : public QObject {
    Q_OBJECT

private slots:
    void wheelInPlotArea_zoomsAroundCursor();
    void wheelOut_undoesWheelIn();
    void wheelOverAxis_zoomsOnlyThatAxis();
    void wheelOutsidePlotAndAxes_isIgnored();
    void wheelOnLogAxis_zoomsInLogSpace();
    void zoomScaleFactor_extremesStillZoom_data();
    void zoomScaleFactor_extremesStillZoom();
    void dragInPlotArea_pansAllAxes();
    void dragOnLogAxis_pansInLogSpace();
    void acceptedPressDoesNotStartDrag();
    void doubleClick_rescalesAxesToData();
    void acceptedDoubleClickDoesNotRescale();

    void axisZoomScaleFactor_clamps();
    void axisDrag_pansViewport();
    void axisDrag_verticalIsInverted();
    void axisDrag_zeroSizeAxisIsIgnored();
    void axisRightButton_doesNotStartDrag();
    void axisKeyL_togglesLogScale();
    void axisDoubleClick_emitsSignal();
    void keyPress_isForwardedToAxisUnderPointer();
    void keyPress_followsWindowHoverOverAxes();

    void plotMouseEvent_resetClearsAcceptance();
};

void TestPlotInteraction::wheelInPlotArea_zoomsAroundCursor()
{
    auto plot = TestablePlot{};
    plot.setSize({300.0, 200.0});
    auto* xAxis = new QAccelPlot::Axis{&plot};
    auto* yAxis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    plot.setYAxis(yAxis);
    setViewport(xAxis, 0.0, 100.0);
    setViewport(yAxis, 0.0, 10.0);

    const auto rect = plot.plotRect();
    const auto cursor = QPointF{rect.x() + rect.width() * 0.25, rect.y() + rect.height() * 0.75};
    const auto xUnderCursor = plot.pixelToDataX(cursor.x());
    const auto yUnderCursor = plot.pixelToDataY(cursor.y());

    wheel(plot, cursor, kWheelStep);

    QCOMPARE_NEAR(xAxis->viewportMax() - xAxis->viewportMin(), 90.0);
    QCOMPARE_NEAR(yAxis->viewportMax() - yAxis->viewportMin(), 9.0);
    QCOMPARE_NEAR(plot.pixelToDataX(cursor.x()), xUnderCursor);
    QCOMPARE_NEAR(plot.pixelToDataY(cursor.y()), yUnderCursor);
}

void TestPlotInteraction::wheelOut_undoesWheelIn()
{
    auto plot = TestablePlot{};
    plot.setSize({300.0, 200.0});
    auto* xAxis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    setViewport(xAxis, -20.0, 80.0);

    const auto rect = plot.plotRect();
    const auto cursor = QPointF{rect.x() + rect.width() * 0.6, rect.center().y()};
    wheel(plot, cursor, kWheelStep);
    wheel(plot, cursor, -kWheelStep);

    QCOMPARE_NEAR(xAxis->viewportMin(), -20.0);
    QCOMPARE_NEAR(xAxis->viewportMax(), 80.0);
}

void TestPlotInteraction::wheelOverAxis_zoomsOnlyThatAxis()
{
    auto plot = TestablePlot{};
    plot.setSize({300.0, 200.0});
    auto* xAxis = new QAccelPlot::Axis{&plot};
    auto* yAxis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    plot.setYAxis(yAxis);
    setViewport(xAxis, 0.0, 100.0);
    setViewport(yAxis, 0.0, 10.0);

    const auto overYAxis = QPointF{yAxis->x() + yAxis->width() / 4.0, yAxis->y() + yAxis->height() / 2.0};
    QVERIFY(!plot.plotRect().contains(overYAxis));
    wheel(plot, overYAxis, kWheelStep);

    QCOMPARE(xAxis->viewportMin(), 0.0);
    QCOMPARE(xAxis->viewportMax(), 100.0);
    QCOMPARE_NEAR(yAxis->viewportMax() - yAxis->viewportMin(), 9.0);
}

void TestPlotInteraction::wheelOutsidePlotAndAxes_isIgnored()
{
    auto plot = TestablePlot{};
    plot.setSize({300.0, 200.0});
    plot.setPadding(20.0);
    auto* xAxis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    setViewport(xAxis, 0.0, 100.0);

    auto event = QWheelEvent{QPointF{2.0, 2.0}, QPointF{2.0, 2.0}, QPoint{}, QPoint{0, kWheelStep}, Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false};
    plot.wheelEvent(&event);

    QVERIFY(!event.isAccepted());
    QCOMPARE(xAxis->viewportMin(), 0.0);
    QCOMPARE(xAxis->viewportMax(), 100.0);
}

void TestPlotInteraction::wheelOnLogAxis_zoomsInLogSpace()
{
    auto plot = TestablePlot{};
    plot.setSize({300.0, 200.0});
    auto* xAxis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    setViewport(xAxis, 1.0, 1e4);
    xAxis->setLogScale(true);

    const auto rect = plot.plotRect();
    const auto cursor = QPointF{rect.x() + rect.width() * 0.5, rect.center().y()};
    wheel(plot, cursor, kWheelStep);

    const auto logMin = std::log10(xAxis->viewportMin());
    const auto logMax = std::log10(xAxis->viewportMax());
    QCOMPARE_NEAR(logMax - logMin, 4.0 * 0.9);
    QCOMPARE_NEAR((logMin + logMax) / 2.0, 2.0);
}

void TestPlotInteraction::zoomScaleFactor_extremesStillZoom_data()
{
    QTest::addColumn<double>("factor");
    QTest::addColumn<double>("expectedRange");

    // Factors of 0 and 1 are clamped to 0.01 and 0.99, so zooming neither collapses nor stalls.
    QTest::newRow("zero") << 0.0 << 1.0;
    QTest::newRow("one") << 1.0 << 99.0;
}

void TestPlotInteraction::zoomScaleFactor_extremesStillZoom()
{
    QFETCH(double, factor);
    QFETCH(double, expectedRange);

    auto plot = TestablePlot{};
    plot.setSize({300.0, 200.0});
    auto* xAxis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    setViewport(xAxis, 0.0, 100.0);
    xAxis->setZoomScaleFactor(factor);

    wheel(plot, plot.plotRect().center(), kWheelStep);

    QCOMPARE_NEAR(xAxis->viewportMax() - xAxis->viewportMin(), expectedRange);
}

void TestPlotInteraction::dragInPlotArea_pansAllAxes()
{
    auto plot = TestablePlot{};
    plot.setSize({300.0, 200.0});
    auto* xAxis = new QAccelPlot::Axis{&plot};
    auto* yAxis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    plot.setYAxis(yAxis);
    setViewport(xAxis, 0.0, 100.0);
    setViewport(yAxis, 0.0, 10.0);

    const auto rect = plot.plotRect();
    const auto start = rect.center();
    // Dragging by a tenth of the plot moves the content with the cursor: right shows lower x, down shows higher y.
    drag(plot, start, start + QPointF{rect.width() / 10.0, rect.height() / 10.0});

    QCOMPARE_NEAR(xAxis->viewportMin(), -10.0);
    QCOMPARE_NEAR(xAxis->viewportMax(), 90.0);
    QCOMPARE_NEAR(yAxis->viewportMin(), 1.0);
    QCOMPARE_NEAR(yAxis->viewportMax(), 11.0);
}

void TestPlotInteraction::dragOnLogAxis_pansInLogSpace()
{
    auto plot = TestablePlot{};
    plot.setSize({300.0, 200.0});
    auto* xAxis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    setViewport(xAxis, 1.0, 1e4);
    xAxis->setLogScale(true);

    const auto rect = plot.plotRect();
    drag(plot, rect.center(), rect.center() - QPointF{rect.width() / 4.0, 0.0});

    QCOMPARE_NEAR(xAxis->viewportMin(), 10.0);
    QCOMPARE_NEAR(xAxis->viewportMax(), 1e5);
}

void TestPlotInteraction::acceptedPressDoesNotStartDrag()
{
    auto plot = TestablePlot{};
    plot.setSize({300.0, 200.0});
    auto* xAxis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    setViewport(xAxis, 0.0, 100.0);
    connect(&plot, &QAccelPlot::QAccelPlot::mousePressed, [](QAccelPlot::PlotMouseEvent* event) { event->accept(); });

    const auto start = plot.plotRect().center();
    drag(plot, start, start + QPointF{50.0, 0.0});

    QCOMPARE(xAxis->viewportMin(), 0.0);
    QCOMPARE(xAxis->viewportMax(), 100.0);
}

void TestPlotInteraction::doubleClick_rescalesAxesToData()
{
    auto plot = TestablePlot{};
    plot.setSize({300.0, 200.0});
    auto* xAxis = new QAccelPlot::Axis{&plot};
    auto* extraAxis = new QAccelPlot::Axis{&plot, QAccelPlot::Axis::Right};
    plot.setXAxis(xAxis);
    auto extraAxes = plot.extraAxes();
    extraAxes.append(&extraAxes, extraAxis);
    setViewport(xAxis, 0.0, 100.0);
    setViewport(extraAxis, 0.0, 100.0);
    xAxis->updateDataRange(5.0, 25.0);
    extraAxis->updateDataRange(-3.0, 3.0);

    const auto pos = plot.plotRect().center();
    auto doubleClick = mouseEvent(QEvent::MouseButtonDblClick, pos, Qt::LeftButton, Qt::LeftButton);
    plot.mouseDoubleClickEvent(&doubleClick);

    QCOMPARE(xAxis->viewportMin(), 5.0);
    QCOMPARE(xAxis->viewportMax(), 25.0);
    QCOMPARE(extraAxis->viewportMin(), -3.0);
    QCOMPARE(extraAxis->viewportMax(), 3.0);
}

void TestPlotInteraction::acceptedDoubleClickDoesNotRescale()
{
    auto plot = TestablePlot{};
    plot.setSize({300.0, 200.0});
    auto* xAxis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    setViewport(xAxis, 0.0, 100.0);
    xAxis->updateDataRange(5.0, 25.0);
    connect(&plot, &QAccelPlot::QAccelPlot::mouseDoubleClicked, [](QAccelPlot::PlotMouseEvent* event) { event->accept(); });

    const auto pos = plot.plotRect().center();
    auto doubleClick = mouseEvent(QEvent::MouseButtonDblClick, pos, Qt::LeftButton, Qt::LeftButton);
    plot.mouseDoubleClickEvent(&doubleClick);

    QCOMPARE(xAxis->viewportMin(), 0.0);
    QCOMPARE(xAxis->viewportMax(), 100.0);
}

void TestPlotInteraction::axisZoomScaleFactor_clamps()
{
    auto axis = QAccelPlot::Axis{};
    auto spy = QSignalSpy{&axis, &QAccelPlot::Axis::zoomScaleFactorChanged};

    QCOMPARE(axis.zoomScaleFactor(), 0.9);
    axis.setZoomScaleFactor(1.5);
    QCOMPARE(axis.zoomScaleFactor(), 1.0);
    axis.setZoomScaleFactor(-0.5);
    QCOMPARE(axis.zoomScaleFactor(), 0.0);
    axis.setZoomScaleFactor(-2.0);
    QCOMPARE(spy.count(), 2);
}

void TestPlotInteraction::axisDrag_pansViewport()
{
    auto axis = TestableAxis{};
    axis.setSize({250.0, 50.0});
    setViewport(&axis, 0.0, 100.0);

    // The horizontal data length excludes the label overflow margin on both ends.
    const auto dataLength = axis.width() - 2.0 * axis.labelOverflow();
    drag(axis, {100.0, 10.0}, {100.0 + dataLength / 5.0, 30.0});

    QCOMPARE_NEAR(axis.viewportMin(), -20.0);
    QCOMPARE_NEAR(axis.viewportMax(), 80.0);
}

void TestPlotInteraction::axisDrag_verticalIsInverted()
{
    auto axis = TestableAxis{nullptr, QAccelPlot::Axis::Left};
    axis.setSize({50.0, 220.0});
    setViewport(&axis, 0.0, 10.0);

    const auto dataLength = axis.height() - 2.0 * axis.labelOverflow();
    drag(axis, {25.0, 100.0}, {5.0, 100.0 + dataLength / 2.0});

    QCOMPARE_NEAR(axis.viewportMin(), 5.0);
    QCOMPARE_NEAR(axis.viewportMax(), 15.0);
}

void TestPlotInteraction::axisDrag_zeroSizeAxisIsIgnored()
{
    auto axis = TestableAxis{};
    // A horizontal axis whose width equals its two label overflow margins has no data length.
    axis.setSize({2.0 * axis.labelOverflow(), 50.0});
    setViewport(&axis, 0.0, 100.0);

    drag(axis, {10.0, 10.0}, {40.0, 10.0});

    QCOMPARE(axis.viewportMin(), 0.0);
    QCOMPARE(axis.viewportMax(), 100.0);
}

void TestPlotInteraction::axisRightButton_doesNotStartDrag()
{
    auto axis = TestableAxis{};
    axis.setSize({250.0, 50.0});
    setViewport(&axis, 0.0, 100.0);

    auto press = mouseEvent(QEvent::MouseButtonPress, {100.0, 10.0}, Qt::RightButton, Qt::RightButton);
    axis.mousePressEvent(&press);
    auto move = mouseEvent(QEvent::MouseMove, {150.0, 10.0}, Qt::NoButton, Qt::RightButton);
    axis.mouseMoveEvent(&move);

    QCOMPARE(axis.viewportMin(), 0.0);
    QCOMPARE(axis.viewportMax(), 100.0);
}

void TestPlotInteraction::axisKeyL_togglesLogScale()
{
    auto axis = TestableAxis{};
    auto spy = QSignalSpy{&axis, &QAccelPlot::Axis::logScaleChanged};

    auto keyL = QKeyEvent{QEvent::KeyPress, Qt::Key_L, Qt::NoModifier, QStringLiteral("l")};
    keyL.ignore();
    axis.keyPressEvent(&keyL);
    QVERIFY(keyL.isAccepted());
    QVERIFY(axis.logScale());

    axis.keyPressEvent(&keyL);
    QVERIFY(!axis.logScale());

    auto keyK = QKeyEvent{QEvent::KeyPress, Qt::Key_K, Qt::NoModifier, QStringLiteral("k")};
    axis.keyPressEvent(&keyK);
    QVERIFY(!axis.logScale());
    QCOMPARE(spy.count(), 2);
}

void TestPlotInteraction::axisDoubleClick_emitsSignal()
{
    auto axis = TestableAxis{};
    auto spy = QSignalSpy{&axis, &QAccelPlot::Axis::doubleClicked};

    auto left = mouseEvent(QEvent::MouseButtonDblClick, {10.0, 10.0}, Qt::LeftButton, Qt::LeftButton);
    axis.mouseDoubleClickEvent(&left);
    auto right = mouseEvent(QEvent::MouseButtonDblClick, {10.0, 10.0}, Qt::RightButton, Qt::RightButton);
    axis.mouseDoubleClickEvent(&right);

    QCOMPARE(spy.count(), 1);
}

void TestPlotInteraction::keyPress_isForwardedToAxisUnderPointer()
{
    auto plot = TestablePlot{};
    plot.setSize({400.0, 300.0});
    auto* xAxis = new QAccelPlot::Axis{&plot};
    auto* yAxis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    plot.setYAxis(yAxis);
    const auto yAxisCenter = QRectF{yAxis->position(), yAxis->size()}.center();
    const auto pressL = [&plot]() {
        auto event = QKeyEvent{QEvent::KeyPress, Qt::Key_L, Qt::NoModifier, QStringLiteral("l")};
        plot.keyPressEvent(&event);
        return event.isAccepted();
    };
    const auto hover = [](const QEvent::Type type, const QPointF& pos) { return QHoverEvent{type, pos, pos, pos}; };

    // Without a known pointer position no axis is targeted.
    QVERIFY(!pressL());

    auto enter = hover(QEvent::HoverEnter, plot.plotRect().center());
    plot.hoverEnterEvent(&enter);
    QVERIFY(!pressL());

    auto move = hover(QEvent::HoverMove, yAxisCenter);
    plot.hoverMoveEvent(&move);
    QVERIFY(pressL());
    QVERIFY(yAxis->logScale());
    QVERIFY(!xAxis->logScale());

    auto leave = hover(QEvent::HoverLeave, yAxisCenter);
    plot.hoverLeaveEvent(&leave);
    QVERIFY(!pressL());
    QVERIFY(yAxis->logScale());

    auto press = mouseEvent(QEvent::MouseButtonPress, yAxisCenter, Qt::RightButton, Qt::RightButton);
    plot.mousePressEvent(&press);
    QVERIFY(pressL());
    QVERIFY(!yAxis->logScale());
}

void TestPlotInteraction::keyPress_followsWindowHoverOverAxes()
{
    // The window stays hidden: synthetic events are still delivered, and the real mouse cannot interfere.
    auto window = QQuickWindow{};
    window.resize(400, 300);
    auto plot = TestablePlot{};
    plot.setParentItem(window.contentItem());
    plot.setSize({400.0, 300.0});
    auto* xAxis = new QAccelPlot::Axis{&plot};
    auto* yAxis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    plot.setYAxis(yAxis);
    QVERIFY(yAxis->acceptHoverEvents());

    // The axis accepts hover itself, yet the plot must still track the pointer over it.
    QTest::mouseMove(&window, plot.plotRect().center().toPoint());
    QTest::mouseMove(&window, QRectF{yAxis->position(), yAxis->size()}.center().toPoint());
    QVERIFY(yAxis->hovered());
    auto event = QKeyEvent{QEvent::KeyPress, Qt::Key_L, Qt::NoModifier, QStringLiteral("l")};
    plot.keyPressEvent(&event);

    QVERIFY(event.isAccepted());
    QVERIFY(yAxis->logScale());
    QVERIFY(!xAxis->logScale());
}

void TestPlotInteraction::plotMouseEvent_resetClearsAcceptance()
{
    auto event = QAccelPlot::PlotMouseEvent{};
    event.accept();
    QVERIFY(event.isAccepted());

    event.reset(Qt::RightButton, 12.5, -3.0, Qt::ShiftModifier);

    QVERIFY(!event.isAccepted());
    QCOMPARE(event.button(), static_cast<int>(Qt::RightButton));
    QCOMPARE(event.x(), 12.5);
    QCOMPARE(event.y(), -3.0);
    QCOMPARE(event.modifiers(), static_cast<int>(Qt::ShiftModifier));
}

QTEST_MAIN(TestPlotInteraction)
#include "tst_plot_interaction.moc"
