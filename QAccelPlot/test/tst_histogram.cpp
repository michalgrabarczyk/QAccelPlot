//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/data/Histogram.hpp"
#include "QAccelPlot/series/BarSeries.hpp"

#include <QtTest/QtTest>

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <vector>

namespace QAccelPlot {

namespace {

constexpr auto kInf = std::numeric_limits<double>::infinity();
constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();

using Values = std::vector<double>;

Histogram histogramOf(const Values& samples, Values edges)
{
    return Histogram::fromSamples(samples.data(), samples.size(), std::move(edges));
}

// Sum of value × bin width over all bins.
double area(const Histogram& histogram)
{
    auto sum = 0.0;
    for (auto i = std::size_t{0}; i < histogram.values().size(); ++i) {
        sum += histogram.values()[i] * (histogram.edges()[i + 1] - histogram.edges()[i]);
    }
    return sum;
}

void expectWarning(const char* pattern)
{
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QString::fromLatin1(pattern)));
}

}

class HistogramTest : public QObject {
    Q_OBJECT

private slots:
    void defaultHistogramIsEmpty();
    void binsIncludeLowerEdgeAndLastBinIncludesUpperEdge();
    void nanAndOutOfRangeSamplesAreNotCounted();
    void binCountSpansTheFiniteSamples();
    void equalSamplesWidenTheRange();
    void noFiniteSamplesUseTheUnitRange();
    void explicitRangeSetsTheEdges();
    void uniformBinningAgreesWithTheEdges();
    void floatSamplesMatchDoubleSamples();
    void unevenEdgesAreNotUniform();
    void edgesFarFromZeroStayUniform();
    void invalidArgumentsGiveAnEmptyHistogram();
    void fromCountsWrapsBinnedData();
    void fromCountsRejectsInvalidData();
    void linearEdgesAreEquallySpaced();
    void logEdgesAreEquallySpacedInLog();
    void decadeEdgesFollowLogAxisTicks();
    void decadeEdgesKeepTheRangeEnds();
    void edgeBuildersRejectInvalidArguments();
    void densityDividesByTotalAndBinWidth();
    void densityAreaIsOne();
    void densityOfNoSamplesIsZero();
    void densityIsAppliedOnce();
    void barDataHoldsEdgesAndValues();
    void barListMatchesTheSeriesFormat();
    void dataFeedsABarSeries();
};

void HistogramTest::defaultHistogramIsEmpty()
{
    const auto histogram = Histogram{};
    QCOMPARE(histogram.binCount(), 0);
    QVERIFY(histogram.edges().empty());
    QVERIFY(histogram.values().empty());
    QCOMPARE(histogram.total(), 0.0);
    QVERIFY(!histogram.isUniform());
    QVERIFY(std::isnan(histogram.binWidth()));
    QVERIFY(histogram.barData().empty());
    QCOMPARE(histogram.density().binCount(), 0);
}

void HistogramTest::binsIncludeLowerEdgeAndLastBinIncludesUpperEdge()
{
    const auto histogram = histogramOf({2.0, 2.5, 3.0, 4.0, 6.0}, {2.0, 3.0, 4.0, 5.0, 6.0});
    QCOMPARE(histogram.binCount(), 4);
    QCOMPARE(histogram.values(), (Values{2.0, 1.0, 1.0, 1.0}));
    QCOMPARE(histogram.total(), 5.0);
}

void HistogramTest::nanAndOutOfRangeSamplesAreNotCounted()
{
    const auto samples = Values{kNaN, kInf, -kInf, -0.001, 0.75, 1.001};
    const auto histogram = Histogram::fromSamples(samples.data(), samples.size(), 2, 0.0, 1.0);
    QCOMPARE(histogram.values(), (Values{0.0, 1.0}));
    QCOMPARE(histogram.total(), 1.0);
}

void HistogramTest::binCountSpansTheFiniteSamples()
{
    const auto samples = Values{1.0, kNaN, 5.0, kInf, 3.0};
    const auto histogram = Histogram::fromSamples(samples.data(), samples.size(), 4);
    QCOMPARE(histogram.edges(), (Values{1.0, 2.0, 3.0, 4.0, 5.0}));
    QCOMPARE(histogram.values(), (Values{1.0, 0.0, 1.0, 1.0}));
    QVERIFY(histogram.isUniform());
    QCOMPARE(histogram.binWidth(), 1.0);
}

void HistogramTest::equalSamplesWidenTheRange()
{
    const auto samples = Values{7.0, 7.0, 7.0};
    const auto histogram = Histogram::fromSamples(samples.data(), samples.size(), 2);
    QCOMPARE(histogram.edges(), (Values{6.5, 7.0, 7.5}));
    QCOMPARE(histogram.values(), (Values{0.0, 3.0}));

    // Half a unit does not change a value this large.
    const auto large = Values{1e18, 1e18};
    const auto wide = Histogram::fromSamples(large.data(), large.size(), 2);
    QCOMPARE(wide.binCount(), 2);
    QCOMPARE(wide.total(), 2.0);
}

void HistogramTest::noFiniteSamplesUseTheUnitRange()
{
    const auto histogram = Histogram::fromSamples(static_cast<const double*>(nullptr), 0, 2);
    QCOMPARE(histogram.edges(), (Values{0.0, 0.5, 1.0}));
    QCOMPARE(histogram.values(), (Values{0.0, 0.0}));
    QCOMPARE(histogram.total(), 0.0);
}

void HistogramTest::explicitRangeSetsTheEdges()
{
    const auto samples = Values{0.5, 1.5, 1.5, 2.5, 2.5, 2.5, 4.0};
    const auto histogram = Histogram::fromSamples(samples.data(), samples.size(), 4, 0.0, 4.0);
    QCOMPARE(histogram.edges(), (Values{0.0, 1.0, 2.0, 3.0, 4.0}));
    QCOMPARE(histogram.values(), (Values{1.0, 2.0, 3.0, 1.0}));
}

void HistogramTest::uniformBinningAgreesWithTheEdges()
{
    // Tenths are not exact in binary, so the arithmetic bin of a sample on an edge can be off by one.
    const auto edges = Histogram::linearEdges(0.0, 1.0, 10);
    auto samples = Values{};
    for (const auto edge : edges) {
        samples.push_back(std::nextafter(edge, -kInf));
        samples.push_back(edge);
        samples.push_back(std::nextafter(edge, kInf));
    }
    auto generator = std::mt19937{7};
    auto distribution = std::uniform_real_distribution<double>{-0.1, 1.1};
    for (auto i = 0; i < 10000; ++i) {
        samples.push_back(distribution(generator));
    }

    auto expected = Values(10, 0.0);
    for (const auto sample : samples) {
        if (sample < edges.front() || sample > edges.back()) {
            continue;
        }
        const auto upper = std::upper_bound(edges.cbegin(), edges.cend(), sample);
        expected[std::min<std::size_t>(static_cast<std::size_t>(upper - edges.cbegin()) - 1, 9)] += 1.0;
    }

    const auto histogram = Histogram::fromSamples(samples.data(), samples.size(), 10, 0.0, 1.0);
    QVERIFY(histogram.isUniform());
    QCOMPARE(histogram.values(), expected);
}

void HistogramTest::floatSamplesMatchDoubleSamples()
{
    const auto doubles = Values{0.25, 0.5, 1.5, 2.75, 3.0, kNaN};
    const auto floats = std::vector<float>(doubles.cbegin(), doubles.cend());

    QCOMPARE(Histogram::fromSamples(floats.data(), floats.size(), 3).values(), Histogram::fromSamples(doubles.data(), doubles.size(), 3).values());
    QCOMPARE(Histogram::fromSamples(floats.data(), floats.size(), 3, 0.0, 3.0).values(),
        Histogram::fromSamples(doubles.data(), doubles.size(), 3, 0.0, 3.0).values());
    QCOMPARE(Histogram::fromSamples(floats.data(), floats.size(), Values{0.0, 0.5, 3.0}).values(), histogramOf(doubles, {0.0, 0.5, 3.0}).values());
}

void HistogramTest::unevenEdgesAreNotUniform()
{
    const auto histogram = histogramOf({0.5, 5.0, 50.0}, {0.0, 1.0, 10.0, 100.0});
    QVERIFY(!histogram.isUniform());
    QVERIFY(std::isnan(histogram.binWidth()));
    QCOMPARE(histogram.values(), (Values{1.0, 1.0, 1.0}));
}

void HistogramTest::edgesFarFromZeroStayUniform()
{
    // Epoch-second edges a tenth of a second apart are rounded to about 2.4e-7.
    auto edges = Values{};
    for (auto i = 0; i <= 5; ++i) {
        edges.push_back(1.7e9 + 0.1 * i);
    }
    const auto histogram = histogramOf({1.7e9 + 0.05, 1.7e9 + 0.25, 1.7e9 + 0.26}, edges);
    QVERIFY(histogram.isUniform());
    QVERIFY(std::abs(histogram.binWidth() - 0.1) < 1e-6);
    QCOMPARE(histogram.values(), (Values{1.0, 0.0, 2.0, 0.0, 0.0}));
}

void HistogramTest::invalidArgumentsGiveAnEmptyHistogram()
{
    const auto samples = Values{1.0, 2.0};

    expectWarning("Histogram bin count must be at least 1: 0");
    QCOMPARE(Histogram::fromSamples(samples.data(), samples.size(), 0).binCount(), 0);
    expectWarning("Histogram range must be finite and increasing: 2 to 1");
    QCOMPARE(Histogram::fromSamples(samples.data(), samples.size(), 4, 2.0, 1.0).binCount(), 0);
    expectWarning("Histogram range must be finite and increasing: 0 to inf");
    QCOMPARE(Histogram::fromSamples(samples.data(), samples.size(), 4, 0.0, kInf).binCount(), 0);
    expectWarning("Histogram received a null sample pointer for 2 samples");
    QCOMPARE(Histogram::fromSamples(static_cast<const double*>(nullptr), 2, 4).binCount(), 0);
    expectWarning("Histogram received a null sample pointer for 2 samples");
    QCOMPARE(Histogram::fromSamples(static_cast<const float*>(nullptr), 2, Values{0.0, 1.0}).binCount(), 0);

    for (const auto& edges : {Values{}, Values{1.0}, Values{0.0, 2.0, 1.0}, Values{0.0, 1.0, 1.0}, Values{0.0, kNaN, 2.0}, Values{0.0, kInf}}) {
        expectWarning("Histogram needs at least two finite, strictly increasing edges");
        QCOMPARE(histogramOf(samples, edges).binCount(), 0);
    }
}

void HistogramTest::fromCountsWrapsBinnedData()
{
    const auto histogram = Histogram::fromCounts({0.0, 1.0, 2.0}, {3.0, 5.0});
    QCOMPARE(histogram.binCount(), 2);
    QCOMPARE(histogram.values(), (Values{3.0, 5.0}));
    QCOMPARE(histogram.total(), 8.0);
    QVERIFY(histogram.isUniform());
    QCOMPARE(histogram.density().values(), (Values{0.375, 0.625}));
}

void HistogramTest::fromCountsRejectsInvalidData()
{
    expectWarning("Histogram received 1 counts for 2 bins");
    QCOMPARE(Histogram::fromCounts({0.0, 1.0, 2.0}, {3.0}).binCount(), 0);
    expectWarning("Histogram counts must be finite and non-negative");
    QCOMPARE(Histogram::fromCounts({0.0, 1.0, 2.0}, {3.0, -1.0}).binCount(), 0);
    expectWarning("Histogram counts must be finite and non-negative");
    QCOMPARE(Histogram::fromCounts({0.0, 1.0, 2.0}, {3.0, kNaN}).binCount(), 0);
    expectWarning("Histogram needs at least two finite, strictly increasing edges");
    QCOMPARE(Histogram::fromCounts({1.0, 0.0}, {3.0}).binCount(), 0);
}

void HistogramTest::linearEdgesAreEquallySpaced()
{
    QCOMPARE(Histogram::linearEdges(-1.0, 1.0, 4), (Values{-1.0, -0.5, 0.0, 0.5, 1.0}));

    const auto edges = Histogram::linearEdges(0.0, 0.7, 7);
    QCOMPARE(edges.size(), std::size_t{8});
    QCOMPARE(edges.front(), 0.0);
    QCOMPARE(edges.back(), 0.7);
}

void HistogramTest::logEdgesAreEquallySpacedInLog()
{
    const auto edges = Histogram::logEdges(0.01, 100.0, 4);
    QCOMPARE(edges.size(), std::size_t{5});
    QCOMPARE(edges.front(), 0.01);
    QCOMPARE(edges.back(), 100.0);
    for (auto i = std::size_t{1}; i < edges.size(); ++i) {
        QVERIFY(std::abs(edges[i] / edges[i - 1] - 10.0) < 1e-9);
    }
}

void HistogramTest::decadeEdgesFollowLogAxisTicks()
{
    QCOMPARE(
        Histogram::decadeEdges(1.0, 100.0), (Values{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0}));
    QCOMPARE(Histogram::decadeEdges(0.1, 1.0), (Values{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0}));

    const auto histogram = histogramOf({1.5, 9.5, 15.0, 95.0, 100.0}, Histogram::decadeEdges(1.0, 100.0));
    QCOMPARE(histogram.binCount(), 18);
    QVERIFY(!histogram.isUniform());
    QCOMPARE(histogram.values()[0], 1.0);
    QCOMPARE(histogram.values()[8], 1.0);
    QCOMPARE(histogram.values()[9], 1.0);
    QCOMPARE(histogram.values()[17], 2.0);
}

void HistogramTest::decadeEdgesKeepTheRangeEnds()
{
    QCOMPARE(Histogram::decadeEdges(1.5, 250.0),
        (Values{1.5, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0, 200.0, 250.0}));
    // An end on a tick does not add a second, nearly equal edge.
    QCOMPARE(Histogram::decadeEdges(0.3, 0.5), (Values{0.3, 0.4, 0.5}));
    QCOMPARE(Histogram::decadeEdges(3.0 * 0.1, 500.0 * 0.001), (Values{3.0 * 0.1, 0.4, 500.0 * 0.001}));
    // No tick lies inside the range.
    QCOMPARE(Histogram::decadeEdges(2.1, 2.9), (Values{2.1, 2.9}));
}

void HistogramTest::edgeBuildersRejectInvalidArguments()
{
    expectWarning("Histogram range must be finite and increasing: 10 to 1");
    QVERIFY(Histogram::decadeEdges(10.0, 1.0).empty());
    expectWarning("Histogram logarithmic edges need a positive minimum: -1");
    QVERIFY(Histogram::decadeEdges(-1.0, 10.0).empty());
    expectWarning("Histogram bin count must be at least 1: -3");
    QVERIFY(Histogram::linearEdges(0.0, 1.0, -3).empty());
    expectWarning("Histogram range must be finite and increasing: 1 to 1");
    QVERIFY(Histogram::linearEdges(1.0, 1.0, 4).empty());
    expectWarning("Histogram range must be finite and increasing: nan to 1");
    QVERIFY(Histogram::logEdges(kNaN, 1.0, 4).empty());
    expectWarning("Histogram logarithmic edges need a positive minimum: 0");
    QVERIFY(Histogram::logEdges(0.0, 1.0, 4).empty());
}

void HistogramTest::densityDividesByTotalAndBinWidth()
{
    const auto counts = histogramOf({0.5, 0.5, 2.0, 2.0}, {0.0, 1.0, 3.0});
    QCOMPARE(counts.values(), (Values{2.0, 2.0}));

    const auto density = counts.density();
    QCOMPARE(density.values(), (Values{0.5, 0.25}));
    QCOMPARE(density.edges(), counts.edges());
    QCOMPARE(density.total(), 4.0);
    // The source histogram keeps its counts.
    QCOMPARE(counts.values(), (Values{2.0, 2.0}));
}

void HistogramTest::densityAreaIsOne()
{
    auto generator = std::mt19937{11};
    auto distribution = std::lognormal_distribution<double>{0.0, 1.0};
    auto samples = Values(5000);
    std::generate(samples.begin(), samples.end(), [&] { return distribution(generator); });

    const auto uniform = Histogram::fromSamples(samples.data(), samples.size(), 40).density();
    QVERIFY(std::abs(area(uniform) - 1.0) < 1e-12);

    const auto uneven = histogramOf(samples, Histogram::logEdges(0.01, 100.0, 40)).density();
    QVERIFY(uneven.total() > 0.0);
    QVERIFY(std::abs(area(uneven) - 1.0) < 1e-12);
}

void HistogramTest::densityOfNoSamplesIsZero()
{
    const auto density = histogramOf({}, {0.0, 1.0, 2.0}).density();
    QCOMPARE(density.values(), (Values{0.0, 0.0}));
}

void HistogramTest::densityIsAppliedOnce()
{
    const auto density = histogramOf({0.5, 0.5, 2.0, 2.0}, {0.0, 1.0, 3.0}).density();
    QCOMPARE(density.density().values(), density.values());
}

void HistogramTest::barDataHoldsEdgesAndValues()
{
    const auto histogram = Histogram::fromCounts({0.0, 1.0, 4.0}, {3.0, 5.0});
    QCOMPARE(histogram.barData(), (Values{0.0, 1.0, 3.0, 1.0, 4.0, 5.0}));
}

void HistogramTest::barListMatchesTheSeriesFormat()
{
    const auto histogram = Histogram::fromCounts({0.0, 1.0, 4.0}, {3.0, 5.0});

    const auto bars = histogram.bars();
    QCOMPARE(bars.size(), 2);
    const auto bar = bars.at(1).toMap();
    QCOMPARE(bar.value(QStringLiteral("from")).toDouble(), 1.0);
    QCOMPARE(bar.value(QStringLiteral("to")).toDouble(), 4.0);
    QCOMPARE(bar.value(QStringLiteral("value")).toDouble(), 5.0);
}

void HistogramTest::dataFeedsABarSeries()
{
    const auto histogram = Histogram::fromCounts({0.0, 1.0, 4.0}, {3.0, 5.0});

    auto bars = BarSeries{};
    bars.setRangedData(histogram.barData(), histogram.binCount());
    QCOMPARE(bars.count(), 2);
    const auto bar = bars.barAt(1);
    QCOMPARE(bar.value(QStringLiteral("from")).toDouble(), 1.0);
    QCOMPARE(bar.value(QStringLiteral("to")).toDouble(), 4.0);
    QCOMPARE(bar.value(QStringLiteral("value")).toDouble(), 5.0);
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::HistogramTest)
#include "tst_histogram.moc"
