//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "DataGenerationWorker.hpp"

#include "renderers/LineCurveLineRenderer.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace {

constexpr auto kSineTableBits = 14;
constexpr auto kSineTableSize = std::size_t{1} << kSineTableBits;
constexpr auto kSineIndexShift = 64 - kSineTableBits;
constexpr auto kSineIndexRounding = std::uint64_t{1} << (kSineIndexShift - 1);
constexpr auto kTwoPi = 6.283185307179586476925286766559;
constexpr auto kAccumulatorRange = 18446744073709551616.0L; // 2^64

const std::array<float, kSineTableSize>& sineLookupTable()
{
    static const auto table = []() {
        auto values = std::array<float, kSineTableSize>{};
        for (auto i = std::size_t{0}; i < values.size(); ++i) {
            const auto angle = kTwoPi * static_cast<double>(i) / static_cast<double>(values.size());
            values[i] = static_cast<float>(std::sin(angle));
        }
        return values;
    }();
    return table;
}

std::uint64_t phaseAccumulator(const double phase)
{
    auto normalizedPhase = std::fmod(phase, kTwoPi);
    if (normalizedPhase < 0.0) {
        normalizedPhase += kTwoPi;
    }

    const auto scaled = static_cast<long double>(normalizedPhase) * (kAccumulatorRange / static_cast<long double>(kTwoPi));
    return scaled >= kAccumulatorRange ? 0 : static_cast<std::uint64_t>(scaled);
}

std::uint64_t phaseStep(const double angleStep)
{
    auto normalizedStep = std::fmod(angleStep, kTwoPi);
    if (normalizedStep < 0.0) {
        normalizedStep += kTwoPi;
    }

    const auto scaled = static_cast<long double>(normalizedStep) * (kAccumulatorRange / static_cast<long double>(kTwoPi));
    return scaled + 0.5L >= kAccumulatorRange ? 0 : static_cast<std::uint64_t>(scaled + 0.5L);
}

class SineLookupCursor final {
public:
    SineLookupCursor(const double initialPhase, const double angleStep)
        : table_(sineLookupTable())
        , accumulator_(phaseAccumulator(initialPhase))
        , step_(phaseStep(angleStep))
    {
    }

    float next() noexcept
    {
        const auto roundedAccumulator = accumulator_ + kSineIndexRounding;
        const auto index = static_cast<std::size_t>(roundedAccumulator >> kSineIndexShift);
        accumulator_ += step_;
        return table_[index];
    }

private:
    const std::array<float, kSineTableSize>& table_;
    std::uint64_t accumulator_;
    std::uint64_t step_;
};

} // namespace

DataGenerationWorker::DataGenerationWorker(const DataGenerationConfig& config)
    : config_(config)
    , pointCount_(config.defaultPointCount)
{
}

DataGenerationWorker::~DataGenerationWorker()
{
    stop();
}

void DataGenerationWorker::start()
{
    if (running_.exchange(true, std::memory_order_acq_rel)) {
        return;
    }

    workerThread_ = std::thread([this]() { run(); });
}

void DataGenerationWorker::stop()
{
    if (!running_.exchange(false, std::memory_order_acq_rel)) {
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
        return;
    }

    conditionVariable_.notify_one();

    if (workerThread_.joinable()) {
        workerThread_.join();
    }
}

void DataGenerationWorker::setPhase(double phase)
{
    phase_.store(phase, std::memory_order_relaxed);
}

void DataGenerationWorker::setPointCount(int pointCount)
{
    if (pointCount <= 0) {
        pointCount = config_.defaultPointCount;
    }

    pointCount_.store(pointCount, std::memory_order_relaxed);
}

void DataGenerationWorker::setRectangleCount(int rectangleCount)
{
    if (rectangleCount <= 0) {
        rectangleCount = config_.defaultPointCount;
    }

    rectangleCount_.store(rectangleCount, std::memory_order_relaxed);
}

void DataGenerationWorker::setRectangleTestMode(bool enabled)
{
    rectangleTestMode_.store(enabled, std::memory_order_relaxed);
}

bool DataGenerationWorker::tryConsume(DataGenerationBatch& batch)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!dataReady_) {
        return false;
    }

    batch.curve1 = std::move(readyBatch_.curve1);
    batch.curve1VertexCache = std::move(readyBatch_.curve1VertexCache);
    batch.rects = std::move(readyBatch_.rects);
    batch.pointCount = readyBatch_.pointCount;
    batch.rectangleCount = readyBatch_.rectangleCount;
    dataReady_ = false;
    conditionVariable_.notify_one();
    return true;
}

void DataGenerationWorker::run()
{
    std::vector<float> workBuffer1;
    std::vector<char> workBuffer1VertexCache;
    std::vector<float> workBufferRects;
    QAccelPlot::LineCurveLineRenderer lineRenderer;
    auto lastVertexCachePointCount = 0;
    (void)sineLookupTable();

    while (running_.load(std::memory_order_relaxed)) {
        const auto n = pointCount_.load(std::memory_order_relaxed);
        const auto rectangleCount = rectangleCount_.load(std::memory_order_relaxed);
        const auto p = phase_.load(std::memory_order_relaxed);
        const auto rectangleTestMode = rectangleTestMode_.load(std::memory_order_relaxed);

        workBuffer1.resize(static_cast<size_t>(n) * 2);

        const auto phaseF = static_cast<float>(p);
        const auto rectHalfW = config_.rectangleWidth * 0.5f;
        const auto rectHalfH = config_.rectangleHeight * 0.5f;
        const auto curveXStep = config_.xDomainMainCurves / static_cast<float>(std::max(1, n - 1));
        auto curveSignal = SineLookupCursor{phaseF, config_.signalAngularFrequency * curveXStep};

        if (rectangleTestMode) {
            workBufferRects.resize(static_cast<size_t>(rectangleCount) * 4);
        }

        for (auto i = 0; i < n; ++i) {
            const auto x = static_cast<float>(i) * curveXStep;
            const auto idx = static_cast<size_t>(i) * 2;
            workBuffer1[idx] = x;
            workBuffer1[idx + 1] = config_.curve1Amplitude * curveSignal.next();
        }

        if (rectangleTestMode) {
            const auto rectangleXStep = config_.xDomainMainCurves / static_cast<float>(std::max(1, rectangleCount - 1));
            auto rectangleSignal = SineLookupCursor{phaseF, config_.signalAngularFrequency * rectangleXStep};
            for (auto i = 0; i < rectangleCount; ++i) {
                const auto x = static_cast<float>(i) * rectangleXStep;
                const auto y = config_.curve1Amplitude * rectangleSignal.next();
                const auto dst = static_cast<size_t>(i) * 4;
                workBufferRects[dst] = x - rectHalfW;
                workBufferRects[dst + 1] = y - rectHalfH;
                workBufferRects[dst + 2] = x + rectHalfW;
                workBufferRects[dst + 3] = y + rectHalfH;
            }
        }

        // The vertex cache contains sequential indices + ±1 side values that are
        // deterministic from pointCount alone. Only rebuild when pointCount changes.
        if (n != lastVertexCachePointCount) {
            lineRenderer.buildVertexCache(workBuffer1, n, workBuffer1VertexCache);
            lastVertexCachePointCount = n;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            readyBatch_.curve1.swap(workBuffer1);
            readyBatch_.curve1VertexCache.swap(workBuffer1VertexCache);
            if (rectangleTestMode) {
                readyBatch_.rects.swap(workBufferRects);
            } else {
                readyBatch_.rects.clear();
            }
            readyBatch_.pointCount = n;
            readyBatch_.rectangleCount = rectangleTestMode ? rectangleCount : 0;
            dataReady_ = true;
        }

        std::unique_lock<std::mutex> lock(mutex_);
        conditionVariable_.wait(lock, [this]() { return !dataReady_ || !running_.load(std::memory_order_relaxed); });
    }
}
