//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "BenchDataWorker.hpp"

#include <cmath>

namespace QAccelPlot {

BenchDataWorker::BenchDataWorker(const int pointCount, const int curveCount)
    : pointCount_(pointCount)
    , curveCount_(curveCount)
{
}

BenchDataWorker::~BenchDataWorker()
{
    stop();
}

void BenchDataWorker::start()
{
    running_ = true;
    thread_ = std::thread(&BenchDataWorker::run, this);
}

void BenchDataWorker::stop()
{
    running_ = false;
    cv_.notify_all();
    if (thread_.joinable()) {
        thread_.join();
    }
}

bool BenchDataWorker::tryConsume(BenchBatch& out)
{
    auto lock = std::unique_lock<std::mutex>{mutex_};
    if (!dataReady_) return false;
    out = std::move(readyBatch_);
    dataReady_ = false;
    lock.unlock();
    cv_.notify_one(); // wake producer to fill the next batch
    return true;
}

void BenchDataWorker::run()
{
    // Pre-allocate once; reuse every iteration.
    auto data        = std::vector<float>(pointCount_ * 2);
    auto vertexCache = std::vector<char>{};

    // Fill x values — they never change.
    for (auto i = 0; i < pointCount_; ++i) {
        data[i * 2] = static_cast<float>(i);
    }

    while (running_) {
        const auto phase = phase_.load(std::memory_order_relaxed);

        // Generate y values (first curve; others share the same data for now).
        for (auto i = 0; i < pointCount_; ++i) {
            data[i * 2 + 1] = std::sin(static_cast<float>(i) * 0.01f + phase);
        }

        // Build vertex cache off the render thread — this is the expensive step.
        renderer_.buildVertexCache(data, pointCount_, vertexCache);

        phase_.store(phase + 0.05f, std::memory_order_relaxed);

        // Hand the batch to the render thread.
        {
            auto lock = std::unique_lock<std::mutex>{mutex_};
            // Wait until the previous batch has been consumed to avoid producing
            // faster than the render thread can consume (which would just waste CPU).
            cv_.wait(lock, [this] { return !dataReady_ || !running_; });
            if (!running_) break;

            readyBatch_.data        = data;          // copy — render thread will move it
            readyBatch_.vertexCache = vertexCache;   // copy — render thread will move it
            readyBatch_.pointCount  = pointCount_;
            dataReady_ = true;
        }
        cv_.notify_one();
    }
}

} // namespace QAccelPlot
