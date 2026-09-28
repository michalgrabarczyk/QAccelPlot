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

void SineWaveGenerator::generate(Batch& batch, const int pointCount, const double timeSeconds)
{
    batch.xy.resize(static_cast<std::size_t>(pointCount) * 2);
    batch.pointCount = pointCount;

    const auto xStep = kDomainWidth / static_cast<float>(std::max(1, pointCount - 1));
    auto sine = SineLookupCursor{kPhaseVelocity * timeSeconds, static_cast<double>(kAngularFrequency * xStep)};
    auto* xy = batch.xy.data();
    for (auto i = 0; i < pointCount; ++i) {
        const auto offset = static_cast<std::size_t>(i) * 2;
        xy[offset] = static_cast<float>(i) * xStep;
        xy[offset + 1] = kAmplitude * sine.next();
    }

    // The vertex cache depends only on the point count, and the curve keeps it while the count is unchanged.
    if (pointCount != vertexCachePointCount_) {
        lineRenderer_.buildVertexCache(batch.xy, pointCount, batch.vertexCache);
        vertexCachePointCount_ = pointCount;
    } else {
        batch.vertexCache.clear();
    }
}

} // namespace QAccelPlotExample
