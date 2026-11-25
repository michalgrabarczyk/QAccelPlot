//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/formatters/UnitTickLabelFormatter.hpp"
#include "QAccelPlot/formatters/internal/NumericLabel.hpp"
#include "QAccelPlot/formatters/internal/UnitPrefix.hpp"

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

UnitTickLabelFormatter::UnitTickLabelFormatter(QObject* parent)
    : TickLabelFormatter(parent)
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
    const auto prefix = Internal::largestMagnitudePrefix(largestTick, Internal::defaultUnitPrefixes());
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
    const auto prefix = Internal::prefixWithExponent(tickSetFormat.prefixExponent, Internal::defaultUnitPrefixes());
    const auto number = format(Internal::toPrefix(value, prefix), Internal::toPrefix(resolution, prefix));
    const auto prefixedUnit = prefix.symbol + unit_;
    return prefixedUnit.isEmpty() ? number : number + QLatin1Char(' ') + prefixedUnit;
}

} // namespace QAccelPlot
