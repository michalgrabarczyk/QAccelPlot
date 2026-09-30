//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/LineCurveGapFilter.hpp"

#include "QAccelPlot/MathUtils.hpp"

#include <QtGlobal>

#include <algorithm>
#include <cstddef>

namespace QAccelPlot::LineCurveGapFilter {

namespace {

// Minimum number of samples a run needs to span an area or a segment.
constexpr auto kMinRunSamples = int{2};

bool isValidPair(const double x, const double y, const bool logScaleX, const bool logScaleY)
{
    return isValidSample(x, logScaleX) && isValidSample(y, logScaleY);
}

// The scans below are specialized on the buffer type so the float/double
// dispatch in CurveDataView happens once per call instead of once per sample.

template <typename T> int countInvalid(const T* data, const CurveDataView& layout, const int pointCount, const bool logScaleX, const bool logScaleY)
{
    const auto stride = static_cast<std::size_t>(layout.stride);
    const auto yOffset = static_cast<std::size_t>(layout.yOffset);
    auto invalidCount = int{0};
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(pointCount); ++i) {
        invalidCount += isValidPair(data[i * stride], data[i * stride + yOffset], logScaleX, logScaleY) ? 0 : 1;
    }
    return invalidCount;
}

template <typename T>
std::vector<SampleRun> findRuns(const T* data, const CurveDataView& layout, const int pointCount, const bool logScaleX, const bool logScaleY)
{
    const auto stride = static_cast<std::size_t>(layout.stride);
    const auto yOffset = static_cast<std::size_t>(layout.yOffset);
    auto runs = std::vector<SampleRun>{};
    auto runStart = int{-1};
    for (auto i = int{0}; i < pointCount; ++i) {
        const auto offset = static_cast<std::size_t>(i) * stride;
        const auto valid = isValidPair(data[offset], data[offset + yOffset], logScaleX, logScaleY);
        if (valid && runStart < 0) {
            runStart = i;
        } else if (!valid && runStart >= 0) {
            runs.push_back(SampleRun{runStart, i - runStart});
            runStart = -1;
        }
    }
    if (runStart >= 0) {
        runs.push_back(SampleRun{runStart, pointCount - runStart});
    }
    return runs;
}

} // namespace

bool isValidPoint(const CurveDataView& data, const int index, const bool logScaleX, const bool logScaleY)
{
    return isValidPair(data.x(index), data.y(index), logScaleX, logScaleY);
}

int countInvalidPoints(const CurveDataView& data, const int pointCount, const bool logScaleX, const bool logScaleY)
{
    return data.doubleData ? countInvalid(data.doubleData, data, pointCount, logScaleX, logScaleY)
                           : countInvalid(data.floatData, data, pointCount, logScaleX, logScaleY);
}

std::vector<SampleRun> findValidRuns(const CurveDataView& data, const int pointCount, const bool logScaleX, const bool logScaleY)
{
    return data.doubleData ? findRuns(data.doubleData, data, pointCount, logScaleX, logScaleY)
                           : findRuns(data.floatData, data, pointCount, logScaleX, logScaleY);
}

std::vector<int> planRunSampling(const std::vector<SampleRun>& runs, const int maxSampledPoints)
{
    auto totalSamples = qint64{0};
    for (const auto& run : runs) {
        if (run.count >= kMinRunSamples) {
            totalSamples += run.count;
        }
    }

    auto sampled = std::vector<int>(runs.size(), 0);
    const auto budget = static_cast<qint64>(std::max(maxSampledPoints, kMinRunSamples));
    for (auto i = std::size_t{0}; i < runs.size(); ++i) {
        const auto count = runs[i].count;
        if (count < kMinRunSamples) {
            continue;
        }
        if (totalSamples <= budget) {
            sampled[i] = count;
            continue;
        }
        const auto share = static_cast<int>((static_cast<qint64>(count) * budget) / totalSamples);
        sampled[i] = std::clamp(share, kMinRunSamples, count);
    }
    return sampled;
}

void appendEnvelopeSamples(const CurveDataView& data, const SampleRun& run, const int sampledCount, std::vector<int>& indices)
{
    if (sampledCount >= run.count || sampledCount < kMinRunSamples) {
        for (auto sample = 0; sample < std::min(sampledCount, run.count); ++sample) {
            indices.push_back(run.start + sample);
        }
        return;
    }

    const auto last = run.start + run.count - 1;
    const auto interiorSlots = sampledCount - kMinRunSamples;
    const auto buckets = interiorSlots / 2;
    const auto interiorStart = run.start + 1;
    const auto interiorCount = static_cast<qint64>(run.count - kMinRunSamples);
    indices.push_back(run.start);
    for (auto bucket = 0; bucket < buckets; ++bucket) {
        const auto begin = interiorStart + static_cast<int>(bucket * interiorCount / buckets);
        const auto end = interiorStart + static_cast<int>((bucket + 1) * interiorCount / buckets);
        auto lowest = begin;
        auto highest = begin;
        for (auto index = begin + 1; index < end; ++index) {
            lowest = data.y(index) < data.y(lowest) ? index : lowest;
            highest = data.y(index) > data.y(highest) ? index : highest;
        }
        indices.push_back(std::min(lowest, highest));
        indices.push_back(std::max(lowest, highest));
    }
    // An odd slot count leaves one slot, which repeats the last sample as a degenerate vertex.
    if (interiorSlots % 2 != 0) {
        indices.push_back(last);
    }
    indices.push_back(last);
}

int compactValidPoints(const std::vector<float>& data, const int pointCount, const bool logScaleX, const bool logScaleY, std::vector<float>& output)
{
    output.resize(static_cast<std::size_t>(pointCount) * 2);
    auto written = std::size_t{0};
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(pointCount); ++i) {
        const auto x = data[i * 2];
        const auto y = data[i * 2 + 1];
        if (isValidPair(x, y, logScaleX, logScaleY)) {
            output[written * 2] = x;
            output[written * 2 + 1] = y;
            ++written;
        }
    }
    output.resize(written * 2);
    return static_cast<int>(written);
}

int compactValidPoints(const std::vector<double>& data, const std::vector<float>& renderData, const int pointCount, const bool logScaleX, const bool logScaleY,
    std::vector<double>& output, std::vector<float>& renderOutput)
{
    output.resize(static_cast<std::size_t>(pointCount) * 2);
    renderOutput.resize(static_cast<std::size_t>(pointCount) * 2);
    auto written = std::size_t{0};
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(pointCount); ++i) {
        if (isValidPair(data[i * 2], data[i * 2 + 1], logScaleX, logScaleY)) {
            output[written * 2] = data[i * 2];
            output[written * 2 + 1] = data[i * 2 + 1];
            renderOutput[written * 2] = renderData[i * 2];
            renderOutput[written * 2 + 1] = renderData[i * 2 + 1];
            ++written;
        }
    }
    output.resize(written * 2);
    renderOutput.resize(written * 2);
    return static_cast<int>(written);
}

} // namespace QAccelPlot::LineCurveGapFilter
