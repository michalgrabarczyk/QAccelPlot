//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/transitions/MorphTransition.hpp"
#include "QAccelPlot/transitions/TransitionRunner.hpp"

#include <QQuickItem>
#include <QQuickWindow>
#include <QThread>
#include <QtTest/QtTest>

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

    QVERIFY(runner.setTransition(&morph));
    QVERIFY(!runner.setTransition(&morph));
    QVERIFY(runner.enabled());

    morph.setEnabled(false);
    QVERIFY(!runner.enabled());
}

void TransitionRunnerTest::frameDueAdvancesTheRunToItsTarget()
{
    auto morph = MorphTransition{};
    morph.setDuration(50);
    auto runner = TransitionRunner{item_.get()};
    runner.setTransition(&morph);
    showFrames(runner);
    auto frameThreads = QList<QThread*>{};
    connect(&runner, &TransitionRunner::frameDue, this, [&frameThreads]() { frameThreads.append(QThread::currentThread()); }, Qt::DirectConnection);

    runner.start(data_, pointCount_, {10.0, 20.0}, 1);
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
    runner->setTransition(&morph);
    showFrames(*runner);

    runner->start(data_, pointCount_, {10.0, 20.0}, 1);
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
    runner.setTransition(transition.get());
    auto interruptions = 0;
    connect(&runner, &TransitionRunner::interrupted, this, [this, &runner, &interruptions]() {
        ++interruptions;
        QVERIFY(runner.finish(data_, pointCount_));
    });
    runner.start(data_, pointCount_, {10.0, 20.0}, 1);

    switch (interruption) {
    case Interruption::Detach:
        runner.setTransition(nullptr);
        break;
    case Interruption::Replace:
        runner.setTransition(&replacement);
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
    runner.setTransition(&morph);
    runner.start(data_, pointCount_, {10.0, 20.0}, 1);

    morph.cancel();

    QVERIFY(!runner.pending());
    QVERIFY(!runner.finish(data_, pointCount_));
    QCOMPARE(data_, (std::vector<double>{0.0, 0.0}));
}

void TransitionRunnerTest::destroyedTransitionIsReported()
{
    auto morph = std::make_unique<MorphTransition>();
    auto runner = TransitionRunner{item_.get()};
    runner.setTransition(morph.get());
    auto destroyed = QSignalSpy{&runner, &TransitionRunner::transitionDestroyed};

    morph.reset();

    QCOMPARE(destroyed.count(), 1);
    QVERIFY(!runner.transition());
    QVERIFY(!runner.enabled());
}

void TransitionRunnerTest::destroyedRunnerEndsItsRun()
{
    auto morph = MorphTransition{};
    morph.setDuration(kLongDuration);
    auto runner = std::make_unique<TransitionRunner>(item_.get());
    runner->setTransition(&morph);
    runner->start(data_, pointCount_, {10.0, 20.0}, 1);
    QVERIFY(morph.running());

    runner.reset();

    QVERIFY(!morph.running());
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
