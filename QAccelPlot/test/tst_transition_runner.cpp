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
#include "QAccelPlot/transitions/TransitionRunner.hpp"

#include <QQuickItem>
#include <QQuickWindow>
#include <QThread>
#include <QtTest/QtTest>

#include <limits>
#include <memory>
#include <vector>

namespace QAccelPlot {

namespace {

// Long enough that the tests always interrupt the animation part way.
constexpr auto kLongDuration = 10'000;

} // namespace

class TransitionRunnerTest : public QObject {
    Q_OBJECT

public:
    enum class Interruption { Detach, Replace, Cancel, Destroy };
    Q_ENUM(Interruption)

private slots:
    void init();
    void cleanup();
    void enabledFollowsTheTransition();
    void frameDueAdvancesTheRunToItsTarget();
    void runStartedBeforeTheItemHasAWindowAdvancesInIt();
    void interruptionLeavesTheTargetDataToFinish_data();
    void interruptionLeavesTheTargetDataToFinish();
    void unhandledInterruptionCancelsTheRun();
    void destroyedTransitionIsReported();
    void destroyedRunnerEndsItsRun();
    void enterAndExitSlotsTakeEnterExitTransitions();
    void changeWithoutAnimatedItemsDoesNotStart();
    void activeChangedFollowsTheRun();
    void restartDropsItemsStillExiting();
    void restartEntersItemsNotShownYet();
    void restartKeepsTheTransitionRunning();
    void runEndsWhenTheLongestSlotEnds();

private:
    // Shows each frame of \a runner in data_.
    void showFrames(TransitionRunner& runner);

    std::unique_ptr<QQuickWindow> window_;
    std::unique_ptr<QQuickItem> item_;
    std::vector<double> data_;
    int pointCount_{0};
    int frameCount_{0};
};

void TransitionRunnerTest::init()
{
    window_ = std::make_unique<QQuickWindow>();
    window_->resize(100, 100);
    item_ = std::make_unique<QQuickItem>();
    item_->setParentItem(window_->contentItem());
    window_->show();
    QVERIFY(QTest::qWaitForWindowExposed(window_.get()));
    data_ = {0.0, 0.0};
    pointCount_ = 1;
    frameCount_ = 0;
}

void TransitionRunnerTest::cleanup()
{
    item_.reset();
    window_.reset();
}

void TransitionRunnerTest::enabledFollowsTheTransition()
{
    auto morph = MorphTransition{};
    auto runner = TransitionRunner{item_.get()};
    QVERIFY(!runner.enabled());

    QVERIFY(runner.setTransition(TransitionRunner::Slot::Update, &morph));
    QVERIFY(!runner.setTransition(TransitionRunner::Slot::Update, &morph));
    QVERIFY(runner.enabled());

    morph.setEnabled(false);
    QVERIFY(!runner.enabled());
}

void TransitionRunnerTest::frameDueAdvancesTheRunToItsTarget()
{
    auto morph = MorphTransition{};
    morph.setDuration(50);
    auto runner = TransitionRunner{item_.get()};
    runner.setTransition(TransitionRunner::Slot::Update, &morph);
    showFrames(runner);
    auto frameThreads = QList<QThread*>{};
    connect(&runner, &TransitionRunner::frameDue, this, [&frameThreads]() { frameThreads.append(QThread::currentThread()); }, Qt::DirectConnection);

    runner.start(data_, pointCount_, {10.0, 20.0}, 1, TransitionRunner::Layout{});
    QVERIFY(runner.active());
    QCOMPARE(runner.targetData(), (std::vector<double>{10.0, 20.0}));
    QCOMPARE(runner.targetPointCount(), 1);
    QTRY_VERIFY_WITH_TIMEOUT(!morph.running(), 2000);

    QVERIFY(frameCount_ > 0);
    QVERIFY(!runner.pending());
    QCOMPARE(data_, (std::vector<double>{10.0, 20.0}));
    for (const auto* frameThread : std::as_const(frameThreads)) {
        QCOMPARE(frameThread, QThread::currentThread());
    }
}

void TransitionRunnerTest::runStartedBeforeTheItemHasAWindowAdvancesInIt()
{
    auto morph = MorphTransition{};
    morph.setDuration(50);
    auto item = std::make_unique<QQuickItem>();
    auto runner = std::make_unique<TransitionRunner>(item.get());
    runner->setTransition(TransitionRunner::Slot::Update, &morph);
    showFrames(*runner);

    runner->start(data_, pointCount_, {10.0, 20.0}, 1, TransitionRunner::Layout{});
    item->setParentItem(window_->contentItem());
    QTRY_VERIFY_WITH_TIMEOUT(!morph.running(), 2000);

    QCOMPARE(data_, (std::vector<double>{10.0, 20.0}));
    runner.reset();
}

void TransitionRunnerTest::interruptionLeavesTheTargetDataToFinish_data()
{
    QTest::addColumn<Interruption>("interruption");
    QTest::newRow("detach") << Interruption::Detach;
    QTest::newRow("replace") << Interruption::Replace;
    QTest::newRow("cancel") << Interruption::Cancel;
    QTest::newRow("destroy") << Interruption::Destroy;
}

void TransitionRunnerTest::interruptionLeavesTheTargetDataToFinish()
{
    QFETCH(Interruption, interruption);
    auto transition = std::make_unique<MorphTransition>();
    transition->setDuration(kLongDuration);
    auto replacement = MorphTransition{};
    auto runner = TransitionRunner{item_.get()};
    runner.setTransition(TransitionRunner::Slot::Update, transition.get());
    auto interruptions = 0;
    connect(&runner, &TransitionRunner::interrupted, this, [this, &runner, &interruptions]() {
        ++interruptions;
        QVERIFY(runner.finish(data_, pointCount_));
    });
    runner.start(data_, pointCount_, {10.0, 20.0}, 1, TransitionRunner::Layout{});

    switch (interruption) {
    case Interruption::Detach:
        runner.setTransition(TransitionRunner::Slot::Update, nullptr);
        break;
    case Interruption::Replace:
        runner.setTransition(TransitionRunner::Slot::Update, &replacement);
        break;
    case Interruption::Cancel:
        transition->cancel();
        break;
    case Interruption::Destroy:
        transition.reset();
        break;
    }

    QCOMPARE(interruptions, 1);
    QCOMPARE(data_, (std::vector<double>{10.0, 20.0}));
    QVERIFY(!runner.pending());
    QVERIFY(!transition || !transition->running());
}

void TransitionRunnerTest::unhandledInterruptionCancelsTheRun()
{
    auto morph = MorphTransition{};
    morph.setDuration(kLongDuration);
    auto runner = TransitionRunner{item_.get()};
    runner.setTransition(TransitionRunner::Slot::Update, &morph);
    runner.start(data_, pointCount_, {10.0, 20.0}, 1, TransitionRunner::Layout{});

    morph.cancel();

    QVERIFY(!runner.pending());
    QVERIFY(!runner.finish(data_, pointCount_));
    QCOMPARE(data_, (std::vector<double>{0.0, 0.0}));
}

void TransitionRunnerTest::destroyedTransitionIsReported()
{
    auto morph = std::make_unique<MorphTransition>();
    auto runner = TransitionRunner{item_.get()};
    runner.setTransition(TransitionRunner::Slot::Update, morph.get());
    auto destroyed = QSignalSpy{&runner, &TransitionRunner::transitionDestroyed};

    morph.reset();

    QCOMPARE(destroyed.count(), 1);
    QVERIFY(!runner.transition(TransitionRunner::Slot::Update));
    QVERIFY(!runner.enabled());
}

void TransitionRunnerTest::destroyedRunnerEndsItsRun()
{
    auto morph = MorphTransition{};
    morph.setDuration(kLongDuration);
    auto runner = std::make_unique<TransitionRunner>(item_.get());
    runner->setTransition(TransitionRunner::Slot::Update, &morph);
    runner->start(data_, pointCount_, {10.0, 20.0}, 1, TransitionRunner::Layout{});
    QVERIFY(morph.running());

    runner.reset();

    QVERIFY(!morph.running());
}

void TransitionRunnerTest::enterAndExitSlotsTakeEnterExitTransitions()
{
    auto morph = MorphTransition{};
    auto grow = GrowTransition{};
    auto runner = TransitionRunner{item_.get()};

    QVERIFY(!runner.setTransition(TransitionRunner::Slot::Enter, &morph));
    QVERIFY(!runner.setTransition(TransitionRunner::Slot::Exit, &morph));
    QVERIFY(runner.setTransition(TransitionRunner::Slot::Enter, &grow));
    QVERIFY(runner.setTransition(TransitionRunner::Slot::Exit, &grow));
    QCOMPARE(runner.transition(TransitionRunner::Slot::Enter), &grow);
    QCOMPARE(runner.transition(TransitionRunner::Slot::Exit), &grow);
}

void TransitionRunnerTest::changeWithoutAnimatedItemsDoesNotStart()
{
    auto grow = GrowTransition{};
    auto runner = TransitionRunner{item_.get()};
    runner.setTransition(TransitionRunner::Slot::Enter, &grow);

    // The same number of items: nothing enters, and no update transition is set.
    QVERIFY(!runner.start(data_, pointCount_, {10.0, 20.0}, 1, TransitionRunner::Layout{}));
    QVERIFY(!runner.pending());
    QVERIFY(!grow.running());
}

void TransitionRunnerTest::activeChangedFollowsTheRun()
{
    auto morph = MorphTransition{};
    morph.setDuration(kLongDuration);
    auto runner = TransitionRunner{item_.get()};
    runner.setTransition(TransitionRunner::Slot::Update, &morph);
    auto activeChanges = QSignalSpy{&runner, &TransitionRunner::activeChanged};

    runner.start(data_, pointCount_, {10.0, 20.0}, 1, TransitionRunner::Layout{});
    QVERIFY(runner.active());
    QCOMPARE(activeChanges.count(), 1);
    runner.cancel();
    QVERIFY(!runner.active());
    QCOMPARE(activeChanges.count(), 2);
}

void TransitionRunnerTest::restartDropsItemsStillExiting()
{
    auto grow = GrowTransition{};
    grow.setDuration(kLongDuration);
    auto runner = TransitionRunner{item_.get()};
    runner.setTransition(TransitionRunner::Slot::Enter, &grow);
    runner.setTransition(TransitionRunner::Slot::Exit, &grow);
    auto frame = std::vector<double>{};
    auto frameCount = 0;
    runner.start({0.0, 1.0, 1.0, 1.0, 2.0, 1.0}, 3, {0.0, 1.0}, 1, TransitionRunner::Layout{});
    runner.advance(frame, frameCount);
    QCOMPARE(frameCount, 3);

    // The two exiting items follow the kept one; only the kept one is drawn from now on.
    QVERIFY(runner.start(frame, frameCount, {0.0, 1.0, 1.0, 5.0}, 2, TransitionRunner::Layout{}));
    QCOMPARE(runner.frameCapacity(), 2);
    runner.advance(frame, frameCount);
    QCOMPARE(frameCount, 2);
    QCOMPARE(frame[0], 0.0);
    QCOMPARE(frame[1], 1.0);
}

void TransitionRunnerTest::restartEntersItemsNotShownYet()
{
    auto draw = DrawTransition{};
    draw.setDuration(kLongDuration);
    auto morph = MorphTransition{};
    morph.setDuration(kLongDuration);
    auto runner = TransitionRunner{item_.get()};
    runner.setTransition(TransitionRunner::Slot::Enter, &draw);
    runner.setTransition(TransitionRunner::Slot::Update, &morph);
    auto frame = std::vector<double>{};
    auto frameCount = 0;
    runner.start(data_, pointCount_, {0.0, 0.0, 1.0, 1.0, 2.0, 2.0}, 3, TransitionRunner::Layout{});
    runner.advance(frame, frameCount);
    QCOMPARE(frameCount, 1);

    // A series that pads the frame to the run's capacity passes the unshown items as invalid.
    frame.resize(static_cast<std::size_t>(runner.frameCapacity()) * 2, std::numeric_limits<double>::quiet_NaN());
    QVERIFY(runner.start(frame, runner.frameCapacity(), {0.0, 5.0, 1.0, 1.0, 2.0, 2.0}, 3, TransitionRunner::Layout{}));
    runner.advance(frame, frameCount);
    QCOMPARE(frameCount, 1);
}

void TransitionRunnerTest::restartKeepsTheTransitionRunning()
{
    auto morph = MorphTransition{};
    morph.setDuration(kLongDuration);
    auto draw = DrawTransition{};
    draw.setDuration(kLongDuration);
    auto runner = TransitionRunner{item_.get()};
    runner.setTransition(TransitionRunner::Slot::Update, &morph);
    runner.setTransition(TransitionRunner::Slot::Enter, &draw);
    runner.start(data_, pointCount_, {10.0, 20.0, 30.0, 40.0}, 2, TransitionRunner::Layout{});
    auto morphChanges = QSignalSpy{&morph, &DataTransition::runningChanged};
    auto drawChanges = QSignalSpy{&draw, &DataTransition::runningChanged};

    // The kept item changes again and nothing enters: only the draw transition stops.
    QVERIFY(runner.start(data_, pointCount_, {50.0, 60.0}, 1, TransitionRunner::Layout{}));
    QVERIFY(morph.running());
    QCOMPARE(morphChanges.count(), 0);
    QVERIFY(!draw.running());
    QCOMPARE(drawChanges.count(), 1);
}

void TransitionRunnerTest::runEndsWhenTheLongestSlotEnds()
{
    auto morph = MorphTransition{};
    morph.setDuration(0);
    auto draw = DrawTransition{};
    draw.setDuration(kLongDuration);
    auto runner = TransitionRunner{item_.get()};
    runner.setTransition(TransitionRunner::Slot::Update, &morph);
    runner.setTransition(TransitionRunner::Slot::Enter, &draw);
    runner.start(data_, pointCount_, {10.0, 20.0, 30.0, 40.0}, 2, TransitionRunner::Layout{});

    auto frame = std::vector<double>{};
    auto frameCount = 0;
    QVERIFY(runner.advance(frame, frameCount));
    QCOMPARE(frame[0], 10.0);
    draw.setDuration(0);
    QVERIFY(!runner.advance(frame, frameCount));
    QCOMPARE(frame, (std::vector<double>{10.0, 20.0, 30.0, 40.0}));
    QVERIFY(!runner.pending());
}

void TransitionRunnerTest::showFrames(TransitionRunner& runner)
{
    connect(&runner, &TransitionRunner::frameDue, this, [this, &runner]() {
        ++frameCount_;
        runner.advance(data_, pointCount_);
    });
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::TransitionRunnerTest)
#include "tst_transition_runner.moc"
