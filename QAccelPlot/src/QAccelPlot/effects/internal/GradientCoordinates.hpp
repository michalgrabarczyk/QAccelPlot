//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/effects/GradientColorTypes.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
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
inline float unboundedGradientCoordinate(const Qt::Orientation direction, const qreal value, const qreal minimum, const qreal maximum)
{
    constexpr auto minimumRange = qreal{1e-12};
    const auto range = std::max(maximum - minimum, minimumRange);
    const auto normalized = (value - minimum) / range;
    return static_cast<float>(direction == Qt::Vertical ? 1.0 - normalized : normalized);
}

/// \brief Converts an interpolated x = log10(value / reference) to the palette coordinate scale * (10^x - 1) + offset.
struct LogGradientMapping {
    qreal reference{1.0}; ///< Positive value that keeps x small and 10^x within float range.
    float scale{0.0f};    ///< Multiplier of 10^x - 1.
    float offset{0.0f};   ///< Palette coordinate of the reference value.
};

/// \brief Returns the mapping of a gradient running from \a minimum to \a maximum along a log axis.
inline LogGradientMapping logGradientMapping(const Qt::Orientation direction, const qreal minimum, const qreal maximum)
{
    const auto largest = std::max(std::abs(minimum), std::abs(maximum));
    const auto reference = largest > 0.0 ? largest : 1.0;
    const auto offset = unboundedGradientCoordinate(direction, reference, minimum, maximum);
    return {reference, offset - unboundedGradientCoordinate(direction, 0.0, minimum, maximum), offset};
}

/// \brief Returns log10(\a value / \a reference), the value to interpolate across a log axis.
inline float logGradientValue(const qreal value, const qreal reference)
{
    return static_cast<float>(std::log10(std::max(value, std::numeric_limits<qreal>::min())) - std::log10(reference));
}

/// \brief Fills each unset gradient value bound of \a payload from [\a dataMin, \a dataMax].
///
/// Bounds resolve independently, so a set bound is kept when the other bound is unset.
template <typename Payload> void resolveGradientValueRange(Payload& payload, const qreal dataMin, const qreal dataMax)
{
    if (!payload.gradientValueMin.has_value()) {
        payload.gradientValueMin = dataMin;
    }
    if (!payload.gradientValueMax.has_value()) {
        payload.gradientValueMax = dataMax;
    }
}

} // namespace QAccelPlot::Internal
