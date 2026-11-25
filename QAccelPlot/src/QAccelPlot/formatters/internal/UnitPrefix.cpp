//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/formatters/internal/UnitPrefix.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>

namespace QAccelPlot::Internal {

namespace {

constexpr auto kMicroSign = char16_t{0x00B5};
constexpr auto kGreekMu = char16_t{0x03BC};

} // namespace

const QList<UnitPrefix>& siUnitPrefixes()
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
        {QString{QChar{kMicroSign}}, -6},
        {QStringLiteral("m"), -3},
        {QStringLiteral("c"), -2},
        {QStringLiteral("d"), -1},
        {QString{}, 0},
        {QStringLiteral("da"), 1},
        {QStringLiteral("h"), 2},
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

const QList<UnitPrefix>& defaultUnitPrefixes()
{
    static const auto prefixes = [] {
        auto result = QList<UnitPrefix>{};
        std::copy_if(
            siUnitPrefixes().cbegin(), siUnitPrefixes().cend(), std::back_inserter(result), [](const UnitPrefix& prefix) { return prefix.exponent % 3 == 0; });
        return result;
    }();
    return prefixes;
}

std::optional<UnitPrefix> unitPrefixFromSymbol(const QString& symbol)
{
    const auto isMicro = symbol == QLatin1String("u") || symbol == QString{QChar{kGreekMu}};
    const auto canonical = isMicro ? QString{QChar{kMicroSign}} : symbol;
    const auto& prefixes = siUnitPrefixes();
    const auto it = std::find_if(prefixes.cbegin(), prefixes.cend(), [&canonical](const UnitPrefix& prefix) { return prefix.symbol == canonical; });
    return it != prefixes.cend() ? std::optional<UnitPrefix>{*it} : std::nullopt;
}

QList<UnitPrefix> sortedUnitPrefixes(QList<UnitPrefix> prefixes)
{
    const auto byExponent = [](const UnitPrefix& left, const UnitPrefix& right) { return left.exponent < right.exponent; };
    std::sort(prefixes.begin(), prefixes.end(), byExponent);
    prefixes.erase(std::unique(prefixes.begin(), prefixes.end()), prefixes.end());
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

UnitPrefix snappedUnitPrefix(const int exponent, const QList<UnitPrefix>& prefixes)
{
    if (prefixes.isEmpty()) {
        return {};
    }
    const auto it = std::find_if(prefixes.crbegin(), prefixes.crend(), [exponent](const UnitPrefix& prefix) { return prefix.exponent <= exponent; });
    return it != prefixes.crend() ? *it : prefixes.first();
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
