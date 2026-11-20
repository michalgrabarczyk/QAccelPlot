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
    // One pixel more keeps rounding in pixelToCoord() from dropping a record at the edge; the pixel distance test is exact.
    const auto reach = query.radius + query.overhang + 1.0;
    const auto x1 = xAxis.pixelToCoord(position.x() - reach, size.width());
    const auto x2 = xAxis.pixelToCoord(position.x() + reach, size.width());
    const auto y1 = yAxis.pixelToCoord(position.y() - reach, size.height());
    const auto y2 = yAxis.pixelToCoord(position.y() + reach, size.height());
    return grid_.queryNearest(std::min(x1, x2), std::min(y1, y2), std::max(x1, x2), std::max(y1, y2), query.radius,
        [&](const int index) -> std::optional<double> { return index < query.drawnCount ? distance(index) : std::nullopt; });
}

} // namespace QAccelPlot::Internal
