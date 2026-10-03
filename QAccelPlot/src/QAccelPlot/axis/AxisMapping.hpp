//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

namespace QAccelPlot {

/// \brief Snapshot of an axis viewport that maps between data values and pixel positions.
///
/// Obtained from \c Axis::mapping(). It is the single implementation of the axis mapping, shared
/// by \c Axis::coordToPixel(), \c Axis::pixelToCoord(), and the inspection queries.
struct AxisMapping {
    double origin{0.0};      ///< \brief Viewport minimum in mapped space: \c log10 on logarithmic axes.
    double span{1.0};        ///< \brief Viewport extent in mapped space; negative for an inverted viewport.
    bool logarithmic{false}; ///< \brief Whether values are mapped through \c log10.
    bool flipped{false};     ///< \brief Whether pixel positions grow against data values, as on vertical axes.
    bool valid{false};       ///< \brief False for a collapsed, nonfinite, or nonpositive logarithmic viewport.

    /// \brief Returns \a value in mapped space.
    [[nodiscard]] double toMapped(double value) const noexcept;
    /// \brief Maps a data-space \a value to a pixel position along an axis of \a length pixels.
    [[nodiscard]] double toPixel(double value, double length) const noexcept;
    /// \brief Maps a \a pixel position along an axis of \a length pixels back to a data-space value.
    [[nodiscard]] double toCoord(double pixel, double length) const noexcept;
};

} // namespace QAccelPlot
