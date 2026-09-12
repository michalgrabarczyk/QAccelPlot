//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "MorphTransition.hpp"
#include "series/LineCurve.hpp"
#include "series/LineCurveVertexCache.hpp"

#include <QtTest/QtTest>

#include <vector>

namespace QAccelPlot {

class LineCurveDataTest : public QObject {
    Q_OBJECT

private slots:
    void hoverEnvironmentControlsAcceptance();
    void compatibleLineCacheCanBeReused();
    void incompatibleCacheCannotBeReused();
    void emptyCacheClearsCachedState();
    void invalidCacheIsRejected();
    void invalidRawArgumentsAreRejected();
    void invalidVectorArgumentsAreRejected();
    void rawUpdateHonorsTransition();
    void axisAggregatesCurrentSeriesRanges();
    void pointListDataPreservesModernEpochPrecision();
    void separateDoubleDataPreservesModernEpochPrecision();
};

namespace {

std::vector<float> makeData(const int pointCount, const float offset = 0.0f)
{
    auto data = std::vector<float>(static_cast<std::size_t>(pointCount) * 2);
    for (auto i = 0; i < pointCount; ++i) {
        data[static_cast<std::size_t>(i) * 2] = static_cast<float>(i);
        data[static_cast<std::size_t>(i) * 2 + 1] = static_cast<float>(i) + offset;
    }
    return data;
}

std::vector<char> makeLineCache(const std::vector<float>& data, const int pointCount)
{
    auto cache = std::vector<char>{};
    LineCurveLineRenderer{}.buildVertexCache(data, pointCount, cache);
    return cache;
}

} // namespace

void LineCurveDataTest::hoverEnvironmentControlsAcceptance()
{
    constexpr auto variableName = "QACCELPLOT_HOVER_ENABLED";
    const auto wasSet = qEnvironmentVariableIsSet(variableName);
    const auto previousValue = qgetenv(variableName);

    qunsetenv(variableName);
    const auto defaultCurve = LineCurve{};
    qputenv(variableName, "0");
    const auto hoverDisabledCurve = LineCurve{};

    if (wasSet) {
        qputenv(variableName, previousValue);
    } else {
        qunsetenv(variableName);
    }

    QVERIFY(defaultCurve.acceptHoverEvents());
    QVERIFY(!hoverDisabledCurve.acceptHoverEvents());
}

void LineCurveDataTest::compatibleLineCacheCanBeReused()
{
    auto data = makeData(4);
    const auto expectedCache = makeLineCache(data, 4);
    auto cache = LineCurveVertexCache{};

    QVERIFY(cache.install(std::vector<char>(expectedCache), LineCurveVertexCache::Layout::Line, 4, expectedCache.size()));
    QVERIFY(cache.isReusableForDataChange(LineCurveVertexCache::Layout::Line, 4));
    QVERIFY(cache.bytes() == expectedCache);
}

void LineCurveDataTest::incompatibleCacheCannotBeReused()
{
    auto data = makeData(6);
    const auto bytes = makeLineCache(data, 6);
    auto cache = LineCurveVertexCache{};
    QVERIFY(cache.install(std::vector<char>(bytes), LineCurveVertexCache::Layout::Line, 6, bytes.size()));

    QVERIFY(!cache.isReusableForDataChange(LineCurveVertexCache::Layout::Line, 3));
    QVERIFY(!cache.isReusableForDataChange(LineCurveVertexCache::Layout::Points, 6));

    cache.rebuild(LineCurveVertexCache::Layout::Points, 3, [](std::vector<char>& rebuiltBytes) { rebuiltBytes.assign(12, 'p'); });
    QVERIFY(cache.valid());
    QVERIFY(cache.layout() == LineCurveVertexCache::Layout::Points);
    QCOMPARE(cache.pointCount(), 3);
    QCOMPARE(cache.bytes().size(), std::size_t{12});
}

void LineCurveDataTest::emptyCacheClearsCachedState()
{
    auto data = makeData(4);
    auto bytes = makeLineCache(data, 4);
    const auto expectedSize = bytes.size();
    auto cache = LineCurveVertexCache{};
    QVERIFY(cache.install(std::move(bytes), LineCurveVertexCache::Layout::Line, 4, expectedSize));
    QVERIFY(cache.valid());

    QVERIFY(!cache.install({}, LineCurveVertexCache::Layout::Line, 4, expectedSize));

    QVERIFY(!cache.valid());
    QVERIFY(cache.layout() == LineCurveVertexCache::Layout::None);
    QCOMPARE(cache.pointCount(), 0);
    QVERIFY(cache.bytes().empty());
}

void LineCurveDataTest::invalidCacheIsRejected()
{
    auto cache = LineCurveVertexCache{};
    auto bytes = std::vector<char>(7);

    QVERIFY(!cache.install(std::move(bytes), LineCurveVertexCache::Layout::Line, 4, 8));
    QVERIFY(!cache.valid());
    QVERIFY(cache.bytes().empty());
}

void LineCurveDataTest::invalidRawArgumentsAreRejected()
{
    auto curve = LineCurve{};
    auto data = makeData(2);
    auto xRangeSpy = QSignalSpy{&curve, &LineCurve::xDataRangeChanged};
    auto yRangeSpy = QSignalSpy{&curve, &LineCurve::yDataRangeChanged};
    curve.setDataF(data.data(), 2);
    QCOMPARE(xRangeSpy.count(), 1);
    QCOMPARE(yRangeSpy.count(), 1);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve received a null data pointer.*"));
    curve.setDataF(nullptr, 2);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve data point count cannot be negative.*"));
    curve.setDataF(data.data(), -1);

    QCOMPARE(xRangeSpy.count(), 1);
    QCOMPARE(yRangeSpy.count(), 1);
}

void LineCurveDataTest::invalidVectorArgumentsAreRejected()
{
    auto curve = LineCurve{};
    auto xRangeSpy = QSignalSpy{&curve, &LineCurve::xDataRangeChanged};
    auto yRangeSpy = QSignalSpy{&curve, &LineCurve::yDataRangeChanged};

    curve.setDataF(makeData(2), 2);
    QCOMPARE(xRangeSpy.count(), 1);
    QCOMPARE(yRangeSpy.count(), 1);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve received .* floats for 3 points.*"));
    curve.setDataF(makeData(2), 3);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve received .* floats for 3 points.*"));
    curve.setDataFNoRange(makeData(2), 3);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve received .* floats for 3 points.*"));
    curve.setDataFNoRangeWithCache(makeData(2), 3, {});
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve data point count cannot be negative.*"));
    curve.setDataF(makeData(2), -1);

    QCOMPARE(xRangeSpy.count(), 1);
    QCOMPARE(yRangeSpy.count(), 1);
}

void LineCurveDataTest::rawUpdateHonorsTransition()
{
    auto curve = LineCurve{};
    auto transition = MorphTransition{&curve};
    transition.setEnabled(false);
    curve.setTransition(&transition);

    auto first = makeData(2);
    curve.setDataFNoRange(first.data(), 2);
    transition.setEnabled(true);

    auto second = makeData(3, 8.0f);
    curve.setDataFNoRange(second.data(), 3);

    QVERIFY(transition.running());
}

void LineCurveDataTest::axisAggregatesCurrentSeriesRanges()
{
    auto axis = Axis{};
    auto first = LineCurve{};
    auto second = LineCurve{};
    first.setXAxis(&axis);
    second.setXAxis(&axis);

    first.setDataF(std::vector<float>{0.0f, 0.0f, 10.0f, 1.0f}, 2);
    second.setDataF(std::vector<float>{20.0f, 0.0f, 30.0f, 1.0f}, 2);
    QCOMPARE(axis.dataMin(), 0.0);
    QCOMPARE(axis.dataMax(), 30.0);

    first.setDataF(std::vector<float>{5.0f, 0.0f, 8.0f, 1.0f}, 2);
    QCOMPARE(axis.dataMin(), 5.0);
    QCOMPARE(axis.dataMax(), 30.0);

    second.clearData();
    QCOMPARE(axis.dataMin(), 5.0);
    QCOMPARE(axis.dataMax(), 8.0);

    second.setDataF(std::vector<float>{20.0f, 0.0f, 30.0f, 1.0f}, 2);
    QCOMPARE(axis.dataMin(), 5.0);
    QCOMPARE(axis.dataMax(), 30.0);
    second.setXAxis(nullptr);
    QCOMPARE(axis.dataMin(), 5.0);
    QCOMPARE(axis.dataMax(), 8.0);

    {
        auto temporary = LineCurve{};
        temporary.setXAxis(&axis);
        temporary.setDataF(std::vector<float>{-10.0f, 0.0f, -5.0f, 1.0f}, 2);
        QCOMPARE(axis.dataMin(), -10.0);
        QCOMPARE(axis.dataMax(), 8.0);
    }
    QCOMPARE(axis.dataMin(), 5.0);
    QCOMPARE(axis.dataMax(), 8.0);

    first.clearData();
    QCOMPARE(axis.dataMin(), 0.0);
    QCOMPARE(axis.dataMax(), 1.0);
}

void LineCurveDataTest::pointListDataPreservesModernEpochPrecision()
{
    constexpr auto epochMilliseconds = qreal{1'789'032'600'000.0};
    auto xAxis = Axis{};
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);

    curve.setData(QList<QPointF>{
        {epochMilliseconds, 1.0},
        {epochMilliseconds + 1.0, 2.0},
        {epochMilliseconds + 2.0, 3.0},
    });

    QCOMPARE(xAxis.dataMin(), epochMilliseconds);
    QCOMPARE(xAxis.dataMax(), epochMilliseconds + 2.0);
}

void LineCurveDataTest::separateDoubleDataPreservesModernEpochPrecision()
{
    constexpr auto epochMilliseconds = double{1'789'032'600'000.0};
    const auto xs = std::vector<double>{epochMilliseconds, epochMilliseconds + 0.5, epochMilliseconds + 1.0};
    const auto ys = std::vector<double>{10.0, 20.0, 30.0};
    auto xAxis = Axis{};
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);

    curve.setData(xs, ys);

    QCOMPARE(xAxis.dataMin(), epochMilliseconds);
    QCOMPARE(xAxis.dataMax(), epochMilliseconds + 1.0);
}

} // namespace QAccelPlot

using QAccelPlot::LineCurveDataTest;
QTEST_MAIN(LineCurveDataTest)
#include "tst_line_curve_data.moc"
