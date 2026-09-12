//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "Axis.hpp"

#include <QtTest/QtTest>

#include <cmath>
#include <memory>

class TestAxisMapping : public QObject {
    Q_OBJECT

private slots:
    // Linear, Horizontal
    void linear_horizontal_minMapsToZero();
    void linear_horizontal_maxMapsToLength();
    void linear_horizontal_midpointMapsToHalfLength();
    void linear_horizontal_roundTrip();

    // Linear, Vertical (inverted: viewportMin→length, viewportMax→0)
    void linear_vertical_minMapsToLength();
    void linear_vertical_maxMapsToZero();
    void linear_vertical_midpointMapsToHalfLength();

    // Log scale
    void log_decadeMapsToHalfLength();
    void log_negativeValue_returnsZero();
    void log_zeroMin_returnsZero();

    // Edge cases
    void zeroRange_returnsZero();
    void zeroLength_fromPosition_returnsZero();

    // Corner cases
    void linear_horizontal_valueAboveMax_extrapolates();
    void linear_horizontal_valueBelowMin_extrapolatesNegative();
    void linear_invertedMinMax_stillMaps();
    void log_mapFromPosition_roundTrip();
    void dataRange_updateReplacesRange();
    void viewportRange_acceptsMicrosecondEpochChanges();
    void constructor_sideSetsOrientation();
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::unique_ptr<QAccelPlot::Axis> makeHorizontalAxis(qreal viewportMin, qreal viewportMax)
{
    auto axis = std::make_unique<QAccelPlot::Axis>(nullptr, QAccelPlot::Axis::Bottom);
    axis->setViewportMin(viewportMin);
    axis->setViewportMax(viewportMax);
    return axis;
}

static std::unique_ptr<QAccelPlot::Axis> makeVerticalAxis(qreal viewportMin, qreal viewportMax)
{
    auto axis = std::make_unique<QAccelPlot::Axis>(nullptr, QAccelPlot::Axis::Left);
    axis->setViewportMin(viewportMin);
    axis->setViewportMax(viewportMax);
    return axis;
}

// ---------------------------------------------------------------------------
// Linear horizontal
// ---------------------------------------------------------------------------

void TestAxisMapping::linear_horizontal_minMapsToZero()
{
    const auto axis = makeHorizontalAxis(0.0, 100.0);
    QCOMPARE(axis->coordToPixel(0.0, 400.0), 0.0);
}

void TestAxisMapping::linear_horizontal_maxMapsToLength()
{
    const auto axis = makeHorizontalAxis(0.0, 100.0);
    QCOMPARE(axis->coordToPixel(100.0, 400.0), 400.0);
}

void TestAxisMapping::linear_horizontal_midpointMapsToHalfLength()
{
    const auto axis = makeHorizontalAxis(0.0, 100.0);
    QCOMPARE(axis->coordToPixel(50.0, 400.0), 200.0);
}

void TestAxisMapping::linear_horizontal_roundTrip()
{
    const auto axis = makeHorizontalAxis(-50.0, 50.0);
    const auto value = qreal{17.3};
    const auto length = qreal{800.0};
    const auto pos = axis->coordToPixel(value, length);
    const auto recovered = axis->pixelToCoord(pos, length);
    QVERIFY(qAbs(recovered - value) < 1e-10);
}

// ---------------------------------------------------------------------------
// Linear vertical (inverted)
// ---------------------------------------------------------------------------

void TestAxisMapping::linear_vertical_minMapsToLength()
{
    const auto axis = makeVerticalAxis(0.0, 100.0);
    QCOMPARE(axis->coordToPixel(0.0, 300.0), 300.0);
}

void TestAxisMapping::linear_vertical_maxMapsToZero()
{
    const auto axis = makeVerticalAxis(0.0, 100.0);
    QCOMPARE(axis->coordToPixel(100.0, 300.0), 0.0);
}

void TestAxisMapping::linear_vertical_midpointMapsToHalfLength()
{
    const auto axis = makeVerticalAxis(0.0, 100.0);
    QCOMPARE(axis->coordToPixel(50.0, 300.0), 150.0);
}

// ---------------------------------------------------------------------------
// Log scale
// ---------------------------------------------------------------------------

void TestAxisMapping::log_decadeMapsToHalfLength()
{
    const auto axis = makeHorizontalAxis(1.0, 100.0);
    axis->setLogScale(true);
    // log10(1)=0, log10(100)=2, log10(10)=1 → ratio = (1-0)/(2-0) = 0.5
    QCOMPARE(axis->coordToPixel(10.0, 500.0), 250.0);
}

void TestAxisMapping::log_negativeValue_returnsZero()
{
    const auto axis = makeHorizontalAxis(1.0, 100.0);
    axis->setLogScale(true);
    QCOMPARE(axis->coordToPixel(-5.0, 500.0), 0.0);
}

void TestAxisMapping::log_zeroMin_returnsZero()
{
    const auto axis = makeHorizontalAxis(0.0, 100.0);
    axis->setLogScale(true);
    // viewportMin <= 0 with log scale → guard returns 0
    QCOMPARE(axis->coordToPixel(10.0, 500.0), 0.0);
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

void TestAxisMapping::zeroRange_returnsZero()
{
    const auto axis = makeHorizontalAxis(5.0, 5.0);
    QCOMPARE(axis->coordToPixel(5.0, 400.0), 0.0);
}

void TestAxisMapping::zeroLength_fromPosition_returnsZero()
{
    const auto axis = makeHorizontalAxis(0.0, 100.0);
    QCOMPARE(axis->pixelToCoord(50.0, 0.0), 0.0);
}

void TestAxisMapping::linear_horizontal_valueAboveMax_extrapolates()
{
    const auto axis = makeHorizontalAxis(0.0, 100.0);
    // value=150 → ratio=1.5 → pos = 1.5 * 400 = 600 (beyond length)
    QCOMPARE(axis->coordToPixel(150.0, 400.0), 600.0);
}

void TestAxisMapping::linear_horizontal_valueBelowMin_extrapolatesNegative()
{
    const auto axis = makeHorizontalAxis(0.0, 100.0);
    // value=-50 → ratio=-0.5 → pos = -0.5 * 400 = -200 (negative)
    QCOMPARE(axis->coordToPixel(-50.0, 400.0), -200.0);
}

void TestAxisMapping::linear_invertedMinMax_stillMaps()
{
    // When viewportMin > viewportMax the linear formula is still well-defined (range is negative
    // but non-zero). viewportMin maps to 0, viewportMax maps to length, midpoint maps to length/2.
    const auto axis = makeHorizontalAxis(100.0, 0.0);
    QCOMPARE(axis->coordToPixel(100.0, 400.0), 0.0);
    QCOMPARE(axis->coordToPixel(0.0, 400.0), 400.0);
    QCOMPARE(axis->coordToPixel(50.0, 400.0), 200.0);
}

void TestAxisMapping::log_mapFromPosition_roundTrip()
{
    const auto axis = makeHorizontalAxis(1.0, 10000.0);
    axis->setLogScale(true);
    // Each decade: coordToPixel → pixelToCoord must recover the original value.
    for (const auto v : {1.0, 10.0, 100.0, 1000.0, 10000.0}) {
        const auto pos = axis->coordToPixel(v, 1000.0);
        const auto recovered = axis->pixelToCoord(pos, 1000.0);
        QVERIFY(qAbs(recovered - v) / v < 1e-10);
    }
}

void TestAxisMapping::dataRange_updateReplacesRange()
{
    auto positiveAxis = QAccelPlot::Axis{};
    positiveAxis.updateDataRange(10.0, 20.0);
    QCOMPARE(positiveAxis.dataMin(), 10.0);
    QCOMPARE(positiveAxis.dataMax(), 20.0);
    positiveAxis.updateDataRange(12.0, 18.0);
    QCOMPARE(positiveAxis.dataMin(), 12.0);
    QCOMPARE(positiveAxis.dataMax(), 18.0);

    auto negativeAxis = QAccelPlot::Axis{};
    negativeAxis.updateDataRange(-20.0, -10.0);
    QCOMPARE(negativeAxis.dataMin(), -20.0);
    QCOMPARE(negativeAxis.dataMax(), -10.0);
}

void TestAxisMapping::viewportRange_acceptsMicrosecondEpochChanges()
{
    constexpr auto epochMilliseconds = qreal{1'789'032'600'000.0};
    auto axis = QAccelPlot::Axis{};
    auto minimumSpy = QSignalSpy{&axis, &QAccelPlot::Axis::viewportMinChanged};

    axis.setViewportMin(epochMilliseconds);
    axis.setViewportMin(epochMilliseconds + 0.001);

    QCOMPARE(minimumSpy.count(), 2);
    QCOMPARE(axis.viewportMin(), epochMilliseconds + 0.001);
}

void TestAxisMapping::constructor_sideSetsOrientation()
{
    const auto left = QAccelPlot::Axis{nullptr, QAccelPlot::Axis::Left};
    const auto right = QAccelPlot::Axis{nullptr, QAccelPlot::Axis::Right};
    const auto top = QAccelPlot::Axis{nullptr, QAccelPlot::Axis::Top};
    const auto bottom = QAccelPlot::Axis{nullptr, QAccelPlot::Axis::Bottom};

    QCOMPARE(left.orientation(), QAccelPlot::Axis::Vertical);
    QCOMPARE(right.orientation(), QAccelPlot::Axis::Vertical);
    QCOMPARE(top.orientation(), QAccelPlot::Axis::Horizontal);
    QCOMPARE(bottom.orientation(), QAccelPlot::Axis::Horizontal);
}

QTEST_MAIN(TestAxisMapping)
#include "tst_axis_mapping.moc"
