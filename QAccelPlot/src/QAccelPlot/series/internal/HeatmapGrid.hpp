//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <optional>
#include <utility>

namespace QAccelPlot {
struct AxisMapping;
}

namespace QAccelPlot::Internal {

/// \brief How heatmap.frag maps an item-local pixel position \a p to a fractional cell coordinate along one dimension.
struct HeatmapShaderMapping {
    /// \brief Formula the shader evaluates; the values match heatmap.frag.
    enum class Mode {
        Affine = 0,      ///< \brief <tt>offset + scale * p</tt>: the cell spacing matches the axis scale.
        Exponential = 1, ///< \brief <tt>offset + scale * expm1(rate * p)</tt>: linear cells on a log axis.
        Logarithmic = 2  ///< \brief <tt>offset + scale * log1p(shift + rate * p)</tt>: log cells on a linear axis.
    };

    Mode mode{Mode::Affine}; ///< \brief Formula to evaluate.
    double scale{0.0};       ///< \brief Factor of the formula.
    double offset{0.0};      ///< \brief Offset of the formula.
    double rate{0.0};        ///< \brief Per-pixel rate of the formula; unused by \c Affine.
    double shift{0.0};       ///< \brief Argument of \c log1p at pixel 0; used by \c Logarithmic only.
};

/// \brief One dimension of a heatmap grid: \c count cells between the edges \c from and \c to.
struct HeatmapGridDimension {
    double from;     ///< \brief Outer edge of cell 0.
    double to;       ///< \brief Outer edge of the last cell.
    int count;       ///< \brief Number of cells.
    bool logSpacing; ///< \brief Whether the cells are evenly spaced in log10 instead of linearly.

    /// \brief Returns whether both edges are finite, and positive under log spacing.
    bool isValid() const;
    /// \brief Returns the fractional cell coordinate of data \a value, or NaN when no cell can contain it.
    double cellCoordinate(double value) const;
    /// \brief Returns the center of cell \a index: arithmetic under linear spacing, geometric under log spacing.
    double cellCenter(int index) const;
    /// \brief Returns the ordered extent of the grid on an axis that is logarithmic when \a logAxis is set.
    ///
    /// On a log axis the extent starts at the first cell edge above zero. Returns \c std::nullopt for an
    /// invalid dimension, and on a log axis for a grid entirely at or below zero.
    std::optional<std::pair<double, double>> dataExtent(bool logAxis) const;
    /// \brief Returns the mapping from pixels along an axis of \a length pixels, mapped by \a axis, to cell coordinates.
    ///
    /// Computed in double precision relative to pixel 0, so that the single-precision shader only works
    /// with cell coordinates near the visible cells.
    HeatmapShaderMapping shaderMapping(const AxisMapping& axis, double length) const;
};

} // namespace QAccelPlot::Internal
