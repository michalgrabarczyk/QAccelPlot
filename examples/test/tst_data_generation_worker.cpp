//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "DataGenerationWorker.hpp"

#include <QtTest/QtTest>

#include <algorithm>
#include <cmath>

namespace {

bool waitForBatch(DataGenerationWorker& worker, DataGenerationBatch& batch)
{
    constexpr auto timeoutMs = 5'000;
    auto elapsed = QElapsedTimer{};
    elapsed.start();
    while (elapsed.elapsed() < timeoutMs) {
        if (worker.tryConsume(batch)) {
            return true;
        }
        QTest::qWait(1);
    }
    return false;
}

} // namespace

class DataGenerationWorkerTest : public QObject {
    Q_OBJECT

private slots:
    void lookupTracksSine();
    void rectanglesTrackCurveSamples();
    void pendingBatchIsNotOverwritten();
};

void DataGenerationWorkerTest::lookupTracksSine()
{
    auto config = DataGenerationConfig{};
    config.defaultPointCount = 10'001;
    config.xDomainMainCurves = 1000.0f;
    config.signalAngularFrequency = 0.1f;
    config.curve1Amplitude = 8.0f;

    auto worker = DataGenerationWorker{config};
    constexpr auto phase = 1.234;
    worker.setPhase(phase);
    worker.start();

    auto batch = DataGenerationBatch{};
    QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for generated curve data");
    worker.stop();

    QCOMPARE(batch.pointCount, config.defaultPointCount);
    QCOMPARE(batch.curve1.size(), static_cast<std::size_t>(config.defaultPointCount) * 2);

    auto maximumError = 0.0f;
    for (auto i = 0; i < batch.pointCount; ++i) {
        const auto offset = static_cast<std::size_t>(i) * 2;
        const auto x = batch.curve1[offset];
        const auto expected = config.curve1Amplitude * std::sin(config.signalAngularFrequency * x + static_cast<float>(phase));
        maximumError = std::max(maximumError, std::abs(batch.curve1[offset + 1] - expected));
    }
    QVERIFY2(maximumError < 0.002f, qPrintable(QStringLiteral("Maximum sine lookup error was %1").arg(maximumError, 0, 'g', 8)));
}

void DataGenerationWorkerTest::rectanglesTrackCurveSamples()
{
    auto config = DataGenerationConfig{};
    config.defaultPointCount = 257;

    auto worker = DataGenerationWorker{config};
    worker.setPointCount(config.defaultPointCount);
    worker.setRectangleCount(config.defaultPointCount);
    worker.setRectangleTestMode(true);
    worker.setPhase(0.75);
    worker.start();

    auto batch = DataGenerationBatch{};
    QVERIFY2(waitForBatch(worker, batch), "Timed out waiting for generated rectangle data");
    worker.stop();

    QCOMPARE(batch.rectangleCount, batch.pointCount);
    QCOMPARE(batch.rects.size(), static_cast<std::size_t>(batch.rectangleCount) * 4);
    for (auto i = 0; i < batch.rectangleCount; ++i) {
        const auto curveOffset = static_cast<std::size_t>(i) * 2;
        const auto rectangleOffset = static_cast<std::size_t>(i) * 4;
        const auto rectangleCenterX = (batch.rects[rectangleOffset] + batch.rects[rectangleOffset + 2]) * 0.5f;
        const auto rectangleCenterY = (batch.rects[rectangleOffset + 1] + batch.rects[rectangleOffset + 3]) * 0.5f;
        QCOMPARE(rectangleCenterX, batch.curve1[curveOffset]);
        QCOMPARE(rectangleCenterY, batch.curve1[curveOffset + 1]);
    }
}

void DataGenerationWorkerTest::pendingBatchIsNotOverwritten()
{
    auto config = DataGenerationConfig{};
    config.defaultPointCount = 1'000;

    auto worker = DataGenerationWorker{config};
    constexpr auto firstPhase = 0.25;
    constexpr auto secondPhase = 1.25;
    worker.setPhase(firstPhase);
    worker.start();
    QTest::qWait(50);
    worker.setPhase(secondPhase);
    QTest::qWait(50);

    auto firstBatch = DataGenerationBatch{};
    QVERIFY2(worker.tryConsume(firstBatch), "Expected the worker's first pending batch");
    QVERIFY(std::abs(firstBatch.curve1[1] - config.curve1Amplitude * std::sin(firstPhase)) < 0.003f);

    auto secondBatch = DataGenerationBatch{};
    QVERIFY2(waitForBatch(worker, secondBatch), "Timed out waiting for the next batch after consumption");
    worker.stop();
    QVERIFY(std::abs(secondBatch.curve1[1] - config.curve1Amplitude * std::sin(secondPhase)) < 0.003f);
}

QTEST_MAIN(DataGenerationWorkerTest)
#include "tst_data_generation_worker.moc"
