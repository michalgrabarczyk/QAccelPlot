//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
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
/// as rect_geometry.glsl extends them past the plot edge.
qreal edgePixel(double value, const Axis& axis, qreal length);

/// \brief Returns the pixel span between edges \a a and \a b, widened around its center to at least \a minimumSize.
///
/// Matches \c widenedSpan() in rect_geometry.glsl, so hit tests use the drawn size.
std::pair<qreal, qreal> widenedSpan(qreal a, qreal b, qreal minimumSize);

} // namespace QAccelPlot::Internal
