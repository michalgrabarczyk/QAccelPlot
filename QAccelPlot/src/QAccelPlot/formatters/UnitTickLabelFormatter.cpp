//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/formatters/UnitTickLabelFormatter.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/formatters/internal/NumericLabel.hpp"
#include "QAccelPlot/formatters/internal/UnitPrefix.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace QAccelPlot {

UnitTickLabelFormatter::UnitTickLabelFormatter(QObject* parent)
    : TickLabelFormatter(parent)
    , allowedPrefixes_(Internal::defaultUnitPrefixes())
{
}

QString UnitTickLabelFormatter::unit() const
{
    return unit_;
}

void UnitTickLabelFormatter::setUnit(const QString& unit)
{
    if (unit_ == unit) {
        return;
    }
    unit_ = unit;
    emit unitChanged();
    emit formatChanged();
}

UnitTickLabelFormatter::PrefixMode UnitTickLabelFormatter::prefixMode() const
{
    return prefixMode_;
}

void UnitTickLabelFormatter::setPrefixMode(const PrefixMode mode)
{
    if (prefixMode_ == mode) {
        return;
    }
    prefixMode_ = mode;
    emit prefixModeChanged();
    emit formatChanged();
}

QStringList UnitTickLabelFormatter::allowedPrefixes() const
{
    auto symbols = QStringList{};
    symbols.reserve(allowedPrefixes_.size());
    for (const auto& prefix : allowedPrefixes_) {
        symbols.append(prefix.symbol);
    }
    return symbols;
}

void UnitTickLabelFormatter::setAllowedPrefixes(const QStringList& symbols)
{
    auto prefixes = QList<Internal::UnitPrefix>{};
    prefixes.reserve(symbols.size());
    for (const auto& symbol : symbols) {
        if (const auto prefix = Internal::unitPrefixFromSymbol(symbol)) {
            prefixes.append(*prefix);
        } else {
            qCWarning(lcQAccelPlot) << "UnitTickLabelFormatter: ignoring unknown unit prefix" << symbol;
        }
    }
    applyAllowedPrefixes(Internal::sortedUnitPrefixes(std::move(prefixes)));
}

void UnitTickLabelFormatter::resetAllowedPrefixes()
{
    applyAllowedPrefixes(Internal::defaultUnitPrefixes());
}

QString UnitTickLabelFormatter::doFormat(const qreal value, const qreal tickStep) const
{
    return Internal::numericLabel(value, tickStep);
}

QString UnitTickLabelFormatter::doFormatLogTick(const qreal value) const
{
    return Internal::logTickLabel(value);
}

TickSetLabels UnitTickLabelFormatter::doFormatTicks(const TickSet& tickSet) const
{
    if (tickSet.logScale || tickSet.values.isEmpty()) {
        auto result = TickLabelFormatter::doFormatTicks(tickSet);
        result.unitText = unit_;
        return result;
    }
    const auto largestTick = std::max(std::abs(tickSet.values.front()), std::abs(tickSet.values.back()));
    const auto prefix = Internal::largestMagnitudePrefix(largestTick, allowedPrefixes_);
    const auto step = Internal::toPrefix(tickSet.step, prefix);
    auto result = TickSetLabels{{}, prefix.symbol + unit_, TickSetFormat{prefix.exponent}};
    result.labels.reserve(tickSet.values.size());
    for (const auto value : tickSet.values) {
        result.labels.append(format(Internal::toPrefix(value, prefix), step));
    }
    return result;
}

QString UnitTickLabelFormatter::doFormatValue(const qreal value, const qreal resolution, const TickSetFormat& tickSetFormat) const
{
    const auto prefix = Internal::prefixWithExponent(tickSetFormat.prefixExponent, allowedPrefixes_);
    const auto number = format(Internal::toPrefix(value, prefix), Internal::toPrefix(resolution, prefix));
    const auto prefixedUnit = prefix.symbol + unit_;
    return prefixedUnit.isEmpty() ? number : number + QLatin1Char(' ') + prefixedUnit;
}

void UnitTickLabelFormatter::applyAllowedPrefixes(const QList<Internal::UnitPrefix>& prefixes)
{
    if (allowedPrefixes_ == prefixes) {
        return;
    }
    allowedPrefixes_ = prefixes;
    emit allowedPrefixesChanged();
    emit formatChanged();
}

} // namespace QAccelPlot
