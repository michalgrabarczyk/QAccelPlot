//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "Axis.hpp"
#include "QAccelPlot.hpp"

#include <QMouseEvent>
#include <QtTest/QtTest>

#include <memory>

class TestablePlot final : public QAccelPlot::QAccelPlot {
public:
    using QAccelPlot::QAccelPlot::mouseMoveEvent;
    using QAccelPlot::QAccelPlot::mousePressEvent;
    using QAccelPlot::QAccelPlot::mouseReleaseEvent;
};

class TestPlotAppearance : public QObject {
    Q_OBJECT

private slots:
    void border_defaultsToDisabled();
    void border_clampsNegativeWidth();
    void baselineWidth_defaultsAndClamps();
    void acceptedReleaseEndsDrag();
    void hiddenAxesDoNotReserveLayoutSpace();
    void hiddenAxisRetainsCoordinateMapping();
    void hiddenExtraAxisDoesNotReserveLayoutSpace();
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

void TestPlotAppearance::acceptedReleaseEndsDrag()
{
    auto plot = TestablePlot{};
    plot.setSize({200.0, 100.0});
    auto* axis = new QAccelPlot::Axis{};
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

void TestPlotAppearance::hiddenAxesDoNotReserveLayoutSpace()
{
    QAccelPlot::QAccelPlot plot;
    plot.setSize({300.0, 200.0});
    plot.setPadding(10.0);

    auto* xAxis = new QAccelPlot::Axis{};
    auto* yAxis = new QAccelPlot::Axis{};
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

    auto* yAxis = new QAccelPlot::Axis{};
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

    auto extraAxis = std::make_unique<QAccelPlot::Axis>();
    auto extraAxes = plot.extraAxes();
    extraAxes.append(&extraAxes, extraAxis.get());
    const auto visiblePlotRect = plot.plotRect();

    extraAxis->setVisible(false);
    QCOMPARE(plot.plotRect(), QRectF(10.0, 10.0, 280.0, 230.0));
    QCOMPARE(extraAxis->size(), QSizeF{});

    extraAxis->setVisible(true);
    QCOMPARE(plot.plotRect(), visiblePlotRect);
    QVERIFY(!extraAxis->size().isEmpty());

    extraAxes.clear(&extraAxes);
}

QTEST_MAIN(TestPlotAppearance)
#include "tst_plot_appearance.moc"
