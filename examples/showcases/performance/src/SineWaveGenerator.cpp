//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "SineWaveGenerator.hpp"
#include "ParallelFor.hpp"
#include "SineLookup.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace QAccelPlotExample {
namespace {

// Replaces the Y of gapFraction of the samples with NaN, in evenly spaced runs across the wave.
void insertGaps(std::vector<SineWavePart>& parts, const int pointCount, const float gapFraction)
{
    const auto period = pointCount / SineWaveGenerator::kGapCount;
    if (gapFraction <= 0.0f || period == 0) {
        return;
    }
    const auto runLength = std::max(1, static_cast<int>(static_cast<float>(period) * gapFraction));
    auto firstPoint = 0;
    for (auto& part : parts) {
        const auto endPoint = firstPoint + part.count;
        for (auto gap = firstPoint / period; gap < SineWaveGenerator::kGapCount; ++gap) {
            const auto runBegin = gap * period + (period - runLength) / 2;
            const auto end = std::min(runBegin + runLength, endPoint);
            for (auto point = std::max(runBegin, firstPoint); point < end; ++point) {
                part.floats[static_cast<std::size_t>(point - firstPoint) * 2 + 1] = std::numeric_limits<float>::quiet_NaN();
            }
        }
        firstPoint = endPoint;
    }
}

// Writes samples [first, last) of part; partBegin is the wave index of the part's first sample,
// and sine starts at wave index 0.
void writeWave(SineWavePart& part, const int first, const int last, const int partBegin, const float xStep, SineLookupCursor sine)
{
    sine.skip(static_cast<std::uint64_t>(partBegin + first));
    auto* xy = part.floats.data();
    for (auto i = first; i < last; ++i) {
        const auto offset = static_cast<std::size_t>(i) * 2;
        xy[offset] = static_cast<float>(partBegin + i) * xStep;
        xy[offset + 1] = SineWaveGenerator::kAmplitude * sine.next();
    }
}

} // namespace

std::size_t SineWaveBatch::byteSize() const
{
    return partsByteSize(parts);
}

bool operator==(const SineWaveParameters& a, const SineWaveParameters& b)
{
    return a.dataset == b.dataset && a.gapFraction == b.gapFraction;
}

void SineWaveGenerator::generate(Batch& batch, const Parameters& parameters, const double timeSeconds)
{
    const auto pointCount = parameters.dataset.count;
    resizeParts(batch.parts, parameters.dataset, 2);

    const auto xStep = kDomainWidth / static_cast<float>(std::max(1, pointCount - 1));
    const auto sine = SineLookupCursor{kPhaseVelocity * timeSeconds, static_cast<double>(kAngularFrequency * xStep)};
    parallelFor(pointCount, kParallelChunk, [&batch, xStep, &sine](const int begin, const int end) {
        forEachPartSlice(batch.parts, begin, end,
            [xStep, &sine](SineWavePart& part, const int first, const int last, const int partBegin) { writeWave(part, first, last, partBegin, xStep, sine); });
    });

    insertGaps(batch.parts, pointCount, parameters.gapFraction);
    applyPrecision(batch.parts, parameters.dataset);
}

} // namespace QAccelPlotExample
