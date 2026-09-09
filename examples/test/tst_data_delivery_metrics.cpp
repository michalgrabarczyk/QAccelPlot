//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "DataDeliveryMetrics.hpp"

#include <QtTest/QtTest>

class DataDeliveryMetricsTest : public QObject {
    Q_OBJECT

private slots:
    void countsAppliedBatches();
    void recordsLongestApplicationGap();
    void snapshotsResetIntervalCounters();
};

void DataDeliveryMetricsTest::countsAppliedBatches()
{
    auto metrics = QAccelPlotExample::DataDeliveryMetrics{};
    metrics.dataApplied(100);
    metrics.dataApplied(200);

    const auto snapshot = metrics.takeSnapshot();
    QCOMPARE(snapshot.appliedBatches, std::uint64_t{2});
}

void DataDeliveryMetricsTest::recordsLongestApplicationGap()
{
    auto metrics = QAccelPlotExample::DataDeliveryMetrics{};
    metrics.dataApplied(100);
    metrics.dataApplied(125);
    metrics.dataApplied(190);

    QCOMPARE(metrics.takeSnapshot().longestDataGapNanoseconds, std::int64_t{65});
}

void DataDeliveryMetricsTest::snapshotsResetIntervalCounters()
{
    auto metrics = QAccelPlotExample::DataDeliveryMetrics{};
    metrics.dataApplied(100);
    (void)metrics.takeSnapshot();

    const auto snapshot = metrics.takeSnapshot();
    QCOMPARE(snapshot.appliedBatches, std::uint64_t{0});
    QCOMPARE(snapshot.longestDataGapNanoseconds, std::int64_t{0});
}

QTEST_MAIN(DataDeliveryMetricsTest)
#include "tst_data_delivery_metrics.moc"
