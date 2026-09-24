//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <vector>

namespace QAccelPlotExample {

/// Interleaved XY coordinates plus one scalar value per point.
struct PointCloudData {
    std::vector<float> xy;
    std::vector<float> values;
    int pointCount{0};
};

/// Cluster-centered samples, so a frame can be rotated without regenerating the random draw.
struct ClusterCloud {
    std::vector<float> localXy;   // offsets relative to the owning cluster center
    std::vector<float> centers;   // per-point cluster center (x, y)
    std::vector<float> spinRates; // per-point relative angular speed; 0 for background noise
    std::vector<float> values;    // normalized radial distance in [0, 1]
    int pointCount{0};
};

/// Generates three anisotropic Gaussian clusters plus uniform background noise.
///
/// The sequence is fully deterministic across compilers and platforms: it uses
/// std::mt19937 (standardized output) with an in-house uniform/normal transform
/// instead of implementation-defined standard distributions.
ClusterCloud generateClusterCloud(int pointCount, unsigned int seed = 20260912u);

/// Rotates every cluster around its own center by \a phaseRadians and writes the result.
void composeClusterFrame(const ClusterCloud& cloud, float phaseRadians, PointCloudData& frame);

/// Power-law samples for log-log axes. Includes a few non-positive and NaN
/// coordinates on purpose: the series must skip them without breaking ranges.
std::vector<float> generatePowerLawScatter(int pointCount, unsigned int seed = 7u);

/// Number of deliberately invalid points appended by generatePowerLawScatter().
constexpr auto kPowerLawInvalidPointCount = 4;

} // namespace QAccelPlotExample
