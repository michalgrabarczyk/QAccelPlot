//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "axis/Axis.hpp"
#include "series/LineCurve.hpp"
#include "series/LineCurveGapFilter.hpp"

#include <QtTest/QtTest>

#include <cmath>
#include <cstring>
#include <limits>
#include <vector>

namespace QAccelPlot {

class LineCurveGapsTest : public QObject {
    Q_OBJECT

private slots:
    // Gap filter helpers
    void countInvalidPointsHonorsLogScale();
    void findValidRunsSplitsAtInvalidSamples();
    void planRunSamplingKeepsSmallCurvesIntact();
    void planRunSamplingDistributesBudget();
    void sampledSourceIndexIncludesRunEndpoints();
    void compactValidPointsFloat();
    void compactValidPointsDouble();
    void shaderFiniteBitPredicateMatchesStdIsFinite_data();
    void shaderFiniteBitPredicateMatchesStdIsFinite();

    // Auto-ranging
    void rangesExcludeInvalidCoordinates();
    void nanIsIgnoredInLongBuffers();
    void doubleRangesExcludeInvalidCoordinates();
    void infinityDoesNotFreezeRanges();
    void allInvalidDataClearsRanges();
    void logScaleExcludesNonPositiveValues();
    void noRangeDataIsNotRescannedOnLogScaleChange();
    void appendDataExcludesInvalidCoordinates();
    void appendDataAfterNoRangeDataRescans();

    // gaps.nanMode and hit testing
    void nanGapModeDefaultsToBreakAndNotifies();
    void breakModeHitTestDoesNotBridgeGap();
    void connectModeHitTestBridgesGap();
    void connectModeBridgesDoubleData();
    void connectModeBridgesAppendedData();
    void markerHitTestSkipsInvalidSamples();
    void emptyChunksAreSkipped();
};

namespace {

constexpr auto kNaN = std::numeric_limits<float>::quiet_NaN();
constexpr auto kInf = std::numeric_limits<float>::infinity();

// Plot geometry used by hit tests: data X in [0, 10] maps to [0, 1000] px and
// data Y in [0, 10] maps to [100, 0] px.
struct HitTestFixture {
    Axis xAxis;
    Axis yAxis;
    LineCurve curve;

    HitTestFixture()
    {
        xAxis.setOrientation(Axis::Horizontal);
        yAxis.setOrientation(Axis::Vertical);
        xAxis.setViewportMin(0.0);
        xAxis.setViewportMax(10.0);
        yAxis.setViewportMin(0.0);
        yAxis.setViewportMax(10.0);
        curve.setXAxis(&xAxis);
        curve.setYAxis(&yAxis);
        curve.setSize(QSizeF{1000.0, 100.0});
    }

    bool hits(const QPointF& point) const
    {
        // QQuickItem::contains() is public; LineCurve narrows its override to protected.
        return static_cast<const QQuickItem&>(curve).contains(point);
    }
};

// Five samples on y = 5 (pixel 50) with a single-sample gap at x = 2.
std::vector<float> gappedLine()
{
    return {0.0f, 5.0f, 1.0f, 5.0f, 2.0f, kNaN, 3.0f, 5.0f, 4.0f, 5.0f};
}

bool shaderIsFiniteBits(const float value)
{
    // Mirrors isFiniteBits() in shaders/math_utils.glsl.
    auto bits = quint32{};
    std::memcpy(&bits, &value, sizeof(bits));
    return (bits & 0x7F800000u) != 0x7F800000u;
}

} // namespace

void LineCurveGapsTest::countInvalidPointsHonorsLogScale()
{
    const auto data = std::vector<float>{1.0f, 1.0f, kNaN, 1.0f, 2.0f, -1.0f, kInf, 3.0f, 4.0f, 0.0f};
    const auto view = CurveDataView{data.data(), nullptr};

    QCOMPARE(LineCurveGapFilter::countInvalidPoints(view, 5, false, false), 2);
    QCOMPARE(LineCurveGapFilter::countInvalidPoints(view, 5, false, true), 4);
    QVERIFY(LineCurveGapFilter::isValidPoint(view, 0, true, true));
    QVERIFY(!LineCurveGapFilter::isValidPoint(view, 2, false, true));
}

void LineCurveGapsTest::findValidRunsSplitsAtInvalidSamples()
{
    const auto data = std::vector<float>{kNaN, 0.0f, 1.0f, 1.0f, 2.0f, 2.0f, 3.0f, kNaN, 4.0f, 4.0f, kNaN, kNaN, 6.0f, 6.0f};
    const auto runs = LineCurveGapFilter::findValidRuns(CurveDataView{data.data(), nullptr}, 7, false, false);

    QCOMPARE(runs.size(), std::size_t{3});
    QCOMPARE(runs[0].start, 1);
    QCOMPARE(runs[0].count, 2);
    QCOMPARE(runs[1].start, 4);
    QCOMPARE(runs[1].count, 1);
    QCOMPARE(runs[2].start, 6);
    QCOMPARE(runs[2].count, 1);
    QVERIFY(LineCurveGapFilter::findValidRuns(CurveDataView{data.data(), nullptr}, 0, false, false).empty());
}

void LineCurveGapsTest::planRunSamplingKeepsSmallCurvesIntact()
{
    const auto runs = std::vector<SampleRun>{{0, 10}, {11, 1}, {13, 20}};
    const auto sampled = LineCurveGapFilter::planRunSampling(runs, 100);

    QVERIFY(sampled == (std::vector<int>{10, 0, 20}));
}

void LineCurveGapsTest::planRunSamplingDistributesBudget()
{
    const auto runs = std::vector<SampleRun>{{0, 1000}, {1001, 3000}, {4002, 3}};
    const auto sampled = LineCurveGapFilter::planRunSampling(runs, 400);

    QCOMPARE(sampled.size(), std::size_t{3});
    QCOMPARE(sampled[0], 99);
    QCOMPARE(sampled[1], 299);
    QCOMPARE(sampled[2], 2); // every drawable run keeps at least its endpoints
}

void LineCurveGapsTest::sampledSourceIndexIncludesRunEndpoints()
{
    const auto run = SampleRun{100, 51};
    QCOMPARE(LineCurveGapFilter::sampledSourceIndex(run, 0, 6), 100);
    QCOMPARE(LineCurveGapFilter::sampledSourceIndex(run, 5, 6), 150);
    QCOMPARE(LineCurveGapFilter::sampledSourceIndex(run, 3, 51), 103);
}

void LineCurveGapsTest::compactValidPointsFloat()
{
    const auto data = std::vector<float>{0.0f, 1.0f, 1.0f, kNaN, 2.0f, -3.0f, kInf, 4.0f};
    auto output = std::vector<float>{};

    QCOMPARE(LineCurveGapFilter::compactValidPoints(data, 4, false, false, output), 2);
    QVERIFY(output == (std::vector<float>{0.0f, 1.0f, 2.0f, -3.0f}));

    QCOMPARE(LineCurveGapFilter::compactValidPoints(data, 4, false, true, output), 1);
    QVERIFY(output == (std::vector<float>{0.0f, 1.0f}));
}

void LineCurveGapsTest::compactValidPointsDouble()
{
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    const auto data = std::vector<double>{1e12, 1.0, 1e12 + 1.0, nan, 1e12 + 2.0, 3.0};
    const auto render = std::vector<float>{0.0f, 1.0f, 1.0f, kNaN, 2.0f, 3.0f};
    auto output = std::vector<double>{};
    auto renderOutput = std::vector<float>{};

    QCOMPARE(LineCurveGapFilter::compactValidPoints(data, render, 3, false, false, output, renderOutput), 2);
    QVERIFY(output == (std::vector<double>{1e12, 1.0, 1e12 + 2.0, 3.0}));
    QVERIFY(renderOutput == (std::vector<float>{0.0f, 1.0f, 2.0f, 3.0f}));
}

void LineCurveGapsTest::shaderFiniteBitPredicateMatchesStdIsFinite_data()
{
    QTest::addColumn<float>("value");

    QTest::newRow("zero") << 0.0f;
    QTest::newRow("negative zero") << -0.0f;
    QTest::newRow("one") << 1.0f;
    QTest::newRow("max") << std::numeric_limits<float>::max();
    QTest::newRow("lowest") << std::numeric_limits<float>::lowest();
    QTest::newRow("denormal") << std::numeric_limits<float>::denorm_min();
    QTest::newRow("quiet nan") << std::numeric_limits<float>::quiet_NaN();
    QTest::newRow("signaling nan") << std::numeric_limits<float>::signaling_NaN();
    QTest::newRow("negative nan") << -std::numeric_limits<float>::quiet_NaN();
    QTest::newRow("+inf") << kInf;
    QTest::newRow("-inf") << -kInf;
}

void LineCurveGapsTest::shaderFiniteBitPredicateMatchesStdIsFinite()
{
    QFETCH(float, value);
    QCOMPARE(shaderIsFiniteBits(value), static_cast<bool>(std::isfinite(value)));
}

void LineCurveGapsTest::rangesExcludeInvalidCoordinates()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);

    // Each coordinate is judged on its own: (1, NaN) and (2, Inf) extend X only; (NaN, 100) extends Y only.
    curve.setDataF(std::vector<float>{0.0f, 1.0f, 1.0f, kNaN, 2.0f, kInf, kNaN, 100.0f, 3.0f, -2.0f}, 5);

    QCOMPARE(xAxis.dataMin(), 0.0);
    QCOMPARE(xAxis.dataMax(), 3.0);
    QCOMPARE(yAxis.dataMin(), -2.0);
    QCOMPARE(yAxis.dataMax(), 100.0);
}

void LineCurveGapsTest::nanIsIgnoredInLongBuffers()
{
    // Long enough for the vectorized fast range scan, with NaN first, last, and throughout.
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);
    constexpr auto pointCount = 4099;
    auto data = std::vector<float>(static_cast<std::size_t>(pointCount) * 2);
    for (auto i = 0; i < pointCount; ++i) {
        const auto invalid = i == 0 || i == pointCount - 1 || i % 7 == 3;
        data[static_cast<std::size_t>(i) * 2] = invalid ? kNaN : static_cast<float>(i);
        data[static_cast<std::size_t>(i) * 2 + 1] = invalid ? kNaN : static_cast<float>(i % 100) - 50.0f;
    }

    curve.setDataF(std::move(data), pointCount);

    QCOMPARE(xAxis.dataMin(), 1.0);
    QCOMPARE(xAxis.dataMax(), static_cast<qreal>(pointCount - 2));
    QCOMPARE(yAxis.dataMin(), -50.0);
    QCOMPARE(yAxis.dataMax(), 49.0);
}

void LineCurveGapsTest::doubleRangesExcludeInvalidCoordinates()
{
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    const auto inf = std::numeric_limits<double>::infinity();
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);

    curve.setData(std::vector<double>{1e12, 1e12 + 1.0, nan, 1e12 + 3.0}, std::vector<double>{4.0, nan, 100.0, -inf});

    QCOMPARE(xAxis.dataMin(), 1e12);
    QCOMPARE(xAxis.dataMax(), 1e12 + 3.0);
    QCOMPARE(yAxis.dataMin(), 4.0);
    QCOMPARE(yAxis.dataMax(), 100.0);
}

void LineCurveGapsTest::infinityDoesNotFreezeRanges()
{
    auto yAxis = Axis{};
    auto curve = LineCurve{};
    curve.setYAxis(&yAxis);

    curve.setDataF(std::vector<float>{0.0f, 1.0f, 1.0f, 2.0f}, 2);
    QCOMPARE(yAxis.dataMax(), 2.0);

    curve.setDataF(std::vector<float>{0.0f, 5.0f, 1.0f, kInf, 2.0f, 7.0f}, 3);
    QCOMPARE(yAxis.dataMin(), 5.0);
    QCOMPARE(yAxis.dataMax(), 7.0);
}

void LineCurveGapsTest::allInvalidDataClearsRanges()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);
    curve.setDataF(std::vector<float>{10.0f, 20.0f, 11.0f, 21.0f}, 2);
    auto xRangeSpy = QSignalSpy{&curve, &LineCurve::xDataRangeChanged};
    auto yRangeSpy = QSignalSpy{&curve, &LineCurve::yDataRangeChanged};

    curve.setDataF(std::vector<float>{kNaN, kNaN, kInf, -kInf}, 2);

    QCOMPARE(xRangeSpy.count(), 0);
    QCOMPARE(yRangeSpy.count(), 0);
    QCOMPARE(xAxis.dataMin(), 0.0);
    QCOMPARE(xAxis.dataMax(), 1.0);
    QCOMPARE(yAxis.dataMin(), 0.0);
    QCOMPARE(yAxis.dataMax(), 1.0);
}

void LineCurveGapsTest::logScaleExcludesNonPositiveValues()
{
    auto yAxis = Axis{};
    yAxis.setLogScale(true);
    auto curve = LineCurve{};
    curve.setYAxis(&yAxis);

    curve.setDataF(std::vector<float>{0.0f, -1.0f, 1.0f, 0.0f, 2.0f, 10.0f, 3.0f, 100.0f}, 4);
    QCOMPARE(yAxis.dataMin(), 10.0);
    QCOMPARE(yAxis.dataMax(), 100.0);

    yAxis.setLogScale(false);
    QCOMPARE(yAxis.dataMin(), -1.0);
    QCOMPARE(yAxis.dataMax(), 100.0);

    yAxis.setLogScale(true);
    QCOMPARE(yAxis.dataMin(), 10.0);
}

void LineCurveGapsTest::noRangeDataIsNotRescannedOnLogScaleChange()
{
    auto yAxis = Axis{};
    auto curve = LineCurve{};
    curve.setYAxis(&yAxis);
    auto yRangeSpy = QSignalSpy{&curve, &LineCurve::yDataRangeChanged};

    curve.setDataFNoRange(std::vector<float>{0.0f, -1.0f, 1.0f, 10.0f}, 2);
    yAxis.setLogScale(true);
    yAxis.setLogScale(false);

    QCOMPARE(yRangeSpy.count(), 0);
}

void LineCurveGapsTest::appendDataExcludesInvalidCoordinates()
{
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    const auto inf = std::numeric_limits<double>::infinity();
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    yAxis.setLogScale(true);
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);

    curve.appendData(nan, -1.0);
    curve.appendData(1.0, 10.0);
    curve.appendData(inf, 0.0);
    curve.appendData(3.0, nan);
    curve.appendData(2.0, 100.0);

    QCOMPARE(xAxis.dataMin(), 1.0);
    QCOMPARE(xAxis.dataMax(), 3.0);
    QCOMPARE(yAxis.dataMin(), 10.0);
    QCOMPARE(yAxis.dataMax(), 100.0);

    yAxis.setLogScale(false);
    QCOMPARE(yAxis.dataMin(), -1.0);
    QCOMPARE(yAxis.dataMax(), 100.0);
}

void LineCurveGapsTest::appendDataAfterNoRangeDataRescans()
{
    auto yAxis = Axis{};
    auto curve = LineCurve{};
    curve.setYAxis(&yAxis);
    curve.setDataF(std::vector<float>{0.0f, 50.0f}, 1);

    curve.setDataFNoRange(std::vector<float>{0.0f, 2.0f, 1.0f, 4.0f}, 2);
    curve.appendData(2.0, 3.0);

    QCOMPARE(yAxis.dataMin(), 2.0);
    QCOMPARE(yAxis.dataMax(), 4.0);
}

void LineCurveGapsTest::nanGapModeDefaultsToBreakAndNotifies()
{
    auto curve = LineCurve{};
    QVERIFY(curve.gaps());
    QCOMPARE(curve.gaps()->parent(), &curve);
    auto spy = QSignalSpy{curve.gaps(), &LineCurveGaps::nanModeChanged};

    QCOMPARE(curve.gaps()->nanMode(), NanGapMode::Break);
    curve.gaps()->setNanMode(NanGapMode::Connect);
    curve.gaps()->setNanMode(NanGapMode::Connect);

    QCOMPARE(curve.gaps()->nanMode(), NanGapMode::Connect);
    QCOMPARE(spy.count(), 1);
}

void LineCurveGapsTest::breakModeHitTestDoesNotBridgeGap()
{
    auto fixture = HitTestFixture{};
    fixture.curve.setDataF(gappedLine(), 5);

    QVERIFY(fixture.hits({50.0, 50.0}));   // on segment 0 -> 1
    QVERIFY(fixture.hits({350.0, 50.0}));  // on segment 3 -> 4
    QVERIFY(!fixture.hits({200.0, 50.0})); // inside the gap
}

void LineCurveGapsTest::connectModeHitTestBridgesGap()
{
    auto fixture = HitTestFixture{};
    fixture.curve.gaps()->setNanMode(NanGapMode::Connect);
    fixture.curve.setDataF(gappedLine(), 5);
    QVERIFY(fixture.hits({200.0, 50.0}));

    fixture.curve.gaps()->setNanMode(NanGapMode::Break);
    QVERIFY(!fixture.hits({200.0, 50.0}));
}

void LineCurveGapsTest::connectModeBridgesDoubleData()
{
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    auto fixture = HitTestFixture{};
    fixture.curve.setData(QList<QPointF>{{0.0, 5.0}, {1.0, 5.0}, {2.0, nan}, {nan, 5.0}, {4.0, 5.0}});
    QVERIFY(!fixture.hits({250.0, 50.0}));

    fixture.curve.gaps()->setNanMode(NanGapMode::Connect);
    QVERIFY(fixture.hits({250.0, 50.0}));
}

void LineCurveGapsTest::connectModeBridgesAppendedData()
{
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    auto fixture = HitTestFixture{};
    fixture.curve.gaps()->setNanMode(NanGapMode::Connect);
    fixture.curve.appendData(0.0, 5.0);
    fixture.curve.appendData(1.0, 5.0);
    fixture.curve.appendData(2.0, nan);
    fixture.curve.appendData(nan, 5.0);
    fixture.curve.appendData(4.0, 5.0);
    QVERIFY(fixture.hits({250.0, 50.0}));

    fixture.curve.gaps()->setNanMode(NanGapMode::Break);
    QVERIFY(!fixture.hits({250.0, 50.0}));
}

void LineCurveGapsTest::markerHitTestSkipsInvalidSamples()
{
    auto fixture = HitTestFixture{};
    fixture.curve.setMarkerShape(LineCurve::PointShape::Circle);
    fixture.curve.setMarkerSize(6.0);
    fixture.yAxis.setViewportMin(1.0);
    fixture.yAxis.setViewportMax(100.0);
    fixture.yAxis.setLogScale(true);

    // y = -1 is invalid on a log axis; Axis::coordToPixel would map it to pixel 0.
    fixture.curve.setDataF(std::vector<float>{2.0f, 10.0f, 5.0f, -1.0f}, 2);

    QVERIFY(fixture.hits({200.0, 50.0}));
    QVERIFY(!fixture.hits({500.0, 0.0}));
}

void LineCurveGapsTest::emptyChunksAreSkipped()
{
    auto fixture = HitTestFixture{};
    fixture.xAxis.setViewportMax(1000.0);
    constexpr auto pointCount = 1200;
    auto data = std::vector<float>(static_cast<std::size_t>(pointCount) * 2, kNaN);
    for (auto i = 700; i < pointCount; ++i) {
        data[static_cast<std::size_t>(i) * 2] = static_cast<float>(i);
        data[static_cast<std::size_t>(i) * 2 + 1] = 5.0f;
    }
    fixture.curve.setDataF(std::move(data), pointCount);

    QVERIFY(fixture.hits({800.0, 50.0}));
    QVERIFY(!fixture.hits({100.0, 50.0}));
}

} // namespace QAccelPlot

using QAccelPlot::LineCurveGapsTest;
QTEST_MAIN(LineCurveGapsTest)
#include "tst_line_curve_gaps.moc"
