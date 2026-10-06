//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/HoverIndexBudget.hpp"

#include <QtTest/QtTest>

#include <chrono>

namespace QAccelPlot {

using std::chrono::nanoseconds;

class HoverIndexBudgetTest : public QObject {
    Q_OBJECT

private slots:
    void buildIsDueOnceScansCostAsMuch();
    void buildCostGrowsWithTheRecordCount();
    void resetForgetsTheScans();
    void untimeableScansStillLeadToABuild();
    void timedBuildReplacesTheAssumedCost();
    void buildsOfFewRecordsAreNotLearnedFrom();
    void timeScanReturnsTheResultAndCountsTheScan();
    void timeBuildRunsTheBuild();
};

void HoverIndexBudgetTest::buildIsDueOnceScansCostAsMuch()
{
    // Indexing 100 records is assumed to cost 1000 ns.
    auto budget = HoverIndexBudget{10.0};
    QVERIFY(!budget.buildDue(100));

    budget.addScan(nanoseconds{400});
    QVERIFY(!budget.buildDue(100));
    budget.addScan(nanoseconds{599});
    QVERIFY(!budget.buildDue(100));
    budget.addScan(nanoseconds{1});
    QVERIFY(budget.buildDue(100));
}

void HoverIndexBudgetTest::buildCostGrowsWithTheRecordCount()
{
    auto budget = HoverIndexBudget{10.0};
    budget.addScan(nanoseconds{1000});

    QVERIFY(budget.buildDue(100));
    QVERIFY(!budget.buildDue(101));
    // Nothing to index, so nothing to wait for.
    QVERIFY(HoverIndexBudget{10.0}.buildDue(0));
}

void HoverIndexBudgetTest::resetForgetsTheScans()
{
    auto budget = HoverIndexBudget{10.0};
    budget.addScan(nanoseconds{5000});
    QVERIFY(budget.buildDue(100));

    budget.reset();
    QVERIFY(!budget.buildDue(100));
}

void HoverIndexBudgetTest::untimeableScansStillLeadToABuild()
{
    auto budget = HoverIndexBudget{1.0};
    for (auto scan = 0; scan < 99; ++scan) {
        budget.addScan(nanoseconds{0});
    }
    QVERIFY(!budget.buildDue(100));
    budget.addScan(nanoseconds{0});
    QVERIFY(budget.buildDue(100));
}

void HoverIndexBudgetTest::timedBuildReplacesTheAssumedCost()
{
    auto budget = HoverIndexBudget{10.0};
    // 2 ns per record instead of the assumed 10.
    budget.addBuild(nanoseconds{200'000}, 100'000);
    budget.reset();

    budget.addScan(nanoseconds{199'999});
    QVERIFY(!budget.buildDue(100'000));
    budget.addScan(nanoseconds{1});
    QVERIFY(budget.buildDue(100'000));
}

void HoverIndexBudgetTest::buildsOfFewRecordsAreNotLearnedFrom()
{
    auto budget = HoverIndexBudget{10.0};
    // A fixed overhead of 50 µs on 10 records would suggest 5000 ns per record.
    budget.addBuild(nanoseconds{50'000}, 10);

    budget.addScan(nanoseconds{1000});
    QVERIFY(budget.buildDue(100));
}

void HoverIndexBudgetTest::timeScanReturnsTheResultAndCountsTheScan()
{
    auto budget = HoverIndexBudget{1.0};
    QVERIFY(!budget.buildDue(1));

    QCOMPARE(budget.timeScan([] { return 42; }), 42);
    QVERIFY(budget.buildDue(1));
}

void HoverIndexBudgetTest::timeBuildRunsTheBuild()
{
    auto budget = HoverIndexBudget{1.0};
    auto built = false;
    budget.timeBuild(100, [&built] { built = true; });
    QVERIFY(built);
}

} // namespace QAccelPlot

using QAccelPlot::HoverIndexBudgetTest;
QTEST_GUILESS_MAIN(HoverIndexBudgetTest)
#include "tst_hover_index_budget.moc"
