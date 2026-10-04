//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "SineWaveGenerator.hpp"
#include "SineLookup.hpp"

#include <algorithm>
#include <cstddef>
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

} // namespace

void SineWaveGenerator::generate(Batch& batch, const Parameters& parameters, const double timeSeconds)
{
    const auto pointCount = parameters.dataset.count;
    resizeParts(batch.parts, parameters.dataset, 2);

    const auto xStep = kDomainWidth / static_cast<float>(std::max(1, pointCount - 1));
    auto sine = SineLookupCursor{kPhaseVelocity * timeSeconds, static_cast<double>(kAngularFrequency * xStep)};
    auto index = 0;
    for (auto& part : batch.parts) {
        auto* xy = part.floats.data();
        for (auto i = 0; i < part.count; ++i, ++index) {
            const auto offset = static_cast<std::size_t>(i) * 2;
            xy[offset] = static_cast<float>(index) * xStep;
            xy[offset + 1] = kAmplitude * sine.next();
        }
    }

    insertGaps(batch.parts, pointCount, parameters.gapFraction);
    updateVertexCaches(batch, parameters);
    applyPrecision(batch.parts, parameters.dataset);
}

void SineWaveGenerator::updateVertexCaches(Batch& batch, const Parameters& parameters)
{
    // A vertex cache depends only on the point count, and a curve keeps it while the count is unchanged.
    const auto& dataset = parameters.dataset;
    const auto rebuild = parameters.vertexCache && (dataset.count != vertexCachePointCount_ || dataset.seriesCount != vertexCacheSeriesCount_);
    for (auto& part : batch.parts) {
        if (rebuild && part.count >= 2) {
            lineRenderer_.buildVertexCache(part.floats, part.count, part.vertexCache);
        } else {
            part.vertexCache.clear();
        }
    }
    // Forgets the caches while they are off, so they are sent again once they are back on.
    vertexCachePointCount_ = parameters.vertexCache ? dataset.count : 0;
    vertexCacheSeriesCount_ = parameters.vertexCache ? dataset.seriesCount : 0;
}

} // namespace QAccelPlotExample
