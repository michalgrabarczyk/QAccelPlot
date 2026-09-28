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
#include <future>
#include <memory>
#include <random>
#include <thread>

using QAccelPlotExample::GenerationWorker;

namespace {

std::atomic<int> generatedBatches{0};

// Records its inputs, so tests can tell which settings produced a batch.
struct RecordingGenerator {
    struct Batch {
        int count{0};
        double timeSeconds{0.0};
    };

    void generate(Batch& batch, const int count, const double timeSeconds)
    {
        batch.count = count;
        batch.timeSeconds = timeSeconds;
        generatedBatches.fetch_add(1, std::memory_order_relaxed);
    }
};

using Worker = GenerationWorker<RecordingGenerator>;

bool waitForBatch(Worker& worker, RecordingGenerator::Batch& batch)
{
    if (!worker.waitForData(std::chrono::seconds{5})) {
        return false;
    }
    return worker.tryConsume(batch);
}

} // namespace

class GenerationWorkerTest : public QObject {
    Q_OBJECT

private slots:
    void batchesUseCurrentSettings();
    void pendingBatchIsNotOverwritten();
    void noBatchesAreGeneratedAfterStop();
    void restartsRightAfterRequestStop();
    void stopReturnsAtAnyPointOfTheCycle();
};

void GenerationWorkerTest::batchesUseCurrentSettings()
{
    auto worker = Worker{};
    worker.setCount(123);
    worker.setTime(4.5);
    worker.start();

    auto batch = RecordingGenerator::Batch{};
    QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for a batch");
    worker.stop();

    QCOMPARE(batch.count, 123);
    QCOMPARE(batch.timeSeconds, 4.5);
}

void GenerationWorkerTest::pendingBatchIsNotOverwritten()
{
    auto worker = Worker{};
    worker.setTime(1.0);
    worker.start();
    QVERIFY2(worker.waitForData(std::chrono::seconds{5}), "Timed out waiting for the first pending batch");
    worker.setTime(2.0);

    auto firstBatch = RecordingGenerator::Batch{};
    QVERIFY2(worker.tryConsume(firstBatch), "Expected the worker's first pending batch");
    QCOMPARE(firstBatch.timeSeconds, 1.0);

    auto secondBatch = RecordingGenerator::Batch{};
    QVERIFY2(waitForBatch(worker, secondBatch), "Timed out waiting for the next batch after consumption");
    worker.stop();
    QCOMPARE(secondBatch.timeSeconds, 2.0);
}

void GenerationWorkerTest::noBatchesAreGeneratedAfterStop()
{
    auto worker = Worker{};
    worker.start();
    auto batch = RecordingGenerator::Batch{};
    QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for a batch");
    worker.stop();
    QVERIFY(!worker.isRunning());

    // The batch generated before stop() may still be pending; nothing follows it.
    (void)worker.tryConsume(batch);
    const auto generatedAtStop = generatedBatches.load();
    QVERIFY(!worker.waitForData(std::chrono::milliseconds{50}));
    QVERIFY(!worker.tryConsume(batch));
    QCOMPARE(generatedBatches.load(), generatedAtStop);
}

void GenerationWorkerTest::restartsRightAfterRequestStop()
{
    auto worker = Worker{};
    worker.start();
    worker.requestStop();
    worker.setTime(3.0);
    worker.start();
    QVERIFY(worker.isRunning());

    // Drain batches generated before the restart until one carries the new time.
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
