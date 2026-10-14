//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/internal/RangeGesture.hpp"

#include <QtTest/QtTest>

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

using Internal::pannedRange;
using Internal::ValueRange;
using Internal::wheelZoomFactor;
using Internal::zoomedRange;

namespace {

constexpr auto kTolerance = 1e-12;

bool fuzzyEqual(const qreal actual, const qreal expected)
{
    return std::abs(actual - expected) <= kTolerance * std::max(1.0, std::abs(expected));
}

} // namespace

#define QCOMPARE_NEAR(actual, expected)                                                                                                                        \
    QVERIFY2(fuzzyEqual((actual), (expected)), qPrintable(QStringLiteral("actual %1, expected %2").arg(actual, 0, 'g', 17).arg(expected, 0, 'g', 17)))

class RangeGestureTest : public QObject {
    Q_OBJECT

private slots:
    void panShiftsByFractionOfExtent();
    void panInLogSpaceKeepsRatio();
    void panOfNonPositiveLogRangeIsLinear();
    void zeroPanReturnsRangeUnchanged();
    void zoomKeepsAnchorInPlace();
    void zoomInLogSpaceKeepsAnchorInPlace();
    void zoomOutUndoesZoomIn();
    void zoomPastTheRangeOfADoubleReturnsRangeUnchanged_data();
    void zoomPastTheRangeOfADoubleReturnsRangeUnchanged();
    void panPastTheRangeOfADoubleReturnsRangeUnchanged_data();
    void panPastTheRangeOfADoubleReturnsRangeUnchanged();
    void repeatedLogZoomOutStaysUsable();
    void wheelZoomFactorIsReciprocalWhenZoomingOut();
    void wheelZoomFactorAlwaysZooms();
};

void RangeGestureTest::panShiftsByFractionOfExtent()
{
    const auto raised = pannedRange({10.0, 30.0}, 0.25, false);
    QCOMPARE_NEAR(raised.min, 15.0);
    QCOMPARE_NEAR(raised.max, 35.0);

    const auto lowered = pannedRange({10.0, 30.0}, -0.5, false);
    QCOMPARE_NEAR(lowered.min, 0.0);
    QCOMPARE_NEAR(lowered.max, 20.0);
}

void RangeGestureTest::panInLogSpaceKeepsRatio()
{
    // Half of two decades is one decade.
    const auto range = pannedRange({1.0, 100.0}, 0.5, true);
    QCOMPARE_NEAR(range.min, 10.0);
    QCOMPARE_NEAR(range.max, 1000.0);
}

void RangeGestureTest::panOfNonPositiveLogRangeIsLinear()
{
    const auto range = pannedRange({-10.0, 10.0}, 0.5, true);
    QCOMPARE_NEAR(range.min, 0.0);
    QCOMPARE_NEAR(range.max, 20.0);
}

void RangeGestureTest::zeroPanReturnsRangeUnchanged()
{
    const auto range = pannedRange({0.3, 0.7}, 0.0, true);
    QCOMPARE(range.min, 0.3);
    QCOMPARE(range.max, 0.7);
}

void RangeGestureTest::zoomKeepsAnchorInPlace()
{
    // The anchor, a quarter into the range, is 20.
    const auto range = zoomedRange({0.0, 80.0}, 0.5, 0.25, false);
    QCOMPARE_NEAR(range.min, 10.0);
    QCOMPARE_NEAR(range.max, 50.0);
    QCOMPARE_NEAR(range.min + 0.25 * (range.max - range.min), 20.0);
}

void RangeGestureTest::zoomInLogSpaceKeepsAnchorInPlace()
{
    // The anchor, halfway through four decades, is 100.
    const auto range = zoomedRange({1.0, 10000.0}, 0.5, 0.5, true);
    QCOMPARE_NEAR(range.min, 10.0);
    QCOMPARE_NEAR(range.max, 1000.0);
}

void RangeGestureTest::zoomOutUndoesZoomIn()
{
    const auto zoomedIn = zoomedRange({-4.0, 12.0}, wheelZoomFactor(0.9, true), 0.3, false);
    const auto restored = zoomedRange(zoomedIn, wheelZoomFactor(0.9, false), 0.3, false);
    QCOMPARE_NEAR(restored.min, -4.0);
    QCOMPARE_NEAR(restored.max, 12.0);
}

void RangeGestureTest::zoomPastTheRangeOfADoubleReturnsRangeUnchanged_data()
{
    QTest::addColumn<qreal>("min");
    QTest::addColumn<qreal>("max");
    QTest::addColumn<bool>("logarithmic");

    // Tripling 400 decades reaches 10^-600 and 10^600, which a double stores as 0 and infinity.
    QTest::newRow("log") << 1e-200 << 1e200 << true;
    QTest::newRow("linear") << -1e308 << 1e308 << false;
}

void RangeGestureTest::zoomPastTheRangeOfADoubleReturnsRangeUnchanged()
{
    QFETCH(qreal, min);
    QFETCH(qreal, max);
    QFETCH(bool, logarithmic);

    // Compared exactly: QCOMPARE takes any two values near zero for equal.
    const auto range = zoomedRange({min, max}, 3.0, 0.5, logarithmic);
    QVERIFY(range.min == min);
    QVERIFY(range.max == max);
}

void RangeGestureTest::panPastTheRangeOfADoubleReturnsRangeUnchanged_data()
{
    QTest::addColumn<qreal>("min");
    QTest::addColumn<qreal>("max");
    QTest::addColumn<qreal>("fraction");
    QTest::addColumn<bool>("logarithmic");

    QTest::newRow("log up") << 1e300 << 1e308 << 1.0 << true;
    QTest::newRow("log down") << 1e-308 << 1e-300 << -4.0 << true;
    QTest::newRow("linear up") << 1e307 << 1.5e308 << 1.0 << false;
    QTest::newRow("linear down") << -1.5e308 << -1e307 << -1.0 << false;
}

void RangeGestureTest::panPastTheRangeOfADoubleReturnsRangeUnchanged()
{
    QFETCH(qreal, min);
    QFETCH(qreal, max);
    QFETCH(qreal, fraction);
    QFETCH(bool, logarithmic);

    const auto range = pannedRange({min, max}, fraction, logarithmic);
    QVERIFY(range.min == min);
    QVERIFY(range.max == max);
}

void RangeGestureTest::repeatedLogZoomOutStaysUsable()
{
    auto range = ValueRange{1.0, 1000.0};
    for (auto step = 0; step < 200; ++step) {
        range = zoomedRange(range, wheelZoomFactor(0.9, false), 0.5, true);
    }
    QVERIFY(std::isfinite(range.max));
    QVERIFY(range.min > 0.0);
    QVERIFY(range.min < range.max);

    // The range still zooms back in.
    const auto narrowed = zoomedRange(range, wheelZoomFactor(0.9, true), 0.5, true);
    QVERIFY(narrowed.min > range.min);
    QVERIFY(narrowed.max < range.max);
}

void RangeGestureTest::wheelZoomFactorIsReciprocalWhenZoomingOut()
{
    QCOMPARE_NEAR(wheelZoomFactor(0.8, true), 0.8);
    QCOMPARE_NEAR(wheelZoomFactor(0.8, false), 1.25);
}

void RangeGestureTest::wheelZoomFactorAlwaysZooms()
{
    QCOMPARE_NEAR(wheelZoomFactor(0.0, true), 0.01);
    QCOMPARE_NEAR(wheelZoomFactor(-1.0, true), 0.01);
    QCOMPARE_NEAR(wheelZoomFactor(1.0, true), 0.99);
    QCOMPARE_NEAR(wheelZoomFactor(2.0, true), 0.99);
}

} // namespace QAccelPlot

using QAccelPlot::RangeGestureTest;
QTEST_GUILESS_MAIN(RangeGestureTest)
#include "tst_range_gesture.moc"
