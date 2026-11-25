//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/formatters/internal/UnitPrefix.hpp"

#include <algorithm>
#include <cmath>

namespace QAccelPlot::Internal {

const QList<UnitPrefix>& defaultUnitPrefixes()
{
    static const auto prefixes = QList<UnitPrefix>{
        {QStringLiteral("q"), -30},
        {QStringLiteral("r"), -27},
        {QStringLiteral("y"), -24},
        {QStringLiteral("z"), -21},
        {QStringLiteral("a"), -18},
        {QStringLiteral("f"), -15},
        {QStringLiteral("p"), -12},
        {QStringLiteral("n"), -9},
        {QStringLiteral("µ"), -6},
        {QStringLiteral("m"), -3},
        {QString{}, 0},
        {QStringLiteral("k"), 3},
        {QStringLiteral("M"), 6},
        {QStringLiteral("G"), 9},
        {QStringLiteral("T"), 12},
        {QStringLiteral("P"), 15},
        {QStringLiteral("E"), 18},
        {QStringLiteral("Z"), 21},
        {QStringLiteral("Y"), 24},
        {QStringLiteral("R"), 27},
        {QStringLiteral("Q"), 30},
    };
    return prefixes;
}

UnitPrefix largestMagnitudePrefix(const qreal magnitude, const QList<UnitPrefix>& prefixes)
{
    if (prefixes.isEmpty()) {
        return {};
    }
    // Lets a tick such as 0.0009999999999 m, left by tick-position arithmetic, still count as 1 mm.
    constexpr static auto kRelativeTolerance = qreal{1e-9};
    const auto value = std::isfinite(magnitude) && magnitude > 0.0 ? magnitude : qreal{1.0};
    for (auto it = prefixes.crbegin(); it != prefixes.crend(); ++it) {
        if (toPrefix(value, *it) >= 1.0 - kRelativeTolerance) {
            return *it;
        }
    }
    return prefixes.first();
}

UnitPrefix prefixWithExponent(const int exponent, const QList<UnitPrefix>& prefixes)
{
    const auto it = std::find_if(prefixes.cbegin(), prefixes.cend(), [exponent](const UnitPrefix& prefix) { return prefix.exponent == exponent; });
    return it != prefixes.cend() ? *it : UnitPrefix{};
}

qreal toPrefix(const qreal value, const UnitPrefix& prefix)
{
    // Multiplying by a positive power of ten keeps values such as 0.003 m closer to 3 mm than dividing by 0.001.
    if (prefix.exponent < 0) {
        return value * std::pow(10.0, -prefix.exponent);
    }
    return value / std::pow(10.0, prefix.exponent);
}

} // namespace QAccelPlot::Internal
