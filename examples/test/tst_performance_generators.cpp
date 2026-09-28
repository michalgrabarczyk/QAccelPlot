//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "GalaxyGenerator.hpp"
#include "PlasmaGenerator.hpp"
#include "SineWaveGenerator.hpp"

#include <QtTest/QtTest>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

using namespace QAccelPlotExample;

class PerformanceGeneratorsTest : public QObject {
    Q_OBJECT

private slots:
    void sineLookupTracksSine();
    void sineSendsVertexCacheOnlyAfterCountChanges();
    void galaxyRotatesEachBand();
    void galaxyValuesFollowPoints();
    void plasmaTilesAreSquaresOnTheGrid_data();
    void plasmaTilesAreSquaresOnTheGrid();
    void plasmaTileSizeFollowsTheField();
    void plasmaMovesOverTime();
};

void PerformanceGeneratorsTest::sineLookupTracksSine()
{
    constexpr auto pointCount = 10'001;
    constexpr auto timeSeconds = 1.234;
    auto generator = SineWaveGenerator{};
    auto batch = SineWaveBatch{};
    generator.generate(batch, pointCount, timeSeconds);

    QCOMPARE(batch.pointCount, pointCount);
    QCOMPARE(batch.xy.size(), std::size_t{pointCount} * 2);

    const auto phase = SineWaveGenerator::kPhaseVelocity * timeSeconds;
    auto maximumError = 0.0;
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(pointCount); ++i) {
        const auto x = static_cast<double>(batch.xy[i * 2]);
        const auto expected = SineWaveGenerator::kAmplitude * std::sin(SineWaveGenerator::kAngularFrequency * x + phase);
        maximumError = std::max(maximumError, std::abs(batch.xy[i * 2 + 1] - expected));
    }
    QCOMPARE(batch.xy[batch.xy.size() - 2], SineWaveGenerator::kDomainWidth);
    QVERIFY2(maximumError < 0.002, qPrintable(QStringLiteral("Maximum sine lookup error was %1").arg(maximumError, 0, 'g', 8)));
}

void PerformanceGeneratorsTest::sineSendsVertexCacheOnlyAfterCountChanges()
{
    auto generator = SineWaveGenerator{};
    auto batch = SineWaveBatch{};
    generator.generate(batch, 100, 0.0);
    QVERIFY(!batch.vertexCache.empty());

    generator.generate(batch, 100, 1.0);
    QVERIFY(batch.vertexCache.empty());

    generator.generate(batch, 200, 1.0);
    QVERIFY(!batch.vertexCache.empty());
}

void PerformanceGeneratorsTest::galaxyRotatesEachBand()
{
    constexpr auto pointCount = 5000;
    constexpr auto timeSeconds = 7.25;
    auto generator = GalaxyGenerator{};
    auto batch = GalaxyBatch{};
    generator.generate(batch, pointCount, timeSeconds);

    QCOMPARE(batch.pointCount, pointCount);
    QCOMPARE(batch.xy.size(), std::size_t{pointCount} * 2);
    const auto& bandStarts = generator.bandStarts();
    QCOMPARE(bandStarts.front(), 0);
    QCOMPARE(bandStarts.back(), pointCount);

    const auto& base = generator.basePositions();
    for (auto band = 0; band < GalaxyGenerator::kBandCount; ++band) {
        const auto angle = GalaxyGenerator::bandAngleAt(band, timeSeconds);
        const auto cosine = static_cast<float>(std::cos(angle));
        const auto sine = static_cast<float>(std::sin(angle));
        for (auto i = bandStarts[static_cast<std::size_t>(band)]; i < bandStarts[static_cast<std::size_t>(band) + 1]; ++i) {
            const auto index = static_cast<std::size_t>(i) * 2;
            const auto x = base[index];
            const auto y = base[index + 1];
            QCOMPARE(batch.xy[index], cosine * x - sine * y);
            QCOMPARE(batch.xy[index + 1], sine * x + cosine * y);
        }
    }
}

void PerformanceGeneratorsTest::galaxyValuesFollowPoints()
{
    constexpr auto pointCount = 5000;
    auto generator = GalaxyGenerator{};
    auto batch = GalaxyBatch{};
    generator.generate(batch, pointCount, 3.0);

    QCOMPARE(batch.values.size(), std::size_t{pointCount});
    for (const auto value : batch.values) {
        QVERIFY(value >= 0.0f && value <= 1.0f);
    }

    // Values stay with their points while the galaxy turns.
    const auto values = batch.values;
    generator.generate(batch, pointCount, 9.0);
    QCOMPARE(batch.values, values);
}

void PerformanceGeneratorsTest::plasmaTilesAreSquaresOnTheGrid_data()
{
    QTest::addColumn<int>("rectangleCount");
    QTest::newRow("one") << 1;
    QTest::newRow("partial last row") << 1'003;
    QTest::newRow("hundreds of thousands") << 300'001;
}

void PerformanceGeneratorsTest::plasmaTilesAreSquaresOnTheGrid()
{
    QFETCH(int, rectangleCount);
    auto generator = PlasmaGenerator{};
    auto batch = PlasmaBatch{};
    generator.generate(batch, rectangleCount, 2.5);

    QCOMPARE(batch.rectangleCount, rectangleCount);
    QCOMPARE(batch.rects.size(), static_cast<std::size_t>(rectangleCount) * 4);
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(rectangleCount); ++i) {
        const auto* rect = batch.rects.data() + i * 4;
        const auto width = rect[2] - rect[0];
        QVERIFY(width > 0.0f);
        QVERIFY(std::abs(width - (rect[3] - rect[1])) <= width * 1e-3f);

        const auto centerX = (rect[0] + rect[2]) * 0.5f;
        const auto centerY = (rect[1] + rect[3]) * 0.5f;
        QVERIFY(centerX > 0.0f && centerX < PlasmaGenerator::kDomainWidth);
        QVERIFY(centerY > 0.0f && centerY < PlasmaGenerator::kDomainHeight);
    }
}

void PerformanceGeneratorsTest::plasmaTileSizeFollowsTheField()
{
    constexpr auto rectangleCount = 1000;
    constexpr auto timeSeconds = 4.2;
    auto generator = PlasmaGenerator{};
    auto batch = PlasmaBatch{};
    generator.generate(batch, rectangleCount, timeSeconds);

    // A tile in a clearly higher part of the field is larger.
    auto values = std::vector<float>(static_cast<std::size_t>(rectangleCount));
    for (auto i = std::size_t{0}; i < values.size(); ++i) {
        const auto* rect = batch.rects.data() + i * 4;
        values[i] = PlasmaGenerator::valueAt((rect[0] + rect[2]) * 0.5f, (rect[1] + rect[3]) * 0.5f, timeSeconds);
    }
    for (auto i = std::size_t{0}; i < values.size(); ++i) {
        for (auto j = std::size_t{0}; j < values.size(); ++j) {
            if (values[i] > values[j] + 0.01f) {
                QVERIFY(batch.rects[i * 4 + 2] - batch.rects[i * 4] > batch.rects[j * 4 + 2] - batch.rects[j * 4]);
            }
        }
    }
    QVERIFY(PlasmaGenerator::tileSide(1.0f, 1.0f) > 1.0f);
}

void PerformanceGeneratorsTest::plasmaMovesOverTime()
{
    constexpr auto rectangleCount = 1000;
    auto generator = PlasmaGenerator{};
    auto first = PlasmaBatch{};
    auto second = PlasmaBatch{};
    generator.generate(first, rectangleCount, 0.0);
    generator.generate(second, rectangleCount, 2.0);
    QVERIFY(first.rects != second.rects);
}

QTEST_GUILESS_MAIN(PerformanceGeneratorsTest)
#include "tst_performance_generators.moc"
