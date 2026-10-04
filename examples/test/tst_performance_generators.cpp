//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "DatasetParts.hpp"
#include "GalaxyGenerator.hpp"
#include "Ingestion.hpp"
#include "PlasmaGenerator.hpp"
#include "SineWaveGenerator.hpp"

#include <QtTest/QtTest>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <vector>

using namespace QAccelPlotExample;

namespace {

template <typename Parameters> Parameters parametersFor(const int count, const int seriesCount = 1)
{
    auto parameters = Parameters{};
    parameters.dataset.count = count;
    parameters.dataset.seriesCount = seriesCount;
    return parameters;
}

// Returns the float records of all parts of a batch generated at timeSeconds, in order.
template <typename Generator> std::vector<float> generatedFloats(const int count, const int seriesCount, const double timeSeconds)
{
    auto generator = Generator{};
    auto batch = typename Generator::Batch{};
    generator.generate(batch, parametersFor<typename Generator::Parameters>(count, seriesCount), timeSeconds);

    auto floats = std::vector<float>{};
    for (const auto& part : batch.parts) {
        floats.insert(floats.end(), part.floats.begin(), part.floats.end());
    }
    return floats;
}

} // namespace

class PerformanceGeneratorsTest : public QObject {
    Q_OBJECT

private slots:
    void recordsAreSplitEvenly();
    void splitDatasetsMatchTheUnsplitOnes();
    void doublePrecisionMovesRecordsToDoubles();
    void ingestionNamesSelectThePrecision();
    void sineLookupTracksSine();
    void sineSendsVertexCacheOnlyAfterCountChanges();
    void sineSendsNoVertexCacheWhileItIsOff();
    void sineFrequencyScalesTheWave();
    void sineGapsReplaceTheRequestedShare();
    void galaxyRotatesEachBand();
    void galaxyValuesFollowPoints();
    void galaxyValuesCanBeOff();
    void galaxyInvalidatesTheRequestedShare();
    void plasmaTilesAreSquaresOnTheGrid_data();
    void plasmaTilesAreSquaresOnTheGrid();
    void plasmaTileSizeFollowsTheField();
    void plasmaMovesOverTime();
    void plasmaTileScaleScalesEveryTile();
    void plasmaCategoriesFollowTheField();
};

void PerformanceGeneratorsTest::recordsAreSplitEvenly()
{
    QCOMPARE(partCount(10, 1, 0), 10);
    QCOMPARE(partCount(10, 3, 0), 4);
    QCOMPARE(partCount(10, 3, 1), 3);
    QCOMPARE(partCount(10, 3, 2), 3);
    // Fewer records than series leaves the last series empty.
    QCOMPARE(partCount(2, 3, 1), 1);
    QCOMPARE(partCount(2, 3, 2), 0);
}

void PerformanceGeneratorsTest::splitDatasetsMatchTheUnsplitOnes()
{
    constexpr auto count = 1'003;
    constexpr auto timeSeconds = 2.5;
    for (const auto seriesCount : {2, 7, 100}) {
        QCOMPARE(generatedFloats<SineWaveGenerator>(count, seriesCount, timeSeconds), generatedFloats<SineWaveGenerator>(count, 1, timeSeconds));
        QCOMPARE(generatedFloats<GalaxyGenerator>(count, seriesCount, timeSeconds), generatedFloats<GalaxyGenerator>(count, 1, timeSeconds));
        QCOMPARE(generatedFloats<PlasmaGenerator>(count, seriesCount, timeSeconds), generatedFloats<PlasmaGenerator>(count, 1, timeSeconds));
    }

    auto generator = GalaxyGenerator{};
    auto batch = GalaxyBatch{};
    generator.generate(batch, parametersFor<GalaxyParameters>(count, 7), timeSeconds);
    QCOMPARE(batch.parts.size(), std::size_t{7});
    for (auto index = 0; index < 7; ++index) {
        const auto& part = batch.parts[static_cast<std::size_t>(index)];
        QCOMPARE(part.count, partCount(count, 7, index));
        QCOMPARE(part.values.size(), static_cast<std::size_t>(part.count));
    }
}

void PerformanceGeneratorsTest::doublePrecisionMovesRecordsToDoubles()
{
    constexpr auto count = 500;
    const auto floats = generatedFloats<PlasmaGenerator>(count, 1, 1.0);

    auto parameters = parametersFor<PlasmaParameters>(count);
    parameters.dataset.doublePrecision = true;
    auto generator = PlasmaGenerator{};
    auto batch = PlasmaBatch{};
    generator.generate(batch, parameters, 1.0);

    const auto& part = batch.parts.front();
    QVERIFY(part.floats.empty());
    QCOMPARE(part.doubles, std::vector<double>(floats.begin(), floats.end()));
}

void PerformanceGeneratorsTest::ingestionNamesSelectThePrecision()
{
    QCOMPARE(ingestionFromName(QStringLiteral("floatNoRangeMove")), Ingestion::FloatNoRangeMove);
    QCOMPARE(ingestionFromName(QStringLiteral("floatMove")), Ingestion::FloatMove);
    QCOMPARE(ingestionFromName(QStringLiteral("floatNoRangeCopy")), Ingestion::FloatNoRangeCopy);
    QCOMPARE(ingestionFromName(QStringLiteral("doubleMove")), Ingestion::DoubleMove);
    QCOMPARE(ingestionFromName(QStringLiteral("floatPost")), Ingestion::FloatPost);
    QCOMPARE(ingestionFromName(QStringLiteral("unknown")), Ingestion::FloatNoRangeMove);

    auto parameters = parametersFor<PlasmaParameters>(10);
    auto generator = PlasmaGenerator{};
    auto batch = PlasmaBatch{};
    generator.generate(batch, parameters, 0.0);
    QVERIFY(hasPrecisionFor(batch.parts.front(), Ingestion::FloatMove));
    QVERIFY(!hasPrecisionFor(batch.parts.front(), Ingestion::DoubleMove));

    parameters.dataset.doublePrecision = true;
    generator.generate(batch, parameters, 0.0);
    QVERIFY(hasPrecisionFor(batch.parts.front(), Ingestion::DoubleMove));
    QVERIFY(!hasPrecisionFor(batch.parts.front(), Ingestion::FloatPost));

    // An empty part clears its series whatever the ingestion.
    QVERIFY(hasPrecisionFor(SeriesPart{}, Ingestion::DoubleMove));
}

void PerformanceGeneratorsTest::sineLookupTracksSine()
{
    constexpr auto pointCount = 10'001;
    constexpr auto timeSeconds = 1.234;
    auto generator = SineWaveGenerator{};
    auto batch = SineWaveBatch{};
    generator.generate(batch, parametersFor<SineWaveParameters>(pointCount), timeSeconds);

    const auto& part = batch.parts.front();
    QCOMPARE(part.count, pointCount);
    QCOMPARE(part.floats.size(), std::size_t{pointCount} * 2);

    const auto phase = SineWaveGenerator::kPhaseVelocity * timeSeconds;
    auto maximumError = 0.0;
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(pointCount); ++i) {
        const auto x = static_cast<double>(part.floats[i * 2]);
        const auto expected = SineWaveGenerator::kAmplitude * std::sin(SineWaveGenerator::kAngularFrequency * x + phase);
        maximumError = std::max(maximumError, std::abs(part.floats[i * 2 + 1] - expected));
    }
    QCOMPARE(part.floats[part.floats.size() - 2], SineWaveGenerator::kDomainWidth);
    QVERIFY2(maximumError < 0.002, qPrintable(QStringLiteral("Maximum sine lookup error was %1").arg(maximumError, 0, 'g', 8)));
}

void PerformanceGeneratorsTest::sineSendsVertexCacheOnlyAfterCountChanges()
{
    auto generator = SineWaveGenerator{};
    auto batch = SineWaveBatch{};
    generator.generate(batch, parametersFor<SineWaveParameters>(100), 0.0);
    QVERIFY(!batch.parts.front().vertexCache.empty());

    generator.generate(batch, parametersFor<SineWaveParameters>(100), 1.0);
    QVERIFY(batch.parts.front().vertexCache.empty());

    generator.generate(batch, parametersFor<SineWaveParameters>(200), 1.0);
    QVERIFY(!batch.parts.front().vertexCache.empty());

    // Splitting the same points across more curves changes every curve's point count.
    generator.generate(batch, parametersFor<SineWaveParameters>(200, 2), 1.0);
    QVERIFY(!batch.parts.front().vertexCache.empty());
    QVERIFY(!batch.parts.back().vertexCache.empty());
}

void PerformanceGeneratorsTest::sineSendsNoVertexCacheWhileItIsOff()
{
    auto generator = SineWaveGenerator{};
    auto batch = SineWaveBatch{};
    auto parameters = parametersFor<SineWaveParameters>(100);
    parameters.vertexCache = false;
    generator.generate(batch, parameters, 0.0);
    QVERIFY(batch.parts.front().vertexCache.empty());

    // Turning the caches back on sends one, although the count is unchanged.
    parameters.vertexCache = true;
    generator.generate(batch, parameters, 0.0);
    QVERIFY(!batch.parts.front().vertexCache.empty());
}

void PerformanceGeneratorsTest::sineFrequencyScalesTheWave()
{
    constexpr auto pointCount = 2'001;
    auto parameters = parametersFor<SineWaveParameters>(pointCount);
    parameters.frequencyScale = 4.0f;
    auto generator = SineWaveGenerator{};
    auto batch = SineWaveBatch{};
    generator.generate(batch, parameters, 0.0);

    const auto& xy = batch.parts.front().floats;
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(pointCount); ++i) {
        const auto expected = SineWaveGenerator::kAmplitude * std::sin(4.0 * SineWaveGenerator::kAngularFrequency * static_cast<double>(xy[i * 2]));
        QVERIFY(std::abs(xy[i * 2 + 1] - expected) < 0.01);
    }
}

void PerformanceGeneratorsTest::sineGapsReplaceTheRequestedShare()
{
    constexpr auto pointCount = 10'000;
    for (const auto seriesCount : {1, 7}) {
        auto parameters = parametersFor<SineWaveParameters>(pointCount, seriesCount);
        parameters.gapFraction = 0.1f;
        auto generator = SineWaveGenerator{};
        auto batch = SineWaveBatch{};
        generator.generate(batch, parameters, 0.0);

        auto gapSamples = 0;
        auto gapRuns = 0;
        auto inGap = false;
        for (const auto& part : batch.parts) {
            for (auto i = std::size_t{0}; i < static_cast<std::size_t>(part.count); ++i) {
                QVERIFY(std::isfinite(part.floats[i * 2]));
                const auto isGap = std::isnan(part.floats[i * 2 + 1]);
                gapSamples += isGap ? 1 : 0;
                gapRuns += isGap && !inGap ? 1 : 0;
                inGap = isGap;
            }
        }
        QCOMPARE(gapSamples, pointCount / 10);
        QCOMPARE(gapRuns, SineWaveGenerator::kGapCount);
    }
}

void PerformanceGeneratorsTest::galaxyRotatesEachBand()
{
    constexpr auto pointCount = 5000;
    constexpr auto timeSeconds = 7.25;
    auto generator = GalaxyGenerator{};
    auto batch = GalaxyBatch{};
    generator.generate(batch, parametersFor<GalaxyParameters>(pointCount), timeSeconds);

    const auto& part = batch.parts.front();
    QCOMPARE(part.count, pointCount);
    QCOMPARE(part.floats.size(), std::size_t{pointCount} * 2);
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
            QCOMPARE(part.floats[index], cosine * x - sine * y);
            QCOMPARE(part.floats[index + 1], sine * x + cosine * y);
        }
    }
}

void PerformanceGeneratorsTest::galaxyValuesFollowPoints()
{
    constexpr auto pointCount = 5000;
    auto generator = GalaxyGenerator{};
    auto batch = GalaxyBatch{};
    generator.generate(batch, parametersFor<GalaxyParameters>(pointCount), 3.0);

    QCOMPARE(batch.parts.front().values.size(), std::size_t{pointCount});
    for (const auto value : batch.parts.front().values) {
        QVERIFY(value >= 0.0f && value <= 1.0f);
    }

    // Values stay with their points while the galaxy turns.
    const auto values = batch.parts.front().values;
    generator.generate(batch, parametersFor<GalaxyParameters>(pointCount), 9.0);
    QCOMPARE(batch.parts.front().values, values);
}

void PerformanceGeneratorsTest::galaxyValuesCanBeOff()
{
    auto parameters = parametersFor<GalaxyParameters>(5000, 3);
    parameters.values = false;
    auto generator = GalaxyGenerator{};
    auto batch = GalaxyBatch{};
    generator.generate(batch, parameters, 1.0);

    for (const auto& part : batch.parts) {
        QVERIFY(part.values.empty());
        QCOMPARE(part.floats.size(), static_cast<std::size_t>(part.count) * 2);
    }
}

void PerformanceGeneratorsTest::galaxyInvalidatesTheRequestedShare()
{
    constexpr auto pointCount = 5000;
    for (const auto seriesCount : {1, 7}) {
        auto parameters = parametersFor<GalaxyParameters>(pointCount, seriesCount);
        parameters.invalidFraction = 0.1f;
        auto generator = GalaxyGenerator{};
        auto batch = GalaxyBatch{};
        generator.generate(batch, parameters, 1.0);

        auto invalidPoints = 0;
        for (const auto& part : batch.parts) {
            for (auto i = std::size_t{0}; i < static_cast<std::size_t>(part.count); ++i) {
                QVERIFY(std::isfinite(part.floats[i * 2 + 1]));
                invalidPoints += std::isnan(part.floats[i * 2]) ? 1 : 0;
            }
        }
        QCOMPARE(invalidPoints, pointCount / 10);
    }
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
    generator.generate(batch, parametersFor<PlasmaParameters>(rectangleCount), 2.5);

    const auto& rects = batch.parts.front().floats;
    QCOMPARE(batch.parts.front().count, rectangleCount);
    QCOMPARE(rects.size(), static_cast<std::size_t>(rectangleCount) * 4);
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(rectangleCount); ++i) {
        const auto* rect = rects.data() + i * 4;
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
    generator.generate(batch, parametersFor<PlasmaParameters>(rectangleCount), timeSeconds);

    // A tile in a clearly higher part of the field is larger.
    const auto& rects = batch.parts.front().floats;
    auto values = std::vector<float>(static_cast<std::size_t>(rectangleCount));
    for (auto i = std::size_t{0}; i < values.size(); ++i) {
        const auto* rect = rects.data() + i * 4;
        values[i] = PlasmaGenerator::valueAt((rect[0] + rect[2]) * 0.5f, (rect[1] + rect[3]) * 0.5f, timeSeconds);
    }
    for (auto i = std::size_t{0}; i < values.size(); ++i) {
        for (auto j = std::size_t{0}; j < values.size(); ++j) {
            if (values[i] > values[j] + 0.01f) {
                QVERIFY(rects[i * 4 + 2] - rects[i * 4] > rects[j * 4 + 2] - rects[j * 4]);
            }
        }
    }
    QVERIFY(PlasmaGenerator::tileSide(1.0f, 1.0f) > 1.0f);
}

void PerformanceGeneratorsTest::plasmaMovesOverTime()
{
    constexpr auto rectangleCount = 1000;
    QVERIFY(generatedFloats<PlasmaGenerator>(rectangleCount, 1, 0.0) != generatedFloats<PlasmaGenerator>(rectangleCount, 1, 2.0));
}

void PerformanceGeneratorsTest::plasmaTileScaleScalesEveryTile()
{
    constexpr auto rectangleCount = 1000;
    const auto unscaled = generatedFloats<PlasmaGenerator>(rectangleCount, 1, 1.5);

    auto parameters = parametersFor<PlasmaParameters>(rectangleCount);
    parameters.tileScale = 2.0f;
    auto generator = PlasmaGenerator{};
    auto batch = PlasmaBatch{};
    generator.generate(batch, parameters, 1.5);

    const auto& scaled = batch.parts.front().floats;
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(rectangleCount); ++i) {
        const auto width = unscaled[i * 4 + 2] - unscaled[i * 4];
        QVERIFY(std::abs((scaled[i * 4 + 2] - scaled[i * 4]) - 2.0f * width) <= width * 1e-3f);
        QVERIFY(std::abs((scaled[i * 4] + scaled[i * 4 + 2]) - (unscaled[i * 4] + unscaled[i * 4 + 2])) <= 1e-3f);
    }
}

void PerformanceGeneratorsTest::plasmaCategoriesFollowTheField()
{
    constexpr auto rectangleCount = 1000;
    constexpr auto categoryCount = 4;
    constexpr auto timeSeconds = 4.2;
    auto parameters = parametersFor<PlasmaParameters>(rectangleCount, 3);
    auto generator = PlasmaGenerator{};
    auto batch = PlasmaBatch{};
    generator.generate(batch, parameters, timeSeconds);
    QVERIFY(batch.parts.front().categories.empty());

    parameters.categoryCount = categoryCount;
    generator.generate(batch, parameters, timeSeconds);
    auto usedCategories = std::vector<bool>(categoryCount, false);
    for (const auto& part : batch.parts) {
        QCOMPARE(part.categories.size(), static_cast<std::size_t>(part.count));
        for (auto i = std::size_t{0}; i < part.categories.size(); ++i) {
            const auto* rect = part.floats.data() + i * 4;
            const auto value = PlasmaGenerator::valueAt((rect[0] + rect[2]) * 0.5f, (rect[1] + rect[3]) * 0.5f, timeSeconds);
            const auto category = part.categories[i];
            QVERIFY(category >= 0 && category < categoryCount);
            // The generator and valueAt() evaluate the field differently, so allow for rounding at a bin edge.
            QVERIFY(std::abs(value * categoryCount - (static_cast<float>(category) + 0.5f)) <= 0.51f);
            usedCategories[static_cast<std::size_t>(category)] = true;
        }
    }
    QVERIFY(std::all_of(usedCategories.begin(), usedCategories.end(), [](const bool used) { return used; }));
}

QTEST_GUILESS_MAIN(PerformanceGeneratorsTest)
#include "tst_performance_generators.moc"
