//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

/// \file BenchmarkMetrics.hpp
/// \brief Frame-timing measurement for benchmark scenarios.

#include <QElapsedTimer>

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <vector>

namespace QAccelPlot {

/// \brief Collects per-frame timing data and computes summary statistics.
///
/// Usage:
/// \code
/// BenchmarkMetrics metrics;
/// metrics.setWarmupFrames(120);   // discard first 2 seconds at 60 Hz
/// metrics.start();
///
/// // render loop
/// metrics.beginFrame();
/// // ... render ...
/// metrics.endFrame(pointCount);
///
/// auto results = metrics.results();
/// \endcode
///
/// \sa BenchmarkReporter, BenchmarkScenario
class BenchmarkMetrics {
public:
    /// \brief Computed summary of a completed benchmark run.
    struct Results {
        double fpsAvg{0.0};             ///< Average FPS (after warmup).
        double fpsMin{0.0};             ///< Minimum instantaneous FPS observed.
        double frameTimeP50Ms{0.0};     ///< Median measured frame time in milliseconds.
        double frameTimeP95Ms{0.0};     ///< 95th percentile frame time.
        double frameTimeP99Ms{0.0};     ///< 99th percentile frame time.
        double frameTimeMinMs{0.0};     ///< Fastest measured frame.
        double frameTimeMaxMs{0.0};     ///< Slowest measured frame.
        double frameIntervalP50Ms{0.0}; ///< Median interval between completed frames.
        double frameIntervalP95Ms{0.0}; ///< 95th percentile completed-frame interval.
        double frameIntervalP99Ms{0.0}; ///< 99th percentile completed-frame interval.
        double frameIntervalMinMs{0.0}; ///< Shortest completed-frame interval.
        double frameIntervalMaxMs{0.0}; ///< Longest completed-frame interval.
        int64_t totalFrames{0};         ///< Total frames recorded (after warmup).
        double elapsedSeconds{0.0};     ///< Wall-clock time of the recorded portion.
        double dataThroughputMpps{0.0}; ///< Million rendered points per second (total points across curves * FPS / 1e6).
    };

    /// \brief Constructs a BenchmarkMetrics instance with default settings.
    BenchmarkMetrics();

    /// \brief Sets the number of warmup frames to discard before recording.
    ///
    /// Warmup frames are timed but excluded from the final statistics to allow
    /// the GPU driver, Qt Scene Graph, and OS scheduler to stabilize.
    /// Default: 120 (2 seconds at 60 Hz).
    void setWarmupFrames(int count);

    /// \brief Returns the configured warmup frame count.
    int warmupFrames() const;

    /// \brief Uses a wall-clock warmup duration instead of a frame count.
    ///
    /// This is appropriate for uncapped offscreen benchmarks where assuming a
    /// display-rate frame count would make the effective warmup far too short.
    void setWarmupDurationMs(int durationMs);

    /// \brief Returns the configured wall-clock warmup duration in milliseconds.
    int warmupDurationMs() const;

    /// \brief Resets all state and starts the wall-clock timer.
    void start();

    /// \brief Marks the beginning of a measured frame.
    ///
    /// Call immediately before updating curve data or the viewport.
    void beginFrame();

    /// \brief Marks the end of a measured frame.
    ///
    /// Call after the graphics frame has been submitted with
    /// QQuickRenderControl::endFrame().
    /// \param pointCount Total number of data points rendered in this frame across all curves.
    void endFrame(int64_t pointCount = 0);

    /// \brief Returns the number of frames recorded so far (excluding warmup).
    int64_t recordedFrameCount() const;

    /// \brief Returns the most recent frame time in milliseconds, or 0.0 if no frames recorded.
    double lastFrameTimeMs() const;

    /// \brief Returns a rolling average FPS over the last N recorded frames.
    /// \param windowSize Number of recent frames to average. Clamped to available frames.
    double rollingFps(int windowSize = 60) const;

    /// \brief Computes and returns the final results. May be called at any time after at least one recorded frame.
    Results results() const;

private:
    double percentile(const std::vector<double>& sorted, double p) const;

    int warmupFrames_{120};
    int warmupDurationMs_{0};
    int64_t frameIndex_{0};
    int64_t lastPointCount_{0};
    QElapsedTimer wallClock_;
    QElapsedTimer frameClock_;
    std::vector<double> frameTimesMs_;
    std::vector<double> frameIntervalsMs_;
    int64_t warmupEndNs_{0};
    int64_t lastFrameTimestampNs_{0};
};

} // namespace QAccelPlot
