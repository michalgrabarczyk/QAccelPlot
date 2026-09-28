//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <array>
#include <vector>

namespace QAccelPlotExample {

/// \brief One PointCloud dataset with one colormap value per point.
struct GalaxyBatch {
    std::vector<float> xy;
    std::vector<float> values;
    int pointCount{0};
};

/// \brief Generates a rotating spiral galaxy for a PointCloud.
///
/// Points are placed once per point count and grouped into radial bands. Each batch rotates every
/// band by its own angle, so a point costs one 2×2 matrix multiply and the arms ripple without
/// winding up over time. Values in [0, 1] grow from the core to the rim; a few arm points get a
/// fixed mid value to stand out as star-forming regions.
class GalaxyGenerator final {
public:
    using Batch = GalaxyBatch;

    /// \brief Number of radial bands rotated independently.
    static constexpr auto kBandCount = 32;
    /// \brief Galaxy radius; halo points may lie slightly outside it.
    static constexpr auto kRadius = 1.0f;

    /// \brief Fills \a batch with \a pointCount points rotated to \a timeSeconds.
    void generate(Batch& batch, int pointCount, double timeSeconds);

    /// \brief Returns the rotation angle of radial band \a band at \a timeSeconds, in radians.
    static double bandAngleAt(int band, double timeSeconds);

    /// \brief Returns the unrotated XY position of every point, grouped by band.
    const std::vector<float>& basePositions() const;
    /// \brief Returns the index of the first point of every band, followed by the point count.
    const std::array<int, kBandCount + 1>& bandStarts() const;

private:
    void rebuildGalaxy(int pointCount);

    std::vector<float> basePositions_;
    std::vector<float> baseValues_;
    std::array<int, kBandCount + 1> bandStarts_{};
    int builtPointCount_{0};
};

} // namespace QAccelPlotExample
