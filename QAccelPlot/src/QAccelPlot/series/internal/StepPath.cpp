//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/internal/StepPath.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/series/LineCurveGapFilter.hpp"

#include <cmath>

namespace QAccelPlot::Internal {

namespace {

// Returns the X of data point \a index when it can place a step corner: finite, and positive on a log axis.
std::optional<qreal> stepX(const CurveDataView& data, const int index, const bool logScaleX)
{
    const auto x = data.x(index);
    return isValidCoordinate(x, logScaleX) ? std::optional<qreal>{x} : std::nullopt;
}

// Halfway between \a a and \a b on screen: the arithmetic mean, or the geometric mean on a log axis.
qreal stepCenterX(const qreal a, const qreal b, const bool logScaleX)
{
    return logScaleX ? std::sqrt(a) * std::sqrt(b) : (a + b) * 0.5;
}

} // namespace

int stepCornerCount(const StepMode mode) noexcept
{
    switch (mode) {
    case StepMode::None:
        return 1;
    case StepMode::Before:
    case StepMode::After:
        return 2;
    case StepMode::Center:
        return 3;
    }
    return 1;
}

std::optional<QPointF> stepCorner(
    const CurveDataView& data, const int from, const int to, const int corner, const StepMode mode, const bool logScaleX, const bool logScaleY)
{
    if (corner == 0 || mode == StepMode::None) {
        return LineCurveGapFilter::isValidPoint(data, from, logScaleX, logScaleY) ? std::optional<QPointF>{QPointF{data.x(from), data.y(from)}} : std::nullopt;
    }
    const auto ownerIsTo = mode == StepMode::Before || corner == 2;
    const auto owner = ownerIsTo ? to : from;
    if (!LineCurveGapFilter::isValidPoint(data, owner, logScaleX, logScaleY)) {
        return std::nullopt;
    }
    const auto fromX = stepX(data, from, logScaleX);
    const auto toX = stepX(data, to, logScaleX);
    auto x = std::optional<qreal>{};
    if (mode == StepMode::After) {
        x = toX;
    } else if (mode == StepMode::Before) {
        x = fromX;
    } else if (fromX && toX) {
        x = stepCenterX(*fromX, *toX, logScaleX);
    }
    return x ? std::optional<QPointF>{QPointF{*x, data.y(owner)}} : std::nullopt;
}

std::optional<QPointF> linePathPoint(
    const CurveDataView& data, const int pointCount, const int index, const int corner, const StepMode mode, const bool logScaleX, const bool logScaleY)
{
    const auto drawnCorner = index >= pointCount - 1 ? 0 : corner;
    return stepCorner(data, index, index + 1, drawnCorner, mode, logScaleX, logScaleY);
}

} // namespace QAccelPlot::Internal
