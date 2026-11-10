//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
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
