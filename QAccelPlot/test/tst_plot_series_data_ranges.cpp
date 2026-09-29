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
    using PlotSeries::PlotSeries;
    using PlotSeries::setDataRanges;

    void setData(const double*, int) override
    {
    }

    void setData(std::vector<double>&&, int) override
    {
    }

    void setDataNoRange(const double*, int) override
    {
    }

    void setDataNoRange(std::vector<double>&&, int) override
    {
    }

    void setDataF(const float*, int) override
    {
    }

    void setDataF(std::vector<float>&&, int) override
    {
    }

    void setDataFNoRange(const float*, int) override
    {
    }

    void setDataFNoRange(std::vector<float>&&, int) override
    {
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

    int scaleChangeCount{0};

protected:
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
    void nonFiniteXDoesNotBlockFiniteYUpdate();
    void nonFiniteYDoesNotBlockFiniteXUpdate();
    void replacingDestroyedLogAxisReportsScaleChange();
    void emptyPointCloudKeepsApplicationDataRange();
    void emptyRectangleSeriesKeepsApplicationDataRange();
    void emptyBarSeriesKeepsApplicationDataRange();
    void clearedEmptyLineCurveKeepsApplicationDataRange();
    void rescaleAfterEmptySeriesUsesApplicationDataRange();
    void clearingReportedRangeUpdatesAxis();
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

void PlotSeriesDataRangesTest::nonFiniteXDoesNotBlockFiniteYUpdate()
{
    auto series = TestPlotSeries{};
    auto xSpy = QSignalSpy{&series, &PlotSeries::xDataRangeChanged};
    auto ySpy = QSignalSpy{&series, &PlotSeries::yDataRangeChanged};

    series.setDataRanges(std::numeric_limits<qreal>::infinity(), std::numeric_limits<qreal>::infinity(), 1.0, 2.0);

    QCOMPARE(xSpy.count(), 0);
    QCOMPARE(ySpy.count(), 1);
}

void PlotSeriesDataRangesTest::nonFiniteYDoesNotBlockFiniteXUpdate()
{
    auto series = TestPlotSeries{};
    auto xSpy = QSignalSpy{&series, &PlotSeries::xDataRangeChanged};
    auto ySpy = QSignalSpy{&series, &PlotSeries::yDataRangeChanged};

    series.setDataRanges(1.0, 2.0, std::numeric_limits<qreal>::quiet_NaN(), std::numeric_limits<qreal>::quiet_NaN());

    QCOMPARE(xSpy.count(), 1);
    QCOMPARE(ySpy.count(), 0);
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

    cloud.setDataFNoRange({0.0F, 0.0F, 1.0F, 1.0F}, {}, 2);
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

} // namespace QAccelPlot

using QAccelPlot::PlotSeriesDataRangesTest;
QTEST_MAIN(PlotSeriesDataRangesTest)
#include "tst_plot_series_data_ranges.moc"
