//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QMetaProperty>
#include <QMouseEvent>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QWheelEvent>
#include <QtTest/QtTest>

#include <cmath>
#include <limits>

namespace {

class TestablePlot final : public QAccelPlot::QAccelPlot {
public:
    using QAccelPlot::QAccelPlot::focusOutEvent;
    using QAccelPlot::QAccelPlot::keyPressEvent;
    using QAccelPlot::QAccelPlot::mouseDoubleClickEvent;
    using QAccelPlot::QAccelPlot::mouseMoveEvent;
    using QAccelPlot::QAccelPlot::mousePressEvent;
    using QAccelPlot::QAccelPlot::mouseReleaseEvent;
    using QAccelPlot::QAccelPlot::mouseUngrabEvent;
    using QAccelPlot::QAccelPlot::wheelEvent;
};

void setRange(QAccelPlot::Axis* axis, const qreal min, const qreal max)
{
    axis->setViewportMin(min);
    axis->setViewportMax(max);
}

struct Fixture {
    TestablePlot plot;
    QAccelPlot::Axis* x{new QAccelPlot::Axis{&plot}};
    QAccelPlot::Axis* y{new QAccelPlot::Axis{&plot}};

    Fixture()
    {
        plot.setSize({600, 400});
        plot.setXAxis(x);
        plot.setYAxis(y);
        setRange(x, 0, 100);
        setRange(y, -10, 10);
        plot.rectangleZoom()->setEnabled(true);
    }

    QPointF point(const qreal xRatio, const qreal yRatio) const
    {
        const auto rect = plot.plotRect();
        return {rect.left() + rect.width() * xRatio, rect.top() + rect.height() * yRatio};
    }

    QRectF selection() const
    {
        return {point(0.25, 0.25), point(0.75, 0.75)};
    }
};

void mouse(TestablePlot& plot, const QEvent::Type type, const QPointF& pos, const Qt::MouseButton button, const Qt::MouseButtons buttons,
    const Qt::KeyboardModifiers modifiers = Qt::ShiftModifier)
{
    auto event = QMouseEvent{type, pos, pos, pos, button, buttons, modifiers};
    if (type == QEvent::MouseButtonPress) {
        plot.mousePressEvent(&event);
    } else if (type == QEvent::MouseMove) {
        plot.mouseMoveEvent(&event);
    } else {
        plot.mouseReleaseEvent(&event);
    }
}

void press(Fixture& fixture, const QPointF& pos, const Qt::KeyboardModifiers modifiers = Qt::ShiftModifier)
{
    mouse(fixture.plot, QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, modifiers);
}

void move(Fixture& fixture, const QPointF& pos, const Qt::KeyboardModifiers modifiers = Qt::ShiftModifier)
{
    mouse(fixture.plot, QEvent::MouseMove, pos, Qt::NoButton, Qt::LeftButton, modifiers);
}

void release(Fixture& fixture, const QPointF& pos)
{
    mouse(fixture.plot, QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::NoButton);
}

void compareRange(const QAccelPlot::Axis* axis, const qreal min, const qreal max)
{
    QVERIFY(std::abs(axis->viewportMin() - min) <= 1e-9 * std::max(1.0, std::abs(min)));
    QVERIFY(std::abs(axis->viewportMax() - max) <= 1e-9 * std::max(1.0, std::abs(max)));
}

} // namespace

class TestRectangleZoom : public QObject {
    Q_OBJECT

private slots:
    void configuration();
    void directZoom_data();
    void directZoom();
    void allAxesUseOwnRanges();
    void clippingAndNormalization();
    void invalidSelection_data();
    void invalidSelection();
    void invalidAxisRejectsWholeZoom();
    void noAxesIsIgnored();
    void rangesComputedBeforeLayoutChanges();
    void dragDirections_data();
    void dragDirections();
    void disabledFeaturePreservesPan();
    void configuredModifiersAndFrozenGesture();
    void pressOutsidePlotDoesNotSelect();
    void acceptedEvents();
    void releaseUsesFinalPosition();
    void unrelatedReleaseKeepsSelection();
    void cancellation_data();
    void cancellation();
    void wheelDuringSelectionIsConsumed();
    void doubleClickCancelsAndRescales();
    void windowDragOutsidePlot();
};

void TestRectangleZoom::configuration()
{
    auto config = QAccelPlot::PlotRectangleZoom{};
    QVERIFY(!config.enabled());
    QCOMPARE(config.modifiers(), static_cast<int>(Qt::ShiftModifier));
    QCOMPARE(config.minimumSize(), 6.0);
    QCOMPARE(config.fillColor(), QAccelPlot::ColorPalette::dark().rectangleZoomFill);
    QCOMPARE(config.borderColor(), QAccelPlot::ColorPalette::dark().rectangleZoomBorder);
    QVERIFY(!config.active());
    QVERIFY(config.selectionRect().isEmpty());
    const auto* metaObject = config.metaObject();
    QVERIFY(!metaObject->property(metaObject->indexOfProperty("active")).isWritable());
    QVERIFY(!metaObject->property(metaObject->indexOfProperty("selectionRect")).isWritable());
    QCOMPARE(metaObject->indexOfMethod("setSelection(bool,QRectF)"), -1);
    auto enabledSpy = QSignalSpy{&config, &QAccelPlot::PlotRectangleZoom::enabledChanged};
    auto sizeSpy = QSignalSpy{&config, &QAccelPlot::PlotRectangleZoom::minimumSizeChanged};
    auto modifiersSpy = QSignalSpy{&config, &QAccelPlot::PlotRectangleZoom::modifiersChanged};
    auto fillSpy = QSignalSpy{&config, &QAccelPlot::PlotRectangleZoom::fillColorChanged};
    auto borderSpy = QSignalSpy{&config, &QAccelPlot::PlotRectangleZoom::borderColorChanged};
    config.setEnabled(true);
    config.setEnabled(true);
    config.setMinimumSize(-1);
    config.setMinimumSize(-2);
    config.setMinimumSize(std::numeric_limits<qreal>::quiet_NaN());
    config.setMinimumSize(std::numeric_limits<qreal>::infinity());
    config.setModifiers(Qt::ControlModifier);
    config.setModifiers(Qt::ControlModifier);
    config.setFillColor(Qt::red);
    config.setFillColor(Qt::red);
    config.setBorderColor(Qt::green);
    config.setBorderColor(Qt::green);
    QCOMPARE(enabledSpy.count(), 1);
    QCOMPARE(sizeSpy.count(), 1);
    QCOMPARE(modifiersSpy.count(), 1);
    QCOMPARE(fillSpy.count(), 1);
    QCOMPARE(borderSpy.count(), 1);
    QCOMPARE(config.minimumSize(), 0.0);
}

void TestRectangleZoom::directZoom_data()
{
    QTest::addColumn<bool>("log");
    QTest::addColumn<bool>("reversed");
    QTest::newRow("linear") << false << false;
    QTest::newRow("linear-reversed") << false << true;
    QTest::newRow("log") << true << false;
    QTest::newRow("log-reversed") << true << true;
}

void TestRectangleZoom::directZoom()
{
    QFETCH(bool, log);
    QFETCH(bool, reversed);
    auto f = Fixture{};
    const auto min = log ? 1.0 : 0.0;
    const auto max = log ? 1e4 : 100.0;
    setRange(f.x, reversed ? max : min, reversed ? min : max);
    setRange(f.y, reversed ? max : min, reversed ? min : max);
    f.x->setLogScale(log);
    f.y->setLogScale(log);
    f.plot.rectangleZoom()->setEnabled(false);
    QVERIFY(f.plot.zoomToRect(f.selection()));
    const auto low = log ? 10.0 : 25.0;
    const auto high = log ? 1000.0 : 75.0;
    compareRange(f.x, reversed ? high : low, reversed ? low : high);
    compareRange(f.y, reversed ? high : low, reversed ? low : high);
}

void TestRectangleZoom::allAxesUseOwnRanges()
{
    auto f = Fixture{};
    auto* x2 = new QAccelPlot::Axis{&f.plot};
    auto* y2 = new QAccelPlot::Axis{&f.plot};
    auto* extraX = new QAccelPlot::Axis{&f.plot, QAccelPlot::Axis::Top};
    auto* extraY = new QAccelPlot::Axis{&f.plot, QAccelPlot::Axis::Right};
    f.plot.setX2Axis(x2);
    f.plot.setY2Axis(y2);
    auto extras = f.plot.extraAxes();
    extras.append(&extras, extraX);
    extras.append(&extras, extraY);
    setRange(x2, -100, 100);
    setRange(y2, 100, 0);
    setRange(extraX, 1, 1e8);
    extraX->setLogScale(true);
    setRange(extraY, -20, 20);
    extraY->setVisible(false);
    QVERIFY(f.plot.zoomToRect(f.selection()));
    compareRange(f.x, 25, 75);
    compareRange(f.y, -5, 5);
    compareRange(x2, -50, 50);
    compareRange(y2, 75, 25);
    compareRange(extraX, 100, 1e6);
    compareRange(extraY, -10, 10);
}

void TestRectangleZoom::clippingAndNormalization()
{
    auto f = Fixture{};
    const auto rect = QRectF{f.point(0.75, 1.5), f.point(-0.5, 0.25)};
    QVERIFY(f.plot.zoomToRect(rect));
    compareRange(f.x, 0, 75);
    compareRange(f.y, -10, 5);
}

void TestRectangleZoom::invalidSelection_data()
{
    QTest::addColumn<QRectF>("rect");
    const auto nan = std::numeric_limits<qreal>::quiet_NaN();
    const auto inf = std::numeric_limits<qreal>::infinity();
    QTest::newRow("empty") << QRectF{};
    QTest::newRow("outside") << QRectF{-100, -100, 20, 20};
    QTest::newRow("nan") << QRectF{nan, 50, 20, 20};
    QTest::newRow("infinite-width") << QRectF{50, 50, inf, 20};
    QTest::newRow("infinite-height") << QRectF{50, 50, 20, inf};
}

void TestRectangleZoom::invalidSelection()
{
    QFETCH(QRectF, rect);
    auto f = Fixture{};
    QVERIFY(!f.plot.zoomToRect(rect));
    QVERIFY(!f.plot.zoomToRect(QRectF{f.point(0.25, 0.25), QSizeF{5, 20}}));
    QVERIFY(!f.plot.zoomToRect(QRectF{f.point(0.25, 0.25), QSizeF{20, 5}}));
    f.plot.rectangleZoom()->setMinimumSize(0);
    QVERIFY(!f.plot.zoomToRect(QRectF{f.point(0.25, 0.25), QSizeF{0, 20}}));
    compareRange(f.x, 0, 100);
    compareRange(f.y, -10, 10);
}

void TestRectangleZoom::invalidAxisRejectsWholeZoom()
{
    auto f = Fixture{};
    f.y->setLogScale(true);
    setRange(f.y, 0, 100);
    QVERIFY(!f.plot.zoomToRect(f.selection()));
    compareRange(f.x, 0, 100);
    f.y->setLogScale(false);
    setRange(f.y, 10, 10);
    QVERIFY(!f.plot.zoomToRect(f.selection()));
    compareRange(f.x, 0, 100);
    setRange(f.y, 0, std::numeric_limits<qreal>::infinity());
    QVERIFY(!f.plot.zoomToRect(f.selection()));
    compareRange(f.x, 0, 100);
}

void TestRectangleZoom::noAxesIsIgnored()
{
    auto plot = TestablePlot{};
    plot.setSize({600, 400});
    QVERIFY(!plot.zoomToRect(plot.plotRect()));
}

void TestRectangleZoom::rangesComputedBeforeLayoutChanges()
{
    auto f = Fixture{};
    connect(f.x, &QAccelPlot::Axis::viewportMinChanged, &f.plot, [&f]() { f.plot.setPadding(50); });
    QVERIFY(f.plot.zoomToRect(f.selection()));
    compareRange(f.x, 25, 75);
    compareRange(f.y, -5, 5);
}

void TestRectangleZoom::dragDirections_data()
{
    QTest::addColumn<QPointF>("start");
    QTest::addColumn<QPointF>("end");
    QTest::newRow("down-right") << QPointF{0.25, 0.25} << QPointF{0.75, 0.75};
    QTest::newRow("up-left") << QPointF{0.75, 0.75} << QPointF{0.25, 0.25};
    QTest::newRow("up-right") << QPointF{0.25, 0.75} << QPointF{0.75, 0.25};
    QTest::newRow("down-left") << QPointF{0.75, 0.25} << QPointF{0.25, 0.75};
}

void TestRectangleZoom::dragDirections()
{
    QFETCH(QPointF, start);
    QFETCH(QPointF, end);
    auto f = Fixture{};
    auto activeSpy = QSignalSpy{f.plot.rectangleZoom(), &QAccelPlot::PlotRectangleZoom::activeChanged};
    press(f, f.point(start.x(), start.y()));
    QVERIFY(f.plot.rectangleZoom()->active());
    move(f, f.point(end.x(), end.y()));
    QCOMPARE(f.plot.rectangleZoom()->selectionRect(), f.selection());
    compareRange(f.x, 0, 100);
    compareRange(f.y, -10, 10);
    release(f, f.point(end.x(), end.y()));
    QVERIFY(!f.plot.rectangleZoom()->active());
    QVERIFY(f.plot.rectangleZoom()->selectionRect().isEmpty());
    QCOMPARE(activeSpy.count(), 2);
    compareRange(f.x, 25, 75);
    compareRange(f.y, -5, 5);
}

void TestRectangleZoom::disabledFeaturePreservesPan()
{
    auto f = Fixture{};
    f.plot.rectangleZoom()->setEnabled(false);
    press(f, f.point(0.25, 0.25));
    move(f, f.point(0.5, 0.5));
    release(f, f.point(0.5, 0.5));
    compareRange(f.x, -25, 75);
    compareRange(f.y, -5, 15);
    QVERIFY(!f.plot.rectangleZoom()->active());
}

void TestRectangleZoom::configuredModifiersAndFrozenGesture()
{
    auto f = Fixture{};
    f.plot.rectangleZoom()->setModifiers(Qt::ControlModifier);
    press(f, f.point(0.25, 0.25), Qt::ControlModifier);
    move(f, f.point(0.75, 0.75), Qt::NoModifier);
    QVERIFY(f.plot.rectangleZoom()->active());
    release(f, f.point(0.75, 0.75));
    compareRange(f.x, 25, 75);
    setRange(f.x, 0, 100);
    press(f, f.point(0.25, 0.25), Qt::ControlModifier | Qt::AltModifier);
    move(f, f.point(0.5, 0.25), Qt::ControlModifier);
    release(f, f.point(0.5, 0.25));
    QVERIFY(!f.plot.rectangleZoom()->active());
    compareRange(f.x, -25, 75);
    f.plot.rectangleZoom()->setModifiers(Qt::NoModifier);
    press(f, f.point(0.25, 0.25), Qt::NoModifier);
    QVERIFY(f.plot.rectangleZoom()->active());
    f.plot.mouseUngrabEvent();
}

void TestRectangleZoom::pressOutsidePlotDoesNotSelect()
{
    auto f = Fixture{};
    press(f, {1, 1});
    move(f, f.point(0.75, 0.75));
    release(f, f.point(0.75, 0.75));
    QVERIFY(!f.plot.rectangleZoom()->active());
    compareRange(f.x, 0, 100);
    compareRange(f.y, -10, 10);
}

void TestRectangleZoom::acceptedEvents()
{
    auto f = Fixture{};
    auto acceptPress = connect(&f.plot, &QAccelPlot::QAccelPlot::mousePressed, [](QAccelPlot::PlotMouseEvent* e) { e->accept(); });
    press(f, f.point(0.25, 0.25));
    QVERIFY(!f.plot.rectangleZoom()->active());
    disconnect(acceptPress);
    press(f, f.point(0.25, 0.25));
    const auto initial = f.plot.rectangleZoom()->selectionRect();
    auto acceptMove = connect(&f.plot, &QAccelPlot::QAccelPlot::mouseMoved, [](QAccelPlot::PlotMouseEvent* e) { e->accept(); });
    move(f, f.point(0.75, 0.75));
    QCOMPARE(f.plot.rectangleZoom()->selectionRect(), initial);
    disconnect(acceptMove);
    move(f, f.point(0.75, 0.75));
    connect(&f.plot, &QAccelPlot::QAccelPlot::mouseReleased, [](QAccelPlot::PlotMouseEvent* e) { e->accept(); });
    release(f, f.point(0.75, 0.75));
    QVERIFY(!f.plot.rectangleZoom()->active());
    QVERIFY(f.plot.rectangleZoom()->selectionRect().isEmpty());
    compareRange(f.x, 0, 100);
    compareRange(f.y, -10, 10);
}

void TestRectangleZoom::releaseUsesFinalPosition()
{
    auto f = Fixture{};
    press(f, f.point(0.25, 0.25));
    release(f, f.point(0.75, 0.75));
    compareRange(f.x, 25, 75);
    compareRange(f.y, -5, 5);
}

void TestRectangleZoom::unrelatedReleaseKeepsSelection()
{
    auto f = Fixture{};
    press(f, f.point(0.25, 0.25));
    mouse(f.plot, QEvent::MouseButtonRelease, f.point(0.5, 0.5), Qt::RightButton, Qt::LeftButton);
    QVERIFY(f.plot.rectangleZoom()->active());
    release(f, f.point(0.75, 0.75));
    compareRange(f.x, 25, 75);
}

void TestRectangleZoom::cancellation_data()
{
    QTest::addColumn<QString>("cause");
    for (const auto* cause : {"escape", "ungrab", "focus", "disabled", "hidden", "item-disabled", "resize", "axis-range", "axis-log", "axis-orientation",
             "axis-replaced", "axis-destroyed", "extra-added", "layout"}) {
        QTest::newRow(cause) << QString::fromLatin1(cause);
    }
}

void TestRectangleZoom::cancellation()
{
    QFETCH(QString, cause);
    auto f = Fixture{};
    press(f, f.point(0.25, 0.25));
    move(f, f.point(0.75, 0.75));
    QVERIFY(f.plot.rectangleZoom()->active());
    if (cause == "escape") {
        auto e = QKeyEvent{QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier};
        f.plot.keyPressEvent(&e);
        QVERIFY(e.isAccepted());
    } else if (cause == "ungrab") {
        f.plot.mouseUngrabEvent();
    } else if (cause == "focus") {
        auto e = QFocusEvent{QEvent::FocusOut};
        f.plot.focusOutEvent(&e);
    } else if (cause == "disabled") {
        f.plot.rectangleZoom()->setEnabled(false);
    } else if (cause == "hidden") {
        f.plot.setVisible(false);
    } else if (cause == "item-disabled") {
        f.plot.setEnabled(false);
    } else if (cause == "resize") {
        f.plot.setWidth(700);
    } else if (cause == "axis-range") {
        f.x->setViewportMin(-10);
    } else if (cause == "axis-log") {
        f.x->setLogScale(true);
    } else if (cause == "axis-orientation") {
        f.x->setSide(QAccelPlot::Axis::Left);
    } else if (cause == "axis-replaced") {
        f.plot.setXAxis(new QAccelPlot::Axis{&f.plot});
    } else if (cause == "axis-destroyed") {
        delete f.x;
        f.x = nullptr;
    } else if (cause == "extra-added") {
        auto extras = f.plot.extraAxes();
        extras.append(&extras, new QAccelPlot::Axis{&f.plot});
    } else if (cause == "layout") {
        f.plot.setPadding(50);
    }
    QVERIFY(!f.plot.rectangleZoom()->active());
    QVERIFY(f.plot.rectangleZoom()->selectionRect().isEmpty());
    const auto min = f.y->viewportMin();
    const auto max = f.y->viewportMax();
    move(f, f.point(0.5, 0.5));
    release(f, f.point(0.5, 0.5));
    compareRange(f.y, min, max);
}

void TestRectangleZoom::wheelDuringSelectionIsConsumed()
{
    auto f = Fixture{};
    press(f, f.point(0.25, 0.25));
    auto e = QWheelEvent{f.point(0.5, 0.5), f.point(0.5, 0.5), QPoint{}, QPoint{0, 120}, Qt::LeftButton, Qt::ShiftModifier, Qt::NoScrollPhase, false};
    f.plot.wheelEvent(&e);
    QVERIFY(e.isAccepted());
    compareRange(f.x, 0, 100);
    QVERIFY(f.plot.rectangleZoom()->active());
    release(f, f.point(0.75, 0.75));
    compareRange(f.x, 25, 75);
}

void TestRectangleZoom::doubleClickCancelsAndRescales()
{
    auto f = Fixture{};
    f.x->setDataRange(20, 40);
    press(f, f.point(0.25, 0.25));
    auto e = QMouseEvent{QEvent::MouseButtonDblClick, f.point(0.5, 0.5), f.point(0.5, 0.5), Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier};
    f.plot.mouseDoubleClickEvent(&e);
    QVERIFY(!f.plot.rectangleZoom()->active());
    compareRange(f.x, 20, 40);
}

void TestRectangleZoom::windowDragOutsidePlot()
{
    auto window = QQuickWindow{};
    window.resize(600, 400);
    auto f = Fixture{};
    f.plot.setParentItem(window.contentItem());
    const auto start = f.point(0.25, 0.25).toPoint();
    const auto end = f.point(1.1, 1.1).toPoint();
    QTest::mousePress(&window, Qt::LeftButton, Qt::ShiftModifier, start);
    QVERIFY(f.plot.rectangleZoom()->active());
    QTest::mouseMove(&window, end);
    const auto expected = QRectF{QPointF{start}, f.plot.plotRect().bottomRight()};
    QCOMPARE(f.plot.rectangleZoom()->selectionRect(), expected);
    QTest::mouseRelease(&window, Qt::LeftButton, Qt::ShiftModifier, end);
    QVERIFY(!f.plot.rectangleZoom()->active());
    QVERIFY(f.x->viewportMin() > 20 && f.x->viewportMin() < 30);
    QCOMPARE(f.x->viewportMax(), 100.0);
    QCOMPARE(f.y->viewportMin(), -10.0);
}

QTEST_MAIN(TestRectangleZoom)
#include "tst_rectangle_zoom.moc"
