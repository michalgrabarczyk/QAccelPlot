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

QJSValue UnitTickLabelFormatter::prefixCallback() const
{
    return prefixCallback_;
}

void UnitTickLabelFormatter::setPrefixCallback(const QJSValue& callback)
{
    prefixCallback_ = callback;
    emit prefixCallbackChanged();
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
    if (tickSet.values.isEmpty()) {
        return TickSetLabels{{}, unit_, {}};
    }
    const auto prefix = sharedPrefix(tickSet);
    const auto step = Internal::toPrefix(tickSet.step, prefix);
    auto result = TickSetLabels{{}, prefix.symbol + unit_, TickSetFormat{prefix.exponent}};
    result.labels.reserve(tickSet.values.size());
    for (const auto value : tickSet.values) {
        const auto prefixedValue = Internal::toPrefix(value, prefix);
        result.labels.append(tickSet.logScale ? formatLogTick(prefixedValue) : format(prefixedValue, step));
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

Internal::UnitPrefix UnitTickLabelFormatter::sharedPrefix(const TickSet& tickSet) const
{
    if (allowedPrefixes_.isEmpty()) {
        return {};
    }
    if (prefixMode_ == Fixed) {
        return Internal::snappedUnitPrefix(fixedPrefix_.exponent, allowedPrefixes_);
    }
    if (prefixMode_ == Custom) {
        if (const auto prefix = callPrefixCallback(tickSet)) {
            return Internal::snappedUnitPrefix(prefix->exponent, allowedPrefixes_);
        }
    }
    const auto largestTick = std::max(std::abs(tickSet.values.front()), std::abs(tickSet.values.back()));
    return Internal::largestMagnitudePrefix(largestTick, allowedPrefixes_);
}

std::optional<Internal::UnitPrefix> UnitTickLabelFormatter::callPrefixCallback(const TickSet& tickSet) const
{
    if (!prefixCallback_.isCallable()) {
        return std::nullopt;
    }
    auto* const engine = qjsEngine(this);
    if (engine == nullptr) {
        qCWarning(lcQAccelPlot) << "UnitTickLabelFormatter: prefix callback needs a formatter created by a QML or JavaScript engine";
        return std::nullopt;
    }
    const auto result = prefixCallback_.call({QJSValue{tickSet.viewportMin}, QJSValue{tickSet.viewportMax}, jsArray(*engine, tickSet.values)});
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
