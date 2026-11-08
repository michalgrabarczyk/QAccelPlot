//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/internal/ItemFade.hpp"

#include <tuple>

namespace QAccelPlot::Internal {

namespace {

int compareGroups(const ItemFadeGroup& left, const ItemFadeGroup& right) noexcept
{
    // Inactive groups draw alike, whatever their other values.
    if (!left.active() || !right.active()) {
        return static_cast<int>(left.active()) - static_cast<int>(right.active());
    }
    const auto leftKey = std::tie(left.first, left.count, left.exit, left.front, left.time, left.spread, left.length, left.curve, left.starts);
    const auto rightKey = std::tie(right.first, right.count, right.exit, right.front, right.time, right.spread, right.length, right.curve, right.starts);
    if (leftKey == rightKey) {
        return 0;
    }
    return leftKey < rightKey ? -1 : 1;
}

} // namespace

int compareItemFades(const ItemFade& left, const ItemFade& right) noexcept
{
    for (auto index = std::size_t{0}; index < left.groups.size(); ++index) {
        if (const auto order = compareGroups(left.groups[index], right.groups[index]); order != 0) {
            return order;
        }
    }
    return 0;
}

int compareItemTrails(const ItemTrail& left, const ItemTrail& right) noexcept
{
    const auto leftKey = std::tie(left.length, left.minOpacity, left.itemCount);
    const auto rightKey = std::tie(right.length, right.minOpacity, right.itemCount);
    if (leftKey == rightKey) {
        return 0;
    }
    return leftKey < rightKey ? -1 : 1;
}

} // namespace QAccelPlot::Internal
