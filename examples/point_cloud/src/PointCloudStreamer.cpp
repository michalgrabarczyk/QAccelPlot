//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "PointCloudStreamer.hpp"

#include <QAccelPlot/series/PointCloud.hpp>

#include <chrono>
#include <utility>

namespace QAccelPlotExample {

namespace {
constexpr auto kPhaseStepRadians = 0.012f;
constexpr auto kMinimumFrameInterval = std::chrono::milliseconds{16};
} // namespace

PointCloudStreamer::PointCloudStreamer(QAccelPlot::PointCloud* target)
    : target_(target)
    , thread_(&PointCloudStreamer::run, this)
{
}

PointCloudStreamer::~PointCloudStreamer()
{
    stop();
}

void PointCloudStreamer::setSource(std::shared_ptr<const ClusterCloud> cloud)
{
    {
        const auto lock = std::lock_guard<std::mutex>{mutex_};
        source_ = std::move(cloud);
    }
    wake_.notify_one();
}

void PointCloudStreamer::setRunning(const bool running)
{
    {
        const auto lock = std::lock_guard<std::mutex>{mutex_};
        running_ = running;
    }
    wake_.notify_one();
}

void PointCloudStreamer::notifyFramePresented()
{
    {
        const auto lock = std::lock_guard<std::mutex>{mutex_};
        framePresented_ = true;
    }
    wake_.notify_one();
}

void PointCloudStreamer::stop()
{
    {
        const auto lock = std::lock_guard<std::mutex>{mutex_};
        quit_ = true;
    }
    wake_.notify_one();
    if (thread_.joinable()) {
        thread_.join();
    }
}

bool PointCloudStreamer::waitForWork()
{
    auto lock = std::unique_lock<std::mutex>{mutex_};
    wake_.wait(lock, [this]() { return quit_ || (running_ && framePresented_ && source_); });
    if (quit_) {
        return false;
    }
    framePresented_ = false;
    return true;
}

void PointCloudStreamer::run()
{
    auto frame = PointCloudData{};
    while (waitForWork()) {
        const auto frameStart = std::chrono::steady_clock::now();

        auto source = std::shared_ptr<const ClusterCloud>{};
        {
            const auto lock = std::lock_guard<std::mutex>{mutex_};
            source = source_;
        }

        const auto phase = phase_.load(std::memory_order_relaxed) + kPhaseStepRadians;
        phase_.store(phase, std::memory_order_relaxed);
        composeClusterFrame(*source, phase, frame);

        {
            // Post under the lock and only for the current source. setSource() callers that
            // queue their own postData() afterwards are then guaranteed to be applied last.
            const auto lock = std::lock_guard<std::mutex>{mutex_};
            if (quit_ || !running_ || source_ != source) {
                continue;
            }
            // The frame buffers are moved into the queued call; allocate fresh ones next time.
            target_->postData(std::move(frame.xy), std::move(frame.values), frame.pointCount);
        }
        frame = PointCloudData{};

        std::this_thread::sleep_until(frameStart + kMinimumFrameInterval);
    }
}

} // namespace QAccelPlotExample
