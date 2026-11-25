//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/formatters/TickLabelFormatter.hpp"

namespace QAccelPlot {

TickLabelFormatter::TickLabelFormatter(QObject* parent)
    : QObject(parent)
{
}

QString TickLabelFormatter::format(const qreal value, const qreal tickStep) const
{
    auto label = QString{};
    if (callTickLabel(value, tickStep, label)) {
        return label;
    }
    return doFormat(value, tickStep);
}

QString TickLabelFormatter::formatLogTick(const qreal value) const
{
    auto label = QString{};
    if (callTickLabel(value, value, label)) {
        return label;
    }
    return doFormatLogTick(value);
}

TickSetLabels TickLabelFormatter::formatTicks(const TickSet& tickSet) const
{
    auto result = doFormatTicks(tickSet);
    result.labels.resize(tickSet.values.size());
    return result;
}

QString TickLabelFormatter::formatValue(const qreal value, const qreal resolution, const TickSetFormat& tickSetFormat) const
{
    return doFormatValue(value, resolution, tickSetFormat);
}

qreal TickLabelFormatter::minimumTickStep() const
{
    return 0.0;
}

QString TickLabelFormatter::doFormatLogTick(const qreal value) const
{
    return doFormat(value, value);
}

TickSetLabels TickLabelFormatter::doFormatTicks(const TickSet& tickSet) const
{
    auto result = TickSetLabels{};
    result.labels.reserve(tickSet.values.size());
    for (const auto value : tickSet.values) {
        result.labels.append(tickSet.logScale ? formatLogTick(value) : format(value, tickSet.step));
    }
    return result;
}

QString TickLabelFormatter::doFormatValue(const qreal value, const qreal resolution, [[maybe_unused]] const TickSetFormat& tickSetFormat) const
{
    return format(value, resolution);
}

bool TickLabelFormatter::callTickLabel(const qreal value, const qreal tickStep, QString& label) const
{
    if (!tickLabel_.isCallable()) {
        return false;
    }
    const auto args = QList<QJSValue>{QJSValue(value), QJSValue(tickStep)};
    const auto result = tickLabel_.call(args);
    if (result.isError()) {
        return false;
    }
    label = result.toString();
    return true;
}

QJSValue TickLabelFormatter::tickLabel() const
{
    return tickLabel_;
}

void TickLabelFormatter::setTickLabel(const QJSValue& tickLabel)
{
    tickLabel_ = tickLabel;
    emit tickLabelChanged();
    emit formatChanged();
}

} // namespace QAccelPlot
