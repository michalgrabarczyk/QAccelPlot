//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/formatters/UnitTickLabelFormatter.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/formatters/internal/NumericLabel.hpp"
#include "QAccelPlot/formatters/internal/UnitPrefix.hpp"

#include <QJSEngine>

#include <algorithm>
#include <cmath>
#include <utility>

namespace QAccelPlot {

namespace {

QJSValue jsArray(QJSEngine& engine, const QList<qreal>& values)
{
    auto array = engine.newArray(static_cast<uint>(values.size()));
    for (auto i = qsizetype{0}; i < values.size(); ++i) {
        array.setProperty(static_cast<quint32>(i), QJSValue{values.at(i)});
    }
    return array;
}

/// \brief Returns \a number followed by a space and \a prefixedUnit, or \a number alone when \a prefixedUnit is empty.
QString withUnit(const QString& number, const QString& prefixedUnit)
{
    return prefixedUnit.isEmpty() ? number : number + QLatin1Char(' ') + prefixedUnit;
}

} // namespace

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

QString UnitTickLabelFormatter::fixedPrefix() const
{
    return fixedPrefix_.symbol;
}

void UnitTickLabelFormatter::setFixedPrefix(const QString& symbol)
{
    const auto prefix = Internal::unitPrefixFromSymbol(symbol);
    if (!prefix) {
        qCWarning(lcQAccelPlot) << "UnitTickLabelFormatter: ignoring unknown unit prefix" << symbol;
        return;
    }
    if (fixedPrefix_ == *prefix) {
        return;
    }
    fixedPrefix_ = *prefix;
    emit fixedPrefixChanged();
    emit formatChanged();
}

QJSValue UnitTickLabelFormatter::customPrefix() const
{
    return customPrefix_;
}

void UnitTickLabelFormatter::setCustomPrefix(const QJSValue& callback)
{
    customPrefix_ = callback;
    emit customPrefixChanged();
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

UnitTickLabelFormatter::UnitPosition UnitTickLabelFormatter::unitPosition() const
{
    return unitPosition_;
}

void UnitTickLabelFormatter::setUnitPosition(const UnitPosition position)
{
    if (unitPosition_ == position) {
        return;
    }
    unitPosition_ = position;
    emit unitPositionChanged();
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
    if (prefixMode_ == PerTick) {
        return perTickLabels(tickSet);
    }
    const auto onTickLabels = unitPosition_ == TickLabels;
    if (tickSet.values.isEmpty()) {
        return TickSetLabels{{}, onTickLabels ? QString{} : unit_, {}};
    }
    const auto prefix = sharedPrefix(tickSet);
    const auto prefixedUnit = prefix.symbol + unit_;
    const auto step = Internal::toPrefix(tickSet.step, prefix);
    auto result = TickSetLabels{{}, onTickLabels ? QString{} : prefixedUnit, TickSetFormat{prefix.exponent}};
    result.labels.reserve(tickSet.values.size());
    for (const auto value : tickSet.values) {
        const auto prefixedValue = Internal::toPrefix(value, prefix);
        const auto number = tickSet.logScale ? formatLogTick(prefixedValue) : format(prefixedValue, step);
        result.labels.append(onTickLabels ? withUnit(number, prefixedUnit) : number);
    }
    return result;
}

QString UnitTickLabelFormatter::doFormatValue(const qreal value, const qreal resolution, const TickSetFormat& tickSetFormat) const
{
    const auto prefix = prefixMode_ == PerTick ? ownPrefix(value, resolution) : Internal::prefixWithExponent(tickSetFormat.prefixExponent, allowedPrefixes_);
    const auto number = format(Internal::toPrefix(value, prefix), Internal::toPrefix(resolution, prefix));
    return withUnit(number, prefix.symbol + unit_);
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

Internal::UnitPrefix UnitTickLabelFormatter::sharedPrefix(const TickSet& tickSet) const
{
    if (allowedPrefixes_.isEmpty()) {
        return {};
    }
    if (prefixMode_ == Fixed) {
        return Internal::snappedUnitPrefix(fixedPrefix_.exponent, allowedPrefixes_);
    }
    if (prefixMode_ == Custom) {
        if (const auto prefix = callCustomPrefix(tickSet)) {
            return Internal::snappedUnitPrefix(prefix->exponent, allowedPrefixes_);
        }
    }
    const auto largestTick = std::max(std::abs(tickSet.values.front()), std::abs(tickSet.values.back()));
    return Internal::largestMagnitudePrefix(largestTick, allowedPrefixes_);
}

Internal::UnitPrefix UnitTickLabelFormatter::ownPrefix(const qreal value, const qreal precision) const
{
    // A value within half the precision of zero, such as a zero tick, takes the prefix of the precision.
    const auto magnitude = std::abs(value) < precision / 2.0 ? precision : std::abs(value);
    return Internal::largestMagnitudePrefix(magnitude, allowedPrefixes_);
}

TickSetLabels UnitTickLabelFormatter::perTickLabels(const TickSet& tickSet) const
{
    auto result = TickSetLabels{};
    result.labels.reserve(tickSet.values.size());
    for (const auto value : tickSet.values) {
        const auto prefix = ownPrefix(value, tickSet.step);
        const auto prefixedValue = Internal::toPrefix(value, prefix);
        const auto number = format(prefixedValue, tickSet.logScale ? prefixedValue : Internal::toPrefix(tickSet.step, prefix));
        result.labels.append(withUnit(number, prefix.symbol + unit_));
    }
    return result;
}

std::optional<Internal::UnitPrefix> UnitTickLabelFormatter::callCustomPrefix(const TickSet& tickSet) const
{
    if (!customPrefix_.isCallable()) {
        return std::nullopt;
    }
    auto* const engine = qjsEngine(this);
    if (engine == nullptr) {
        qCWarning(lcQAccelPlot) << "UnitTickLabelFormatter: prefix callback needs a formatter created by a QML or JavaScript engine";
        return std::nullopt;
    }
    const auto result = customPrefix_.call({QJSValue{tickSet.viewportMin}, QJSValue{tickSet.viewportMax}, jsArray(*engine, tickSet.values)});
    if (result.isError()) {
        qCWarning(lcQAccelPlot) << "UnitTickLabelFormatter: prefix callback failed:" << result.toString();
        return std::nullopt;
    }
    if (!result.isString()) {
        qCWarning(lcQAccelPlot) << "UnitTickLabelFormatter: prefix callback returned a non-string" << result.toString();
        return std::nullopt;
    }
    const auto prefix = Internal::unitPrefixFromSymbol(result.toString());
    if (!prefix) {
        qCWarning(lcQAccelPlot) << "UnitTickLabelFormatter: prefix callback returned an unknown unit prefix" << result.toString();
    }
    return prefix;
}

} // namespace QAccelPlot
