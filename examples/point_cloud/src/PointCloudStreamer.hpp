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
/// rendering falls behind.
class PointCloudStreamer {
public:
    explicit PointCloudStreamer(QAccelPlot::PointCloud* target);
    ~PointCloudStreamer();

    PointCloudStreamer(const PointCloudStreamer&) = delete;
    PointCloudStreamer& operator=(const PointCloudStreamer&) = delete;

    /// Replaces the source cloud (e.g. after the point count changed). Frames composed from
    /// the previous source are dropped; a postData() queued after this call is applied last.
    void setSource(std::shared_ptr<const ClusterCloud> cloud);
    /// Starts or pauses streaming; the last posted frame stays on screen when paused.
    void setRunning(bool running);
    /// Call from QQuickWindow::frameSwapped (any thread).
    void notifyFramePresented();

    void stop();

private:
    void run();
    bool waitForWork();

    QAccelPlot::PointCloud* target_{nullptr};
    std::mutex mutex_;
    std::condition_variable wake_;
    std::shared_ptr<const ClusterCloud> source_;
    bool running_{false};
    bool quit_{false};
    bool framePresented_{true};
    std::atomic<float> phase_{0.0f};
    std::thread thread_;
};

} // namespace QAccelPlotExample
