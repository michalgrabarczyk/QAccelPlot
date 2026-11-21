//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/effects/GradientColorTypes.hpp"

#include <algorithm>
#include <cmath>
#include <optional>

namespace QAccelPlot::Internal {

/// \brief Returns \a value, or \c std::nullopt when it is NaN, the unset state of an effect property.
inline std::optional<qreal> valueIfSet(const qreal value)
{
    return std::isnan(value) ? std::nullopt : std::optional<qreal>{value};
}

/// \brief Returns an unbounded palette coordinate for a data-space \a value.
///
/// Coordinates outside [0, 1] are intentionally preserved for interpolation.
/// Renderers must clamp only after interpolation, immediately before sampling
/// the gradient, so values beyond the configured range retain endpoint colors.
/// A \a minimum above \a maximum reverses the ramp; equal bounds make a step at that value.
inline float unboundedGradientCoordinate(const Qt::Orientation direction, const qreal value, const qreal minimum, const qreal maximum)
{
    constexpr auto zeroRangeStep = qreal{1e-12};
    const auto signedRange = maximum - minimum;
    const auto range = signedRange == 0.0 ? zeroRangeStep : signedRange;
    const auto normalized = (value - minimum) / range;
    return static_cast<float>(direction == Qt::Vertical ? 1.0 - normalized : normalized);
}

/// \brief Fills each unset gradient value bound of \a payload from [\a dataMin, \a dataMax].
///
/// A set bound is kept. An unset bound stops at the set one, so a bound beyond the data
/// makes a step instead of reversing the ramp.
template <typename Payload> void resolveGradientValueRange(Payload& payload, const qreal dataMin, const qreal dataMax)
{
    const auto fixedMin = payload.gradientValueMin;
    const auto fixedMax = payload.gradientValueMax;
    payload.gradientValueMin = fixedMin.value_or(fixedMax ? std::min(dataMin, *fixedMax) : dataMin);
    payload.gradientValueMax = fixedMax.value_or(fixedMin ? std::max(dataMax, *fixedMin) : dataMax);
}

} // namespace QAccelPlot::Internal
