//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QtGlobal>

namespace QAccelPlot::Internal {

/// \brief Bounds of an axis viewport or a colormap value range.
struct ValueRange {
    qreal min{0.0}; ///< \brief Lower bound.
    qreal max{1.0}; ///< \brief Upper bound.
};

/// \brief Returns \a range shifted by \a fraction of its extent; a positive \a fraction raises both bounds.
///
/// With \a logarithmic set, the shift is applied in \c log10 space, so both bounds are scaled by
/// the same ratio. A range that is not positive is shifted linearly instead.
[[nodiscard]] ValueRange pannedRange(const ValueRange& range, qreal fraction, bool logarithmic);

/// \brief Returns \a range with its extent multiplied by \a factor, keeping the value at \a anchorRatio in place.
///
/// \a anchorRatio is a position inside the range: 0 at \c min and 1 at \c max. With \a logarithmic
/// set, the extent is measured in \c log10 space. A range that is not positive is zoomed linearly
/// instead.
[[nodiscard]] ValueRange zoomedRange(const ValueRange& range, qreal factor, qreal anchorRatio, bool logarithmic);

/// \brief Returns the extent multiplier of one mouse-wheel step for a \a zoomScaleFactor in (0, 1).
///
/// Zooming in multiplies the extent by the factor and zooming out by its reciprocal. A factor of 0
/// or below counts as 0.01 and a factor of 1 or above as 0.99, so a step always zooms and never
/// collapses the range.
[[nodiscard]] qreal wheelZoomFactor(qreal zoomScaleFactor, bool zoomingIn);

} // namespace QAccelPlot::Internal
