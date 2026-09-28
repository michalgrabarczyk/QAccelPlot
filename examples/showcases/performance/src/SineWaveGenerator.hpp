//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QAccelPlot/renderers/LineCurveLineRenderer.hpp>

#include <vector>

namespace QAccelPlotExample {

/// \brief One LineCurve dataset.
struct SineWaveBatch {
    std::vector<float> xy;
    /// \brief Prebuilt line vertex cache; empty unless the point count changed.
    std::vector<char> vertexCache;
    int pointCount{0};
};

/// \brief Generates a scrolling sine wave for a LineCurve.
///
/// Samples come from a small sine table stepped by a fixed-point phase accumulator, so the
/// table stays in cache and no trigonometry runs per point.
class SineWaveGenerator final {
public:
    using Batch = SineWaveBatch;

    /// \brief X extent of the wave, starting at 0.
    static constexpr auto kDomainWidth = 1000.0f;
    /// \brief Phase change per X unit, in radians.
    static constexpr auto kAngularFrequency = 0.1f;
    /// \brief Peak Y value.
    static constexpr auto kAmplitude = 8.0f;
    /// \brief Phase change per second, in radians.
    static constexpr auto kPhaseVelocity = 1.2;

    /// \brief Fills \a batch with \a pointCount samples of the wave at \a timeSeconds.
    void generate(Batch& batch, int pointCount, double timeSeconds);

private:
    QAccelPlot::LineCurveLineRenderer lineRenderer_;
    int vertexCachePointCount_{0};
};

} // namespace QAccelPlotExample
