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

struct DataGenerationConfig {
    int defaultPointCount{1000000};
    float xDomainMainCurves{1000.0f};
    float signalAngularFrequency{0.1f};
    float curve1Amplitude{8.0f};
    float rectangleWidth{10.0f};
    float rectangleHeight{0.125f};
};

struct DataGenerationBatch {
    std::vector<float> curve1;
    std::vector<char> curve1VertexCache;
    std::vector<float> rects; // 4 floats per rect (x1, y1, x2, y2)
    int pointCount{0};
    int rectangleCount{0};
};

/// Produces one complete curve-data batch at a time on a background thread.
///
/// At most one completed batch remains pending. The worker waits until that
/// batch is consumed before generating another, avoiding obsolete full-buffer
/// work when the consumer temporarily falls behind.
class DataGenerationWorker {
public:
    explicit DataGenerationWorker(const DataGenerationConfig& config = {});
    ~DataGenerationWorker();

    DataGenerationWorker(const DataGenerationWorker&) = delete;
    DataGenerationWorker& operator=(const DataGenerationWorker&) = delete;

    void start();
    void stop();

    void setPhase(double phase);
    void setPointCount(int pointCount);
    void setRectangleCount(int rectangleCount);
    void setRectangleTestMode(bool enabled);

    bool tryConsume(DataGenerationBatch& batch);

private:
    void run();

    DataGenerationConfig config_;
    std::mutex mutex_;
    std::condition_variable conditionVariable_;
    std::thread workerThread_;
    DataGenerationBatch readyBatch_;
    bool dataReady_{false};
    std::atomic<bool> running_{false};
    std::atomic<double> phase_{0.0};
    std::atomic<int> pointCount_{1000000};
    std::atomic<int> rectangleCount_{1000};
    std::atomic<bool> rectangleTestMode_{false};
};
