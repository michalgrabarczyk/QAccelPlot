//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "DatasetParts.hpp"

#include <array>
#include <vector>

namespace QAccelPlotExample {

/// \brief One cloud's share of a PointCloud dataset: interleaved XY pairs.
struct GalaxyPart : SeriesPart {
    /// \brief One colormap value per point; empty when values are off.
    std::vector<float> values;
};

/// \brief One PointCloud dataset, split into consecutive rings of the galaxy.
struct GalaxyBatch {
    std::vector<GalaxyPart> parts;
};

/// \brief Settings of a GalaxyGenerator batch.
struct GalaxyParameters {
    DatasetParameters dataset;
    /// \brief Whether every point carries a colormap value.
    bool values{true};
    /// \brief Share of the points whose X is NaN, spread evenly over the galaxy.
    float invalidFraction{0.0f};
};

/// \brief Generates a rotating spiral galaxy for one or more PointClouds.
///
/// Points are placed once per point count and grouped into radial bands. Each batch rotates every
/// band by its own angle, so a point costs one 2×2 matrix multiply and the arms ripple without
/// winding up over time. Values in [0, 1] grow from the core to the rim; a few arm points get a
/// fixed mid value to stand out as star-forming regions.
class GalaxyGenerator final {
public:
    using Batch = GalaxyBatch;
    using Parameters = GalaxyParameters;

    /// \brief Number of radial bands rotated independently.
    static constexpr auto kBandCount = 32;
    /// \brief Galaxy radius; halo points may lie slightly outside it.
    static constexpr auto kRadius = 1.0f;

    /// \brief Fills \a batch with the galaxy described by \a parameters, rotated to \a timeSeconds.
    void generate(Batch& batch, const Parameters& parameters, double timeSeconds);

    /// \brief Returns the rotation angle of radial band \a band at \a timeSeconds, in radians.
    static double bandAngleAt(int band, double timeSeconds);

    /// \brief Returns the unrotated XY position of every point, grouped by band.
    const std::vector<float>& basePositions() const;
    /// \brief Returns the index of the first point of every band, followed by the point count.
    const std::array<int, kBandCount + 1>& bandStarts() const;

private:
    void rebuildGalaxy(int pointCount);
    void rotateInto(GalaxyPart& part, int firstPoint, double timeSeconds) const;

    std::vector<float> basePositions_;
    std::vector<float> baseValues_;
    std::array<int, kBandCount + 1> bandStarts_{};
    int builtPointCount_{0};
};

} // namespace QAccelPlotExample
