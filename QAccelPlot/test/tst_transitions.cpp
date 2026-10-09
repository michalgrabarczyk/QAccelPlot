//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/transitions/DrawTransition.hpp"
#include "QAccelPlot/transitions/MorphTransition.hpp"

#include <QtTest/QtTest>

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

// ---------------------------------------------------------------------------
// Helper subclasses to expose protected interpolate() for direct testing,
// avoiding dependence on the real-time QElapsedTimer inside advance().
// ---------------------------------------------------------------------------

template <typename Transition> class Exposed : public Transition {
public:
    using Dataset = QAccelPlot::DataTransition::Dataset;

    void callInterpolate(double progress, const std::vector<double>& from, int fromCount, const std::vector<double>& to, int toCount, std::vector<double>& out,
        int& outCount, int stride = 2)
    {
        auto frame = Dataset{std::move(out), outCount, stride};
        this->interpolate(progress, Dataset{from, fromCount, stride}, Dataset{to, toCount, stride}, frame);
        out = std::move(frame.values);
        outCount = frame.count;
    }
};

using TestMorphTransition = Exposed<QAccelPlot::MorphTransition>;
using TestDrawTransition = Exposed<QAccelPlot::DrawTransition>;

// ---------------------------------------------------------------------------

class TestTransitions : public QObject {
    Q_OBJECT

private slots:
    // MorphTransition
    void morph_equalPointCounts_data();
    void morph_equalPointCounts();
    void morph_modernEpochData_preservesSubFloatPrecision();
    void morph_expandingPointCount();
    void morph_shrinkingPointCount();
    void morph_emptyFrom_outputEqualsTo();
    void morph_emptyTo_outputIsEmpty();
    void morph_threeValuesPerPoint();

    // DrawTransition
    void draw_progress_data();
    void draw_progress();
    void draw_emptyTo_outputIsEmpty();
    void draw_singlePointTo_outputIsSinglePoint();
    void draw_threeValuesPerPoint();

    // DataTransition state
    void transition_startSetsRunning();
    void transition_cancelClearsRunning();
    void transition_sharedRunsAdvanceIndependently();
    void transition_cancelLeavesRunsPending();
    void transition_destructionLeavesRunPending();
    void transition_destroyedRunEndsItsAnimation();
    void transition_startingRunOnAnotherTransitionEndsTheFirst();

    // Corner cases
    void morph_identicalFromAndTo_outputUnchanged();
    void morph_singlePointBothSides();
    void draw_keepsAtLeastOnePoint_data();
    void draw_keepsAtLeastOnePoint();
    void transition_advance_whenNotRunning_returnsFalse();
    void transition_advance_interpolatesUntilDurationElapses();
    void transition_advance_keepsTheStride();

    // Invalid samples
    void morph_invalidTarget_appearsImmediately();
    void morph_invalidSource_jumpsToTarget();
    void draw_preservesInvalidSamples();
};

// ---------------------------------------------------------------------------
// MorphTransition tests
// ---------------------------------------------------------------------------

void TestTransitions::morph_equalPointCounts_data()
{
    QTest::addColumn<double>("progress");

    QTest::newRow("start") << 0.0;
    QTest::newRow("midpoint") << 0.5;
    QTest::newRow("end") << 1.0;
}

void TestTransitions::morph_equalPointCounts()
{
    QFETCH(double, progress);

    auto transition = TestMorphTransition{};
    const auto from = std::vector<double>{1.0, 2.0, 3.0, 4.0};
    const auto to = std::vector<double>{9.0, 8.0, 7.0, 6.0};
    auto out = std::vector<double>{};
    auto outCount = int{};

    transition.callInterpolate(progress, from, 2, to, 2, out, outCount);

    QCOMPARE(outCount, 2);
    for (auto index = std::size_t{0}; index < out.size(); ++index) {
        QCOMPARE(out[index], from[index] + progress * (to[index] - from[index]));
    }
}

void TestTransitions::morph_modernEpochData_preservesSubFloatPrecision()
{
    constexpr auto epochMilliseconds = double{1'789'032'600'000.0};
    auto transition = TestMorphTransition{};
    const auto from = std::vector<double>{epochMilliseconds, 0.0};
    const auto to = std::vector<double>{epochMilliseconds + 1.0, 2.0};
    auto out = std::vector<double>{};
    auto outCount = int{};

    transition.callInterpolate(0.5, from, 1, to, 1, out, outCount);

    QCOMPARE(outCount, 1);
    QCOMPARE(out[0], epochMilliseconds + 0.5);
    QCOMPARE(out[1], 1.0);
}

void TestTransitions::morph_expandingPointCount()
{
    // from has 1 point, to has 3 points → maxCount=3, last from-point clamped
    auto t = TestMorphTransition{};
    const auto from = std::vector<double>{0.0f, 0.0f};
    const auto to = std::vector<double>{10.0f, 10.0f, 20.0f, 20.0f, 30.0f, 30.0f};
    auto out = std::vector<double>{};
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
    const auto from = std::vector<double>{0.0f, 0.0f, 5.0f, 5.0f, 10.0f, 10.0f};
    const auto to = std::vector<double>{20.0f, 20.0f};
    auto out = std::vector<double>{};
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
    const auto from = std::vector<double>{};
    const auto to = std::vector<double>{1.0f, 2.0f};
    auto out = std::vector<double>{};
    auto outCount = int{};

    t.callInterpolate(0.5f, from, 0, to, 1, out, outCount);

    QCOMPARE(outCount, 1);
    QCOMPARE(out[0], to[0]);
    QCOMPARE(out[1], to[1]);
}

void TestTransitions::morph_emptyTo_outputIsEmpty()
{
    auto t = TestMorphTransition{};
    const auto from = std::vector<double>{1.0f, 2.0f, 3.0f, 4.0f};
    const auto to = std::vector<double>{};
    auto out = std::vector<double>{};
    auto outCount = int{};

    t.callInterpolate(0.5f, from, 2, to, 0, out, outCount);

    QCOMPARE(outCount, 0);
    QVERIFY(out.empty());
}

void TestTransitions::morph_threeValuesPerPoint()
{
    // Two points of three values each morph into three: the extra point starts from the last one.
    auto t = TestMorphTransition{};
    const auto from = std::vector<double>{0.0, 1.0, 2.0, 10.0, 11.0, 12.0};
    const auto to = std::vector<double>{2.0, 3.0, 4.0, 20.0, 21.0, 22.0, 30.0, 31.0, 32.0};
    auto out = std::vector<double>{};
    auto outCount = int{};

    t.callInterpolate(0.5, from, 2, to, 3, out, outCount, 3);

    QCOMPARE(outCount, 3);
    QCOMPARE(out, (std::vector<double>{1.0, 2.0, 3.0, 15.0, 16.0, 17.0, 20.0, 21.0, 22.0}));
}

// ---------------------------------------------------------------------------
// DrawTransition tests
// ---------------------------------------------------------------------------

void TestTransitions::draw_progress_data()
{
    QTest::addColumn<double>("progress");
    QTest::addColumn<int>("expectedPointCount");

    QTest::newRow("start-keeps-one-point") << 0.0 << 1;
    QTest::newRow("half") << 0.5 << 2;
    QTest::newRow("three-quarters") << 0.75 << 3;
    QTest::newRow("end") << 1.0 << 4;
}

void TestTransitions::draw_progress()
{
    QFETCH(double, progress);
    QFETCH(int, expectedPointCount);

    auto transition = TestDrawTransition{};
    const auto to = std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    auto out = std::vector<double>{};
    auto outCount = int{};

    transition.callInterpolate(progress, {}, 0, to, 4, out, outCount);

    QCOMPARE(outCount, expectedPointCount);
    for (auto index = 0; index < outCount * 2; ++index) {
        QCOMPARE(out[index], to[index]);
    }
}

void TestTransitions::draw_emptyTo_outputIsEmpty()
{
    auto t = TestDrawTransition{};
    const auto to = std::vector<double>{};
    auto out = std::vector<double>{};
    auto outCount = int{};

    t.callInterpolate(0.5f, {}, 0, to, 0, out, outCount);

    QCOMPARE(outCount, 0);
    QVERIFY(out.empty());
}

void TestTransitions::draw_singlePointTo_outputIsSinglePoint()
{
    auto t = TestDrawTransition{};
    const auto to = std::vector<double>{5.0f, 7.0f};
    auto out = std::vector<double>{};
    auto outCount = int{};

    t.callInterpolate(0.5f, {}, 0, to, 1, out, outCount);

    QCOMPARE(outCount, 1);
    QCOMPARE(out, to);
}

void TestTransitions::draw_threeValuesPerPoint()
{
    auto t = TestDrawTransition{};
    const auto to = std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0};
    auto out = std::vector<double>{};
    auto outCount = int{};

    t.callInterpolate(0.5, {}, 0, to, 4, out, outCount, 3);

    QCOMPARE(outCount, 2);
    QCOMPARE(out, (std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0}));
}

// ---------------------------------------------------------------------------
// DataTransition state tests
// ---------------------------------------------------------------------------

void TestTransitions::transition_startSetsRunning()
{
    auto t = QAccelPlot::MorphTransition{};
    auto run = QAccelPlot::DataTransition::Run{};
    QCOMPARE(t.running(), false);

    auto data = std::vector<double>{1.0f, 2.0f};
    t.start(run, {}, 0, std::move(data), 1);

    QCOMPARE(t.running(), true);
    QVERIFY(run.active());
    QVERIFY(run.pending());
}

void TestTransitions::transition_cancelClearsRunning()
{
    auto t = QAccelPlot::MorphTransition{};
    auto run = QAccelPlot::DataTransition::Run{};
    auto data = std::vector<double>{1.0f, 2.0f};
    t.start(run, {}, 0, std::move(data), 1);
    QCOMPARE(t.running(), true);

    t.cancel();
    QCOMPARE(t.running(), false);
}

void TestTransitions::transition_sharedRunsAdvanceIndependently()
{
    auto t = QAccelPlot::MorphTransition{};
    t.setDuration(10'000);
    auto first = QAccelPlot::DataTransition::Run{};
    auto second = QAccelPlot::DataTransition::Run{};
    t.start(first, {0.0, 0.0}, 1, {10.0, 10.0}, 1);
    t.start(second, {100.0, 100.0}, 1, {200.0, 200.0}, 1);

    auto firstOut = std::vector<double>{};
    auto secondOut = std::vector<double>{};
    auto firstCount = 0;
    auto secondCount = 0;
    QVERIFY(t.advance(first, firstOut, firstCount));
    QVERIFY(t.advance(second, secondOut, secondCount));
    QVERIFY(firstOut[0] >= 0.0 && firstOut[0] < 10.0);
    QVERIFY(secondOut[0] >= 100.0 && secondOut[0] < 200.0);

    t.setDuration(0);
    QTest::qWait(2);
    QVERIFY(!t.advance(first, firstOut, firstCount));
    QCOMPARE(firstOut, (std::vector<double>{10.0, 10.0}));
    QVERIFY(t.running());
    QVERIFY(!t.advance(second, secondOut, secondCount));
    QCOMPARE(secondOut, (std::vector<double>{200.0, 200.0}));
    QVERIFY(!t.running());
}

void TestTransitions::transition_cancelLeavesRunsPending()
{
    auto t = QAccelPlot::MorphTransition{};
    auto run = QAccelPlot::DataTransition::Run{};
    t.start(run, {0.0, 0.0}, 1, {10.0, 20.0}, 1);

    t.cancel();

    QVERIFY(!run.active());
    QVERIFY(run.pending());
    auto out = std::vector<double>{};
    auto outCount = 0;
    QVERIFY(!t.advance(run, out, outCount));
    QVERIFY(run.finish(out, outCount));
    QCOMPARE(out, (std::vector<double>{10.0, 20.0}));
    QCOMPARE(outCount, 1);
    QVERIFY(!run.pending());
    QVERIFY(!run.finish(out, outCount));
}

void TestTransitions::transition_destructionLeavesRunPending()
{
    auto run = QAccelPlot::DataTransition::Run{};
    auto t = std::make_unique<QAccelPlot::MorphTransition>();
    t->start(run, {0.0, 0.0}, 1, {10.0, 20.0}, 1);

    t.reset();

    QVERIFY(!run.active());
    QVERIFY(run.pending());
    auto out = std::vector<double>{};
    auto outCount = 0;
    QVERIFY(run.finish(out, outCount));
    QCOMPARE(out, (std::vector<double>{10.0, 20.0}));
}

void TestTransitions::transition_destroyedRunEndsItsAnimation()
{
    auto t = QAccelPlot::MorphTransition{};
    auto kept = QAccelPlot::DataTransition::Run{};
    t.start(kept, {0.0, 0.0}, 1, {10.0, 20.0}, 1);
    {
        auto destroyed = QAccelPlot::DataTransition::Run{};
        t.start(destroyed, {0.0, 0.0}, 1, {10.0, 20.0}, 1);
    }
    QVERIFY(t.running());

    kept.cancel();

    QVERIFY(!t.running());
    QVERIFY(!kept.pending());
}

void TestTransitions::transition_startingRunOnAnotherTransitionEndsTheFirst()
{
    auto first = QAccelPlot::MorphTransition{};
    auto second = QAccelPlot::DrawTransition{};
    auto run = QAccelPlot::DataTransition::Run{};
    first.start(run, {0.0, 0.0}, 1, {10.0, 20.0}, 1);

    second.start(run, {0.0, 0.0}, 1, {30.0, 40.0}, 1);

    QVERIFY(!first.running());
    QVERIFY(second.running());
    QCOMPARE(run.targetData(), (std::vector<double>{30.0, 40.0}));
}

void TestTransitions::morph_identicalFromAndTo_outputUnchanged()
{
    auto t = TestMorphTransition{};
    const auto data = std::vector<double>{1.0f, 2.0f, 3.0f, 4.0f};
    auto out = std::vector<double>{};
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
    const auto from = std::vector<double>{0.0f, 0.0f};
    const auto to = std::vector<double>{10.0f, 20.0f};
    auto out = std::vector<double>{};
    auto outCount = int{};

    t.callInterpolate(0.5f, from, 1, to, 1, out, outCount);

    QCOMPARE(outCount, 1);
    QCOMPARE(out[0], 5.0f);
    QCOMPARE(out[1], 10.0f);
}

void TestTransitions::draw_keepsAtLeastOnePoint_data()
{
    QTest::addColumn<double>("progress");
    QTest::addColumn<int>("expectedPointCount");

    QTest::newRow("start") << 0.0 << 1;
    // Easing curves such as InBack and OutElastic leave the 0-1 range.
    QTest::newRow("undershoot") << -0.2 << 1;
    QTest::newRow("overshoot") << 1.3 << 3;
}

void TestTransitions::draw_keepsAtLeastOnePoint()
{
    QFETCH(double, progress);
    QFETCH(int, expectedPointCount);

    auto t = TestDrawTransition{};
    const auto to = std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    auto out = std::vector<double>{};
    auto outCount = int{};

    t.callInterpolate(progress, {}, 0, to, 3, out, outCount);

    QCOMPARE(outCount, expectedPointCount);
    QCOMPARE(out, std::vector<double>(to.begin(), to.begin() + expectedPointCount * 2));
}

void TestTransitions::transition_advance_whenNotRunning_returnsFalse()
{
    auto t = QAccelPlot::MorphTransition{};
    auto run = QAccelPlot::DataTransition::Run{};
    auto out = std::vector<double>{};
    auto outCount = int{};

    // advance() without a prior start() must return false immediately.
    const auto stillRunning = t.advance(run, out, outCount);
    QCOMPARE(stillRunning, false);
}

void TestTransitions::transition_advance_interpolatesUntilDurationElapses()
{
    auto t = QAccelPlot::MorphTransition{};
    auto run = QAccelPlot::DataTransition::Run{};
    t.setDuration(10'000);
    t.setEasing(QEasingCurve{QEasingCurve::Linear});
    t.start(run, {0.0, 0.0}, 1, {10.0, 20.0}, 1);

    auto out = std::vector<double>{};
    auto outCount = 0;
    QVERIFY(t.advance(run, out, outCount));
    QVERIFY(t.running());
    QCOMPARE(outCount, 1);
    QVERIFY(out[0] < 10.0);
    QVERIFY(out[1] < 20.0);

    auto runningSpy = QSignalSpy{&t, &QAccelPlot::DataTransition::runningChanged};
    t.setDuration(0);
    QTest::qWait(2);
    QVERIFY(!t.advance(run, out, outCount));
    QVERIFY(!t.running());
    QVERIFY(!run.pending());
    QCOMPARE(runningSpy.count(), 1);
    QCOMPARE(outCount, 1);
    QCOMPARE(out, (std::vector<double>{10.0, 20.0}));
}

void TestTransitions::transition_advance_keepsTheStride()
{
    auto t = QAccelPlot::DrawTransition{};
    auto run = QAccelPlot::DataTransition::Run{};
    t.setDuration(10'000);
    t.start(run, {}, 0, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0}, 2, 3);

    auto out = std::vector<double>{};
    auto outCount = 0;
    QVERIFY(t.advance(run, out, outCount));

    QCOMPARE(outCount, 1);
    QCOMPARE(out, (std::vector<double>{1.0, 2.0, 3.0}));
}

// ---------------------------------------------------------------------------
// Invalid samples
// ---------------------------------------------------------------------------

void TestTransitions::morph_invalidTarget_appearsImmediately()
{
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    const auto inf = std::numeric_limits<double>::infinity();
    auto t = TestMorphTransition{};
    const auto from = std::vector<double>{0.0, 0.0, 1.0, 1.0};
    const auto to = std::vector<double>{10.0, nan, inf, 11.0};
    auto out = std::vector<double>{};
    auto outCount = int{};

    t.callInterpolate(0.1, from, 2, to, 2, out, outCount);

    QCOMPARE(outCount, 2);
    QCOMPARE(out[0], 1.0);
    QVERIFY(std::isnan(out[1]));
    QVERIFY(std::isinf(out[2]));
    QCOMPARE(out[3], 2.0);
}

void TestTransitions::morph_invalidSource_jumpsToTarget()
{
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    auto t = TestMorphTransition{};
    const auto from = std::vector<double>{nan, 0.0, 1.0, -std::numeric_limits<double>::infinity()};
    const auto to = std::vector<double>{10.0, 20.0, 30.0, 40.0};
    auto out = std::vector<double>{};
    auto outCount = int{};

    t.callInterpolate(0.25, from, 2, to, 2, out, outCount);

    QCOMPARE(out[0], 10.0);
    QCOMPARE(out[1], 5.0);
    QCOMPARE(out[2], 8.25);
    QCOMPARE(out[3], 40.0);
}

void TestTransitions::draw_preservesInvalidSamples()
{
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    auto t = TestDrawTransition{};
    const auto to = std::vector<double>{0.0, 1.0, 1.0, nan, 2.0, 3.0};
    auto out = std::vector<double>{};
    auto outCount = int{};

    t.callInterpolate(1.0, {}, 0, to, 3, out, outCount);

    QCOMPARE(outCount, 3);
    QVERIFY(std::isnan(out[3]));
}

QTEST_GUILESS_MAIN(TestTransitions)
#include "tst_transitions.moc"
