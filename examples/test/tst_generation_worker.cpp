//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "GenerationWorker.hpp"

#include <QtTest/QtTest>

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <future>
#include <memory>
#include <random>
#include <thread>
#include <vector>

using QAccelPlotExample::GenerationWorker;
using QAccelPlotExample::ReplayLoop;

namespace {

std::atomic<int> generatedBatches{0};

// Records its inputs, so tests can tell which settings produced a batch.
struct RecordingGenerator {
    struct Parameters {
        int count{1};
        // Size a batch reports, which sets how many frames a replay loop holds; two by default.
        std::size_t frameBytes{std::size_t{1} << 29};
        // Time each batch takes to generate.
        std::chrono::milliseconds delay{0};

        friend bool operator==(const Parameters& a, const Parameters& b)
        {
            return a.count == b.count && a.frameBytes == b.frameBytes && a.delay == b.delay;
        }
    };

    struct Batch {
        int count{0};
        double timeSeconds{0.0};
        std::size_t frameBytes{0};
        // count values on the heap, so a test can tell where a batch's storage went.
        std::vector<int> samples;

        std::size_t byteSize() const
        {
            return frameBytes;
        }
    };

    void generate(Batch& batch, const Parameters& parameters, const double timeSeconds)
    {
        batch.count = parameters.count;
        batch.timeSeconds = timeSeconds;
        batch.frameBytes = parameters.frameBytes;
        batch.samples.assign(static_cast<std::size_t>(parameters.count), parameters.count);
        std::this_thread::sleep_for(parameters.delay);
        generatedBatches.fetch_add(1, std::memory_order_relaxed);
    }
};

using Worker = GenerationWorker<RecordingGenerator>;
using Loop = ReplayLoop<RecordingGenerator>;

constexpr auto kFrameInterval = Loop::kFrameIntervalSeconds;

bool waitForBatch(Worker& worker, RecordingGenerator::Batch& batch)
{
    if (!worker.waitForData(std::chrono::seconds{5})) {
        return false;
    }
    return worker.tryConsume(batch);
}

// Counts the batches the worker copies.
std::shared_ptr<std::atomic<int>> countCopies(Worker& worker)
{
    auto copies = std::make_shared<std::atomic<int>>(0);
    worker.setCopyCallback([copies](const std::chrono::nanoseconds duration) {
        if (duration.count() >= 0) {
            copies->fetch_add(1);
        }
    });
    return copies;
}

} // namespace

class GenerationWorkerTest : public QObject {
    Q_OBJECT

private slots:
    void batchesUseCurrentSettings();
    void replaysFramesForwardThenBackward();
    void generatesTheLoopOnlyOnce();
    void zeroIntervalReplaysOneStillFrame();
    void newSettingsRebuildTheLoop();
    void batchesReportTheSettingsTheyWereBuiltFor();
    void newSettingsAbandonAnUnfinishedLoop();
    void pendingBatchIsNotOverwritten();
    void copiesOneBatchAheadOfThePendingOne();
    void consumingHandsTheBatchStorageBack();
    void reportsTheCopyOfEveryBatch();
    void loopFitsTheMemoryBudget();
    void loopPlaysForwardThenBackward();
    void loopWithoutIntervalHoldsOneFrame();
    void noBatchesAreCopiedAfterStop();
    void restartsRightAfterRequestStop();
    void stopReturnsAtAnyPointOfTheCycle();
};

void GenerationWorkerTest::batchesUseCurrentSettings()
{
    auto worker = Worker{};
    worker.setParameters({123});
    worker.setTime(4.5);
    worker.start();

    auto batch = RecordingGenerator::Batch{};
    QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for a batch");
    worker.stop();

    QCOMPARE(batch.count, 123);
    QCOMPARE(batch.timeSeconds, 4.5);
}

void GenerationWorkerTest::replaysFramesForwardThenBackward()
{
    auto parameters = RecordingGenerator::Parameters{};
    parameters.frameBytes = Loop::kMemoryBudgetBytes / 3;
    auto worker = Worker{};
    worker.setParameters(parameters);
    worker.setTime(5.0);
    worker.start();

    auto times = std::vector<double>{};
    for (auto index = 0; index < 5; ++index) {
        auto batch = RecordingGenerator::Batch{};
        QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for a replayed batch");
        times.push_back(batch.timeSeconds);
    }
    worker.stop();

    QCOMPARE(times, (std::vector<double>{5.0, 5.0 + kFrameInterval, 5.0 + 2 * kFrameInterval, 5.0 + kFrameInterval, 5.0}));
}

void GenerationWorkerTest::generatesTheLoopOnlyOnce()
{
    const auto generatedBefore = generatedBatches.load();
    auto worker = Worker{};
    worker.start();
    auto batch = RecordingGenerator::Batch{};
    for (auto index = 0; index < 6; ++index) {
        QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for a replayed batch");
    }
    worker.stop();
    // The loop holds two frames by default; every batch after them is a copy.
    QCOMPARE(generatedBatches.load() - generatedBefore, 2);
}

void GenerationWorkerTest::zeroIntervalReplaysOneStillFrame()
{
    const auto generatedBefore = generatedBatches.load();
    auto worker = Worker{};
    worker.setFrameInterval(0.0);
    worker.setTime(5.0);
    worker.start();
    auto batch = RecordingGenerator::Batch{};
    for (auto index = 0; index < 3; ++index) {
        QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for a replayed batch");
        QCOMPARE(batch.timeSeconds, 5.0);
    }
    worker.stop();
    QCOMPARE(generatedBatches.load() - generatedBefore, 1);
}

void GenerationWorkerTest::newSettingsRebuildTheLoop()
{
    auto worker = Worker{};
    worker.setParameters({3});
    worker.start();
    auto batch = RecordingGenerator::Batch{};
    QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for a replayed batch");
    QCOMPARE(batch.count, 3);

    worker.setParameters({9});
    // Up to one batch copied for the old settings may come first.
    for (auto attempt = 0; attempt < 3 && batch.count != 9; ++attempt) {
        QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for a batch with the new settings");
    }
    worker.stop();
    QCOMPARE(batch.count, 9);
}

void GenerationWorkerTest::batchesReportTheSettingsTheyWereBuiltFor()
{
    auto worker = Worker{};
    worker.setParameters({3});
    worker.start();
    auto batch = RecordingGenerator::Batch{};
    auto builtFor = RecordingGenerator::Parameters{};
    QVERIFY2(worker.waitForData(std::chrono::seconds{5}) && worker.tryConsume(batch, builtFor), "Timed out waiting for a replayed batch");
    QCOMPARE(builtFor.count, 3);

    worker.setParameters({9});
    for (auto attempt = 0; attempt < 3 && batch.count != 9; ++attempt) {
        QVERIFY2(worker.waitForData(std::chrono::seconds{5}) && worker.tryConsume(batch, builtFor), "Timed out waiting for a batch");
        QCOMPARE(builtFor.count, batch.count);
    }
    worker.stop();
    QCOMPARE(builtFor.count, 9);
}

void GenerationWorkerTest::newSettingsAbandonAnUnfinishedLoop()
{
    auto slow = RecordingGenerator::Parameters{};
    slow.frameBytes = 1;
    slow.delay = std::chrono::milliseconds{20};
    auto worker = Worker{};
    worker.setParameters(slow);
    const auto generatedBefore = generatedBatches.load();
    worker.start();
    // The loop of slow frames would take more than a second to complete.
    QTRY_VERIFY(generatedBatches.load() > generatedBefore);

    worker.setParameters({9});
    auto batch = RecordingGenerator::Batch{};
    QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for a batch with the new settings");
    worker.stop();
    QCOMPARE(batch.count, 9);
    QVERIFY(generatedBatches.load() - generatedBefore < Loop::kMaximumFrameCount);
}

void GenerationWorkerTest::pendingBatchIsNotOverwritten()
{
    auto worker = Worker{};
    worker.setTime(1.0);
    worker.start();
    QVERIFY2(worker.waitForData(std::chrono::seconds{5}), "Timed out waiting for the first pending batch");
    // Leaves the worker time to copy the next batch, which must not replace the pending one.
    std::this_thread::sleep_for(std::chrono::milliseconds{50});

    auto batch = RecordingGenerator::Batch{};
    QVERIFY2(worker.tryConsume(batch), "Expected the worker's first pending batch");
    QCOMPARE(batch.timeSeconds, 1.0);
    QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for the next batch after consumption");
    worker.stop();
    QCOMPARE(batch.timeSeconds, 1.0 + kFrameInterval);
}

void GenerationWorkerTest::copiesOneBatchAheadOfThePendingOne()
{
    auto worker = Worker{};
    const auto copies = countCopies(worker);
    worker.start();
    QVERIFY2(worker.waitForData(std::chrono::seconds{5}), "Timed out waiting for the first pending batch");
    QTRY_COMPARE(copies->load(), 2);
    // Nothing beyond the copied batch while the pending one is not consumed.
    std::this_thread::sleep_for(std::chrono::milliseconds{50});
    QCOMPARE(copies->load(), 2);

    // The copied batch becomes pending as soon as the first one is consumed.
    auto batch = RecordingGenerator::Batch{};
    QVERIFY(worker.tryConsume(batch));
    QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for the copied batch");
    worker.stop();
}

void GenerationWorkerTest::consumingHandsTheBatchStorageBack()
{
    auto worker = Worker{};
    worker.setParameters({1'000});
    worker.start();
    auto batch = RecordingGenerator::Batch{};
    QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for a batch");
    const auto* storage = batch.samples.data();

    // The next consume hands that batch back; the worker copies a later frame into its storage.
    auto reused = false;
    for (auto attempt = 0; attempt < 4 && !reused; ++attempt) {
        QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for a batch");
        reused = attempt > 0 && batch.samples.data() == storage;
    }
    worker.stop();
    QVERIFY(reused);
}

void GenerationWorkerTest::reportsTheCopyOfEveryBatch()
{
    auto worker = Worker{};
    const auto copies = countCopies(worker);
    worker.start();
    auto batch = RecordingGenerator::Batch{};
    for (auto index = 0; index < 3; ++index) {
        QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for a batch");
    }
    worker.stop();
    // Besides the consumed batches, one may be pending and one copied but not yet published.
    QVERIFY2(copies->load() >= 3 && copies->load() <= 5, qPrintable(QString::number(copies->load())));
}

void GenerationWorkerTest::loopFitsTheMemoryBudget()
{
    QCOMPARE(Loop::frameCountFor(0), Loop::kMaximumFrameCount);
    QCOMPARE(Loop::frameCountFor(Loop::kMemoryBudgetBytes / 5), 5);
    QCOMPARE(Loop::frameCountFor(Loop::kMemoryBudgetBytes / 5 + 1), 4);
    // Even a frame above the budget gets a second one, so the data still moves.
    QCOMPARE(Loop::frameCountFor(Loop::kMemoryBudgetBytes * 2), 2);
}

void GenerationWorkerTest::loopPlaysForwardThenBackward()
{
    auto generator = RecordingGenerator{};
    auto parameters = RecordingGenerator::Parameters{};
    parameters.frameBytes = Loop::kMemoryBudgetBytes / 3;
    auto loop = Loop{};
    QVERIFY(!loop.holds(parameters));
    QVERIFY(loop.build(generator, parameters, 1.0, kFrameInterval, []() { return false; }));
    QVERIFY(loop.holds(parameters));
    QCOMPARE(loop.frameCount(), 3);

    auto frames = std::vector<int>{};
    auto batch = RecordingGenerator::Batch{};
    for (auto step = 0; step < 7; ++step) {
        loop.next(batch);
        frames.push_back(static_cast<int>(std::lround((batch.timeSeconds - 1.0) / kFrameInterval)));
    }
    QCOMPARE(frames, (std::vector<int>{0, 1, 2, 1, 0, 1, 2}));

    // A stop request abandons the build and empties the loop.
    QVERIFY(!loop.build(generator, parameters, 1.0, kFrameInterval, []() { return true; }));
    QCOMPARE(loop.frameCount(), 0);
    QVERIFY(!loop.holds(parameters));
}

void GenerationWorkerTest::loopWithoutIntervalHoldsOneFrame()
{
    auto generator = RecordingGenerator{};
    auto loop = Loop{};
    QVERIFY(loop.build(generator, {}, 2.0, 0.0, []() { return false; }));
    QCOMPARE(loop.frameCount(), 1);
    auto batch = RecordingGenerator::Batch{};
    for (auto step = 0; step < 3; ++step) {
        loop.next(batch);
        QCOMPARE(batch.timeSeconds, 2.0);
    }
}

void GenerationWorkerTest::noBatchesAreCopiedAfterStop()
{
    auto worker = Worker{};
    const auto copies = countCopies(worker);
    worker.start();
    auto batch = RecordingGenerator::Batch{};
    QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for a batch");
    worker.stop();
    QVERIFY(!worker.isRunning());

    // The batch copied before stop() may still be pending; nothing follows it.
    (void)worker.tryConsume(batch);
    const auto copiesAtStop = copies->load();
    QVERIFY(!worker.waitForData(std::chrono::milliseconds{50}));
    QVERIFY(!worker.tryConsume(batch));
    QCOMPARE(copies->load(), copiesAtStop);
}

void GenerationWorkerTest::restartsRightAfterRequestStop()
{
    auto worker = Worker{};
    worker.start();
    worker.requestStop();
    worker.setTime(3.0);
    worker.start();
    QVERIFY(worker.isRunning());

    // Drain batches copied before the restart until one comes from the loop built at the new time.
    auto batch = RecordingGenerator::Batch{};
    for (auto attempt = 0; attempt < 3 && batch.timeSeconds != 3.0; ++attempt) {
        QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for a batch after the restart");
    }
    worker.stop();
    QCOMPARE(batch.timeSeconds, 3.0);
}

void GenerationWorkerTest::stopReturnsAtAnyPointOfTheCycle()
{
    // Stops the worker at random moments while it produces its next batch. A wake-up lost by
    // stop() leaves the worker waiting forever, so the loop runs on a detached thread with a deadline.
    auto finished = std::make_shared<std::promise<void>>();
    auto done = finished->get_future();
    std::thread([finished]() {
        auto random = std::mt19937{12345};
        auto delay = std::uniform_int_distribution<int>{0, 4000};
        for (auto iteration = 0; iteration < 40'000; ++iteration) {
            auto worker = Worker{};
            worker.start();
            auto batch = RecordingGenerator::Batch{};
            if (worker.waitForData(std::chrono::seconds{5})) {
                // Consuming wakes the worker to produce the next batch.
                worker.tryConsume(batch);
            }
            const auto spins = delay(random);
            for (volatile auto spin = 0; spin < spins; spin = spin + 1) { }
            worker.stop();
        }
        finished->set_value();
    }).detach();

    QVERIFY2(done.wait_for(std::chrono::seconds{60}) == std::future_status::ready, "GenerationWorker::stop() did not return");
}

QTEST_GUILESS_MAIN(GenerationWorkerTest)
#include "tst_generation_worker.moc"
