//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QtGlobal>

#include <utility>

namespace QAccelPlot {
class Axis;
}

namespace QAccelPlot::Internal {

/// \brief Maps rectangle edge \a value on \a axis to item pixels along an item side \a length pixels long.
///
/// Infinite edges, and non-positive edges on a log axis, map to infinity on the matching side,
/// as axis_pixel.glsl extends them past the plot edge.
qreal edgePixel(double value, const Axis& axis, qreal length);

/// \brief Returns the pixel span between edges \a a and \a b, widened around its center to at least \a minimumSize.
///
/// Matches \c widenedSpan() in axis_pixel.glsl, so hit tests use the drawn size.
std::pair<qreal, qreal> widenedSpan(qreal a, qreal b, qreal minimumSize);

/// \brief Returns the pixel distance from (\a x, \a y) to the box spanning \a xs and \a ys; 0 inside.
qreal distanceToBox(qreal x, qreal y, const std::pair<qreal, qreal>& xs, const std::pair<qreal, qreal>& ys);

} // namespace QAccelPlot::Internal
