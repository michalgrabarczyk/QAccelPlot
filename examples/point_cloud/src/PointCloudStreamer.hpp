//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "PointCloudGenerators.hpp"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

namespace QAccelPlot {
class PointCloud;
}

namespace QAccelPlotExample {

/// Rotates the cluster cloud on a background thread and hands frames to a
/// PointCloud through its thread-safe postData().
///
/// Back-pressure: a new frame is composed only after the window has presented
/// a frame since the previous post, so queued frames can never pile up when
/// rendering falls behind. That gate alone paces the stream to the display;
/// sleeping on top of it would add to the wait rather than overlap with it,
/// which halved the frame rate regardless of how much data was being sent.
class PointCloudStreamer {
public:
    explicit PointCloudStreamer(QAccelPlot::PointCloud* target);
    ~PointCloudStreamer();

    PointCloudStreamer(const PointCloudStreamer&) = delete;
    PointCloudStreamer& operator=(const PointCloudStreamer&) = delete;

    /// Requests a cloud of \a pointCount points. Generating it is the expensive part of a
    /// point-count change, so it runs on the streaming thread: the caller returns immediately
    /// and the first frame arrives through postData() once it is ready. A later request
    /// supersedes an earlier one that has not been applied yet.
    void requestPointCount(int pointCount);
    /// Starts or pauses streaming; the last posted frame stays on screen when paused.
    void setRunning(bool running);
    /// Call from QQuickWindow::frameSwapped (any thread).
    void notifyFramePresented();

    void stop();

private:
    enum class Work { Quit, BuildSource, ComposeFrame };

    void run();
    /// Blocks until there is something to do, reporting which job the caller should run.
    /// \a pointCount receives the requested size for \c BuildSource, \a source the cloud
    /// to compose from for \c ComposeFrame.
    Work waitForWork(int& pointCount, std::shared_ptr<const ClusterCloud>& source);
    void buildSource(int pointCount);
    void composeFrame(const std::shared_ptr<const ClusterCloud>& source, PointCloudData& frame);

    QAccelPlot::PointCloud* target_{nullptr};
    std::mutex mutex_;
    std::condition_variable wake_;
    std::shared_ptr<const ClusterCloud> source_;
    // Non-zero while a point-count change is waiting to be generated on the streaming thread.
    int pendingPointCount_{0};
    bool running_{false};
    bool quit_{false};
    bool framePresented_{true};
    std::atomic<float> phase_{0.0f};
    std::thread thread_;
};

} // namespace QAccelPlotExample
