//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "BenchmarkMetrics.hpp"
#include "BenchmarkWorkload.hpp"
#include "ExistingDataBufferPool.hpp"

#include <QtTest/QtTest>

#include <algorithm>
#include <cstddef>

class BenchmarkWorkloadTest : public QObject {
    Q_OBJECT

private slots:
    void sourceBufferCountIsBounded();
    void existingDataPoolHasPredictableMemory();
    void liveBuffersContainDifferentExistingData();
    void staticPanStaysInsideDataDomain();
    void frameTimeAndCadenceAreMeasuredSeparately();
};

void BenchmarkWorkloadTest::sourceBufferCountIsBounded()
{
    QCOMPARE(QAccelPlot::sourceBufferCount(QAccelPlot::BenchmarkScenario::UpdateMode::Static), 1);
    QCOMPARE(QAccelPlot::sourceBufferCount(QAccelPlot::BenchmarkScenario::UpdateMode::Live), 2);
    QCOMPARE(QAccelPlot::sourceBufferCount(QAccelPlot::BenchmarkScenario::UpdateMode::MaxRate), 2);
}

void BenchmarkWorkloadTest::existingDataPoolHasPredictableMemory()
{
    constexpr auto pointCount = 100;
    constexpr auto curveCount = 3;
    constexpr auto bufferCount = 2;
    const auto pool = QAccelPlot::ExistingDataBufferPool{pointCount, curveCount, bufferCount};
    const auto expectedBytes = static_cast<std::size_t>(pointCount) * curveCount * bufferCount * 2 * sizeof(float);

    QCOMPARE(pool.bufferCount(), bufferCount);
    QCOMPARE(pool.sourceBytes(), expectedBytes);
    QVERIFY(!pool.vertexCache().empty());
}

void BenchmarkWorkloadTest::liveBuffersContainDifferentExistingData()
{
    constexpr auto pointCount = 32;
    const auto pool = QAccelPlot::ExistingDataBufferPool{pointCount, 1, 2};
    const auto* first = pool.curveBufferData(0, 0);
    const auto* second = pool.curveBufferData(0, 1);

    QVERIFY(first != nullptr);
    QVERIFY(second != nullptr);
    QVERIFY(!std::equal(first, first + pointCount * 2, second));
}

void BenchmarkWorkloadTest::staticPanStaysInsideDataDomain()
{
    constexpr auto pointCount = 1'000;
    for (const auto elapsed : {std::int64_t{0}, std::int64_t{500}, std::int64_t{1000}, std::int64_t{1500}, std::int64_t{1999}}) {
        const auto viewport = QAccelPlot::staticPanViewport(pointCount, elapsed);
        QVERIFY(viewport.minimum >= 0.0);
        QVERIFY(viewport.maximum <= static_cast<double>(pointCount));
        QCOMPARE(viewport.maximum - viewport.minimum, 700.0);
    }
}

void BenchmarkWorkloadTest::frameTimeAndCadenceAreMeasuredSeparately()
{
    auto metrics = QAccelPlot::BenchmarkMetrics{};
    metrics.setWarmupFrames(0);
    metrics.start();

    metrics.beginFrame();
    QTest::qSleep(2);
    metrics.endFrame(100);

    QTest::qSleep(10);
    metrics.beginFrame();
    QTest::qSleep(2);
    metrics.endFrame(100);

    const auto results = metrics.results();
    QCOMPARE(results.totalFrames, 2);
    QVERIFY(results.frameTimeP50Ms > 0.0);
    QVERIFY(results.frameIntervalP50Ms > results.frameTimeP50Ms);
    QVERIFY(metrics.rollingFps() > 0.0);
}

QTEST_GUILESS_MAIN(BenchmarkWorkloadTest)
#include "tst_benchmark_workload.moc"
