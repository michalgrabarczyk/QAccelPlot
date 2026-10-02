//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "InspectionPlotRig.hpp"

#include <QAccelPlot/PlotRectangleZoom.hpp>
#include <QAccelPlot/inspection/SelectionTool.hpp>

#include <QKeyEvent>
#include <QSignalSpy>
#include <QtTest>

#include <cmath>
#include <limits>

using namespace QAccelPlot;
using namespace InspectionTest;

namespace {

constexpr auto kInfinity = std::numeric_limits<double>::infinity();
constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();

} // namespace

class TestSelectionTool : public QObject {
    Q_OBJECT
private slots:
    void gesture();
    void gestureGuards();
    void ranges();
    void boxAndCode();
    void followsData();
};

void TestSelectionTool::gesture()
{
    auto rig = PlotRig{0, 10, 0, 10};
    rig.plot.rectangleZoom()->setEnabled(true);
    auto* curve = rig.addCurve({5, 5});
    QTRY_COMPARE(rig.plot.series().size(), 1);
    auto tool = SelectionTool{};
    tool.setPlot(&rig.plot);
    auto completed = QSignalSpy{&tool, &SelectionTool::completed};
    auto selecting = QSignalSpy{&tool, &SelectionTool::selectingChanged};
    const auto first = rig.pixel(2.5, 7.5);
    const auto last = rig.pixel(7.5, 2.5);

    // The gesture takes precedence over rectangle zoom and leaves the viewport alone.
    auto press = QMouseEvent{QEvent::MouseButtonPress, first, first, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier};
    rig.plot.mousePressEvent(&press);
    QVERIFY(tool.selecting());
    QVERIFY(!rig.plot.rectangleZoom()->active());
    auto move = QMouseEvent{QEvent::MouseMove, last, last, Qt::NoButton, Qt::LeftButton, Qt::ShiftModifier};
    rig.plot.mouseMoveEvent(&move);
    QCOMPARE(tool.pixelRect(), (QRectF{first, last}.normalized()));
    auto release = QMouseEvent{QEvent::MouseButtonRelease, last, last, Qt::LeftButton, Qt::NoButton, Qt::ShiftModifier};
    rig.plot.mouseReleaseEvent(&release);
    QCOMPARE(rig.x.viewportMin(), 0.0);
    QVERIFY(tool.hasSelection());
    QVERIFY(!tool.selecting());
    QCOMPARE(completed.count(), 1);
    QCOMPARE(selecting.count(), 2);
    QVERIFY(std::abs(tool.xMin() - 2.5) < 1e-9 && std::abs(tool.xMax() - 7.5) < 1e-9);
    QVERIFY(std::abs(tool.yMin() - 2.5) < 1e-9 && std::abs(tool.yMax() - 7.5) < 1e-9);
    QCOMPARE(tool.model()->count(), 1);
    QCOMPARE(tool.model()->get(0).value("summaryCount").toInt(), 1);
    QCOMPARE(tool.model()->get(0).value("meanText").toString(), QStringLiteral("5.000"));
    QCOMPARE(tool.indices(curve).indices, QList<int>{0});

    // A press without a drag selects nothing; it only clears the previous selection.
    rig.drag(first, first);
    QVERIFY(!tool.hasSelection());
    QCOMPARE(completed.count(), 1);
    QCOMPARE(tool.model()->count(), 0);
    QVERIFY(std::isnan(tool.xMin()));
    QCOMPARE(tool.indices(curve).status, InspectionStatus::Unavailable);
}

void TestSelectionTool::gestureGuards()
{
    auto rig = PlotRig{0, 10, 0, 10};
    rig.addCurve({5, 5});
    QTRY_COMPARE(rig.plot.series().size(), 1);
    auto tool = SelectionTool{};
    tool.setPlot(&rig.plot);
    const auto first = rig.pixel(2.5, 7.5);
    const auto last = rig.pixel(7.5, 2.5);
    auto press = QMouseEvent{QEvent::MouseButtonPress, first, first, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier};
    auto release = QMouseEvent{QEvent::MouseButtonRelease, last, last, Qt::LeftButton, Qt::NoButton, Qt::ShiftModifier};

    // Escape and losing the mouse grab cancel a gesture.
    rig.plot.mousePressEvent(&press);
    QVERIFY(tool.selecting());
    auto escape = QKeyEvent{QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier};
    rig.plot.keyPressEvent(&escape);
    QVERIFY(!tool.selecting());
    rig.plot.mousePressEvent(&press);
    rig.plot.mouseUngrabEvent();
    QVERIFY(!tool.selecting() && !tool.hasSelection());

    // Disabling cancels a gesture but keeps a finished selection.
    QVERIFY(tool.select(1, 2, 3, 4));
    tool.setEnabled(false);
    rig.plot.mousePressEvent(&press);
    QVERIFY(!tool.selecting());
    QVERIFY(tool.hasSelection());
    rig.plot.mouseReleaseEvent(&release);
    tool.setEnabled(true);
    tool.clear();

    // The modifiers must match exactly, and a plain drag still pans.
    auto extraPress = QMouseEvent{QEvent::MouseButtonPress, first, first, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier | Qt::ControlModifier};
    rig.plot.mousePressEvent(&extraPress);
    QVERIFY(!tool.selecting());
    rig.plot.mouseReleaseEvent(&release);
    rig.x.setViewportMin(0);
    rig.x.setViewportMax(10);
    rig.drag(first, last, Qt::NoModifier);
    QVERIFY(!tool.hasSelection());
    QVERIFY(rig.x.viewportMin() != 0);
}

void TestSelectionTool::ranges()
{
    auto rig = PlotRig{0, 10, 0, 10};
    auto y2 = Axis{};
    y2.setSide(Axis::Right);
    y2.setViewportMin(0);
    y2.setViewportMax(1000);
    // The middle sample is a spike far above the visible Y viewport.
    auto* primary = rig.addCurve({4, 5, 5, 100, 6, 5}, "primary");
    auto* secondary = rig.addCurve({4, 500, 5, 600, 6, 700}, "secondary");
    secondary->setYAxis(&y2);
    QTRY_COMPARE(rig.plot.series().size(), 2);
    auto tool = SelectionTool{};
    tool.setPlot(&rig.plot);
    const auto area = rig.plot.plotRect();

    // An X range is unbounded in Y: it keeps off-screen samples and series on other Y axes.
    tool.setMode(SelectionTool::XRange);
    rig.drag(rig.pixel(3, 5), rig.pixel(7, 6));
    QVERIFY(tool.hasSelection());
    QVERIFY(std::isinf(tool.yMin()) && std::isinf(tool.yMax()));
    QCOMPARE(tool.model()->count(), 2);
    QCOMPARE(tool.model()->get(0).value("summaryCount").toInt(), 3);
    QCOMPARE(tool.model()->get(0).value("maximum").toDouble(), 100.0);
    QCOMPARE(tool.model()->get(1).value("series").value<PlotSeries*>(), secondary);
    QCOMPARE(tool.model()->get(1).value("summaryCount").toInt(), 3);
    QCOMPARE(tool.indices(primary).total, 3);

    // The overlay spans the plot height and keeps doing so when Y is panned.
    QCOMPARE(tool.pixelRect().top(), area.top());
    QCOMPARE(tool.pixelRect().height(), area.height());
    auto rect = QSignalSpy{&tool, &SelectionTool::pixelRectChanged};
    rig.y.setViewportMin(5);
    rig.y.setViewportMax(15);
    QVERIFY(rect.count() > 0);
    QCOMPARE(tool.pixelRect().top(), area.top());
    QCOMPARE(tool.pixelRect().height(), area.height());

    // A range gesture only has to be long enough along its own axis.
    rig.drag(rig.pixel(3, 8), rig.pixel(3, 8) + QPointF{2, 50});
    QVERIFY(!tool.hasSelection());
    tool.setMode(SelectionTool::YRange);
    rig.drag(rig.pixel(3, 14), rig.pixel(3, 14) + QPointF{2, 50});
    QVERIFY(std::isinf(tool.xMin()) && std::isinf(tool.xMax()));
    QCOMPARE(tool.pixelRect().width(), area.width());
    QCOMPARE(tool.model()->count(), 1);
    QCOMPARE(tool.model()->get(0).value("series").value<PlotSeries*>(), primary);
}

void TestSelectionTool::boxAndCode()
{
    auto rig = PlotRig{0, 10, 0, 10};
    auto y2 = Axis{};
    y2.setSide(Axis::Right);
    y2.setViewportMin(0);
    y2.setViewportMax(1000);
    auto* primary = rig.addCurve({4, 5, 5, 100, 6, 5}, "primary");
    auto* secondary = rig.addCurve({4, 500, 5, 600, 6, 700}, "secondary");
    secondary->setYAxis(&y2);
    QTRY_COMPARE(rig.plot.series().size(), 2);
    auto tool = SelectionTool{};
    tool.setPlot(&rig.plot);

    // A box is bounded on both axes, so it only selects series on exactly those axes.
    rig.drag(rig.pixel(3, 9), rig.pixel(7, 1));
    QCOMPARE(tool.model()->count(), 1);
    QCOMPARE(tool.model()->get(0).value("series").value<PlotSeries*>(), primary);
    QCOMPARE(tool.model()->get(0).value("summaryCount").toInt(), 2);
    QCOMPARE(tool.indices(secondary).status, InspectionStatus::InvalidArgument);

    // The same regions can be set from code; reversed limits are swapped.
    auto changed = QSignalSpy{&tool, &SelectionTool::selectionChanged};
    QVERIFY(tool.select(6, 4.5, -kInfinity, kInfinity));
    QCOMPARE(changed.count(), 1);
    QCOMPARE(tool.xMin(), 4.5);
    QCOMPARE(tool.model()->count(), 2);
    QCOMPARE(tool.model()->get(0).value("summaryCount").toInt(), 2);
    QVERIFY(!tool.select(kNaN, 1, 0, 1));
    tool.clear();
    QVERIFY(!tool.hasSelection());
    QCOMPARE(tool.pixelRect(), QRectF{});

    // Limits refer to the selection axes, so changing them drops the selection.
    QVERIFY(tool.select(4, 6, 0, 10));
    tool.setYAxis(&y2);
    QVERIFY(!tool.hasSelection());
    QVERIFY(tool.select(4, 6, 0, 1000));
    QCOMPARE(tool.model()->get(0).value("series").value<PlotSeries*>(), secondary);
    secondary->setYAxis(&rig.y);
    QCOMPARE(tool.model()->count(), 0);
}

void TestSelectionTool::followsData()
{
    auto rig = PlotRig{0, 10, 0, 10};
    auto* curve = rig.addCurve({1, 1, 2, 2, 3, 3});
    QTRY_COMPARE(rig.plot.series().size(), 1);
    auto tool = SelectionTool{};
    tool.setPlot(&rig.plot);
    QVERIFY(tool.select(1.5, 8, -kInfinity, kInfinity));
    QCOMPARE(tool.model()->get(0).value("summaryCount").toInt(), 2);
    const auto revision = tool.indices(curve).dataRevision;

    // New data keeps the region and updates the statistics; pages report the revision they used.
    auto changed = QSignalSpy{&tool, &SelectionTool::selectionChanged};
    auto resets = QSignalSpy{tool.model(), &QAbstractItemModel::modelReset};
    curve->appendData(4, 9);
    QVERIFY(tool.hasSelection());
    QCOMPARE(changed.count(), 0);
    QCOMPARE(resets.count(), 0);
    QCOMPARE(tool.model()->get(0).value("summaryCount").toInt(), 3);
    QCOMPARE(tool.model()->get(0).value("maximum").toDouble(), 9.0);
    const auto page = tool.indices(curve);
    QVERIFY(page.dataRevision > revision);
    QCOMPARE(page.indices, (QList<int>{1, 2, 3}));
    curve->setData(std::vector<double>{2, 7}, 1);
    QCOMPARE(tool.model()->get(0).value("summaryCount").toInt(), 1);

    // Panning moves the overlay with the data.
    const auto before = tool.pixelRect();
    rig.x.setViewportMin(1);
    rig.x.setViewportMax(11);
    QVERIFY(tool.pixelRect().left() < before.left());

    // Series joining or leaving the plot update the rows.
    auto* second = rig.addCurve({2, 1, 3, 1, 4, 1});
    QTRY_COMPARE(tool.model()->count(), 2);
    QCOMPARE(tool.model()->get(1).value("summaryCount").toInt(), 3);
    delete second;
    QCOMPARE(tool.model()->count(), 1);
    curve->setVisible(false);
    QCOMPARE(tool.model()->count(), 0);
}

QTEST_MAIN(TestSelectionTool)
#include "tst_selection_tool.moc"
