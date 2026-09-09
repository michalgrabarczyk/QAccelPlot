//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "DrawTransition.hpp"
#include "MorphTransition.hpp"

#include <QtTest/QtTest>

#include <vector>

// ---------------------------------------------------------------------------
// Helper subclasses to expose protected interpolate() for direct testing,
// avoiding dependence on the real-time QElapsedTimer inside advance().
// ---------------------------------------------------------------------------

class TestMorphTransition : public QAccelPlot::MorphTransition {
public:
    using QAccelPlot::MorphTransition::MorphTransition;

    void callInterpolate(
        float progress, const std::vector<float>& from, int fromCount, const std::vector<float>& to, int toCount, std::vector<float>& out, int& outCount)
    {
        interpolate(progress, from, fromCount, to, toCount, out, outCount);
    }
};

class TestDrawTransition : public QAccelPlot::DrawTransition {
public:
    using QAccelPlot::DrawTransition::DrawTransition;

    void callInterpolate(
        float progress, const std::vector<float>& from, int fromCount, const std::vector<float>& to, int toCount, std::vector<float>& out, int& outCount)
    {
        interpolate(progress, from, fromCount, to, toCount, out, outCount);
    }
};

// ---------------------------------------------------------------------------

class TestTransitions : public QObject {
    Q_OBJECT

private slots:
    // MorphTransition
    void morph_progressZero_outputEqualsFrom();
    void morph_progressOne_outputEqualsTo();
    void morph_progressHalf_outputIsMidpoint();
    void morph_expandingPointCount();
    void morph_shrinkingPointCount();
    void morph_emptyFrom_outputEqualsTo();
    void morph_emptyTo_outputIsEmpty();

    // DrawTransition
    void draw_progressZero_minimumTwoPoints();
    void draw_progressOne_allPoints();
    void draw_progressHalf_halfPointsRevealed();
    void draw_outputIsPrefix_ofToData();
    void draw_emptyTo_outputIsEmpty();
    void draw_singlePointTo_outputIsSinglePoint();

    // DataTransition state
    void transition_startSetsRunning();
    void transition_cancelClearsRunning();

    // Corner cases
    void morph_identicalFromAndTo_outputUnchanged();
    void morph_singlePointBothSides();
    void draw_minimumToPointCount_alwaysTwo();
    void transition_advance_whenNotRunning_returnsFalse();
};

// ---------------------------------------------------------------------------
// MorphTransition tests
// ---------------------------------------------------------------------------

void TestTransitions::morph_progressZero_outputEqualsFrom()
{
    auto t = TestMorphTransition{};
    const auto from = std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f};
    const auto to = std::vector<float>{9.0f, 8.0f, 7.0f, 6.0f};
    auto out = std::vector<float>{};
    auto outCount = int{};

    t.callInterpolate(0.0f, from, 2, to, 2, out, outCount);

    QCOMPARE(outCount, 2);
    QCOMPARE(out[0], from[0]);
    QCOMPARE(out[1], from[1]);
    QCOMPARE(out[2], from[2]);
    QCOMPARE(out[3], from[3]);
}

void TestTransitions::morph_progressOne_outputEqualsTo()
{
    auto t = TestMorphTransition{};
    const auto from = std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f};
    const auto to = std::vector<float>{9.0f, 8.0f, 7.0f, 6.0f};
    auto out = std::vector<float>{};
    auto outCount = int{};

    t.callInterpolate(1.0f, from, 2, to, 2, out, outCount);

    QCOMPARE(outCount, 2);
    QCOMPARE(out[0], to[0]);
    QCOMPARE(out[1], to[1]);
    QCOMPARE(out[2], to[2]);
    QCOMPARE(out[3], to[3]);
}

void TestTransitions::morph_progressHalf_outputIsMidpoint()
{
    auto t = TestMorphTransition{};
    const auto from = std::vector<float>{0.0f, 0.0f, 0.0f, 0.0f};
    const auto to = std::vector<float>{4.0f, 8.0f, 2.0f, 6.0f};
    auto out = std::vector<float>{};
    auto outCount = int{};

    t.callInterpolate(0.5f, from, 2, to, 2, out, outCount);

    QCOMPARE(outCount, 2);
    QCOMPARE(out[0], 2.0f);
    QCOMPARE(out[1], 4.0f);
    QCOMPARE(out[2], 1.0f);
    QCOMPARE(out[3], 3.0f);
}

void TestTransitions::morph_expandingPointCount()
{
    // from has 1 point, to has 3 points → maxCount=3, last from-point clamped
    auto t = TestMorphTransition{};
    const auto from = std::vector<float>{0.0f, 0.0f};
    const auto to = std::vector<float>{10.0f, 10.0f, 20.0f, 20.0f, 30.0f, 30.0f};
    auto out = std::vector<float>{};
    auto outCount = int{};

    t.callInterpolate(1.0f, from, 1, to, 3, out, outCount);

    QCOMPARE(outCount, 3);
    QCOMPARE(out[0], to[0]);
    QCOMPARE(out[2], to[2]);
    QCOMPARE(out[4], to[4]);
}

void TestTransitions::morph_shrinkingPointCount()
{
    // from has 3 points, to has 1 point → maxCount=3, last to-point clamped
    auto t = TestMorphTransition{};
    const auto from = std::vector<float>{0.0f, 0.0f, 5.0f, 5.0f, 10.0f, 10.0f};
    const auto to = std::vector<float>{20.0f, 20.0f};
    auto out = std::vector<float>{};
    auto outCount = int{};

    t.callInterpolate(1.0f, from, 3, to, 1, out, outCount);

    // At progress=1, all output points should equal to[clamped_index * 2]
    QCOMPARE(outCount, 3);
    // to index clamped to 0 for all three → all equal to[0], to[1]
    QCOMPARE(out[0], to[0]);
    QCOMPARE(out[1], to[1]);
    QCOMPARE(out[2], to[0]);
    QCOMPARE(out[3], to[1]);
}

void TestTransitions::morph_emptyFrom_outputEqualsTo()
{
    auto t = TestMorphTransition{};
    const auto from = std::vector<float>{};
    const auto to = std::vector<float>{1.0f, 2.0f};
    auto out = std::vector<float>{};
    auto outCount = int{};

    t.callInterpolate(0.5f, from, 0, to, 1, out, outCount);

    QCOMPARE(outCount, 1);
    QCOMPARE(out[0], to[0]);
    QCOMPARE(out[1], to[1]);
}

void TestTransitions::morph_emptyTo_outputIsEmpty()
{
    auto t = TestMorphTransition{};
    const auto from = std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f};
    const auto to = std::vector<float>{};
    auto out = std::vector<float>{};
    auto outCount = int{};

    t.callInterpolate(0.5f, from, 2, to, 0, out, outCount);

    QCOMPARE(outCount, 0);
    QVERIFY(out.empty());
}

// ---------------------------------------------------------------------------
// DrawTransition tests
// ---------------------------------------------------------------------------

void TestTransitions::draw_progressZero_minimumTwoPoints()
{
    auto t = TestDrawTransition{};
    const auto to = std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    auto out = std::vector<float>{};
    auto outCount = int{};

    // progress=0 → ceil(4 * 0) = 0 → viewportMax(2, 0) = 2
    t.callInterpolate(0.0f, {}, 0, to, 4, out, outCount);

    QCOMPARE(outCount, 2);
    QCOMPARE(out[0], to[0]);
    QCOMPARE(out[1], to[1]);
}

void TestTransitions::draw_progressOne_allPoints()
{
    auto t = TestDrawTransition{};
    const auto to = std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
    auto out = std::vector<float>{};
    auto outCount = int{};

    t.callInterpolate(1.0f, {}, 0, to, 3, out, outCount);

    QCOMPARE(outCount, 3);
    for (auto i = 0; i < 6; ++i) {
        QCOMPARE(out[i], to[i]);
    }
}

void TestTransitions::draw_progressHalf_halfPointsRevealed()
{
    auto t = TestDrawTransition{};
    const auto to = std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    auto out = std::vector<float>{};
    auto outCount = int{};

    // progress=0.5 → ceil(4 * 0.5) = ceil(2) = 2 → viewportMax(2, 2) = 2
    t.callInterpolate(0.5f, {}, 0, to, 4, out, outCount);

    QCOMPARE(outCount, 2);
}

void TestTransitions::draw_outputIsPrefix_ofToData()
{
    auto t = TestDrawTransition{};
    const auto to = std::vector<float>{10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f};
    auto out = std::vector<float>{};
    auto outCount = int{};

    // progress=0.75 → ceil(3 * 0.75) = ceil(2.25) = 3 → all 3 points
    t.callInterpolate(0.75f, {}, 0, to, 3, out, outCount);

    QCOMPARE(outCount, 3);
    for (auto i = 0; i < outCount * 2; ++i) {
        QCOMPARE(out[i], to[i]);
    }
}

void TestTransitions::draw_emptyTo_outputIsEmpty()
{
    auto t = TestDrawTransition{};
    const auto to = std::vector<float>{};
    auto out = std::vector<float>{};
    auto outCount = int{};

    t.callInterpolate(0.5f, {}, 0, to, 0, out, outCount);

    QCOMPARE(outCount, 0);
    QVERIFY(out.empty());
}

void TestTransitions::draw_singlePointTo_outputIsSinglePoint()
{
    auto t = TestDrawTransition{};
    const auto to = std::vector<float>{5.0f, 7.0f};
    auto out = std::vector<float>{};
    auto outCount = int{};

    t.callInterpolate(0.5f, {}, 0, to, 1, out, outCount);

    QCOMPARE(outCount, 1);
    QCOMPARE(out, to);
}

// ---------------------------------------------------------------------------
// DataTransition state tests
// ---------------------------------------------------------------------------

void TestTransitions::transition_startSetsRunning()
{
    auto t = QAccelPlot::MorphTransition{};
    QCOMPARE(t.running(), false);

    auto data = std::vector<float>{1.0f, 2.0f};
    t.start({}, 0, std::move(data), 1);

    QCOMPARE(t.running(), true);
}

void TestTransitions::transition_cancelClearsRunning()
{
    auto t = QAccelPlot::MorphTransition{};
    auto data = std::vector<float>{1.0f, 2.0f};
    t.start({}, 0, std::move(data), 1);
    QCOMPARE(t.running(), true);

    t.cancel();
    QCOMPARE(t.running(), false);
}

void TestTransitions::morph_identicalFromAndTo_outputUnchanged()
{
    auto t = TestMorphTransition{};
    const auto data = std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f};
    auto out = std::vector<float>{};
    auto outCount = int{};

    // When from==to every progress value must leave the data unchanged.
    t.callInterpolate(0.5f, data, 2, data, 2, out, outCount);

    QCOMPARE(outCount, 2);
    QCOMPARE(out[0], data[0]);
    QCOMPARE(out[1], data[1]);
    QCOMPARE(out[2], data[2]);
    QCOMPARE(out[3], data[3]);
}

void TestTransitions::morph_singlePointBothSides()
{
    auto t = TestMorphTransition{};
    const auto from = std::vector<float>{0.0f, 0.0f};
    const auto to = std::vector<float>{10.0f, 20.0f};
    auto out = std::vector<float>{};
    auto outCount = int{};

    t.callInterpolate(0.5f, from, 1, to, 1, out, outCount);

    QCOMPARE(outCount, 1);
    QCOMPARE(out[0], 5.0f);
    QCOMPARE(out[1], 10.0f);
}

void TestTransitions::draw_minimumToPointCount_alwaysTwo()
{
    auto t = TestDrawTransition{};
    // toPointCount==2 is the minimum meaningful dataset. At progress=0,
    // ceil(2*0)=0 but viewportMax(2,0)=2 ensures both points are always copied.
    const auto to = std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f};
    auto out = std::vector<float>{};
    auto outCount = int{};

    t.callInterpolate(0.0f, {}, 0, to, 2, out, outCount);

    QCOMPARE(outCount, 2);
    QCOMPARE(out[0], to[0]);
    QCOMPARE(out[1], to[1]);
    QCOMPARE(out[2], to[2]);
    QCOMPARE(out[3], to[3]);
}

void TestTransitions::transition_advance_whenNotRunning_returnsFalse()
{
    auto t = QAccelPlot::MorphTransition{};
    auto out = std::vector<float>{};
    auto outCount = int{};

    // advance() without a prior start() must return false immediately.
    const auto stillRunning = t.advance(out, outCount);
    QCOMPARE(stillRunning, false);
}

QTEST_GUILESS_MAIN(TestTransitions)
#include "tst_transitions.moc"
