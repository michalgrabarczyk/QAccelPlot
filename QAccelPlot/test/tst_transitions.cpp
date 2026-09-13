//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "transitions/DrawTransition.hpp"
#include "transitions/MorphTransition.hpp"

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
        double progress, const std::vector<double>& from, int fromCount, const std::vector<double>& to, int toCount, std::vector<double>& out, int& outCount)
    {
        interpolate(progress, from, fromCount, to, toCount, out, outCount);
    }
};

class TestDrawTransition : public QAccelPlot::DrawTransition {
public:
    using QAccelPlot::DrawTransition::DrawTransition;

    void callInterpolate(
        double progress, const std::vector<double>& from, int fromCount, const std::vector<double>& to, int toCount, std::vector<double>& out, int& outCount)
    {
        interpolate(progress, from, fromCount, to, toCount, out, outCount);
    }
};

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

    // DrawTransition
    void draw_progress_data();
    void draw_progress();
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

// ---------------------------------------------------------------------------
// DrawTransition tests
// ---------------------------------------------------------------------------

void TestTransitions::draw_progress_data()
{
    QTest::addColumn<double>("progress");
    QTest::addColumn<int>("expectedPointCount");

    QTest::newRow("start-keeps-minimum") << 0.0 << 2;
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

// ---------------------------------------------------------------------------
// DataTransition state tests
// ---------------------------------------------------------------------------

void TestTransitions::transition_startSetsRunning()
{
    auto t = QAccelPlot::MorphTransition{};
    QCOMPARE(t.running(), false);

    auto data = std::vector<double>{1.0f, 2.0f};
    t.start({}, 0, std::move(data), 1);

    QCOMPARE(t.running(), true);
}

void TestTransitions::transition_cancelClearsRunning()
{
    auto t = QAccelPlot::MorphTransition{};
    auto data = std::vector<double>{1.0f, 2.0f};
    t.start({}, 0, std::move(data), 1);
    QCOMPARE(t.running(), true);

    t.cancel();
    QCOMPARE(t.running(), false);
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

void TestTransitions::draw_minimumToPointCount_alwaysTwo()
{
    auto t = TestDrawTransition{};
    // toPointCount==2 is the minimum meaningful dataset. At progress=0,
    // ceil(2*0)=0 but viewportMax(2,0)=2 ensures both points are always copied.
    const auto to = std::vector<double>{1.0f, 2.0f, 3.0f, 4.0f};
    auto out = std::vector<double>{};
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
    auto out = std::vector<double>{};
    auto outCount = int{};

    // advance() without a prior start() must return false immediately.
    const auto stillRunning = t.advance(out, outCount);
    QCOMPARE(stillRunning, false);
}

QTEST_GUILESS_MAIN(TestTransitions)
#include "tst_transitions.moc"
