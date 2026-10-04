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

namespace QAccelPlotExample {

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

    updateVertexCaches(batch, parameters.dataset);
    applyPrecision(batch.parts, parameters.dataset);
}

void SineWaveGenerator::updateVertexCaches(Batch& batch, const DatasetParameters& dataset)
{
    // A vertex cache depends only on the point count, and a curve keeps it while the count is unchanged.
    const auto rebuild = dataset.count != vertexCachePointCount_ || dataset.seriesCount != vertexCacheSeriesCount_;
    for (auto& part : batch.parts) {
        if (rebuild && part.count >= 2) {
            lineRenderer_.buildVertexCache(part.floats, part.count, part.vertexCache);
        } else {
            part.vertexCache.clear();
        }
    }
    vertexCachePointCount_ = dataset.count;
    vertexCacheSeriesCount_ = dataset.seriesCount;
}

} // namespace QAccelPlotExample
