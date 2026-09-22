//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QtGlobal>

#include <vector>

namespace QAccelPlot {

/// \brief Uniform-grid spatial index for nearest-point queries over large point sets.
///
/// Points are read from a flat float array with a configurable stride whose first two
/// components are \c x and \c y. Coordinates are indexed in \e mapped space: log10 of the
/// value on logarithmic axes, the value itself otherwise, so a pixel radius converts to a
/// constant mapped-space radius at any zoom level. Non-finite points, and non-positive
/// points on logarithmic axes, are excluded.
///
/// Cells are stored in a compressed layout (\c cellStart_ offsets into one index array),
/// which keeps a rebuild over a million points to a handful of allocations.
///
/// \sa PointCloud
class PointSpatialIndex {
public:
    /// \brief Coordinate mapping applied before indexing.
    struct Mapping {
        bool logX{false}; ///< \brief Index log10(x) instead of x.
        bool logY{false}; ///< \brief Index log10(y) instead of y.

        /// \brief Returns \c true when both mappings are identical.
        bool operator==(const Mapping& other) const;
    };

    /// \brief Rebuilds the index from \a pointCount points in \a data using \a stride floats per point.
    void build(const float* data, int pointCount, int stride, Mapping mapping);
    /// \brief Rebuilds the index with the identity mapping, for linear axes.
    ///
    /// A separate overload rather than a defaulted \c Mapping argument: a nested class's default
    /// member initializers are not usable in a default argument until the enclosing class is
    /// complete, which MSVC accepts but Clang and GCC reject.
    void build(const float* data, int pointCount, int stride = 2);
    /// \brief Removes all indexed points.
    void clear();

    /// \brief Returns \c true when no valid point is indexed.
    bool isEmpty() const;
    /// \brief Returns the number of indexed (valid) points.
    int validPointCount() const;
    /// \brief Returns the mapping used by the last build.
    Mapping mapping() const;

    /// \brief Returns the index of the point nearest to (\a x, \a y) inside the ellipse with radii
    /// \a radiusX and \a radiusY, or -1 when none. All arguments are in mapped space.
    ///
    /// Distance is normalized by the radii, so an anisotropic pixel radius behaves like a circle
    /// on screen. Ties resolve to the highest index, the point drawn last.
    int nearest(double x, double y, double radiusX, double radiusY) const;

    /// \brief Maps \a value for indexing. Returns \c false when the value cannot be indexed.
    static bool mapCoordinate(double value, bool logarithmic, double& mapped);

private:
    struct IndexedPoint {
        float x;
        float y;
        int sourceIndex;
    };

    void collectValidPoints(const float* data, int pointCount, int stride);
    void computeGridDimensions();
    void fillCells();
    int cellColumn(double x) const;
    int cellRow(double y) const;

    Mapping mapping_;
    double minX_{0.0};
    double minY_{0.0};
    double cellWidth_{1.0};
    double cellHeight_{1.0};
    int columns_{0};
    int rows_{0};
    std::vector<IndexedPoint> points_;
    std::vector<int> cellStart_;   // columns_ * rows_ + 1 offsets into cellEntries_
    std::vector<int> cellEntries_; // positions in points_, grouped by cell
};

} // namespace QAccelPlot
