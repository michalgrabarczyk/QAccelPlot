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

int compareItemFades(const ItemFade& left, const ItemFade& right) noexcept
{
    const auto leftKey = std::tie(left.first, left.count, left.progress, left.stagger, left.exit, left.easing);
    const auto rightKey = std::tie(right.first, right.count, right.progress, right.stagger, right.exit, right.easing);
    if (leftKey == rightKey) {
        return 0;
    }
    return leftKey < rightKey ? -1 : 1;
}

} // namespace QAccelPlot::Internal
