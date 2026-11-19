//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/series/internal/RectGeometry.hpp"

#include "QAccelPlot/axis/Axis.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace QAccelPlot::Internal {

qreal edgePixel(const double value, const Axis& axis, const qreal length)
{
    constexpr auto kInf = std::numeric_limits<qreal>::infinity();
    const auto belowRange = value == -kInf || (axis.logScale() && value <= 0.0);
    if (!belowRange && value != kInf) {
        return axis.coordToPixel(value, length);
    }
    const auto pixelsGrowWithData = axis.coordToPixel(axis.viewportMax(), length) >= axis.coordToPixel(axis.viewportMin(), length);
    return pixelsGrowWithData == belowRange ? -kInf : kInf;
}

std::pair<qreal, qreal> widenedSpan(const qreal a, const qreal b, const qreal minimumSize)
{
    const auto low = std::min(a, b);
    const auto high = std::max(a, b);
    const auto grow = 0.5 * std::max(minimumSize - (high - low), qreal{0.0});
    return {low - grow, high + grow};
}

qreal distanceToBox(const qreal x, const qreal y, const std::pair<qreal, qreal>& xs, const std::pair<qreal, qreal>& ys)
{
    const auto dx = std::max({xs.first - x, qreal{0.0}, x - xs.second});
    const auto dy = std::max({ys.first - y, qreal{0.0}, y - ys.second});
    return std::hypot(dx, dy);
}

} // namespace QAccelPlot::Internal
