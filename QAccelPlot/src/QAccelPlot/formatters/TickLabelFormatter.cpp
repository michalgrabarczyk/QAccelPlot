//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
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

QString TickLabelFormatter::doFormatLogTick(const qreal value) const
{
    return doFormat(value, value);
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
