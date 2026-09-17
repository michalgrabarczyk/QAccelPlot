//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/PlotSeries.hpp"

#include <QSignalSpy>
#include <QtTest/QtTest>

#include <limits>

namespace QAccelPlot {

namespace {

class TestPlotSeries final : public PlotSeries {
public:
    using PlotSeries::PlotSeries;
    using PlotSeries::setDataRanges;
};

} // namespace

class PlotSeriesDataRangesTest : public QObject {
    Q_OBJECT

private slots:
    void nonFiniteXDoesNotBlockFiniteYUpdate();
    void nonFiniteYDoesNotBlockFiniteXUpdate();
};

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

} // namespace QAccelPlot

using QAccelPlot::PlotSeriesDataRangesTest;
QTEST_MAIN(PlotSeriesDataRangesTest)
#include "tst_plot_series_data_ranges.moc"
