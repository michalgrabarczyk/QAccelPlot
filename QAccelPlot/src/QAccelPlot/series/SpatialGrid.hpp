//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <functional>
#include <vector>

namespace QAccelPlot {

/// \brief Uniform-grid spatial index with bounded per-rectangle storage.
///
/// Each item is stored as four consecutive doubles (x1, y1, x2, y2) in a flat array
/// with a configurable \a valuesPerItem stride. Intended for use by RectangleSeries
/// and similar shape types.
/// Rectangles spanning more than 64 cells are stored once and checked separately during queries.
/// An infinite edge leaves a rectangle unbounded in that direction; a rectangle with a NaN edge is never hit.
class SpatialGrid {
public:
    /// \brief Rebuilds the spatial index from \a data containing \a itemCount axis-aligned rectangles.
    /// \param data Pointer to the flat double array (x1, y1, x2, y2 per item, with \a valuesPerItem stride).
    /// \param itemCount Number of rectangles in \a data.
    /// \param valuesPerItem Number of doubles per rectangle entry (default 4).
    void build(const double* data, int itemCount, int valuesPerItem = 4);
    /// \brief Rebuilds the spatial index from single-precision \a data, like \c build().
    void buildF(const float* data, int itemCount, int valuesPerItem = 4);
    /// \brief Returns the index of the topmost rectangle that contains point (\a x, \a y), or -1 if none.
    int query(double x, double y) const;
    /// \brief Returns the highest index among rectangles overlapping the box for which \a accept returns true, or -1.
    ///
    /// Lets callers apply a precise hit test, e.g. in pixel space, to the few candidates near a point.
    int queryTopmost(double minX, double minY, double maxX, double maxY, const std::function<bool(int)>& accept) const;

    /// \brief Returns what \c queryTopmost() returns on a grid built from the same rectangles, without building one.
    ///
    /// Tests rectangles from the last to the first, so a call costs up to one pass over \a data. That
    /// is cheaper than a rebuild while the data is replaced after only a few queries.
    static int scanTopmost(
        const double* data, int itemCount, double minX, double minY, double maxX, double maxY, const std::function<bool(int)>& accept, int valuesPerItem = 4);
    /// \brief Scans single-precision \a data, like \c scanTopmost().
    static int scanTopmostF(
        const float* data, int itemCount, double minX, double minY, double maxX, double maxY, const std::function<bool(int)>& accept, int valuesPerItem = 4);

private:
    struct ItemBounds {
        double minX;
        double minY;
        double maxX;
        double maxY;

        bool contains(double x, double y) const;
        bool overlaps(double boxMinX, double boxMinY, double boxMaxX, double boxMaxY) const;
        bool isValid() const;
    };

    // Cell coordinate of \a value, clamped to [0, count - 1] so infinite values map to the edge cells.
    static int cellIndex(double value, double min, double cellSize, int count);

    template <typename T>
    static int scanTopmostFrom(const T* data, int itemCount, int valuesPerItem, const ItemBounds& box, const std::function<bool(int)>& accept);
    template <typename T> void buildFrom(const T* data, int itemCount, int valuesPerItem);
    template <typename T> void computeDataBounds(const T* data, int itemCount, int valuesPerItem);
    void computeGridDimensions(int itemCount);
    void fillSpatialGrid(int itemCount);

    // The grid covers the finite extent of all edges. An axis without finite edges gets a single cell.
    double minX_{0.0};
    double minY_{0.0};
    double maxX_{1.0};
    double maxY_{1.0};
    bool boundedX_{false};
    bool boundedY_{false};
    int cols_{0};
    int rows_{0};
    double cellW_{1.0};
    double cellH_{1.0};
    std::vector<ItemBounds> itemBounds_;
    std::vector<std::vector<int>> cells_;
    std::vector<int> largeItems_;
};

} // namespace QAccelPlot
