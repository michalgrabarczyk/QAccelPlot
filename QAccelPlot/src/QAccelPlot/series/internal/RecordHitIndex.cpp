//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/series/internal/RecordHitIndex.hpp"

#include "QAccelPlot/axis/Axis.hpp"

namespace QAccelPlot::Internal {

void RecordHitIndex::invalidate()
{
    valid_ = false;
}

bool RecordHitIndex::valid() const
{
    return valid_;
}

int RecordHitIndex::nearest(const Query& query, const Axis& xAxis, const Axis& yAxis, const QSizeF& size, const Distance& distance) const
{
    const auto& position = query.position;
    const auto x1 = xAxis.pixelToCoord(position.x() - query.reach, size.width());
    const auto x2 = xAxis.pixelToCoord(position.x() + query.reach, size.width());
    const auto y1 = yAxis.pixelToCoord(position.y() - query.reach, size.height());
    const auto y2 = yAxis.pixelToCoord(position.y() + query.reach, size.height());

    auto nearest = -1;
    auto nearestDistance = query.radius;
    // Accepts no candidate, so that the grid offers each one in the box to find the nearest.
    grid_.queryTopmost(std::min(x1, x2), std::min(y1, y2), std::max(x1, x2), std::max(y1, y2), [&](const int index) {
        const auto candidate = index < query.drawnCount ? distance(index) : std::nullopt;
        if (candidate && *candidate <= query.radius && (nearest < 0 || *candidate < nearestDistance || (*candidate == nearestDistance && index > nearest))) {
            nearest = index;
            nearestDistance = *candidate;
        }
        return false;
    });
    return nearest;
}

} // namespace QAccelPlot::Internal
