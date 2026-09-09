//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "axis/Axis.hpp"

#include <vector>

namespace QAccelPlot {

/// \brief Axis-aligned bounding box (AABB) for a contiguous block of curve points, used for hit-test culling.
///
/// The AABB is stored in data-space so it never needs rebuilding during pan/zoom — only when
/// the underlying data changes. It intentionally extends one point beyond each chunk boundary
/// so that line segments bridging adjacent chunks are always covered.
struct CurveChunk {
    int start;  ///< \brief Index of the first point in the data buffer.
    int count;  ///< \brief Number of points owned by this chunk.
    float minX; ///< \brief Minimum X of the data-space AABB (extended to bridging points).
    float maxX; ///< \brief Maximum X of the data-space AABB (extended to bridging points).
    float minY; ///< \brief Minimum Y of the data-space AABB (extended to bridging points).
    float maxY; ///< \brief Maximum Y of the data-space AABB (extended to bridging points).
};

/// \brief All inputs required for a \c contains() hit-test, bundled to reduce parameter count.
struct CurveHitTestParams {
    const std::vector<float>& data;        ///< \brief Interleaved XY float data buffer.
    int pointCount;                        ///< \brief Number of points in \c data.
    const std::vector<CurveChunk>& chunks; ///< \brief Precomputed chunk bounding boxes.
    Axis* xAxis;                           ///< \brief Horizontal axis for coordinate mapping.
    Axis* yAxis;                           ///< \brief Vertical axis for coordinate mapping.
    qreal width;                           ///< \brief Width of the curve item in pixels.
    qreal height;                          ///< \brief Height of the curve item in pixels.
    qreal hitThreshold;                    ///< \brief Hit distance threshold in pixels.
};

} // namespace QAccelPlot
