//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/series/LineCurve.hpp"

#include <QHoverEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtTest/QtTest>

class TestablePlot final : public QAccelPlot::QAccelPlot {
public:
    using QAccelPlot::QAccelPlot::hoverMoveEvent;
    using QAccelPlot::QAccelPlot::mouseMoveEvent;
    using QAccelPlot::QAccelPlot::mousePressEvent;
    using QAccelPlot::QAccelPlot::mouseReleaseEvent;
    using QAccelPlot::QAccelPlot::wheelEvent;
};

class TestPlotAppearance : public QObject {
    Q_OBJECT

private slots:
    void border_defaultsToDisabled();
    void border_clampsNegativeWidth();
    void baselineWidth_defaultsAndClamps();
    void layoutSize_defaultsAndClamps();
    void namedAxesAssignSidesAutomatically();
    void customAxisLayoutSizesUpdatePlotRect();
    void extraAxesUseIndividualLayoutSizes();
    void topSideExtraAxisStacksAboveThePlot();
    void rightSideExtraAxisStacksRightOfThePlot();
    void duplicateExtraAxisIsIgnored();
    void outOfBoundsExtraAxisIndexReturnsNull();
    void tinyPlotNeverGivesAxesNegativeSize();
    void axisHoverEnvironmentControlsAcceptance();
    void acceptedReleaseEndsDrag();
    void horizontalOnlyWheelScrollDoesNotZoom();
    void mouseMovedFiresWhileHoveringWithoutButton();
    void mouseMovedDoesNotFireTwiceWhileDragging();
    void tickOverlapChangeTriggersRelayout();
    void labelOverflowChangeTriggersRelayout();
    void hiddenAxesDoNotReserveLayoutSpace();
    void hiddenAxisRetainsCoordinateMapping();
    void hiddenExtraAxisDoesNotReserveLayoutSpace();
    void destroyedAxesAreUnregistered();
    void destroyedSeriesAreUnregistered();
    void reparentedSeriesAreUnregistered();
};

void TestPlotAppearance::border_defaultsToDisabled()
{
    QAccelPlot::QAccelPlot plot;

    QCOMPARE(plot.border()->width(), 0.0);
    QCOMPARE(plot.border()->color(), QColor(Qt::transparent));
}

void TestPlotAppearance::border_clampsNegativeWidth()
{
    QAccelPlot::QAccelPlot plot;
    QSignalSpy widthChanged(plot.border(), &QAccelPlot::PlotBorder::widthChanged);
    QSignalSpy colorChanged(plot.border(), &QAccelPlot::PlotBorder::colorChanged);

    plot.border()->setWidth(-2.0);
    QCOMPARE(plot.border()->width(), 0.0);
    QCOMPARE(widthChanged.count(), 0);

    plot.border()->setWidth(3.0);
    QCOMPARE(plot.border()->width(), 3.0);
    QCOMPARE(widthChanged.count(), 1);

    plot.border()->setColor(QColor("#303030"));
    QCOMPARE(plot.border()->color(), QColor("#303030"));
    QCOMPARE(colorChanged.count(), 1);
}

void TestPlotAppearance::baselineWidth_defaultsAndClamps()
{
    QAccelPlot::Axis axis;
    QSignalSpy widthChanged(&axis, &QAccelPlot::Axis::baselineWidthChanged);

    QCOMPARE(axis.baselineWidth(), 2.0);
    axis.setBaselineWidth(-1.0);
    QCOMPARE(axis.baselineWidth(), 0.0);
    QCOMPARE(widthChanged.count(), 1);

    axis.setBaselineWidth(3.0);
    QCOMPARE(axis.baselineWidth(), 3.0);
    QCOMPARE(widthChanged.count(), 2);
}

void TestPlotAppearance::layoutSize_defaultsAndClamps()
{
    QAccelPlot::Axis axis;
    QSignalSpy sizeChanged(&axis, &QAccelPlot::Axis::layoutSizeChanged);

    QCOMPARE(axis.layoutSize(), 50.0);
    axis.setLayoutSize(-1.0);
    QCOMPARE(axis.layoutSize(), 0.0);
    QCOMPARE(sizeChanged.count(), 1);

    axis.setLayoutSize(75.0);
    QCOMPARE(axis.layoutSize(), 75.0);
    QCOMPARE(sizeChanged.count(), 2);
}

void TestPlotAppearance::namedAxesAssignSidesAutomatically()
{
    QAccelPlot::QAccelPlot plot;
    auto* xAxis = new QAccelPlot::Axis{&plot, QAccelPlot::Axis::Left};
    auto* yAxis = new QAccelPlot::Axis{&plot, QAccelPlot::Axis::Top};
    auto* x2Axis = new QAccelPlot::Axis{&plot, QAccelPlot::Axis::Bottom};
    auto* y2Axis = new QAccelPlot::Axis{&plot, QAccelPlot::Axis::Bottom};

    plot.setXAxis(xAxis);
    plot.setYAxis(yAxis);
    plot.setX2Axis(x2Axis);
    plot.setY2Axis(y2Axis);

    QCOMPARE(xAxis->side(), QAccelPlot::Axis::Bottom);
    QCOMPARE(xAxis->orientation(), QAccelPlot::Axis::Horizontal);
    QCOMPARE(yAxis->side(), QAccelPlot::Axis::Left);
    QCOMPARE(yAxis->orientation(), QAccelPlot::Axis::Vertical);
    QCOMPARE(x2Axis->side(), QAccelPlot::Axis::Top);
    QCOMPARE(x2Axis->orientation(), QAccelPlot::Axis::Horizontal);
    QCOMPARE(y2Axis->side(), QAccelPlot::Axis::Right);
    QCOMPARE(y2Axis->orientation(), QAccelPlot::Axis::Vertical);

    auto* extraAxis = new QAccelPlot::Axis{&plot, QAccelPlot::Axis::Top};
    auto extraAxes = plot.extraAxes();
    extraAxes.append(&extraAxes, extraAxis);
    QCOMPARE(extraAxis->side(), QAccelPlot::Axis::Top);
    QCOMPARE(extraAxis->orientation(), QAccelPlot::Axis::Horizontal);
}

void TestPlotAppearance::customAxisLayoutSizesUpdatePlotRect()
{
    QAccelPlot::QAccelPlot plot;
    plot.setSize({400.0, 300.0});
    plot.setPadding(10.0);

    auto* xAxis = new QAccelPlot::Axis{&plot};
    xAxis->setLayoutSize(70.0);
    auto* yAxis = new QAccelPlot::Axis{&plot};
    yAxis->setLayoutSize(90.0);
    auto* x2Axis = new QAccelPlot::Axis{&plot};
    x2Axis->setLayoutSize(40.0);
    auto* y2Axis = new QAccelPlot::Axis{&plot};
    y2Axis->setLayoutSize(60.0);
    plot.setXAxis(xAxis);
    plot.setYAxis(yAxis);
    plot.setX2Axis(x2Axis);
    plot.setY2Axis(y2Axis);

    QCOMPARE(plot.plotRect(), QRectF(100.0, 50.0, 230.0, 170.0));
    QCOMPARE(xAxis->height(), 78.0);
    QCOMPARE(yAxis->width(), 98.0);
    QCOMPARE(x2Axis->height(), 48.0);
    QCOMPARE(y2Axis->width(), 68.0);

    QSignalSpy plotRectChanged(&plot, &QAccelPlot::QAccelPlot::plotRectChanged);
    yAxis->setLayoutSize(100.0);
    QCOMPARE(plot.plotRect(), QRectF(110.0, 50.0, 220.0, 170.0));
    QCOMPARE(plotRectChanged.count(), 1);
}

void TestPlotAppearance::extraAxesUseIndividualLayoutSizes()
{
    QAccelPlot::QAccelPlot plot;
    plot.setSize({300.0, 250.0});
    plot.setPadding(10.0);

    auto* xAxis = new QAccelPlot::Axis{&plot};
    xAxis->setLayoutSize(60.0);
    plot.setXAxis(xAxis);

    auto* horizontalAxis = new QAccelPlot::Axis{&plot};
    horizontalAxis->setLayoutSize(30.0);
    auto* verticalAxis = new QAccelPlot::Axis{&plot};
    verticalAxis->setSide(QAccelPlot::Axis::Left);
    verticalAxis->setLayoutSize(75.0);

    auto extraAxes = plot.extraAxes();
    extraAxes.append(&extraAxes, horizontalAxis);
    extraAxes.append(&extraAxes, verticalAxis);

    QCOMPARE(plot.plotRect(), QRectF(93.0, 10.0, 197.0, 132.0));
    QCOMPARE(horizontalAxis->height(), 38.0);
    QCOMPARE(verticalAxis->width(), 83.0);
}

void TestPlotAppearance::topSideExtraAxisStacksAboveThePlot()
{
    QAccelPlot::QAccelPlot plot;
    plot.setSize({300.0, 250.0});
    plot.setPadding(10.0);

    auto* topExtra = new QAccelPlot::Axis{&plot};
    topExtra->setSide(QAccelPlot::Axis::Top);
    topExtra->setLayoutSize(40.0);

    auto extraAxes = plot.extraAxes();
    extraAxes.append(&extraAxes, topExtra);

    QCOMPARE(plot.plotRect(), QRectF(10.0, 58.0, 280.0, 182.0));
    QCOMPARE(topExtra->y(), 10.0);
    QCOMPARE(topExtra->height(), 48.0);
}

void TestPlotAppearance::rightSideExtraAxisStacksRightOfThePlot()
{
    QAccelPlot::QAccelPlot plot;
    plot.setSize({300.0, 250.0});
    plot.setPadding(10.0);

    auto* rightExtra = new QAccelPlot::Axis{&plot};
    rightExtra->setSide(QAccelPlot::Axis::Right);
    rightExtra->setLayoutSize(50.0);

    auto extraAxes = plot.extraAxes();
    extraAxes.append(&extraAxes, rightExtra);

    QCOMPARE(plot.plotRect(), QRectF(10.0, 10.0, 222.0, 230.0));
    QCOMPARE(rightExtra->width(), 58.0);
}

void TestPlotAppearance::duplicateExtraAxisIsIgnored()
{
    QAccelPlot::QAccelPlot plot;
    plot.setSize({300.0, 250.0});
    plot.setPadding(10.0);

    auto* extraAxis = new QAccelPlot::Axis{&plot};
    extraAxis->setSide(QAccelPlot::Axis::Right);
    extraAxis->setLayoutSize(50.0);

    auto extraAxes = plot.extraAxes();
    extraAxes.append(&extraAxes, extraAxis);
    const auto plotRectAfterFirstAppend = plot.plotRect();

    extraAxes.append(&extraAxes, extraAxis);

    QCOMPARE(extraAxes.count(&extraAxes), 1);
    QCOMPARE(plot.plotRect(), plotRectAfterFirstAppend);
}

void TestPlotAppearance::outOfBoundsExtraAxisIndexReturnsNull()
{
    QAccelPlot::QAccelPlot plot;
    auto* extraAxis = new QAccelPlot::Axis{&plot};
    auto extraAxes = plot.extraAxes();
    extraAxes.append(&extraAxes, extraAxis);

    QCOMPARE(extraAxes.at(&extraAxes, 0), extraAxis);
    QCOMPARE(extraAxes.at(&extraAxes, 1), nullptr);
    QCOMPARE(extraAxes.at(&extraAxes, -1), nullptr);
}

void TestPlotAppearance::tinyPlotNeverGivesAxesNegativeSize()
{
    // The reserved padding and axis sizes add up to far more than the item itself.
    QAccelPlot::QAccelPlot plot;
    plot.setSize({20.0, 20.0});
    plot.setPadding(10.0);

    auto* xAxis = new QAccelPlot::Axis{&plot};
    auto* yAxis = new QAccelPlot::Axis{&plot};
    auto* x2Axis = new QAccelPlot::Axis{&plot};
    auto* y2Axis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    plot.setYAxis(yAxis);
    plot.setX2Axis(x2Axis);
    plot.setY2Axis(y2Axis);

    auto* extraAxis = new QAccelPlot::Axis{&plot};
    auto extraAxes = plot.extraAxes();
    extraAxes.append(&extraAxes, extraAxis);

    QVERIFY(plot.plotRect().width() >= 1.0);
    QVERIFY(plot.plotRect().height() >= 1.0);
    for (const auto* axis : {xAxis, yAxis, x2Axis, y2Axis, extraAxis}) {
        QVERIFY2(axis->width() >= 0.0, qPrintable(QStringLiteral("axis width %1").arg(axis->width())));
        QVERIFY2(axis->height() >= 0.0, qPrintable(QStringLiteral("axis height %1").arg(axis->height())));
    }
}

void TestPlotAppearance::axisHoverEnvironmentControlsAcceptance()
{
    constexpr auto variableName = "QACCELPLOT_HOVER_ENABLED";
    const auto wasSet = qEnvironmentVariableIsSet(variableName);
    const auto previousValue = qgetenv(variableName);

    qunsetenv(variableName);
    const auto defaultAxis = QAccelPlot::Axis{};
    qputenv(variableName, "0");
    const auto hoverDisabledAxis = QAccelPlot::Axis{};

    if (wasSet) {
        qputenv(variableName, previousValue);
    } else {
        qunsetenv(variableName);
    }

    QVERIFY(defaultAxis.acceptHoverEvents());
    QVERIFY(!hoverDisabledAxis.acceptHoverEvents());
}

void TestPlotAppearance::acceptedReleaseEndsDrag()
{
    auto plot = TestablePlot{};
    plot.setSize({200.0, 100.0});
    auto* axis = new QAccelPlot::Axis{&plot};
    axis->setViewportMin(0.0);
    axis->setViewportMax(100.0);
    plot.setXAxis(axis);
    connect(&plot, &QAccelPlot::QAccelPlot::mouseReleased, [](QAccelPlot::PlotMouseEvent* event) { event->accept(); });

    const auto pressPosition = QPointF{50.0, 50.0};
    auto press = QMouseEvent{QEvent::MouseButtonPress, pressPosition, pressPosition, pressPosition, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
    plot.mousePressEvent(&press);
    auto release = QMouseEvent{QEvent::MouseButtonRelease, pressPosition, pressPosition, pressPosition, Qt::LeftButton, Qt::NoButton, Qt::NoModifier};
    plot.mouseReleaseEvent(&release);

    const auto minAfterRelease = axis->viewportMin();
    const auto maxAfterRelease = axis->viewportMax();
    const auto movePosition = QPointF{75.0, 50.0};
    auto move = QMouseEvent{QEvent::MouseMove, movePosition, movePosition, movePosition, Qt::NoButton, Qt::NoButton, Qt::NoModifier};
    plot.mouseMoveEvent(&move);

    QCOMPARE(axis->viewportMin(), minAfterRelease);
    QCOMPARE(axis->viewportMax(), maxAfterRelease);
}

void TestPlotAppearance::horizontalOnlyWheelScrollDoesNotZoom()
{
    auto plot = TestablePlot{};
    plot.setSize({200.0, 100.0});
    auto* axis = new QAccelPlot::Axis{&plot};
    axis->setViewportMin(0.0);
    axis->setViewportMax(100.0);
    plot.setXAxis(axis);

    const auto minBefore = axis->viewportMin();
    const auto maxBefore = axis->viewportMax();

    const auto pos = QPointF{100.0, 50.0};
    auto horizontalScroll = QWheelEvent{pos, pos, QPoint(0, 0), QPoint(120, 0), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false};
    plot.wheelEvent(&horizontalScroll);

    QCOMPARE(axis->viewportMin(), minBefore);
    QCOMPARE(axis->viewportMax(), maxBefore);
}

void TestPlotAppearance::mouseMovedFiresWhileHoveringWithoutButton()
{
    auto plot = TestablePlot{};
    plot.setSize({200.0, 100.0});

    auto movedX = qreal{};
    auto movedButtons = -1;
    connect(&plot, &QAccelPlot::QAccelPlot::mouseMoved, [&](QAccelPlot::PlotMouseEvent* event) {
        movedX = event->x();
        movedButtons = event->button();
    });

    const auto pos = QPointF{40.0, 25.0};
    auto hover = QHoverEvent{QEvent::HoverMove, pos, pos, QPointF{10.0, 25.0}};
    plot.hoverMoveEvent(&hover);

    QCOMPARE(movedX, 40.0);
    QCOMPARE(movedButtons, static_cast<int>(Qt::NoButton));
}

void TestPlotAppearance::mouseMovedDoesNotFireTwiceWhileDragging()
{
    auto plot = TestablePlot{};
    plot.setSize({200.0, 100.0});
    auto* axis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(axis);

    auto moveCount = 0;
    connect(&plot, &QAccelPlot::QAccelPlot::mouseMoved, [&](QAccelPlot::PlotMouseEvent*) { ++moveCount; });

    const auto pressPosition = QPointF{50.0, 50.0};
    auto press = QMouseEvent{QEvent::MouseButtonPress, pressPosition, pressPosition, pressPosition, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
    plot.mousePressEvent(&press);

    // A drag delivers a mouse move; any hover move Qt also synthesizes must not emit a second time.
    const auto movePosition = QPointF{75.0, 50.0};
    auto move = QMouseEvent{QEvent::MouseMove, movePosition, movePosition, movePosition, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
    plot.mouseMoveEvent(&move);
    auto hover = QHoverEvent{QEvent::HoverMove, movePosition, movePosition, pressPosition};
    plot.hoverMoveEvent(&hover);

    QCOMPARE(moveCount, 1);
}

void TestPlotAppearance::tickOverlapChangeTriggersRelayout()
{
    QAccelPlot::QAccelPlot plot;
    plot.setSize({300.0, 200.0});
    plot.setPadding(10.0);

    auto* xAxis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    const auto heightBefore = xAxis->height();

    xAxis->ticker()->setTickLengthIn(50.0);

    QCOMPARE(xAxis->height(), heightBefore + 42.0);
}

void TestPlotAppearance::labelOverflowChangeTriggersRelayout()
{
    QAccelPlot::QAccelPlot plot;
    plot.setSize({300.0, 200.0});
    plot.setPadding(10.0);

    auto* xAxis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    const auto widthBefore = xAxis->width();

    xAxis->setOrientation(QAccelPlot::Axis::Vertical);

    QCOMPARE(xAxis->width(), widthBefore - 30.0);
}

void TestPlotAppearance::hiddenAxesDoNotReserveLayoutSpace()
{
    QAccelPlot::QAccelPlot plot;
    plot.setSize({300.0, 200.0});
    plot.setPadding(10.0);

    auto* xAxis = new QAccelPlot::Axis{&plot};
    auto* yAxis = new QAccelPlot::Axis{&plot};
    plot.setXAxis(xAxis);
    plot.setYAxis(yAxis);

    QCOMPARE(plot.plotRect(), QRectF(60.0, 10.0, 230.0, 130.0));

    QSignalSpy plotRectChanged(&plot, &QAccelPlot::QAccelPlot::plotRectChanged);
    yAxis->setVisible(false);
    QCOMPARE(plot.plotRect(), QRectF(10.0, 10.0, 280.0, 130.0));
    QCOMPARE(yAxis->size(), QSizeF{});
    QCOMPARE(plotRectChanged.count(), 1);

    xAxis->setVisible(false);
    QCOMPARE(plot.plotRect(), QRectF(10.0, 10.0, 280.0, 180.0));
    QCOMPARE(xAxis->size(), QSizeF{});
    QCOMPARE(plotRectChanged.count(), 2);

    yAxis->setVisible(true);
    QCOMPARE(plot.plotRect(), QRectF(60.0, 10.0, 230.0, 180.0));
    QVERIFY(!yAxis->size().isEmpty());
    QCOMPARE(plotRectChanged.count(), 3);
}

void TestPlotAppearance::hiddenAxisRetainsCoordinateMapping()
{
    TestablePlot plot;
    plot.setSize({300.0, 200.0});
    plot.setPadding(10.0);

    auto* yAxis = new QAccelPlot::Axis{&plot};
    yAxis->setViewportMin(-20.0);
    yAxis->setViewportMax(80.0);
    yAxis->setVisible(false);
    plot.setYAxis(yAxis);

    const auto dataValue = 35.0;
    const auto pixel = plot.dataToPixelY(dataValue);
    QVERIFY(qFuzzyCompare(plot.pixelToDataY(pixel), dataValue));
    QCOMPARE(yAxis->viewportMin(), -20.0);
    QCOMPARE(yAxis->viewportMax(), 80.0);

    const auto pressPosition = QPointF{150.0, 100.0};
    auto press = QMouseEvent{QEvent::MouseButtonPress, pressPosition, pressPosition, pressPosition, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier};
    plot.mousePressEvent(&press);
    const auto movePosition = QPointF{150.0, 120.0};
    auto move = QMouseEvent{QEvent::MouseMove, movePosition, movePosition, movePosition, Qt::NoButton, Qt::LeftButton, Qt::NoModifier};
    plot.mouseMoveEvent(&move);

    QVERIFY(yAxis->viewportMin() > -20.0);
    QVERIFY(yAxis->viewportMax() > 80.0);
}

void TestPlotAppearance::hiddenExtraAxisDoesNotReserveLayoutSpace()
{
    QAccelPlot::QAccelPlot plot;
    plot.setSize({300.0, 250.0});
    plot.setPadding(10.0);

    auto* extraAxis = new QAccelPlot::Axis{&plot};
    auto extraAxes = plot.extraAxes();
    extraAxes.append(&extraAxes, extraAxis);
    const auto visiblePlotRect = plot.plotRect();

    extraAxis->setVisible(false);
    QCOMPARE(plot.plotRect(), QRectF(10.0, 10.0, 280.0, 230.0));
    QCOMPARE(extraAxis->size(), QSizeF{});

    extraAxis->setVisible(true);
    QCOMPARE(plot.plotRect(), visiblePlotRect);
    QVERIFY(!extraAxis->size().isEmpty());
}

void TestPlotAppearance::destroyedAxesAreUnregistered()
{
    QAccelPlot::QAccelPlot plot;
    plot.setSize({300.0, 250.0});
    plot.setPadding(10.0);

    auto* xAxis = new QAccelPlot::Axis;
    plot.setXAxis(xAxis);
    delete xAxis;
    QCOMPARE(plot.xAxis(), nullptr);

    auto* extraAxis = new QAccelPlot::Axis;
    auto extraAxes = plot.extraAxes();
    extraAxes.append(&extraAxes, extraAxis);
    QCOMPARE(extraAxes.count(&extraAxes), 1);

    delete extraAxis;
    QCOMPARE(extraAxes.count(&extraAxes), 0);
    QCOMPARE(plot.plotRect(), QRectF(10.0, 10.0, 280.0, 230.0));
}

void TestPlotAppearance::destroyedSeriesAreUnregistered()
{
    auto plot = QAccelPlot::QAccelPlot{};
    auto* curve = new QAccelPlot::LineCurve;
    curve->setParentItem(&plot);
    QCOMPARE(plot.series().size(), 1);
    auto changed = QSignalSpy{&plot, &QAccelPlot::QAccelPlot::seriesChanged};

    delete curve;

    QVERIFY(plot.series().isEmpty());
    QCOMPARE(changed.count(), 1);
    plot.setPadding(25.0);
}

void TestPlotAppearance::reparentedSeriesAreUnregistered()
{
    auto first = QAccelPlot::QAccelPlot{};
    auto second = QAccelPlot::QAccelPlot{};
    auto* curve = new QAccelPlot::LineCurve;
    curve->setParentItem(&first);
    curve->setParentItem(&second);
    QVERIFY(first.series().isEmpty());
    QCOMPARE(second.series().size(), 1);
    auto firstChanged = QSignalSpy{&first, &QAccelPlot::QAccelPlot::seriesChanged};

    delete curve;

    QVERIFY(second.series().isEmpty());
    QCOMPARE(firstChanged.count(), 0);
}

QTEST_MAIN(TestPlotAppearance)
#include "tst_plot_appearance.moc"
