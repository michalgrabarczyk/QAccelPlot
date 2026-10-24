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
#include <QAccelPlot/series/HeatmapSeries.hpp>

#include <QQmlComponent>
#include <QQmlEngine>
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

// A shown window with three dense curves, an inspector, and a tooltip whose delegates count their creations.
// The inspector's cursor is set from code, so the real mouse cannot interfere.
struct TooltipScene {
    QQuickWindow window;
    InspectionPlot* plot{new InspectionPlot{}};
    Axis x;
    Axis y;
    PlotInspector inspector;
    QQmlEngine engine;
    std::unique_ptr<QObject> object;
    QQuickItem* tooltip{nullptr};
    QString error;

    TooltipScene()
    {
        window.resize(800, 400);
        plot->setParentItem(window.contentItem());
        plot->setSize({800, 400});
        x.setViewportMin(0);
        x.setViewportMax(10);
        y.setViewportMin(-2);
        y.setViewportMax(3);
        plot->setXAxis(&x);
        plot->setYAxis(&y);
        for (auto series = 0; series < 3; ++series) {
            auto* curve = new LineCurve(plot);
            curve->setName(QStringLiteral("S%1").arg(series));
            curve->setXAxis(&x);
            curve->setYAxis(&y);
            auto data = std::vector<double>{};
            for (auto i = 0; i < 2000; ++i) {
                data.push_back(i * 9.8 / 2000);
                data.push_back(std::sin(i * 0.01 + series));
            }
            curve->setData(std::move(data), 2000);
        }
        inspector.setPlot(plot);
        inspector.setFollowPointer(false);
        auto component = QQmlComponent{&engine};
        component.setData(R"(
            import QtQuick
            import QAccelPlot as QAccelPlot
            QAccelPlot.InspectionTooltip {
                id: tip
                property int created: 0
                rowDelegate: Text {
                    required property bool valid
                    required property string seriesName
                    required property string yText
                    visible: valid
                    text: seriesName + ": " + yText
                    Component.onCompleted: tip.created++
                }
            }
        )",
            QUrl{});
        object.reset(component.createWithInitialProperties({{"inspector", QVariant::fromValue(&inspector)}}));
        tooltip = qobject_cast<QQuickItem*>(object.get());
        error = component.errorString();
        window.show();
    }
};

} // namespace

class TestPlotInspector : public QObject {
    Q_OBJECT
private slots:
    void rows();
    void matching();
    void steppingFromHeldValues();
    void matchingByY();
    void summariesAndFilters();
    void notifications();
    void cursor();
    void pixelCursor();
    void snapsToClosest();
    void heatmapCells();
    void formatterMutation();
    void touch();
    void tooltipReusesDelegates();
    void tooltipPlacement();
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
    QCOMPARE(row.value("dataPointIndex").toInt(), 0);
    QCOMPARE(row.value("dataPointY").toDouble(), 3.14159);
    // Texts follow the pixel resolution instead of the tick step.
    QCOMPARE(row.value("yText").toString(), QStringLiteral("3.142"));
    QCOMPARE(row.value("xText").toString(), QStringLiteral("1.000"));
    QCOMPARE(row.value("pixelPosition").toPointF(), rig.pixel(1, 3.14159));
    QCOMPARE(resets.count(), 1);

    // Moving the cursor updates the row in place.
    auto changes = QSignalSpy{model, &QAbstractItemModel::dataChanged};
    rig.hover(rig.pixel(8, 5));
    inspector.refresh();
    QCOMPARE(model->get(0).value("dataPointIndex").toInt(), 1);
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
    QCOMPARE(beyond.value("dataPointStatus").value<InspectionStatus>(), InspectionStatus::NoMatch);
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
    QVERIFY(std::abs(between.value("dataPointX").toDouble() - 5.0) < 1e-9);
    QVERIFY(std::abs(between.value("dataPointY").toDouble() - 4.5) < 1e-9);
}

void TestPlotInspector::steppingFromHeldValues()
{
    auto rig = PlotRig{0, 10, 0, 10};
    auto* curve = rig.addCurve({2, 1, 4, 9, 6, 3});
    curve->setStepMode(StepMode::Before);
    QTRY_COMPARE(rig.plot.series().size(), 1);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    inspector.setMode(PlotInspector::NearestX);
    inspector.setInterpolate(true);
    const auto indexAfterStep = [&](const int steps) {
        inspector.setFollowPointer(true);
        rig.hover(rig.pixel(3, 5));
        inspector.refresh();
        inspector.stepCursor(steps);
        return inspector.model()->get(0).value("dataPointIndex").toInt();
    };

    // Between x 2 and 4 a Before step holds the value of the data point at 4, yet steps go to either neighbor.
    rig.hover(rig.pixel(3, 5));
    inspector.refresh();
    const auto held = inspector.model()->get(0);
    QVERIFY(held.value("interpolated").toBool());
    QCOMPARE(held.value("dataPointIndex").toInt(), 1);
    QCOMPARE(held.value("dataPointY").toDouble(), 9.0);
    QCOMPARE(indexAfterStep(-1), 0);
    QCOMPARE(indexAfterStep(1), 1);
    QCOMPARE(indexAfterStep(2), 2);
}

void TestPlotInspector::matchingByY()
{
    // A profile whose independent variable is Y.
    auto rig = PlotRig{0, 10, 0, 10};
    rig.addCurve({3, 1, 6, 9});
    QTRY_COMPARE(rig.plot.series().size(), 1);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    inspector.setMode(PlotInspector::NearestY);
    const auto rowAt = [&](const QPointF& point) {
        rig.hover(point);
        inspector.refresh();
        return inspector.model()->get(0);
    };

    // Between two samples a series matches whatever the horizontal distance; past its ends the radius decides.
    QCOMPARE(rowAt(rig.pixel(9, 2)).value("dataPointIndex").toInt(), 0);
    QCOMPARE(rowAt(rig.pixel(1, 8)).value("dataPointIndex").toInt(), 1);
    QVERIFY(rowAt(rig.pixel(5, 9.1)).value("valid").toBool());
    QVERIFY(!rowAt(rig.pixel(5, 9.9)).value("valid").toBool());

    // Interpolation reports the point on the line at the cursor's Y.
    inspector.setInterpolate(true);
    const auto between = rowAt(rig.pixel(9, 5));
    QVERIFY(between.value("interpolated").toBool());
    QVERIFY(std::abs(between.value("dataPointX").toDouble() - 4.5) < 1e-9);
    QVERIFY(std::abs(between.value("dataPointY").toDouble() - 5.0) < 1e-9);
    inspector.setInterpolate(false);

    // Snapping moves the crosshair's Y only, and summaries cover the band around the cursor.
    inspector.setSnapToDataPoint(true);
    inspector.setSummaries(true);
    inspector.setSummaryRadius(1000);
    const auto snapped = rowAt(rig.pixel(9, 2));
    QCOMPARE(inspector.crosshairPosition().x(), rig.pixel(9, 2).x());
    QCOMPARE(inspector.crosshairPosition().y(), rig.pixel(3, 1).y());
    QCOMPARE(snapped.value("summaryCount").toInt(), 2);

    // A cursor set from code with only a Y has no vertical line and matches by Y in every mode.
    inspector.setSnapToDataPoint(false);
    inspector.setMode(PlotInspector::NearestX);
    inspector.setFollowPointer(false);
    inspector.setCursorY(8);
    inspector.refresh();
    QVERIFY(inspector.active());
    QVERIFY(std::isnan(inspector.crosshairPosition().x()));
    QCOMPARE(inspector.crosshairPosition().y(), rig.pixel(0, 8).y());
    QCOMPARE(inspector.model()->get(0).value("dataPointIndex").toInt(), 1);
    QCOMPARE(inspector.cursorXText(), QString{});
    QCOMPARE(inspector.cursorYText(), QStringLiteral("8.000"));
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
    curve->setData(std::vector<double>{1, 8, 9, 2}, 2);
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
    auto position = QSignalSpy{&inspector, &PlotInspector::crosshairPositionChanged};
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

void TestPlotInspector::cursor()
{
    auto rig = PlotRig{0, 10, 0, 10};
    rig.addCurve({1, 1, 3, 2, 5, 9, 7, 4});
    QTRY_COMPARE(rig.plot.series().size(), 1);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);

    // Following the pointer publishes its data coordinates; a written cursor waits until following stops.
    rig.hover(rig.pixel(2.5, 6));
    inspector.refresh();
    QVERIFY(std::abs(inspector.cursorX() - 2.5) < 1e-9);
    QVERIFY(std::abs(inspector.cursorY() - 6.0) < 1e-9);
    QCOMPARE(inspector.cursorXText(), QStringLiteral("2.500"));
    inspector.setCursorX(8);
    QVERIFY(std::abs(inspector.cursorX() - 2.5) < 1e-9);
    rig.leave();
    inspector.refresh();
    QVERIFY(std::isnan(inspector.cursorX()));

    // A cursor set from code needs no pointer; without a Y it has no horizontal position.
    inspector.setFollowPointer(false);
    inspector.setCursorX(5.2);
    inspector.refresh();
    QVERIFY(inspector.active());
    QCOMPARE(inspector.crosshairPosition().x(), rig.pixel(5.2, 0).x());
    QVERIFY(std::isnan(inspector.crosshairPosition().y()));
    QCOMPARE(inspector.model()->get(0).value("dataPointIndex").toInt(), 2);
    QCOMPARE(inspector.cursorYText(), QString{});
    inspector.setCursorY(4);
    inspector.refresh();
    QCOMPARE(inspector.crosshairPosition(), rig.pixel(5.2, 4));

    // Stepping walks the records of the first matching series.
    inspector.stepCursor(1);
    QCOMPARE(inspector.model()->get(0).value("dataPointIndex").toInt(), 3);
    QVERIFY(std::abs(inspector.cursorX() - 7.0) < 1e-9);
    inspector.stepCursor(-2);
    QCOMPARE(inspector.model()->get(0).value("dataPointIndex").toInt(), 1);
    inspector.stepCursor(-10);
    QCOMPARE(inspector.model()->get(0).value("dataPointIndex").toInt(), 0);

    // Outside the viewport the cursor is inactive but kept.
    inspector.setCursorX(50);
    inspector.refresh();
    QVERIFY(!inspector.active());
    QCOMPARE(inspector.cursorX(), 50.0);

    // Stepping from a pointer-driven cursor stops following the pointer.
    inspector.setFollowPointer(true);
    rig.hover(rig.pixel(3, 2));
    inspector.refresh();
    inspector.stepCursor(1);
    QVERIFY(!inspector.followPointer());
    QCOMPARE(inspector.model()->get(0).value("dataPointIndex").toInt(), 2);
}

void TestPlotInspector::pixelCursor()
{
    auto rig = PlotRig{0, 10, 0, 10};
    rig.addCurve({1, 1, 3, 2, 5, 9, 7, 4});
    QTRY_COMPARE(rig.plot.series().size(), 1);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    const auto cursorPixel = [&inspector] { return QPointF{inspector.cursorPixelX(), inspector.cursorPixelY()}; };

    // While following the pointer, the cursor pixel is the pointer's.
    rig.hover(rig.pixel(3, 2));
    inspector.refresh();
    QCOMPARE(cursorPixel(), rig.pixel(3, 2));

    // A pixel cursor reports the data under it at once.
    inspector.setFollowPointer(false);
    const auto pixel = rig.pixel(5, 4);
    inspector.setCursorPixelX(pixel.x());
    inspector.setCursorPixelY(pixel.y());
    QCOMPARE(cursorPixel(), pixel);
    QVERIFY(std::abs(inspector.cursorX() - 5.0) < 1e-9);
    QVERIFY(std::abs(inspector.cursorY() - 4.0) < 1e-9);
    inspector.refresh();
    QCOMPARE(inspector.crosshairPosition(), pixel);
    QCOMPARE(inspector.model()->get(0).value("dataPointIndex").toInt(), 2);

    // Zooming keeps it on the same pixels and re-maps its data coordinates.
    auto cursor = QSignalSpy{&inspector, &PlotInspector::cursorChanged};
    rig.x.setViewportMax(14);
    QTRY_COMPARE(inspector.model()->get(0).value("dataPointIndex").toInt(), 3);
    QVERIFY(cursor.count() > 0);
    QCOMPARE(cursorPixel(), pixel);
    QCOMPARE(inspector.crosshairPosition(), pixel);
    QVERIFY(std::abs(inspector.cursorX() - 7.0) < 1e-9);
    QVERIFY(inspector.cursorXText().startsWith(QLatin1Char('7')));

    // Writing cursorX places X in data units again; Y stays on its pixel.
    inspector.setCursorX(9);
    QCOMPARE(inspector.cursorX(), 9.0);
    QCOMPARE(inspector.cursorPixelX(), rig.pixel(9, 0).x());
    rig.x.setViewportMin(4);
    rig.y.setViewportMax(20);
    QTRY_COMPARE(inspector.cursorPixelX(), rig.pixel(9, 0).x());
    QTRY_VERIFY(std::abs(inspector.cursorY() - 8.0) < 1e-9);
    QCOMPARE(inspector.cursorX(), 9.0);
    QCOMPARE(inspector.cursorPixelY(), pixel.y());

    // A NaN coordinate leaves the cursor without it.
    inspector.setCursorPixelY(qQNaN());
    inspector.refresh();
    QVERIFY(inspector.active());
    QVERIFY(std::isnan(inspector.cursorY()));
    QVERIFY(std::isnan(inspector.crosshairPosition().y()));
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
    inspector.setSnapToDataPoint(true);
    rig.hover(rig.pixel(5, 5));
    inspector.refresh();
    QCOMPARE(inspector.validCount(), 2);
    // The crosshair moves to the closest sample, not to the first registered series.
    QCOMPARE(inspector.crosshairPosition(), rig.pixel(5.2, 5));
    inspector.setMode(PlotInspector::NearestX);
    inspector.refresh();
    QCOMPARE(inspector.crosshairPosition().x(), rig.pixel(5.2, 5).x());
    QCOMPARE(inspector.crosshairPosition().y(), rig.pixel(5, 5).y());
}

void TestPlotInspector::heatmapCells()
{
    auto rig = PlotRig{0, 10, 0, 10};
    auto* curve = rig.addCurve({1, 1, 9, 9}, "curve");
    auto* heatmap = new HeatmapSeries(&rig.plot);
    heatmap->setName(QStringLiteral("heatmap"));
    heatmap->setXAxis(&rig.x);
    heatmap->setYAxis(&rig.y);
    // Columns span 0..4 and 4..8; rows span 0..5 and 5..10.
    heatmap->setXMax(8);
    heatmap->setYMax(10);
    heatmap->setValues({0.5, 1.25, 2, std::numeric_limits<qreal>::quiet_NaN()}, 2);
    QTRY_COMPARE(rig.plot.series().size(), 2);
    auto inspector = PlotInspector{};
    inspector.setPlot(&rig.plot);
    auto* model = inspector.model();
    const auto rowOf = [&](const PlotSeries* series) {
        for (auto row = 0; row < model->count(); ++row) {
            if (model->get(row).value("series").value<PlotSeries*>() == series) {
                return model->get(row);
            }
        }
        return QVariantMap{};
    };
    const auto cellAt = [&](const QPointF& point) {
        rig.hover(point);
        inspector.refresh();
        return rowOf(heatmap);
    };

    // The cell under the cursor matches in every mode, reported at its center.
    for (const auto mode : {PlotInspector::NearestX, PlotInspector::NearestY, PlotInspector::NearestXY}) {
        inspector.setMode(mode);
        const auto row = cellAt(rig.pixel(5, 3));
        QVERIFY(row.value("valid").toBool());
        QCOMPARE(row.value("seriesName").toString(), QStringLiteral("heatmap"));
        QCOMPARE(row.value("dataPointIndex").toInt(), 1);
        QCOMPARE(row.value("dataPointX").toDouble(), 6.0);
        QCOMPARE(row.value("dataPointY").toDouble(), 2.5);
        QCOMPARE(row.value("dataPointValue").toDouble(), 1.25);
        QCOMPARE(row.value("xText").toString(), QStringLiteral("6.000"));
        QCOMPARE(row.value("yText").toString(), QStringLiteral("2.500"));
        QCOMPARE(row.value("valueText").toString(), QStringLiteral("1.25"));
        QCOMPARE(row.value("pixelPosition").toPointF(), rig.pixel(6, 2.5));
    }
    // Rows without a value have no value text.
    QCOMPARE(rowOf(curve).value("valueText").toString(), QString{});
    // A cell with a non-finite value still matches; outside the grid nothing does.
    const auto invalidCell = cellAt(rig.pixel(5, 7));
    QCOMPARE(invalidCell.value("dataPointIndex").toInt(), 3);
    QCOMPARE(invalidCell.value("valueText").toString(), QString{});
    const auto outside = cellAt(rig.pixel(9, 3));
    QCOMPARE(outside.value("dataPointStatus").value<InspectionStatus>(), InspectionStatus::NoMatch);

    // Snapping moves both crosshair coordinates to the cell center in every mode; summaries do not apply to cells.
    inspector.setSnapToDataPoint(true);
    inspector.setSummaries(true);
    inspector.setExcludedSeries({curve});
    for (const auto mode : {PlotInspector::NearestX, PlotInspector::NearestY, PlotInspector::NearestXY}) {
        inspector.setMode(mode);
        cellAt(rig.pixel(5, 3));
        QCOMPARE(inspector.crosshairPosition(), rig.pixel(6, 2.5));
    }
    inspector.setExcludedSeries({});
    const auto snapped = cellAt(rig.pixel(5, 3));
    QVERIFY(snapped.value("valid").toBool());
    QVERIFY(snapped.value("summaryStatus").value<InspectionStatus>() != InspectionStatus::Ready);
    inspector.setSnapToDataPoint(false);
    inspector.setSummaries(false);

    // Data changes refresh the row.
    heatmap->setValues({0.5, 7, 2, 3}, 2);
    QTRY_COMPARE(rowOf(heatmap).value("dataPointValue").toDouble(), 7.0);

    // A cursor set from code matches cells only with both coordinates.
    inspector.setFollowPointer(false);
    inspector.setCursorX(5);
    inspector.refresh();
    QVERIFY(inspector.active());
    QVERIFY(!rowOf(heatmap).value("valid").toBool());
    QVERIFY(rowOf(curve).value("valid").toBool());
    inspector.setCursorY(3);
    inspector.refresh();
    QCOMPARE(rowOf(heatmap).value("dataPointIndex").toInt(), 1);

    // Stepping walks XY series only.
    inspector.setIncludedSeries({heatmap});
    inspector.refresh();
    inspector.stepCursor(1);
    QCOMPARE(inspector.cursorX(), 5.0);
    QCOMPARE(rowOf(heatmap).value("dataPointIndex").toInt(), 1);
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
        curve->setData(std::vector<double>{5, 8}, 1);
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
    QCOMPARE(inspector.model()->get(0).value("dataPointY").toDouble(), 8.0);
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
    QTRY_COMPARE(inspector.crosshairPosition().toPoint(), point);
    QVERIFY(inspector.active());
    QCOMPARE(inspector.validCount(), 1);

    // The mouse takes the cursor back as soon as it moves over the plot.
    QTest::mouseMove(&window, point + QPoint{40, 0});
    QTRY_COMPARE(inspector.crosshairPosition().toPoint(), point + QPoint(40, 0));
    QTest::mouseMove(&window, QPoint{-20, -20});
    QTRY_VERIFY(!inspector.active());
}

void TestPlotInspector::tooltipReusesDelegates()
{
    auto scene = TooltipScene{};
    QVERIFY2(scene.tooltip != nullptr, qPrintable(scene.error));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window));

    // Row positions are sampled where the scene graph reads them, with the GUI thread blocked.
    auto frames = 0;
    auto overlapping = 0;
    connect(
        &scene.window, &QQuickWindow::beforeSynchronizing, this,
        [&] {
            auto positions = QList<qreal>{};
            const auto rows = scene.tooltip->childItems().first()->childItems();
            for (const auto* row : rows) {
                if (row->isVisible() && row->height() > 0) {
                    positions.push_back(row->y());
                }
            }
            if (!scene.tooltip->isVisible() || positions.size() < 2) {
                return;
            }
            ++frames;
            overlapping += positions.at(0) == positions.at(1) ? 1 : 0;
        },
        Qt::DirectConnection);

    scene.inspector.setCursorX(5);
    QTRY_COMPARE(scene.inspector.validCount(), 3);
    QTRY_VERIFY(scene.tooltip->isVisible());
    QTest::qWait(50);
    QCOMPARE(scene.tooltip->property("created").toInt(), 3);
    for (auto i = 1; i <= 30; ++i) {
        scene.inspector.setCursorX(5 + i * 0.01);
        QTest::qWait(16);
    }
    // Delegates are reused and their rows never share a position in a rendered frame.
    QCOMPARE(scene.tooltip->property("created").toInt(), 3);
    QVERIFY(frames > 5);
    QCOMPARE(overlapping, 0);
}

void TestPlotInspector::tooltipPlacement()
{
    auto scene = TooltipScene{};
    QVERIFY2(scene.tooltip != nullptr, qPrintable(scene.error));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window));
    scene.inspector.setCursorX(5);
    QTRY_VERIFY(scene.tooltip->isVisible());
    QTRY_VERIFY(scene.tooltip->x() > scene.inspector.crosshairPosition().x());

    // Near the right edge the tooltip flips to the other side of the cursor instead of covering it.
    scene.inspector.setCursorX(9.7);
    QTRY_VERIFY(scene.tooltip->x() + scene.tooltip->width() < scene.inspector.crosshairPosition().x());

    // Beyond every series' last sample nothing matches and the tooltip hides.
    scene.inspector.setRadius(2);
    scene.inspector.setCursorX(9.97);
    QTRY_COMPARE(scene.inspector.validCount(), 0);
    QVERIFY(scene.inspector.active());
    QTRY_VERIFY(!scene.tooltip->isVisible());
}

QTEST_MAIN(TestPlotInspector)
#include "tst_plot_inspector.moc"
