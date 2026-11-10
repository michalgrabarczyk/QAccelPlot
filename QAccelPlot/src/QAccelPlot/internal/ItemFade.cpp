//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/internal/ItemFade.hpp"

#include <algorithm>
#include <cmath>
#include <tuple>
#include <vector>

namespace QAccelPlot::Internal {

namespace {

/// \brief Returns \a samples, evenly spaced over [0, 1], interpolated at \a position as the shaders do.
float sampleStarts(const std::array<float, kFadeStartSamples>& samples, const double position)
{
    const auto scaled = std::clamp(position, 0.0, 1.0) * (kFadeStartSamples - 1);
    const auto lower = std::min(static_cast<int>(std::floor(scaled)), kFadeStartSamples - 2);
    const auto a = static_cast<double>(samples[static_cast<std::size_t>(lower)]);
    const auto b = static_cast<double>(samples[static_cast<std::size_t>(lower) + 1]);
    return static_cast<float>(a + (scaled - lower) * (b - a));
}

/// \brief Returns the start time, over the spread, that \a group gives the item at stagger \a order.
double startShare(const ItemFadeGroup& group, const double order)
{
    if (group.front) {
        return sampleStarts(group.starts, (order + (group.exit ? 1.0 : 0.0)) / group.count);
    }
    return group.count > 1 ? order / (group.count - 1) : 0.0;
}

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

ItemFadeGroup compactedFadeGroup(const ItemFadeGroup& group, const int first, const std::vector<int>& kept)
{
    auto result = group;
    result.first = first;
    result.count = static_cast<int>(kept.size());
    if (result.count == group.count || result.count == 0) {
        return result;
    }
    // The stagger order in group of the remaining item at the given order in the result.
    const auto originalOrder = [&](const int order) {
        const auto item = kept[static_cast<std::size_t>(group.exit ? result.count - 1 - order : order)];
        return static_cast<double>(group.exit ? group.count - 1 - item : item);
    };
    const auto exitOffset = group.exit ? 1.0 : 0.0;
    for (auto sample = 0; sample < kFadeStartSamples; ++sample) {
        // The shaders sample the starts at share (order + exitOffset) / count.
        const auto order = std::clamp(static_cast<double>(sample) * result.count / (kFadeStartSamples - 1) - exitOffset, 0.0, result.count - 1.0);
        const auto lower = static_cast<int>(std::floor(order));
        const auto upper = std::min(lower + 1, result.count - 1);
        const auto original = originalOrder(lower) + (order - lower) * (originalOrder(upper) - originalOrder(lower));
        result.starts[static_cast<std::size_t>(sample)] = static_cast<float>(startShare(group, original));
    }
    result.front = true;
    return result;
}

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
    const auto leftKey = std::tie(left.length, left.minOpacity, left.itemCount, left.easing);
    const auto rightKey = std::tie(right.length, right.minOpacity, right.itemCount, right.easing);
    if (leftKey == rightKey) {
        return 0;
    }
    return leftKey < rightKey ? -1 : 1;
}

} // namespace QAccelPlot::Internal
