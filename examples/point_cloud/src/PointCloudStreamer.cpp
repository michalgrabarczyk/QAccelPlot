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

void PointCloudStreamer::requestPointCount(const int pointCount)
{
    {
        const auto lock = std::lock_guard<std::mutex>{mutex_};
        pendingPointCount_ = pointCount;
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

PointCloudStreamer::Work PointCloudStreamer::waitForWork(int& pointCount, std::shared_ptr<const ClusterCloud>& source)
{
    auto lock = std::unique_lock<std::mutex>{mutex_};
    // A pending build is serviced even while paused, so changing the point count with the
    // animation off still refreshes the cloud.
    wake_.wait(lock, [this]() { return quit_ || pendingPointCount_ > 0 || (running_ && framePresented_ && source_); });
    if (quit_) {
        return Work::Quit;
    }
    if (pendingPointCount_ > 0) {
        pointCount = pendingPointCount_;
        pendingPointCount_ = 0;
        return Work::BuildSource;
    }
    framePresented_ = false;
    source = source_;
    return Work::ComposeFrame;
}

void PointCloudStreamer::buildSource(const int pointCount)
{
    // The expensive part: generating the cloud runs here, off the GUI thread.
    auto cloud = std::make_shared<const ClusterCloud>(generateClusterCloud(pointCount));
    auto frame = PointCloudData{};
    composeClusterFrame(*cloud, 0.0f, frame);

    const auto lock = std::lock_guard<std::mutex>{mutex_};
    if (quit_ || pendingPointCount_ > 0) {
        // Superseded while we were generating; drop this cloud and build the newer one.
        return;
    }
    source_ = std::move(cloud);
    // The first frame is composed at phase 0, so continue stepping from there.
    phase_.store(0.0f, std::memory_order_relaxed);
    target_->postData(std::move(frame.xy), std::move(frame.values), frame.pointCount);
}

void PointCloudStreamer::composeFrame(const std::shared_ptr<const ClusterCloud>& source, PointCloudData& frame)
{
    const auto phase = phase_.load(std::memory_order_relaxed) + kPhaseStepRadians;
    phase_.store(phase, std::memory_order_relaxed);
    composeClusterFrame(*source, phase, frame);

    // Post under the lock and only for the current source, so a cloud replaced while this
    // frame was being composed never overwrites the newer one.
    const auto lock = std::lock_guard<std::mutex>{mutex_};
    if (quit_ || !running_ || source_ != source) {
        return;
    }
    // The frame buffers are moved into the queued call; allocate fresh ones next time.
    target_->postData(std::move(frame.xy), std::move(frame.values), frame.pointCount);
}

void PointCloudStreamer::run()
{
    auto frame = PointCloudData{};
    while (true) {
        auto pointCount = 0;
        auto source = std::shared_ptr<const ClusterCloud>{};
        const auto work = waitForWork(pointCount, source);
        if (work == Work::Quit) {
            return;
        }

        if (work == Work::BuildSource) {
            buildSource(pointCount);
            frame = PointCloudData{};
            continue;
        }

        const auto frameStart = std::chrono::steady_clock::now();
        composeFrame(source, frame);
        frame = PointCloudData{};
        std::this_thread::sleep_until(frameStart + kMinimumFrameInterval);
    }
}

} // namespace QAccelPlotExample
