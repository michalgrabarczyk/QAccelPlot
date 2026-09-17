//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <vector>

namespace QAccelPlot {

/// \brief Uniform-grid spatial index for O(1) point-in-rectangle hit-test queries.
///
/// Each item is stored as four consecutive floats (x1, y1, x2, y2) in a flat array
/// with a configurable \a floatsPerItem stride. Intended for use by RectangleList
/// and similar shape types.
class SpatialGrid {
public:
    /// \brief Rebuilds the spatial index from \a data containing \a itemCount axis-aligned rectangles.
    /// \param data Pointer to the flat float array (x1, y1, x2, y2 per item, with \a floatsPerItem stride).
    /// \param itemCount Number of rectangles in \a data.
    /// \param floatsPerItem Number of floats per rectangle entry (default 4).
    void build(const float* data, int itemCount, int floatsPerItem = 4);
    /// \brief Returns the index of the topmost rectangle that contains point (\a x, \a y), or -1 if none.
    int query(float x, float y) const;

private:
    struct ItemBounds {
        float minX;
        float minY;
        float maxX;
        float maxY;

        bool contains(float x, float y) const;
    };

    void computeDataBounds(const float* data, int itemCount, int floatsPerItem);
    void computeGridDimensions(int itemCount);
    void fillSpatialGrid(int itemCount);

    float minX_{0.0f};
    float minY_{0.0f};
    float maxX_{1.0f};
    float maxY_{1.0f};
    int cols_{0};
    int rows_{0};
    float cellW_{1.0f};
    float cellH_{1.0f};
    std::vector<ItemBounds> itemBounds_;
    std::vector<std::vector<int>> cells_;
};

} // namespace QAccelPlot
