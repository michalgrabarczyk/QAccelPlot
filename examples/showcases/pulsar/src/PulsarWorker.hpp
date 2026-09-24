//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace QAccelPlotExample {

constexpr auto pulsarRidgeCount = 80;
constexpr auto pulsarPointsPerRidge = 1500;

struct PulsarBatch {
    int ridgeCount{pulsarRidgeCount};
    int pointsPerRidge{pulsarPointsPerRidge};
    std::vector<std::vector<float>> ridges;
    float peakX{500.0f};
    float peakY{20.0f};
    float burstOpacity{1.0f};
    int peakRidgeIndex{40};
};

/// Produces one complete pulsar-ridge batch at a time on a background thread.
///
/// At most one completed batch remains pending. The worker waits until that
/// batch is consumed before generating another, avoiding obsolete full-buffer
/// work when the consumer temporarily falls behind.
class PulsarWorker {
public:
    PulsarWorker();
    ~PulsarWorker();

    PulsarWorker(const PulsarWorker&) = delete;
    PulsarWorker& operator=(const PulsarWorker&) = delete;

    void start();
    void stop();

    void setElapsedSeconds(double elapsedSeconds);

    bool tryConsume(PulsarBatch& batch);

    static PulsarBatch computeBatchForPhase(double phase);

private:
    void run();
    static void generateBatch(PulsarBatch& batch, double phase);

    std::mutex mutex_;
    std::condition_variable conditionVariable_;
    std::thread workerThread_;
    PulsarBatch readyBatch_;
    bool dataReady_{false};
    std::atomic<bool> running_{false};
    std::atomic<double> elapsedSeconds_{0.0};
};

} // namespace QAccelPlotExample
