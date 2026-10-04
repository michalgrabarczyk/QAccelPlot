//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "DatasetParts.hpp"

#include <QAccelPlot/renderers/LineCurveLineRenderer.hpp>

#include <vector>

namespace QAccelPlotExample {

/// \brief One curve's share of a LineCurve dataset: interleaved XY pairs.
struct SineWavePart : SeriesPart {
    /// \brief Prebuilt line vertex cache; empty unless the curve's point count changed.
    std::vector<char> vertexCache;
};

/// \brief One LineCurve dataset, split into consecutive stretches of the wave.
struct SineWaveBatch {
    std::vector<SineWavePart> parts;
};

/// \brief Settings of a SineWaveGenerator batch.
struct SineWaveParameters {
    DatasetParameters dataset;
    /// \brief Multiplies \c SineWaveGenerator::kAngularFrequency.
    float frequencyScale{1.0f};
    /// \brief Share of the samples whose Y is NaN, spread over \c SineWaveGenerator::kGapCount runs.
    float gapFraction{0.0f};
    /// \brief Whether prebuilt vertex caches accompany the points. Only a solid line without markers or effects takes one.
    bool vertexCache{true};
};

/// \brief Generates a scrolling sine wave for one or more LineCurves.
///
/// Samples come from a small sine table stepped by a fixed-point phase accumulator, so the
/// table stays in cache and no trigonometry runs per point.
class SineWaveGenerator final {
public:
    using Batch = SineWaveBatch;
    using Parameters = SineWaveParameters;

    /// \brief X extent of the wave, starting at 0.
    static constexpr auto kDomainWidth = 1000.0f;
    /// \brief Phase change per X unit, in radians.
    static constexpr auto kAngularFrequency = 0.1f;
    /// \brief Peak Y value.
    static constexpr auto kAmplitude = 8.0f;
    /// \brief Phase change per second, in radians.
    static constexpr auto kPhaseVelocity = 1.2;
    /// \brief Number of evenly spaced runs of NaN samples when gaps are requested.
    static constexpr auto kGapCount = 20;

    /// \brief Fills \a batch with the wave described by \a parameters at \a timeSeconds.
    void generate(Batch& batch, const Parameters& parameters, double timeSeconds);

private:
    void updateVertexCaches(Batch& batch, const Parameters& parameters);

    QAccelPlot::LineCurveLineRenderer lineRenderer_;
    int vertexCachePointCount_{0};
    int vertexCacheSeriesCount_{0};
};

} // namespace QAccelPlotExample
