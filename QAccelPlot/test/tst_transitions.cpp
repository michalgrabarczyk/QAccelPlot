//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/transitions/DrawTransition.hpp"
#include "QAccelPlot/transitions/GrowTransition.hpp"
#include "QAccelPlot/transitions/MorphTransition.hpp"
#include "QAccelPlot/transitions/internal/TransitionFrame.hpp"

#include <QAnimationDriver>
#include <QtTest/QtTest>

#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <vector>

Q_DECLARE_METATYPE(std::optional<QAccelPlot::Internal::TransitionKind>)

using QAccelPlot::Internal::ItemFade;
using QAccelPlot::Internal::TransitionKind;
using QAccelPlot::Internal::TransitionLayout;
using QAccelPlot::Internal::TransitionStep;

namespace {

constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();

TransitionStep step(const TransitionKind kind, const double progress, const double stagger = 0.0)
{
    return TransitionStep{kind, progress, stagger, QEasingCurve{QEasingCurve::Linear}};
}

// One frame of a change from `from` to `to`, `stride` values per item.
struct Frame {
    std::vector<double> values;
    int count{0};
    ItemFade fade;
};

struct Change {
    std::vector<double> from;
    std::vector<double> to;
    TransitionLayout layout;
    std::optional<TransitionStep> enter;
    std::optional<TransitionStep> update;
    std::optional<TransitionStep> exit;

    Frame frame() const
    {
        const auto stride = layout.stride;
        auto input = QAccelPlot::Internal::TransitionFrameInput{
            {from.data(), static_cast<int>(from.size()) / stride}, {to.data(), static_cast<int>(to.size()) / stride}, layout, enter, update, exit};
        auto result = Frame{};
        result.count = QAccelPlot::Internal::writeTransitionFrame(input, result.values, result.fade);
        return result;
    }
};

} // namespace

class TestTransitions : public QObject {
    Q_OBJECT

private slots:
    // Per-item progress
    void itemProgress_data();
    void itemProgress();
    void easingSamples_spanTheCurve();

    // Slot planning
    void plannedSlots_data();
    void plannedSlots();

    // Kept items
    void morph_interpolatesKeptItems_data();
    void morph_interpolatesKeptItems();
    void morph_modernEpochData_preservesSubFloatPrecision();
    void morph_staggerMovesTheFirstItemFirst();
    void morph_invalidValues_jumpToTheTarget();
    void keptItems_withoutUpdate_jumpToTheTarget();

    // Added items
    void addedItems_withoutEnter_appearAtOnce();
    void draw_revealsAddedItems_data();
    void draw_revealsAddedItems();
    void draw_keepsTheFirstItemOfAWholeSeries();
    void draw_revealsTheLastSegmentPartway();
    void draw_staggerBelowOne_revealsByDelay();
    void grow_raisesAddedItemsFromTheBaseline();
    void grow_usesItsOwnBaseline();
    void grow_leavesInvalidValues();

    // Removed items
    void removedItems_withoutExit_disappear();
    void draw_exit_hidesTheLastItemFirst();
    void grow_exit_shrinksToTheBaseline();
    void grow_exit_staggersFromTheLastItem();

    // Redraw
    void drawAsUpdate_redrawsEveryItem();
    void growAsUpdate_growsEveryItem();

    // Fade
    void fade_describesTheEnteringGroup();
    void fade_isOffWithoutStagger();
    void fade_ofAnExitPlaysBackwards();

    // DataTransition state
    void transition_startSetsRunning();
    void transition_cancelEndsEveryRun();
    void transition_runsEndIndependently();
    void transition_destroyedRunEndsItsAnimation();
    void transition_startingRunOnAnotherTransitionEndsTheFirst();
    void transition_destructionEndsItsRuns();
    void transition_progressFollowsTheDuration();
    void transition_zeroDuration_isComplete();
    void transition_staggerIsClamped();
    void transition_defaults();
    void grow_baselineResets();

    // Animation clock
    void transition_progress_followsTheAnimationDriver();
};

void TestTransitions::itemProgress_data()
{
    QTest::addColumn<double>("progress");
    QTest::addColumn<int>("item");
    QTest::addColumn<int>("itemCount");
    QTest::addColumn<double>("stagger");
    QTest::addColumn<double>("expected");

    QTest::newRow("no stagger") << 0.25 << 3 << 5 << 0.0 << 0.25;
    QTest::newRow("single item") << 0.25 << 0 << 1 << 0.5 << 0.25;
    QTest::newRow("first item starts at once") << 0.25 << 0 << 3 << 0.5 << 0.5;
    QTest::newRow("middle item waits") << 0.25 << 1 << 3 << 0.5 << 0.0;
    QTest::newRow("middle item halfway") << 0.5 << 1 << 3 << 0.5 << 0.5;
    QTest::newRow("last item ends with the run") << 1.0 << 2 << 3 << 0.5 << 1.0;
    QTest::newRow("full stagger before the delay") << 0.4 << 1 << 3 << 1.0 << 0.0;
    QTest::newRow("full stagger at the delay") << 0.5 << 1 << 3 << 1.0 << 1.0;
    QTest::newRow("progress clamped") << 1.5 << 0 << 3 << 0.0 << 1.0;
}

void TestTransitions::itemProgress()
{
    QFETCH(double, progress);
    QFETCH(int, item);
    QFETCH(int, itemCount);
    QFETCH(double, stagger);
    QFETCH(double, expected);

    QCOMPARE(QAccelPlot::Internal::itemProgress(progress, item, itemCount, stagger), expected);
}

void TestTransitions::easingSamples_spanTheCurve()
{
    const auto samples = QAccelPlot::Internal::easingSamples(QEasingCurve{QEasingCurve::InQuad});

    QCOMPARE(samples.front(), 0.0f);
    QCOMPARE(samples.back(), 1.0f);
    QVERIFY(samples[7] < 7.0f / 15.0f);
}

void TestTransitions::plannedSlots_data()
{
    using Kind = std::optional<TransitionKind>;
    QTest::addColumn<int>("fromCount");
    QTest::addColumn<int>("toCount");
    QTest::addColumn<Kind>("enter");
    QTest::addColumn<Kind>("update");
    QTest::addColumn<Kind>("exit");
    QTest::addColumn<bool>("expectEnter");
    QTest::addColumn<bool>("expectUpdate");
    QTest::addColumn<bool>("expectExit");

    const auto draw = Kind{TransitionKind::Draw};
    const auto morph = Kind{TransitionKind::Morph};
    const auto grow = Kind{TransitionKind::Grow};
    const auto none = Kind{};
    QTest::newRow("first data enters") << 0 << 4 << draw << morph << grow << true << false << false;
    QTest::newRow("same count updates") << 4 << 4 << draw << morph << grow << false << true << false;
    QTest::newRow("more items update and enter") << 2 << 4 << draw << morph << grow << true << true << false;
    QTest::newRow("fewer items update and exit") << 4 << 2 << draw << morph << grow << false << true << true;
    QTest::newRow("clearing exits") << 4 << 0 << draw << morph << grow << false << false << true;
    QTest::newRow("draw update redraws") << 2 << 4 << draw << draw << grow << false << true << false;
    QTest::newRow("draw update on first data") << 0 << 4 << none << draw << none << false << false << false;
    QTest::newRow("no transitions") << 2 << 4 << none << none << none << false << false << false;
}

void TestTransitions::plannedSlots()
{
    using Kind = std::optional<TransitionKind>;
    QFETCH(int, fromCount);
    QFETCH(int, toCount);
    QFETCH(Kind, enter);
    QFETCH(Kind, update);
    QFETCH(Kind, exit);
    QFETCH(bool, expectEnter);
    QFETCH(bool, expectUpdate);
    QFETCH(bool, expectExit);

    const auto planned = QAccelPlot::Internal::plannedSlots(fromCount, toCount, enter, update, exit);

    QCOMPARE(planned.enter, expectEnter);
    QCOMPARE(planned.update, expectUpdate);
    QCOMPARE(planned.exit, expectExit);
}

void TestTransitions::morph_interpolatesKeptItems_data()
{
    QTest::addColumn<double>("progress");

    QTest::newRow("start") << 0.0;
    QTest::newRow("midpoint") << 0.5;
    QTest::newRow("end") << 1.0;
}

void TestTransitions::morph_interpolatesKeptItems()
{
    QFETCH(double, progress);
    auto change = Change{{1.0, 2.0, 3.0, 4.0}, {9.0, 8.0, 7.0, 6.0}};
    change.update = step(TransitionKind::Morph, progress);

    const auto frame = change.frame();

    QCOMPARE(frame.count, 2);
    for (auto index = std::size_t{0}; index < frame.values.size(); ++index) {
        QCOMPARE(frame.values[index], change.from[index] + progress * (change.to[index] - change.from[index]));
    }
}

void TestTransitions::morph_modernEpochData_preservesSubFloatPrecision()
{
    constexpr auto epochMilliseconds = double{1'789'032'600'000.0};
    auto change = Change{{epochMilliseconds, 0.0}, {epochMilliseconds + 1.0, 2.0}};
    change.update = step(TransitionKind::Morph, 0.5);

    const auto frame = change.frame();

    QCOMPARE(frame.values, (std::vector<double>{epochMilliseconds + 0.5, 1.0}));
}

void TestTransitions::morph_staggerMovesTheFirstItemFirst()
{
    auto change = Change{{0.0, 0.0, 1.0, 0.0, 2.0, 0.0}, {0.0, 10.0, 1.0, 10.0, 2.0, 10.0}};
    change.update = step(TransitionKind::Morph, 0.25, 0.5);

    const auto frame = change.frame();

    QCOMPARE(frame.count, 3);
    QCOMPARE(frame.values[1], 5.0);
    QCOMPARE(frame.values[3], 0.0);
    QCOMPARE(frame.values[5], 0.0);
}

void TestTransitions::morph_invalidValues_jumpToTheTarget()
{
    const auto inf = std::numeric_limits<double>::infinity();
    auto change = Change{{0.0, 0.0, kNaN, 1.0}, {10.0, kNaN, inf, 11.0}};
    change.update = step(TransitionKind::Morph, 0.1);

    const auto frame = change.frame();

    QCOMPARE(frame.values[0], 1.0);
    QVERIFY(std::isnan(frame.values[1]));
    QCOMPARE(frame.values[2], inf);
    QCOMPARE(frame.values[3], 2.0);
}

void TestTransitions::keptItems_withoutUpdate_jumpToTheTarget()
{
    auto change = Change{{0.0, 0.0, 1.0, 1.0}, {5.0, 5.0, 6.0, 6.0, 7.0, 7.0}};
    change.enter = step(TransitionKind::Grow, 0.5);

    const auto frame = change.frame();

    QCOMPARE(frame.count, 3);
    QCOMPARE(frame.values, (std::vector<double>{5.0, 5.0, 6.0, 6.0, 7.0, 3.5}));
}

void TestTransitions::addedItems_withoutEnter_appearAtOnce()
{
    auto change = Change{{0.0, 0.0}, {10.0, 10.0, 20.0, 20.0, 30.0, 30.0}};
    change.update = step(TransitionKind::Morph, 0.5);

    const auto frame = change.frame();

    QCOMPARE(frame.count, 3);
    QCOMPARE(frame.values, (std::vector<double>{5.0, 5.0, 20.0, 20.0, 30.0, 30.0}));
}

void TestTransitions::draw_revealsAddedItems_data()
{
    QTest::addColumn<double>("progress");
    QTest::addColumn<int>("expectedCount");

    QTest::newRow("start") << 0.0 << 1;
    QTest::newRow("half") << 0.5 << 3;
    QTest::newRow("end") << 1.0 << 5;
}

void TestTransitions::draw_revealsAddedItems()
{
    QFETCH(double, progress);
    QFETCH(int, expectedCount);
    // One kept item, then four added ones.
    auto change = Change{{0.0, 0.0}, {0.0, 0.0, 1.0, 1.0, 2.0, 2.0, 3.0, 3.0, 4.0, 4.0}};
    change.enter = step(TransitionKind::Draw, progress, 1.0);

    const auto frame = change.frame();

    QCOMPARE(frame.count, expectedCount);
    QCOMPARE(frame.values, std::vector<double>(change.to.begin(), change.to.begin() + expectedCount * 2));
}

void TestTransitions::draw_keepsTheFirstItemOfAWholeSeries()
{
    auto change = Change{{}, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0}};
    change.enter = step(TransitionKind::Draw, 0.0, 1.0);

    const auto frame = change.frame();

    QCOMPARE(frame.count, 1);
    QCOMPARE(frame.values, (std::vector<double>{1.0, 2.0}));
}

void TestTransitions::draw_revealsTheLastSegmentPartway()
{
    auto change = Change{{}, {0.0, 0.0, 10.0, 10.0, 20.0, 0.0, 30.0, 10.0}};
    change.layout.continuous = true;
    // The reveal front is one and a half items in: the second item sits halfway along its segment.
    change.enter = step(TransitionKind::Draw, 0.375, 1.0);

    const auto frame = change.frame();

    QCOMPARE(frame.count, 2);
    QCOMPARE(frame.values, (std::vector<double>{0.0, 0.0, 5.0, 5.0}));
}

void TestTransitions::draw_staggerBelowOne_revealsByDelay()
{
    auto change = Change{{}, {0.0, 0.0, 1.0, 1.0, 2.0, 2.0, 3.0, 3.0, 4.0, 4.0}};
    // Delays of 0, 0.125, 0.25, 0.375, and 0.5: three items have started at 0.3.
    change.enter = step(TransitionKind::Draw, 0.3, 0.5);

    const auto frame = change.frame();

    QCOMPARE(frame.count, 3);
}

void TestTransitions::grow_raisesAddedItemsFromTheBaseline()
{
    auto change = Change{{}, {0.0, 10.0, 1.0, 20.0}};
    change.layout.baseline = 2.0;
    change.enter = step(TransitionKind::Grow, 0.5);

    const auto frame = change.frame();

    QCOMPARE(frame.count, 2);
    QCOMPARE(frame.values, (std::vector<double>{0.0, 6.0, 1.0, 11.0}));
}

void TestTransitions::grow_usesItsOwnBaseline()
{
    auto change = Change{{}, {0.0, 10.0}};
    change.layout.baseline = 2.0;
    change.enter = step(TransitionKind::Grow, 0.5);
    change.enter->baseline = -10.0;

    const auto frame = change.frame();

    QCOMPARE(frame.values, (std::vector<double>{0.0, 0.0}));
}

void TestTransitions::grow_leavesInvalidValues()
{
    auto change = Change{{}, {0.0, kNaN}};
    change.enter = step(TransitionKind::Grow, 0.5);

    const auto frame = change.frame();

    QVERIFY(std::isnan(frame.values[1]));
}

void TestTransitions::removedItems_withoutExit_disappear()
{
    auto change = Change{{0.0, 0.0, 1.0, 1.0, 2.0, 2.0}, {5.0, 5.0}};
    change.update = step(TransitionKind::Morph, 0.5);

    const auto frame = change.frame();

    QCOMPARE(frame.count, 1);
    QCOMPARE(frame.values, (std::vector<double>{2.5, 2.5}));
}

void TestTransitions::draw_exit_hidesTheLastItemFirst()
{
    auto change = Change{{0.0, 0.0, 1.0, 1.0, 2.0, 2.0, 3.0, 3.0, 4.0, 4.0}, {0.0, 0.0}};
    change.exit = step(TransitionKind::Draw, 0.5, 1.0);

    const auto frame = change.frame();

    // Two of the four removed items remain after the kept one.
    QCOMPARE(frame.count, 3);
    QCOMPARE(frame.values, (std::vector<double>{0.0, 0.0, 1.0, 1.0, 2.0, 2.0}));
}

void TestTransitions::grow_exit_shrinksToTheBaseline()
{
    auto change = Change{{0.0, 10.0, 1.0, 20.0}, {}};
    change.exit = step(TransitionKind::Grow, 0.25);

    const auto frame = change.frame();

    QCOMPARE(frame.count, 2);
    QCOMPARE(frame.values, (std::vector<double>{0.0, 7.5, 1.0, 15.0}));
}

void TestTransitions::grow_exit_staggersFromTheLastItem()
{
    auto change = Change{{0.0, 10.0, 1.0, 10.0, 2.0, 10.0}, {}};
    change.exit = step(TransitionKind::Grow, 0.25, 0.5);

    const auto frame = change.frame();

    QCOMPARE(frame.values[1], 10.0);
    QCOMPARE(frame.values[3], 10.0);
    QCOMPARE(frame.values[5], 5.0);
}

void TestTransitions::drawAsUpdate_redrawsEveryItem()
{
    auto change = Change{{9.0, 9.0, 9.0, 9.0}, {0.0, 0.0, 1.0, 1.0, 2.0, 2.0, 3.0, 3.0}};
    change.update = step(TransitionKind::Draw, 0.5, 1.0);

    const auto frame = change.frame();

    QCOMPARE(frame.count, 2);
    QCOMPARE(frame.values, (std::vector<double>{0.0, 0.0, 1.0, 1.0}));
}

void TestTransitions::growAsUpdate_growsEveryItem()
{
    auto change = Change{{9.0, 9.0}, {0.0, 10.0, 1.0, 20.0}};
    change.update = step(TransitionKind::Grow, 0.5);

    const auto frame = change.frame();

    QCOMPARE(frame.values, (std::vector<double>{0.0, 5.0, 1.0, 10.0}));
}

void TestTransitions::fade_describesTheEnteringGroup()
{
    auto change = Change{{0.0, 0.0}, {0.0, 0.0, 1.0, 1.0, 2.0, 2.0}};
    change.enter = step(TransitionKind::Draw, 0.25, 0.5);
    change.enter->fade = true;

    const auto frame = change.frame();

    QCOMPARE(frame.fade.first, 1);
    QCOMPARE(frame.fade.count, 2);
    QCOMPARE(frame.fade.progress, 0.25f);
    QCOMPARE(frame.fade.stagger, 0.5f);
    QVERIFY(!frame.fade.exit);
}

void TestTransitions::fade_isOffWithoutStagger()
{
    auto change = Change{{}, {0.0, 0.0, 1.0, 1.0}};
    change.enter = step(TransitionKind::Draw, 0.25, 1.0);
    change.enter->fade = true;

    QVERIFY(!change.frame().fade.active());
}

void TestTransitions::fade_ofAnExitPlaysBackwards()
{
    auto change = Change{{0.0, 0.0, 1.0, 1.0}, {}};
    change.exit = step(TransitionKind::Grow, 0.25);
    change.exit->fade = true;

    const auto frame = change.frame();

    QCOMPARE(frame.fade.first, 0);
    QCOMPARE(frame.fade.count, 2);
    QVERIFY(frame.fade.exit);
}

void TestTransitions::transition_startSetsRunning()
{
    auto transition = QAccelPlot::MorphTransition{};
    auto run = QAccelPlot::DataTransition::Run{};
    QVERIFY(!transition.running());

    transition.start(run);

    QVERIFY(transition.running());
    QVERIFY(run.active());
    QCOMPARE(run.transition(), &transition);
}

void TestTransitions::transition_cancelEndsEveryRun()
{
    auto transition = QAccelPlot::MorphTransition{};
    auto first = QAccelPlot::DataTransition::Run{};
    auto second = QAccelPlot::DataTransition::Run{};
    transition.start(first);
    transition.start(second);

    transition.cancel();

    QVERIFY(!transition.running());
    QVERIFY(!first.active());
    QVERIFY(!second.active());
    QCOMPARE(transition.progress(first), 1.0);
}

void TestTransitions::transition_runsEndIndependently()
{
    auto transition = QAccelPlot::MorphTransition{};
    auto first = QAccelPlot::DataTransition::Run{};
    auto second = QAccelPlot::DataTransition::Run{};
    transition.start(first);
    transition.start(second);

    first.end();

    QVERIFY(transition.running());
    second.end();
    QVERIFY(!transition.running());
}

void TestTransitions::transition_destroyedRunEndsItsAnimation()
{
    auto transition = QAccelPlot::MorphTransition{};
    {
        auto run = QAccelPlot::DataTransition::Run{};
        transition.start(run);
        QVERIFY(transition.running());
    }
    QVERIFY(!transition.running());
}

void TestTransitions::transition_startingRunOnAnotherTransitionEndsTheFirst()
{
    auto first = QAccelPlot::MorphTransition{};
    auto second = QAccelPlot::DrawTransition{};
    auto run = QAccelPlot::DataTransition::Run{};
    first.start(run);

    second.start(run);

    QVERIFY(!first.running());
    QVERIFY(second.running());
    QCOMPARE(run.transition(), &second);
}

void TestTransitions::transition_destructionEndsItsRuns()
{
    auto run = QAccelPlot::DataTransition::Run{};
    auto transition = std::make_unique<QAccelPlot::MorphTransition>();
    transition->start(run);

    transition.reset();

    QVERIFY(!run.active());
    QCOMPARE(run.transition(), nullptr);
}

void TestTransitions::transition_progressFollowsTheDuration()
{
    auto transition = QAccelPlot::MorphTransition{};
    transition.setDuration(10'000);
    auto run = QAccelPlot::DataTransition::Run{};
    transition.start(run);

    QVERIFY(transition.progress(run) < 1.0);
    transition.setDuration(0);
    QCOMPARE(transition.progress(run), 1.0);
}

void TestTransitions::transition_zeroDuration_isComplete()
{
    auto transition = QAccelPlot::GrowTransition{};
    transition.setDuration(0);
    auto run = QAccelPlot::DataTransition::Run{};
    transition.start(run);

    QCOMPARE(transition.progress(run), 1.0);
}

void TestTransitions::transition_staggerIsClamped()
{
    auto transition = QAccelPlot::MorphTransition{};
    auto spy = QSignalSpy{&transition, &QAccelPlot::DataTransition::staggerChanged};

    transition.setStagger(2.0);
    QCOMPARE(transition.stagger(), 1.0);
    transition.setStagger(-1.0);
    QCOMPARE(transition.stagger(), 0.0);
    transition.setStagger(0.0);
    QCOMPARE(spy.count(), 2);
}

void TestTransitions::transition_defaults()
{
    QCOMPARE(QAccelPlot::MorphTransition{}.stagger(), 0.0);
    QCOMPARE(QAccelPlot::GrowTransition{}.stagger(), 0.0);
    QCOMPARE(QAccelPlot::DrawTransition{}.stagger(), 1.0);
    QVERIFY(!QAccelPlot::DrawTransition{}.fade());
    QVERIFY(std::isnan(QAccelPlot::GrowTransition{}.baseline()));
    QCOMPARE(QAccelPlot::MorphTransition{}.kind(), TransitionKind::Morph);
    QCOMPARE(QAccelPlot::DrawTransition{}.kind(), TransitionKind::Draw);
    QCOMPARE(QAccelPlot::GrowTransition{}.kind(), TransitionKind::Grow);
}

void TestTransitions::grow_baselineResets()
{
    auto transition = QAccelPlot::GrowTransition{};
    auto spy = QSignalSpy{&transition, &QAccelPlot::GrowTransition::baselineChanged};

    transition.setBaseline(3.0);
    QCOMPARE(transition.baseline(), 3.0);
    transition.resetBaseline();
    QVERIFY(std::isnan(transition.baseline()));
    transition.setBaseline(kNaN);
    QCOMPARE(spy.count(), 2);
}

// Animation time that moves only when stepped, as in a frame recorder.
class SteppedAnimationDriver : public QAnimationDriver {
public:
    qint64 elapsed() const override
    {
        return elapsed_;
    }

    void step(const qint64 milliseconds)
    {
        elapsed_ += milliseconds;
        advance();
    }

protected:
    void start() override
    {
        elapsed_ = 0;
        QAnimationDriver::start();
    }

private:
    qint64 elapsed_{0};
};

void TestTransitions::transition_progress_followsTheAnimationDriver()
{
    auto driver = SteppedAnimationDriver{};
    driver.install();
    auto transition = QAccelPlot::MorphTransition{};
    auto run = QAccelPlot::DataTransition::Run{};
    transition.setDuration(300);
    transition.start(run);
    // Qt starts its animation timer through queued calls, and the first tick takes in the wall
    // time they took. Restarting the run after that tick leaves the run only the stepped time.
    QTRY_VERIFY(driver.isRunning());
    driver.step(0);
    transition.start(run);

    QTest::qWait(transition.duration() + 100);
    QCOMPARE(transition.progress(run), 0.0);
    driver.step(100);
    QCOMPARE(transition.progress(run), 1.0 / 3.0);
    driver.step(200);
    QCOMPARE(transition.progress(run), 1.0);
}

QTEST_GUILESS_MAIN(TestTransitions)
#include "tst_transitions.moc"
