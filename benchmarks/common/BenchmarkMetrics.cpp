//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "BenchmarkMetrics.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace QAccelPlot {

BenchmarkMetrics::BenchmarkMetrics() = default;

void BenchmarkMetrics::setWarmupFrames(const int count)
{
    warmupFrames_ = std::max(0, count);
    warmupDurationMs_ = 0;
}

int BenchmarkMetrics::warmupFrames() const
{
    return warmupFrames_;
}

void BenchmarkMetrics::setWarmupDurationMs(const int durationMs)
{
    warmupDurationMs_ = std::max(0, durationMs);
    warmupFrames_ = 0;
}

int BenchmarkMetrics::warmupDurationMs() const
{
    return warmupDurationMs_;
}

void BenchmarkMetrics::start()
{
    frameIndex_ = 0;
    lastPointCount_ = 0;
    frameTimesMs_.clear();
    frameIntervalsMs_.clear();
    warmupEndNs_ = static_cast<qint64>(warmupDurationMs_) * 1'000'000;
    lastFrameTimestampNs_ = 0;
    frameClock_.invalidate();
    wallClock_.start();
}

void BenchmarkMetrics::beginFrame()
{
    frameClock_.start();
}

void BenchmarkMetrics::endFrame(const int64_t pointCount)
{
    const auto currentNs = wallClock_.nsecsElapsed();
    const auto hasPreviousFrame = frameIndex_ > 0;
    const auto intervalNs = hasPreviousFrame ? currentNs - lastFrameTimestampNs_ : 0;
    const auto frameNs = frameClock_.isValid() ? frameClock_.nsecsElapsed() : 0;

    lastFrameTimestampNs_ = currentNs;

    lastPointCount_ = pointCount;
    ++frameIndex_;

    if (warmupDurationMs_ > 0 && currentNs < warmupEndNs_) {
        return;
    }

    // Frame-count warmup remains available for callers with a fixed presentation rate.
    if (warmupDurationMs_ == 0 && frameIndex_ <= warmupFrames_) {
        if (frameIndex_ == warmupFrames_) {
            warmupEndNs_ = currentNs;
        }
        return;
    }

    frameTimesMs_.push_back(static_cast<double>(frameNs) / 1'000'000.0);
    if (hasPreviousFrame) {
        frameIntervalsMs_.push_back(static_cast<double>(intervalNs) / 1'000'000.0);
    }
}

int64_t BenchmarkMetrics::recordedFrameCount() const
{
    return static_cast<int64_t>(frameTimesMs_.size());
}

double BenchmarkMetrics::lastFrameTimeMs() const
{
    if (frameTimesMs_.empty()) {
        return 0.0;
    }
    return frameTimesMs_.back();
}

double BenchmarkMetrics::rollingFps(const int windowSize) const
{
    if (frameIntervalsMs_.empty()) {
        return 0.0;
    }

    const auto count = std::min(static_cast<int>(frameIntervalsMs_.size()), std::max(1, windowSize));
    auto sum = 0.0;
    for (auto i = static_cast<int>(frameIntervalsMs_.size()) - count; i < static_cast<int>(frameIntervalsMs_.size()); ++i) {
        sum += frameIntervalsMs_[static_cast<size_t>(i)];
    }

    const auto avgMs = sum / static_cast<double>(count);
    if (avgMs <= 0.0) {
        return 0.0;
    }

    return 1000.0 / avgMs;
}

BenchmarkMetrics::Results BenchmarkMetrics::results() const
{
    auto r = Results{};

    if (frameTimesMs_.empty()) {
        return r;
    }

    r.totalFrames = static_cast<int64_t>(frameTimesMs_.size());

    // Elapsed time: wall clock minus warmup portion.
    const auto totalNs = wallClock_.nsecsElapsed();
    const auto recordedNs = std::max<qint64>(0, totalNs - warmupEndNs_);
    r.elapsedSeconds = static_cast<double>(recordedNs) / 1'000'000'000.0;

    // Average FPS from wall-clock time (more accurate than averaging per-frame FPS).
    if (r.elapsedSeconds > 0.0) {
        r.fpsAvg = static_cast<double>(r.totalFrames) / r.elapsedSeconds;
    }

    // Sort a copy for percentile computation.
    auto sorted = frameTimesMs_;
    std::sort(sorted.begin(), sorted.end());

    r.frameTimeMinMs = sorted.front();
    r.frameTimeMaxMs = sorted.back();
    r.frameTimeP50Ms = percentile(sorted, 50.0);
    r.frameTimeP95Ms = percentile(sorted, 95.0);
    r.frameTimeP99Ms = percentile(sorted, 99.0);

    if (!frameIntervalsMs_.empty()) {
        sorted = frameIntervalsMs_;
        std::sort(sorted.begin(), sorted.end());
        r.frameIntervalMinMs = sorted.front();
        r.frameIntervalMaxMs = sorted.back();
        r.frameIntervalP50Ms = percentile(sorted, 50.0);
        r.frameIntervalP95Ms = percentile(sorted, 95.0);
        r.frameIntervalP99Ms = percentile(sorted, 99.0);
    }

    // Minimum FPS is based on completed-frame cadence, not frame duration.
    if (r.frameIntervalMaxMs > 0.0) {
        r.fpsMin = 1000.0 / r.frameIntervalMaxMs;
    }

    // Data throughput.
    if (lastPointCount_ > 0 && r.fpsAvg > 0.0) {
        r.dataThroughputMpps = static_cast<double>(lastPointCount_) * r.fpsAvg / 1'000'000.0;
    }

    return r;
}

double BenchmarkMetrics::percentile(const std::vector<double>& sorted, const double p) const
{
    if (sorted.empty()) {
        return 0.0;
    }
    if (sorted.size() == 1) {
        return sorted[0];
    }

    const auto rank = (p / 100.0) * static_cast<double>(sorted.size() - 1);
    const auto lower = static_cast<size_t>(std::floor(rank));
    const auto upper = static_cast<size_t>(std::ceil(rank));
    const auto fraction = rank - std::floor(rank);

    if (lower == upper) {
        return sorted[lower];
    }

    return sorted[lower] * (1.0 - fraction) + sorted[upper] * fraction;
}

} // namespace QAccelPlot
