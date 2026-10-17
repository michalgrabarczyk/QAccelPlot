//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/inspection/SeriesInspection.hpp"
#include "QAccelPlot/legend/LegendEntry.hpp"
#include "QAccelPlot/series/HeatmapSeries.hpp"

#include <QSignalSpy>
#include <QtTest/QtTest>

#include <cmath>
#include <limits>
#include <thread>
#include <vector>

namespace QAccelPlot {

namespace {

constexpr auto kNaN = std::numeric_limits<float>::quiet_NaN();

// A 3 × 2 grid: row 0 holds 1, 2, 3 and row 1 holds 4, 5, 6.
std::vector<float> sixValues()
{
    return {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
}

// Binds \a heatmap to axes that show x 0..10 and y 0..10 in a 100 × 100 plot area.
struct InspectableHeatmap {
    Axis xAxis;
    Axis yAxis;
    HeatmapSeries heatmap;

    InspectableHeatmap()
    {
        xAxis.setViewportMin(0.0);
        xAxis.setViewportMax(10.0);
        yAxis.setOrientation(Qt::Vertical);
        yAxis.setViewportMin(0.0);
        yAxis.setViewportMax(10.0);
        heatmap.setXAxis(&xAxis);
        heatmap.setYAxis(&yAxis);
        heatmap.setPlotRect({0, 0, 100, 100});
    }
};

} // namespace

class HeatmapSeriesDataTest : public QObject {
    Q_OBJECT

private slots:
    void defaultsRenderWithOwnColormap();
    void setValuesStoresShapeAndValues();
    void listValuesDeriveRows();
    void mismatchedShapesAreRejected();
    void plotSeriesSettersKeepColumns();
    void plotSeriesSettersRejectIncompleteRows();
    void dataBoundsReplaceTheEdgesUntilTheNextChange();
    void changingColumnsReshapesTheValues();
    void shapeSignalsSeeTheNewState();
    void boundSignalsSeeTheNewState();
    void postedValuesAreAppliedFromWorkerThread();
    void clearDataKeepsColumns();
    void unsetBoundsUseIndexSpace();
    void boundsAreReportedAsDataRanges();
    void logAxisReportsNoRange();
    void valueRangeSkipsNonFiniteValues();
    void valueRangeFollowsColormapBounds();
    void recordsDescribeCells();
    void recordAtPositionFindsTheCell();
    void nonFiniteCellsAreStillRecords();
    void infiniteEdgesHaveNoCells();
};

void HeatmapSeriesDataTest::defaultsRenderWithOwnColormap()
{
    auto heatmap = HeatmapSeries{};
    QCOMPARE(heatmap.columns(), 0);
    QCOMPARE(heatmap.rows(), 0);
    QVERIFY(std::isnan(heatmap.xMin()));
    QVERIFY(std::isnan(heatmap.yMax()));
    QVERIFY(heatmap.colormap());
    QCOMPARE(heatmap.colormap()->parent(), &heatmap);
    QCOMPARE(heatmap.colormap()->preset(), Colormap::Preset::Viridis);
    // A color bar is its key, so it has no legend entry.
    QVERIFY(!heatmap.legendEntry());

    heatmap.setColormap(nullptr);
    QVERIFY(!heatmap.colormap());
}

void HeatmapSeriesDataTest::setValuesStoresShapeAndValues()
{
    auto heatmap = HeatmapSeries{};
    auto columnsSpy = QSignalSpy{&heatmap, &HeatmapSeries::columnsChanged};
    auto rowsSpy = QSignalSpy{&heatmap, &HeatmapSeries::rowsChanged};
    const auto revision = heatmap.dataRevision();

    heatmap.setValues(sixValues(), 3, 2);
    QCOMPARE(heatmap.columns(), 3);
    QCOMPARE(heatmap.rows(), 2);
    QCOMPARE(columnsSpy.count(), 1);
    QCOMPARE(rowsSpy.count(), 1);
    QVERIFY(heatmap.dataRevision() > revision);
    QCOMPARE(heatmap.valueAt(0, 0), 1.0);
    QCOMPARE(heatmap.valueAt(0, 2), 3.0);
    QCOMPARE(heatmap.valueAt(1, 0), 4.0);
    QVERIFY(std::isnan(heatmap.valueAt(2, 0)));
    QVERIFY(std::isnan(heatmap.valueAt(0, 3)));
    QVERIFY(std::isnan(heatmap.valueAt(-1, 0)));

    heatmap.setValues(sixValues(), 3, 2);
    QCOMPARE(columnsSpy.count(), 1);
    QCOMPARE(rowsSpy.count(), 1);
}

void HeatmapSeriesDataTest::listValuesDeriveRows()
{
    auto heatmap = HeatmapSeries{};
    heatmap.setValues(QList<qreal>{1, 2, 3, 4, 5, 6}, 2);
    QCOMPARE(heatmap.columns(), 2);
    QCOMPARE(heatmap.rows(), 3);
    QCOMPARE(heatmap.valueAt(2, 1), 6.0);

    heatmap.setValues(QList<qreal>{}, 4);
    QCOMPARE(heatmap.columns(), 2);
    QCOMPARE(heatmap.rows(), 0);
}

void HeatmapSeriesDataTest::mismatchedShapesAreRejected()
{
    auto heatmap = HeatmapSeries{};
    heatmap.setValues(sixValues(), 3, 2);

    QTest::ignoreMessage(QtWarningMsg, "HeatmapSeries received 6 values for 4 x 2 cells");
    heatmap.setValues(sixValues(), 4, 2);
    QTest::ignoreMessage(QtWarningMsg, "HeatmapSeries columns and rows cannot be negative: -3 x -2");
    heatmap.setValues({}, -3, -2);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("HeatmapSeries grid of 65536 x 65536 cells exceeds"));
    heatmap.setValues({}, 65536, 65536);
    QTest::ignoreMessage(QtWarningMsg, "HeatmapSeries received 5 values, which is not a whole number of rows of 2 columns");
    heatmap.setValues(QList<qreal>{1, 2, 3, 4, 5}, 2);
    QTest::ignoreMessage(QtWarningMsg, "HeatmapSeries received 2 values, which is not a whole number of rows of 0 columns");
    heatmap.setValues(QList<qreal>{1, 2}, 0);

    QCOMPARE(heatmap.columns(), 3);
    QCOMPARE(heatmap.rows(), 2);
    QCOMPARE(heatmap.valueAt(1, 2), 6.0);
}

void HeatmapSeriesDataTest::plotSeriesSettersKeepColumns()
{
    auto heatmap = HeatmapSeries{};
    heatmap.setColumns(2);

    const auto doubles = std::vector<double>{1.0, 2.0, 3.0, 4.0};
    heatmap.setData(doubles.data(), 4);
    QCOMPARE(heatmap.rows(), 2);
    QCOMPARE(heatmap.valueAt(1, 1), 4.0);

    heatmap.setData(std::vector<double>{5.0, 6.0}, 2);
    QCOMPARE(heatmap.rows(), 1);
    QCOMPARE(heatmap.valueAt(0, 0), 5.0);

    const auto floats = std::vector<float>{7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f};
    heatmap.setDataF(floats.data(), 6);
    QCOMPARE(heatmap.rows(), 3);
    QCOMPARE(heatmap.valueAt(2, 1), 12.0);

    heatmap.setDataF(std::vector<float>{13.0f, 14.0f}, 2);
    QCOMPARE(heatmap.columns(), 2);
    QCOMPARE(heatmap.rows(), 1);
    QCOMPARE(heatmap.valueAt(0, 1), 14.0);
}

void HeatmapSeriesDataTest::plotSeriesSettersRejectIncompleteRows()
{
    auto heatmap = HeatmapSeries{};
    QTest::ignoreMessage(QtWarningMsg, "HeatmapSeries received 2 values, which is not a whole number of rows of 0 columns");
    heatmap.setDataF(std::vector<float>{1.0f, 2.0f}, 2);
    QCOMPARE(heatmap.rows(), 0);

    heatmap.setColumns(2);
    QTest::ignoreMessage(QtWarningMsg, "HeatmapSeries received 3 values, which is not a whole number of rows of 2 columns");
    heatmap.setDataF(std::vector<float>{1.0f, 2.0f, 3.0f}, 3);
    QTest::ignoreMessage(QtWarningMsg, "HeatmapSeries received 3 values for 2 cells; expected 2");
    heatmap.setDataF(std::vector<float>{1.0f, 2.0f, 3.0f}, 2);
    QCOMPARE(heatmap.rows(), 0);
}

void HeatmapSeriesDataTest::dataBoundsReplaceTheEdgesUntilTheNextChange()
{
    auto heatmap = HeatmapSeries{};
    heatmap.setColumns(2);
    const auto bounds = PlotSeries::DataBounds{-1.0, 1.0, -2.0, 2.0};

    heatmap.setDataF(std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f}, 4, bounds);
    QCOMPARE(heatmap.xDataRange()->min, -1.0);
    QCOMPARE(heatmap.xDataRange()->max, 1.0);
    QCOMPARE(heatmap.yDataRange()->max, 2.0);

    heatmap.setXMax(5.0);
    QCOMPARE(heatmap.xDataRange()->min, 0.0);
    QCOMPARE(heatmap.xDataRange()->max, 5.0);
    QCOMPARE(heatmap.yDataRange()->min, 0.0);
    QCOMPARE(heatmap.yDataRange()->max, 2.0);

    heatmap.setDataF(std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f}, 4, bounds);
    QCOMPARE(heatmap.xDataRange()->max, 1.0);
    heatmap.setColumns(4);
    QCOMPARE(heatmap.xDataRange()->max, 5.0);
    QCOMPARE(heatmap.yDataRange()->max, 1.0);
}

void HeatmapSeriesDataTest::changingColumnsReshapesTheValues()
{
    auto heatmap = HeatmapSeries{};
    heatmap.setValues(sixValues(), 3, 2);
    auto rowsSpy = QSignalSpy{&heatmap, &HeatmapSeries::rowsChanged};

    heatmap.setColumns(2);
    QCOMPARE(heatmap.rows(), 3);
    QCOMPARE(heatmap.valueAt(1, 0), 3.0);
    QCOMPARE(rowsSpy.count(), 1);

    // The incomplete last row of 4 columns is kept but not part of the grid.
    heatmap.setColumns(4);
    QCOMPARE(heatmap.rows(), 1);
    QVERIFY(std::isnan(heatmap.valueAt(1, 0)));
    QCOMPARE(heatmap.dataValueMin(), 1.0);
    QCOMPARE(heatmap.dataValueMax(), 4.0);

    heatmap.setColumns(-1);
    QCOMPARE(heatmap.columns(), 0);
    QCOMPARE(heatmap.rows(), 0);

    heatmap.setColumns(3);
    QCOMPARE(heatmap.rows(), 2);
    QCOMPARE(heatmap.valueAt(1, 2), 6.0);
}

void HeatmapSeriesDataTest::shapeSignalsSeeTheNewState()
{
    auto heatmap = HeatmapSeries{};
    heatmap.setValues(sixValues(), 3, 2);
    const auto revision = heatmap.dataRevision();
    auto handled = false;
    QObject::connect(&heatmap, &HeatmapSeries::rowsChanged, &heatmap, [&] {
        handled = true;
        QCOMPARE(heatmap.dataValueMin(), 10.0);
        QCOMPARE(heatmap.dataValueMax(), 18.0);
        QCOMPARE(heatmap.yDataRange()->max, 3.0);
        QVERIFY(heatmap.dataRevision() > revision);
    });

    heatmap.setValues({10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f, 18.0f}, 3, 3);
    QVERIFY(handled);
}

void HeatmapSeriesDataTest::boundSignalsSeeTheNewState()
{
    auto heatmap = HeatmapSeries{};
    heatmap.setValues(sixValues(), 3, 2);
    QCOMPARE(heatmap.xDataRange()->min, 0.0);
    const auto revision = heatmap.dataRevision();
    auto handled = false;
    QObject::connect(&heatmap, &HeatmapSeries::xMinChanged, &heatmap, [&] {
        handled = true;
        QCOMPARE(heatmap.xDataRange()->min, -4.0);
        QVERIFY(heatmap.dataRevision() > revision);
    });

    heatmap.setXMin(-4.0);
    QVERIFY(handled);
}

void HeatmapSeriesDataTest::postedValuesAreAppliedFromWorkerThread()
{
    auto heatmap = HeatmapSeries{};
    auto worker = std::thread{[&heatmap] { heatmap.postValues(sixValues(), 2, 3); }};
    worker.join();
    QCOMPARE(heatmap.rows(), 0);
    QTRY_COMPARE(heatmap.rows(), 3);
    QCOMPARE(heatmap.columns(), 2);
    QCOMPARE(heatmap.valueAt(2, 1), 6.0);
}

void HeatmapSeriesDataTest::clearDataKeepsColumns()
{
    auto heatmap = HeatmapSeries{};
    heatmap.setValues(sixValues(), 3, 2);
    heatmap.clearData();
    QCOMPARE(heatmap.columns(), 3);
    QCOMPARE(heatmap.rows(), 0);
    QVERIFY(!heatmap.xDataRange());
}

void HeatmapSeriesDataTest::unsetBoundsUseIndexSpace()
{
    auto heatmap = HeatmapSeries{};
    heatmap.setValues(sixValues(), 3, 2);
    QCOMPARE(heatmap.xDataRange()->min, 0.0);
    QCOMPARE(heatmap.xDataRange()->max, 3.0);
    QCOMPARE(heatmap.yDataRange()->min, 0.0);
    QCOMPARE(heatmap.yDataRange()->max, 2.0);

    heatmap.setXMax(30.0);
    QCOMPARE(heatmap.xDataRange()->min, 0.0);
    QCOMPARE(heatmap.xDataRange()->max, 30.0);
}

void HeatmapSeriesDataTest::boundsAreReportedAsDataRanges()
{
    auto heatmap = HeatmapSeries{};
    auto xMinSpy = QSignalSpy{&heatmap, &HeatmapSeries::xMinChanged};
    heatmap.setValues(sixValues(), 3, 2);
    heatmap.setXMin(10.0);
    heatmap.setXMax(-5.0);
    heatmap.setYMin(1.0);
    heatmap.setYMax(2.0);
    QCOMPARE(xMinSpy.count(), 1);
    heatmap.setXMin(10.0);
    QCOMPARE(xMinSpy.count(), 1);

    // A reversed extent flips the grid; the range is ordered.
    QCOMPARE(heatmap.xDataRange()->min, -5.0);
    QCOMPARE(heatmap.xDataRange()->max, 10.0);
    QCOMPARE(heatmap.yDataRange()->min, 1.0);
    QCOMPARE(heatmap.yDataRange()->max, 2.0);

    heatmap.setXMin(std::numeric_limits<qreal>::quiet_NaN());
    QCOMPARE(xMinSpy.count(), 2);
    QCOMPARE(heatmap.xDataRange()->min, -5.0);
    QCOMPARE(heatmap.xDataRange()->max, 0.0);
}

void HeatmapSeriesDataTest::logAxisReportsNoRange()
{
    auto rig = InspectableHeatmap{};
    rig.heatmap.setValues(sixValues(), 3, 2);
    QVERIFY(rig.heatmap.xDataRange());

    rig.xAxis.setViewportMin(1.0);
    rig.xAxis.setLogScale(true);
    QVERIFY(!rig.heatmap.xDataRange());
    QVERIFY(rig.heatmap.yDataRange());
    QCOMPARE(rig.heatmap.inspection()->recordAtPosition({5.0, 95.0}).status, InspectionStatus::NoMatch);

    rig.xAxis.setLogScale(false);
    QVERIFY(rig.heatmap.xDataRange());
}

void HeatmapSeriesDataTest::valueRangeSkipsNonFiniteValues()
{
    auto heatmap = HeatmapSeries{};
    auto rangeSpy = QSignalSpy{&heatmap, &ColormappedSeries::valueRangeChanged};
    heatmap.setValues({kNaN, -2.0f, std::numeric_limits<float>::infinity(), 7.0f}, 2, 2);
    QCOMPARE(heatmap.dataValueMin(), -2.0);
    QCOMPARE(heatmap.dataValueMax(), 7.0);
    QCOMPARE(rangeSpy.count(), 1);

    heatmap.setValues({kNaN, kNaN}, 2, 1);
    QCOMPARE(heatmap.dataValueMin(), 0.0);
    QCOMPARE(heatmap.dataValueMax(), 1.0);
}

void HeatmapSeriesDataTest::valueRangeFollowsColormapBounds()
{
    auto heatmap = HeatmapSeries{};
    heatmap.setValues(sixValues(), 3, 2);
    heatmap.colormap()->setMin(-10.0);
    QCOMPARE(heatmap.dataValueMin(), -10.0);
    QCOMPARE(heatmap.dataValueMax(), 6.0);

    auto shared = Colormap{};
    shared.setRange(0.0, 100.0);
    heatmap.setColormap(&shared);
    QCOMPARE(heatmap.dataValueMin(), 0.0);
    QCOMPARE(heatmap.dataValueMax(), 100.0);
}

void HeatmapSeriesDataTest::recordsDescribeCells()
{
    auto heatmap = HeatmapSeries{};
    heatmap.setValues(sixValues(), 3, 2);
    heatmap.setXMin(0.0);
    heatmap.setXMax(6.0);
    heatmap.setYMin(10.0);
    heatmap.setYMax(20.0);

    const auto record = heatmap.inspection()->recordAt(5);
    QCOMPARE(record.status, InspectionStatus::Ready);
    QCOMPARE(record.index, 5);
    QCOMPARE(record.dataRevision, heatmap.dataRevision());
    QCOMPARE(record.fields.value(QStringLiteral("row")).toInt(), 1);
    QCOMPARE(record.fields.value(QStringLiteral("column")).toInt(), 2);
    QCOMPARE(record.fields.value(QStringLiteral("x")).toDouble(), 5.0);
    QCOMPARE(record.fields.value(QStringLiteral("y")).toDouble(), 17.5);
    QCOMPARE(record.fields.value(QStringLiteral("value")).toDouble(), 6.0);

    QCOMPARE(heatmap.inspection()->recordAt(6).status, InspectionStatus::InvalidArgument);
    QCOMPARE(heatmap.inspection()->recordAt(-1).status, InspectionStatus::InvalidArgument);
}

void HeatmapSeriesDataTest::recordAtPositionFindsTheCell()
{
    auto rig = InspectableHeatmap{};
    // Cells of 2 × 5 data units starting at x = 2: columns span 2..4, 4..6, 6..8.
    rig.heatmap.setValues(sixValues(), 3, 2);
    rig.heatmap.setXMin(2.0);
    rig.heatmap.setXMax(8.0);
    rig.heatmap.setYMin(0.0);
    rig.heatmap.setYMax(10.0);

    // Pixel (50, 25) is data (5, 7.5): column 1, row 1.
    const auto record = rig.heatmap.inspection()->recordAtPosition({50.0, 25.0});
    QCOMPARE(record.status, InspectionStatus::Ready);
    QCOMPARE(record.index, 4);
    QCOMPARE(record.fields.value(QStringLiteral("value")).toDouble(), 5.0);

    // Data (2.5, 2.5): column 0, row 0.
    QCOMPARE(rig.heatmap.inspection()->recordAtPosition({25.0, 75.0}).index, 0);
    // Left of the grid and below it there is no cell.
    QCOMPARE(rig.heatmap.inspection()->recordAtPosition({10.0, 50.0}).status, InspectionStatus::NoMatch);
    QCOMPARE(rig.heatmap.inspection()->recordAtPosition({90.0, 50.0}).status, InspectionStatus::NoMatch);
}

void HeatmapSeriesDataTest::nonFiniteCellsAreStillRecords()
{
    auto rig = InspectableHeatmap{};
    rig.heatmap.setValues({kNaN, 1.0f}, 2, 1);
    rig.heatmap.setXMax(10.0);
    rig.heatmap.setYMax(10.0);

    const auto record = rig.heatmap.inspection()->recordAtPosition({25.0, 50.0});
    QCOMPARE(record.status, InspectionStatus::Ready);
    QCOMPARE(record.index, 0);
    QVERIFY(std::isnan(record.fields.value(QStringLiteral("value")).toDouble()));
}

void HeatmapSeriesDataTest::infiniteEdgesHaveNoCells()
{
    constexpr auto kInfinity = std::numeric_limits<qreal>::infinity();
    auto rig = InspectableHeatmap{};
    rig.heatmap.setValues(sixValues(), 3, 2);
    rig.heatmap.setXMax(6.0);
    rig.heatmap.setYMax(10.0);
    QCOMPARE(rig.heatmap.inspection()->recordAtPosition({5.0, 95.0}).status, InspectionStatus::Ready);

    rig.heatmap.setXMax(kInfinity);
    QCOMPARE(rig.heatmap.inspection()->recordAtPosition({5.0, 95.0}).status, InspectionStatus::NoMatch);
    QCOMPARE(rig.heatmap.inspection()->recordAt(0).status, InspectionStatus::Unavailable);
    QVERIFY(!rig.heatmap.xDataRange());

    rig.heatmap.setXMax(6.0);
    rig.heatmap.setYMin(-kInfinity);
    QCOMPARE(rig.heatmap.inspection()->recordAtPosition({5.0, 95.0}).status, InspectionStatus::NoMatch);
    QCOMPARE(rig.heatmap.inspection()->recordAt(0).status, InspectionStatus::Unavailable);
    QCOMPARE(rig.heatmap.inspection()->recordAt(6).status, InspectionStatus::InvalidArgument);

    rig.heatmap.setYMin(0.0);
    QCOMPARE(rig.heatmap.inspection()->recordAt(0).status, InspectionStatus::Ready);
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::HeatmapSeriesDataTest)

#include "tst_heatmap_series_data.moc"
