//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "effects/GradientColorTypes.hpp"

#include <algorithm>

namespace QAccelPlot {

/// \brief Returns an unbounded palette coordinate for a data-space \a value.
///
/// Coordinates outside [0, 1] are intentionally preserved for interpolation.
/// Renderers must clamp only after interpolation, immediately before sampling
/// the gradient, so values beyond the configured range retain endpoint colors.
inline float unboundedGradientCoordinate(const GradientDirection direction, const qreal value, const qreal minimum, const qreal maximum)
{
    constexpr auto minimumRange = qreal{1e-12};
    const auto range = std::max(maximum - minimum, minimumRange);
    const auto normalized = (value - minimum) / range;
    return static_cast<float>(direction == GradientDirection::Vertical ? 1.0 - normalized : normalized);
}

} // namespace QAccelPlot
