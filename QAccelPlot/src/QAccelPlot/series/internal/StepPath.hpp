//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/renderers/CurveRendererParams.hpp"
#include "QAccelPlot/series/StepMode.hpp"

#include <QPointF>

#include <optional>

namespace QAccelPlot::Internal {

/// \brief Returns the number of line path points per data point for \a mode: the data point and the step corners after it.
[[nodiscard]] int stepCornerCount(StepMode mode) noexcept;

/// \brief Returns corner \a corner of the step from data point \a from to data point \a to, in data space.
///
/// Corner 0 is data point \a from. A step corner holds the Y of the data point whose flat step it ends or
/// starts, and is drawn when that data point is valid and the X values placing the corner are usable. Returns
/// \c std::nullopt for a corner that is not drawn. Matches the line shader.
[[nodiscard]] std::optional<QPointF> stepCorner(const CurveDataView& data, int from, int to, int corner, StepMode mode, bool logScaleX, bool logScaleY);

/// \brief Returns path point \a corner of data point \a index on the drawn line of \a pointCount data points.
///
/// The last data point has no step after it, so its corners repeat it. Matches the vertices of the line shader.
[[nodiscard]] std::optional<QPointF> linePathPoint(
    const CurveDataView& data, int pointCount, int index, int corner, StepMode mode, bool logScaleX, bool logScaleY);

} // namespace QAccelPlot::Internal
