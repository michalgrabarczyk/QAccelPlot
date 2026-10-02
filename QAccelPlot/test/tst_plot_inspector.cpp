//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "InspectionPlotRig.hpp"

#include <QAccelPlot/inspection/PlotInspector.hpp>
#include <QAccelPlot/series/BarSeries.hpp>

#include <QQuickWindow>
#include <QSignalSpy>
#include <QtTest>

#include <cmath>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

using namespace QAccelPlot;
using namespace InspectionTest;

namespace {

// A formatter that runs arbitrary code the first time it formats a value.
class MutationFormatter : public TickLabelFormatter {
public:
    mutable std::function<void()> action;

protected:
    QString doFormat(const qreal value, const qreal /*step*/) const override
    {
        if (action) {
            std::exchange(action, {})();
        }
        return QString::number(value);
    }
};

} // namespace

class TestPlotInspector : public QObject {
    Q_OBJECT
private slots:
    void rows();
    void matching();
    void summariesAndFilters();
    void notifications();
    void snapsToClosest();
    void formatterMutation();
    void touch();
};

void TestPlotInspector::rows()
{
    auto rig = PlotRig{0, 10, 0, 10};
    auto* curve = rig.addCurve({1, 3.14159, 9, 6}, "curve");
    auto* bars = new BarSeries(&rig.plot);
    bars->setXAxis(&rig.x);
    bars->setYAxis(&rig.y);
    bars->setData(std::vector<double>{2, 7, 5, 4}, 2);
    QTRY_COMPARE(rig.plot.series().size(), 2);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    auto* model = inspector.model();
    auto resets = QSignalSpy{model, &QAbstractItemModel::modelReset};

    // Only series with XY samples get a row.
    rig.hover(rig.pixel(4, 5));
    inspector.refresh();
    QVERIFY(inspector.active());
    QCOMPARE(model->count(), 1);
    QCOMPARE(inspector.validCount(), 1);
    const auto row = model->get(0);
    QCOMPARE(row.value("series").value<PlotSeries*>(), curve);
    QCOMPARE(row.value("seriesName").toString(), QStringLiteral("curve"));
    QCOMPARE(row.value("seriesColor").value<QColor>(), curve->color());
    QVERIFY(row.value("valid").toBool());
    QCOMPARE(row.value("sampleIndex").toInt(), 0);
    QCOMPARE(row.value("sampleY").toDouble(), 3.14159);
    // Texts follow the pixel resolution instead of the tick step.
    QCOMPARE(row.value("yText").toString(), QStringLiteral("3.142"));
    QCOMPARE(row.value("xText").toString(), QStringLiteral("1.000"));
    QCOMPARE(row.value("pixelPosition").toPointF(), rig.pixel(1, 3.14159));
    QCOMPARE(resets.count(), 1);

    // Moving the cursor updates the row in place.
    auto changes = QSignalSpy{model, &QAbstractItemModel::dataChanged};
    rig.hover(rig.pixel(8, 5));
    inspector.refresh();
    QCOMPARE(model->get(0).value("sampleIndex").toInt(), 1);
    QCOMPARE(resets.count(), 1);
    QCOMPARE(changes.count(), 1);

    // Leaving the plot keeps the row, without a match; removing the series removes it at once.
    rig.leave();
    QTRY_VERIFY(!inspector.active());
    QCOMPARE(inspector.validCount(), 0);
    QCOMPARE(model->count(), 1);
    delete curve;
    QCOMPARE(model->count(), 0);
}

void TestPlotInspector::matching()
{
    auto rig = PlotRig{0, 10, 0, 10};
    rig.addCurve({1, 3, 9, 6});
    QTRY_COMPARE(rig.plot.series().size(), 1);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    const auto rowAt = [&](const QPointF& point) {
        rig.hover(point);
        inspector.refresh();
        return inspector.model()->get(0);
    };

    // Between two samples a series matches whatever the distance; past its ends the radius decides.
    QVERIFY(rowAt(rig.pixel(4, 5)).value("valid").toBool());
    QVERIFY(rowAt(rig.pixel(9.1, 5)).value("valid").toBool());
    const auto beyond = rowAt(rig.pixel(9.9, 5));
    QVERIFY(!beyond.value("valid").toBool());
    QCOMPARE(beyond.value("sampleStatus").value<InspectionStatus>(), InspectionStatus::NoMatch);
    QCOMPARE(inspector.validCount(), 0);

    // NearestXY always applies the radius.
    inspector.setMode(PlotInspector::NearestXY);
    QVERIFY(!rowAt(rig.pixel(4, 5)).value("valid").toBool());
    QCOMPARE(rowAt(rig.pixel(9, 6) + QPointF{3, 4}).value("distance").toDouble(), 5.0);
    inspector.setMode(PlotInspector::NearestX);

    // Interpolation reports the point on the line under the cursor.
    inspector.setInterpolate(true);
    const auto between = rowAt(rig.pixel(5, 5));
    QVERIFY(between.value("interpolated").toBool());
    QVERIFY(std::abs(between.value("sampleX").toDouble() - 5.0) < 1e-9);
    QVERIFY(std::abs(between.value("sampleY").toDouble() - 4.5) < 1e-9);
}

void TestPlotInspector::summariesAndFilters()
{
    auto rig = PlotRig{0, 10, 0, 10};
    auto* curve = rig.addCurve({1, 3, 9, 6});
    QTRY_COMPARE(rig.plot.series().size(), 1);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    auto* model = inspector.model();
    rig.hover(rig.pixel(5, 5));

    // Summaries cover the samples around the cursor and follow data changes.
    inspector.setSummaries(true);
    inspector.setSummaryRadius(1000);
    curve->setDataNoRange(std::vector<double>{1, 8, 9, 2}, 2);
    QTRY_COMPARE(model->get(0).value("summaryCount").toInt(), 2);
    const auto row = model->get(0);
    QVERIFY(row.value("hasSummary").toBool());
    QCOMPARE(row.value("maximum").toDouble(), 8.0);
    QCOMPARE(row.value("maximumText").toString(), QStringLiteral("8.000"));

    // Excluded and hidden series have no row.
    inspector.setExcludedSeries({curve});
    QTRY_COMPARE(model->count(), 0);
    inspector.setExcludedSeries({});
    QTRY_COMPARE(model->count(), 1);
    inspector.setIncludedSeries({curve});
    QTRY_COMPARE(inspector.validCount(), 1);
    curve->setVisible(false);
    QTRY_COMPARE(model->count(), 0);
    curve->setVisible(true);
    QTRY_COMPARE(inspector.validCount(), 1);
}

void TestPlotInspector::notifications()
{
    auto rig = PlotRig{0, 10, 0, 10};
    rig.addCurve({5, 5});
    QTRY_COMPARE(rig.plot.series().size(), 1);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    QTest::qWait(20);
    auto active = QSignalSpy{&inspector, &PlotInspector::activeChanged};
    auto position = QSignalSpy{&inspector, &PlotInspector::positionChanged};
    auto cursor = QSignalSpy{&inspector, &PlotInspector::cursorChanged};
    auto valid = QSignalSpy{&inspector, &PlotInspector::validCountChanged};
    auto radius = QSignalSpy{&inspector, &PlotInspector::radiusChanged};
    // Nothing observable changes while the pointer is outside the plot.
    for (auto i = 0; i < 5; ++i) {
        rig.x.setViewportMax(11 + i);
        QTest::qWait(5);
    }
    QCOMPARE(active.count() + position.count() + cursor.count() + valid.count(), 0);

    // Each property has its own signal and only fires on a change.
    inspector.setRadius(20);
    inspector.setRadius(20);
    inspector.setRadius(-1);
    QCOMPARE(radius.count(), 1);
    QCOMPARE(inspector.radius(), 20.0);

    rig.hover(rig.pixel(5, 5));
    QTRY_VERIFY(inspector.active());
    QCOMPARE(active.count(), 1);
    QCOMPARE(valid.count(), 1);
    const auto positions = position.count();
    rig.hover(rig.pixel(5, 5));
    inspector.refresh();
    QCOMPARE(position.count(), positions);

    // A disabled inspector goes inactive once and then does nothing.
    inspector.setEnabled(false);
    QTRY_VERIFY(!inspector.active());
    QCOMPARE(inspector.model()->count(), 0);
    QTest::qWait(20);
    auto refreshed = QSignalSpy{&inspector, &PlotInspector::refreshed};
    for (auto i = 0; i < 5; ++i) {
        rig.hover(rig.pixel(5, 5) + QPointF{static_cast<qreal>(i), 0});
        QTest::qWait(5);
    }
    QCOMPARE(refreshed.count(), 0);
    inspector.setEnabled(true);
    QTRY_VERIFY(inspector.active());
}

void TestPlotInspector::snapsToClosest()
{
    auto rig = PlotRig{0, 10, 0, 10};
    rig.addCurve({2, 2}, "far");
    rig.addCurve({5.2, 5}, "near");
    QTRY_COMPARE(rig.plot.series().size(), 2);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    inspector.setMode(PlotInspector::NearestXY);
    inspector.setRadius(1000);
    inspector.setSnapToSample(true);
    rig.hover(rig.pixel(5, 5));
    inspector.refresh();
    QCOMPARE(inspector.validCount(), 2);
    // The crosshair moves to the closest sample, not to the first registered series.
    QCOMPARE(inspector.position(), rig.pixel(5.2, 5));
    inspector.setMode(PlotInspector::NearestX);
    inspector.refresh();
    QCOMPARE(inspector.position().x(), rig.pixel(5.2, 5).x());
    QCOMPARE(inspector.position().y(), rig.pixel(5, 5).y());
}

void TestPlotInspector::formatterMutation()
{
    auto rig = PlotRig{0, 10, 0, 10};
    auto* curve = rig.addCurve({5, 3}, "curve");
    auto* victim = rig.addCurve({5, 3}, "victim");
    QTRY_COMPARE(rig.plot.series().size(), 2);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    auto formatter = MutationFormatter{};
    rig.x.ticker()->setTickLabelFormatter(&formatter);
    QTest::qWait(20);
    // The formatter changes the data, deletes a series, and re-enters the inspector mid-refresh.
    formatter.action = [&] {
        curve->setDataNoRange(std::vector<double>{5, 8}, 1);
        delete victim;
        inspector.refresh();
    };
    rig.hover(rig.pixel(5, 5));
    inspector.refresh();
    // The interrupted refresh publishes nothing; in particular no row of the deleted series.
    QVERIFY(!inspector.active());
    QCOMPARE(inspector.model()->count(), 1);
    QTRY_COMPARE(inspector.validCount(), 1);
    QCOMPARE(inspector.model()->get(0).value("series").value<PlotSeries*>(), curve);
    QCOMPARE(inspector.model()->get(0).value("sampleY").toDouble(), 8.0);
    rig.x.ticker()->setTickLabelFormatter(nullptr);
}

void TestPlotInspector::touch()
{
    auto window = QQuickWindow{};
    window.resize(800, 400);
    auto* plot = new InspectionPlot{};
    plot->setParentItem(window.contentItem());
    plot->setSize({800, 400});
    auto x = Axis{};
    auto y = Axis{};
    x.setViewportMin(0);
    x.setViewportMax(10);
    y.setViewportMin(0);
    y.setViewportMax(10);
    plot->setXAxis(&x);
    plot->setYAxis(&y);
    auto* curve = new LineCurve(plot);
    curve->setXAxis(&x);
    curve->setYAxis(&y);
    curve->setData(std::vector<double>{1, 1, 9, 9}, 2);
    auto inspector = PlotInspector{};
    inspector.setPlot(plot);
    // The window stays hidden: synthetic events are still delivered, and the real mouse cannot interfere.
    const auto device = std::unique_ptr<QPointingDevice>{QTest::createTouchDevice()};
    const auto point = plot->plotRect().center().toPoint();
    QTest::mouseMove(&window, QPoint{-20, -20});

    // A tap inspects at the touched position and the cursor stays there after the finger lifts.
    QTest::touchEvent(&window, device.get()).press(0, point, &window);
    QTest::touchEvent(&window, device.get()).release(0, point, &window);
    QTRY_COMPARE(inspector.position().toPoint(), point);
    QVERIFY(inspector.active());
    QCOMPARE(inspector.validCount(), 1);

    // The mouse takes the cursor back as soon as it moves over the plot.
    QTest::mouseMove(&window, point + QPoint{40, 0});
    QTRY_COMPARE(inspector.position().toPoint(), point + QPoint(40, 0));
    QTest::mouseMove(&window, QPoint{-20, -20});
    QTRY_VERIFY(!inspector.active());
}

QTEST_MAIN(TestPlotInspector)
#include "tst_plot_inspector.moc"
