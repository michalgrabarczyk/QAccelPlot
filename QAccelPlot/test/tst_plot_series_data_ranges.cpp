//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/BarSeries.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"
#include "QAccelPlot/series/PointCloud.hpp"
#include "QAccelPlot/series/RectangleSeries.hpp"

#include <QSignalSpy>
#include <QtTest/QtTest>

#include <limits>
#include <memory>
#include <vector>

namespace QAccelPlot {

namespace {

class TestPlotSeries final : public PlotSeries {
public:
    using PlotSeries::DataRanges;
    using PlotSeries::invalidateDataRanges;
    using PlotSeries::PlotSeries;
    using PlotSeries::setData;
    using PlotSeries::setDataF;

    void setData(const double*, int) override
    {
        invalidateDataRanges();
    }

    void setData(std::vector<double>&&, int) override
    {
        invalidateDataRanges();
    }

    void setDataF(const float*, int) override
    {
        invalidateDataRanges();
    }

    void setDataF(std::vector<float>&&, int) override
    {
        invalidateDataRanges();
    }

    void postData(std::vector<double>&&, int) override
    {
    }

    void postData(std::vector<float>&&, int) override
    {
    }

    void clearData() override
    {
    }

    // What a scan of the records finds, and how often they were scanned.
    DataRanges ranges;
    mutable int scanCount{0};
    int scaleChangeCount{0};

protected:
    DataRanges computeDataRanges() const override
    {
        ++scanCount;
        return ranges;
    }

    void onAxisScaleChanged() override
    {
        ++scaleChangeCount;
    }
};

struct AppRangeAxes {
    Axis x;
    Axis y;

    AppRangeAxes()
    {
        x.setDataMin(-5.0);
        x.setDataMax(5.0);
        y.setDataMin(-1.15);
        y.setDataMax(1.15);
    }

    void verifyAppRange() const
    {
        QCOMPARE(x.dataMin(), -5.0);
        QCOMPARE(x.dataMax(), 5.0);
        QCOMPARE(y.dataMin(), -1.15);
        QCOMPARE(y.dataMax(), 1.15);
    }
};

} // namespace

class PlotSeriesDataRangesTest : public QObject {
    Q_OBJECT

private slots:
    void commonDataApiDispatchesThroughBase();
    void nonFiniteXDoesNotBlockFiniteYRange();
    void nonFiniteYDoesNotBlockFiniteXRange();
    void dataRangesAreScannedOnlyWhenRead();
    void rescaleToDataScansOnce();
    void autoRescaleAxisScansOnEveryUpdate();
    void dataBoundsSkipTheScan();
    void staleDataRangeIsAnnouncedOncePerRead();
    void replacingDestroyedLogAxisReportsScaleChange();
    void emptyPointCloudKeepsApplicationDataRange();
    void emptyRectangleSeriesKeepsApplicationDataRange();
    void emptyBarSeriesKeepsApplicationDataRange();
    void clearedEmptyLineCurveKeepsApplicationDataRange();
    void rescaleAfterEmptySeriesUsesApplicationDataRange();
    void clearingReportedRangeUpdatesAxis();
    void autoRescaleFollowsEachAppendedPoint();
    void autoRescaleFollowsReplacedData();
    void autoRescaleKeepsViewportWhenLastSeriesClears();
    void autoRescaleFitsRemainingSeriesWhenOneClears();
};

void PlotSeriesDataRangesTest::commonDataApiDispatchesThroughBase()
{
    auto curve = LineCurve{};
    auto cloud = PointCloud{};
    auto rectangles = RectangleSeries{};
    auto curveAxis = Axis{};
    auto cloudAxis = Axis{};
    auto rectangleAxis = Axis{};
    curve.setXAxis(&curveAxis);
    cloud.setXAxis(&cloudAxis);
    rectangles.setXAxis(&rectangleAxis);

    auto* series = static_cast<PlotSeries*>(&curve);
    series->setDataF(std::vector<float>{1.0F, 2.0F, 3.0F, 4.0F}, 2);
    QCOMPARE(curveAxis.dataMax(), 3.0);

    series = &cloud;
    series->setDataF(std::vector<float>{5.0F, 6.0F}, 1);
    QCOMPARE(cloud.pointAt(0), QPointF(5.0, 6.0));

    series = &rectangles;
    series->setDataF(std::vector<float>{7.0F, 8.0F, 9.0F, 10.0F}, 1);
    QCOMPARE(rectangleAxis.dataMax(), 9.0);
    series->clearData();
    QCOMPARE(rectangles.count(), 0);

    auto bars = BarSeries{};
    auto barAxis = Axis{};
    bars.setYAxis(&barAxis);
    series = &bars;
    series->setDataF(std::vector<float>{0.0F, 11.0F}, 1);
    QCOMPARE(barAxis.dataMax(), 11.0);
    series->clearData();
    QCOMPARE(bars.count(), 0);
}

void PlotSeriesDataRangesTest::nonFiniteXDoesNotBlockFiniteYRange()
{
    auto series = TestPlotSeries{};
    const auto infinity = std::numeric_limits<qreal>::infinity();
    series.ranges = {PlotSeries::DataExtent{infinity, infinity}, PlotSeries::DataExtent{1.0, 2.0}};
    series.invalidateDataRanges();

    QVERIFY(!series.xDataRange());
    QVERIFY(series.yDataRange());
    QCOMPARE(series.yDataRange()->max, 2.0);
}

void PlotSeriesDataRangesTest::nonFiniteYDoesNotBlockFiniteXRange()
{
    auto series = TestPlotSeries{};
    const auto nan = std::numeric_limits<qreal>::quiet_NaN();
    series.ranges = {PlotSeries::DataExtent{1.0, 2.0}, PlotSeries::DataExtent{nan, nan}};
    series.invalidateDataRanges();

    QVERIFY(series.xDataRange());
    QCOMPARE(series.xDataRange()->min, 1.0);
    QVERIFY(!series.yDataRange());
}

void PlotSeriesDataRangesTest::dataRangesAreScannedOnlyWhenRead()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto series = TestPlotSeries{};
    series.setXAxis(&xAxis);
    series.setYAxis(&yAxis);
    series.ranges = {PlotSeries::DataExtent{1.0, 2.0}, PlotSeries::DataExtent{3.0, 4.0}};

    series.setDataF(std::vector<float>{}, 0);
    series.setDataF(std::vector<float>{}, 0);
    series.setDataF(std::vector<float>{}, 0);
    QCOMPARE(series.scanCount, 0);

    QCOMPARE(xAxis.dataMin(), 1.0);
    QCOMPARE(yAxis.dataMax(), 4.0);
    QCOMPARE(xAxis.dataMax(), 2.0);
    QCOMPARE(series.scanCount, 1);

    series.setDataF(std::vector<float>{}, 0);
    QCOMPARE(series.scanCount, 1);
    QVERIFY(series.xDataRange());
    QCOMPARE(series.scanCount, 2);
}

void PlotSeriesDataRangesTest::rescaleToDataScansOnce()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto series = TestPlotSeries{};
    series.setXAxis(&xAxis);
    series.setYAxis(&yAxis);
    series.ranges = {PlotSeries::DataExtent{1.0, 2.0}, PlotSeries::DataExtent{3.0, 4.0}};
    series.setDataF(std::vector<float>{}, 0);

    xAxis.rescaleToData();
    yAxis.rescaleToData();

    QCOMPARE(series.scanCount, 1);
    QCOMPARE(xAxis.viewportMin(), 1.0);
    QCOMPARE(xAxis.viewportMax(), 2.0);
    QCOMPARE(yAxis.viewportMin(), 3.0);
    QCOMPARE(yAxis.viewportMax(), 4.0);
}

void PlotSeriesDataRangesTest::autoRescaleAxisScansOnEveryUpdate()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    yAxis.setAutoRescale(true);
    auto series = TestPlotSeries{};
    series.setXAxis(&xAxis);
    series.setYAxis(&yAxis);
    const auto scansAfterBinding = series.scanCount;

    series.ranges = {PlotSeries::DataExtent{1.0, 2.0}, PlotSeries::DataExtent{3.0, 4.0}};
    series.setDataF(std::vector<float>{}, 0);
    QCOMPARE(series.scanCount, scansAfterBinding + 1);
    QCOMPARE(yAxis.viewportMax(), 4.0);

    series.ranges.y = PlotSeries::DataExtent{3.0, 9.0};
    series.setDataF(std::vector<float>{}, 0);
    QCOMPARE(series.scanCount, scansAfterBinding + 2);
    QCOMPARE(yAxis.viewportMax(), 9.0);
    // The axis without auto-rescale did not ask.
    QCOMPARE(xAxis.viewportMax(), 1.0);
}

void PlotSeriesDataRangesTest::dataBoundsSkipTheScan()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    yAxis.setAutoRescale(true);
    auto series = TestPlotSeries{};
    series.setXAxis(&xAxis);
    series.setYAxis(&yAxis);
    const auto scansAfterBinding = series.scanCount;
    series.ranges = {PlotSeries::DataExtent{1.0, 2.0}, PlotSeries::DataExtent{3.0, 4.0}};

    series.setDataF(std::vector<float>{}, 0, PlotSeries::DataBounds{-10.0, 10.0, -20.0, 20.0});

    QCOMPARE(yAxis.viewportMin(), -20.0);
    QCOMPARE(yAxis.viewportMax(), 20.0);
    QCOMPARE(xAxis.dataMin(), -10.0);
    QCOMPARE(xAxis.dataMax(), 10.0);
    QCOMPARE(series.scanCount, scansAfterBinding);

    // The bounds belong to that update only.
    series.setDataF(std::vector<float>{}, 0);
    QCOMPARE(yAxis.viewportMax(), 4.0);
    QCOMPARE(series.scanCount, scansAfterBinding + 1);
}

void PlotSeriesDataRangesTest::staleDataRangeIsAnnouncedOncePerRead()
{
    auto xAxis = Axis{};
    auto series = TestPlotSeries{};
    series.setXAxis(&xAxis);
    series.ranges = {PlotSeries::DataExtent{1.0, 2.0}, PlotSeries::DataExtent{3.0, 4.0}};
    QCOMPARE(xAxis.dataMax(), 1.0);
    auto maxSpy = QSignalSpy{&xAxis, &Axis::dataMaxChanged};

    series.setDataF(std::vector<float>{}, 0);
    series.setDataF(std::vector<float>{}, 0);
    QCOMPARE(maxSpy.count(), 1);
    QCOMPARE(series.scanCount, 0);

    // Reading the range emits nothing, so a binding to it does not loop.
    QCOMPARE(xAxis.dataMax(), 2.0);
    QCOMPARE(maxSpy.count(), 1);

    series.setDataF(std::vector<float>{}, 0);
    QCOMPARE(maxSpy.count(), 2);
}

void PlotSeriesDataRangesTest::replacingDestroyedLogAxisReportsScaleChange()
{
    auto series = TestPlotSeries{};
    auto logAxis = std::make_unique<Axis>();
    logAxis->setLogScale(true);
    series.setXAxis(logAxis.get());
    series.setYAxis(logAxis.get());
    QCOMPARE(series.scaleChangeCount, 2);

    logAxis.reset();
    auto linearAxis = Axis{};
    series.setXAxis(&linearAxis);
    series.setYAxis(&linearAxis);

    QCOMPARE(series.scaleChangeCount, 4);
}

void PlotSeriesDataRangesTest::emptyPointCloudKeepsApplicationDataRange()
{
    auto axes = AppRangeAxes{};
    auto cloud = PointCloud{};
    cloud.setXAxis(&axes.x);
    cloud.setYAxis(&axes.y);
    axes.verifyAppRange();
}

void PlotSeriesDataRangesTest::emptyRectangleSeriesKeepsApplicationDataRange()
{
    auto axes = AppRangeAxes{};
    auto rects = RectangleSeries{};
    rects.setXAxis(&axes.x);
    rects.setYAxis(&axes.y);
    rects.clearData();
    axes.verifyAppRange();
}

void PlotSeriesDataRangesTest::emptyBarSeriesKeepsApplicationDataRange()
{
    auto axes = AppRangeAxes{};
    auto bars = BarSeries{};
    bars.setXAxis(&axes.x);
    bars.setYAxis(&axes.y);
    bars.setBarWidth(2.0);
    bars.clearData();
    axes.verifyAppRange();
}

void PlotSeriesDataRangesTest::clearedEmptyLineCurveKeepsApplicationDataRange()
{
    auto axes = AppRangeAxes{};
    auto curve = LineCurve{};
    curve.setXAxis(&axes.x);
    curve.setYAxis(&axes.y);
    curve.clearData();
    axes.verifyAppRange();
}

void PlotSeriesDataRangesTest::rescaleAfterEmptySeriesUsesApplicationDataRange()
{
    auto axes = AppRangeAxes{};
    auto cloud = PointCloud{};
    cloud.setXAxis(&axes.x);
    cloud.setYAxis(&axes.y);

    axes.y.rescaleToData();

    QCOMPARE(axes.y.viewportMin(), -1.15);
    QCOMPARE(axes.y.viewportMax(), 1.15);
}

void PlotSeriesDataRangesTest::clearingReportedRangeUpdatesAxis()
{
    auto axes = AppRangeAxes{};
    auto cloud = PointCloud{};
    cloud.setXAxis(&axes.x);
    cloud.setYAxis(&axes.y);
    auto reporter = PointCloud{};
    reporter.setXAxis(&axes.x);
    reporter.setYAxis(&axes.y);

    cloud.setData(QList<QPointF>{{2.0, 3.0}, {4.0, 6.0}});
    reporter.setData(QList<QPointF>{{1.0, 2.0}});
    QCOMPARE(axes.x.dataMin(), 1.0);
    QCOMPARE(axes.x.dataMax(), 4.0);

    cloud.clearData();
    QCOMPARE(axes.x.dataMin(), 1.0);
    QCOMPARE(axes.x.dataMax(), 1.0);
    QCOMPARE(axes.y.dataMin(), 2.0);
    QCOMPARE(axes.y.dataMax(), 2.0);

    reporter.clearData();
    QCOMPARE(axes.x.dataMin(), 0.0);
    QCOMPARE(axes.x.dataMax(), 1.0);
}

void PlotSeriesDataRangesTest::autoRescaleFollowsEachAppendedPoint()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    xAxis.setAutoRescale(true);
    yAxis.setAutoRescale(true);
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);

    curve.appendData(10.0, 3.0);
    curve.appendData(11.0, 5.0);
    QCOMPARE(xAxis.viewportMin(), 10.0);
    QCOMPARE(xAxis.viewportMax(), 11.0);
    QCOMPARE(yAxis.viewportMin(), 3.0);
    QCOMPARE(yAxis.viewportMax(), 5.0);

    curve.appendData(12.0, -4.0);
    QCOMPARE(xAxis.viewportMin(), 10.0);
    QCOMPARE(xAxis.viewportMax(), 12.0);
    QCOMPARE(yAxis.viewportMin(), -4.0);
    QCOMPARE(yAxis.viewportMax(), 5.0);
}

void PlotSeriesDataRangesTest::autoRescaleFollowsReplacedData()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    yAxis.setAutoRescale(true);
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);

    curve.setDataF(std::vector<float>{0.0F, -8.0F, 1.0F, 8.0F}, 2);
    QCOMPARE(yAxis.viewportMin(), -8.0);
    QCOMPARE(yAxis.viewportMax(), 8.0);

    curve.setDataF(std::vector<float>{1.0F, 2.0F, 2.0F, 4.0F}, 2);
    QCOMPARE(yAxis.viewportMin(), 2.0);
    QCOMPARE(yAxis.viewportMax(), 4.0);
    QCOMPARE(xAxis.viewportMin(), 0.0);
    QCOMPARE(xAxis.viewportMax(), 1.0);
}

void PlotSeriesDataRangesTest::autoRescaleKeepsViewportWhenLastSeriesClears()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    yAxis.setAutoRescale(true);
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);
    curve.setData(QList<QPointF>{{0.0, 20.0}, {1.0, 30.0}});

    curve.clearData();
    QCOMPARE(yAxis.viewportMin(), 20.0);
    QCOMPARE(yAxis.viewportMax(), 30.0);

    // The cleared axis reports the 0..1 fallback range; new data matching it must still be fitted.
    curve.setData(QList<QPointF>{{0.0, 0.0}, {1.0, 1.0}});
    QCOMPARE(yAxis.viewportMin(), 0.0);
    QCOMPARE(yAxis.viewportMax(), 1.0);
}

void PlotSeriesDataRangesTest::autoRescaleFitsRemainingSeriesWhenOneClears()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    yAxis.setAutoRescale(true);
    auto wide = LineCurve{};
    wide.setXAxis(&xAxis);
    wide.setYAxis(&yAxis);
    auto narrow = LineCurve{};
    narrow.setXAxis(&xAxis);
    narrow.setYAxis(&yAxis);
    wide.setData(QList<QPointF>{{0.0, -50.0}, {1.0, 50.0}});
    narrow.setData(QList<QPointF>{{0.0, 1.0}, {1.0, 2.0}});
    QCOMPARE(yAxis.viewportMin(), -50.0);
    QCOMPARE(yAxis.viewportMax(), 50.0);

    wide.clearData();

    QCOMPARE(yAxis.viewportMin(), 1.0);
    QCOMPARE(yAxis.viewportMax(), 2.0);
}

} // namespace QAccelPlot

using QAccelPlot::PlotSeriesDataRangesTest;
QTEST_MAIN(PlotSeriesDataRangesTest)
#include "tst_plot_series_data_ranges.moc"
