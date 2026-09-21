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

/// \brief Uniform-grid spatial index with bounded per-rectangle storage.
///
/// Each item is stored as four consecutive doubles (x1, y1, x2, y2) in a flat array
/// with a configurable \a valuesPerItem stride. Intended for use by RectangleList
/// and similar shape types.
/// Rectangles spanning more than 64 cells are stored once and checked separately during queries.
class SpatialGrid {
public:
    /// \brief Rebuilds the spatial index from \a data containing \a itemCount axis-aligned rectangles.
    /// \param data Pointer to the flat double array (x1, y1, x2, y2 per item, with \a valuesPerItem stride).
    /// \param itemCount Number of rectangles in \a data.
    /// \param valuesPerItem Number of doubles per rectangle entry (default 4).
    void build(const double* data, int itemCount, int valuesPerItem = 4);
    /// \brief Returns the index of the topmost rectangle that contains point (\a x, \a y), or -1 if none.
    int query(double x, double y) const;

private:
    struct ItemBounds {
        double minX;
        double minY;
        double maxX;
        double maxY;

        bool contains(double x, double y) const;
    };

    void computeDataBounds(const double* data, int itemCount, int valuesPerItem);
    void computeGridDimensions(int itemCount);
    void fillSpatialGrid(int itemCount);

    double minX_{0.0};
    double minY_{0.0};
    double maxX_{1.0};
    double maxY_{1.0};
    int cols_{0};
    int rows_{0};
    double cellW_{1.0};
    double cellH_{1.0};
    std::vector<ItemBounds> itemBounds_;
    std::vector<std::vector<int>> cells_;
    std::vector<int> largeItems_;
};

} // namespace QAccelPlot
