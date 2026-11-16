//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/internal/GlowStyle.hpp"

#include <tuple>

namespace QAccelPlot::Internal {

int compareGlowStyles(const GlowStyle& left, const GlowStyle& right) noexcept
{
    const auto leftKey = std::make_tuple(left.color.rgba(), left.radius, left.intensity);
    const auto rightKey = std::make_tuple(right.color.rgba(), right.radius, right.intensity);
    if (leftKey == rightKey) {
        return 0;
    }
    return leftKey < rightKey ? -1 : 1;
}

} // namespace QAccelPlot::Internal
