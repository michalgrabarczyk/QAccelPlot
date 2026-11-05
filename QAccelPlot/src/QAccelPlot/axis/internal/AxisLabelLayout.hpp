//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <Qt>

namespace QAccelPlot::Internal {

/// \brief Inputs that place an axis label relative to the axis baseline.
struct AxisLabelLayoutParams {
    QRectF dataRect;                  ///< \brief Axis span the label is aligned along, in item coordinates.
    qreal baseline{0.0};              ///< \brief Baseline position across the axis: y for horizontal axes, x for vertical ones.
    Qt::Edge outward{Qt::BottomEdge}; ///< \brief Direction from the baseline toward the label.
    qreal alongFraction{0.5};         ///< \brief Position along the axis: 0 at the minimum-value end, 1 at the maximum-value end.
    QSizeF textSize;                  ///< \brief Unrotated size of the label text.
    qreal rotation{0.0};              ///< \brief Clockwise text rotation in degrees.
    qreal padding{0.0};               ///< \brief Gap between the baseline and the near edge of the rotated text.
    QPointF offset;                   ///< \brief Pixel offset applied after alignment and padding.
};

/// \brief Returns the size of the box that holds \a size rotated by \a degrees.
[[nodiscard]] QSizeF rotatedBoundingSize(const QSizeF& size, qreal degrees);

/// \brief Returns the center of the axis label described by \a params, in item coordinates.
///
/// The rotated text box is kept flush with the axis ends: at fraction 0 or 1 its edge meets the
/// end of \c dataRect. On vertical axes the minimum-value end is the bottom.
[[nodiscard]] QPointF axisLabelCenter(const AxisLabelLayoutParams& params);

} // namespace QAccelPlot::Internal
