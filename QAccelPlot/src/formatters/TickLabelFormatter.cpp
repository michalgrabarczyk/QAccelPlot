//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "formatters/TickLabelFormatter.hpp"

namespace QAccelPlot {

TickLabelFormatter::TickLabelFormatter(QObject* parent)
    : QObject(parent)
{
}

QString TickLabelFormatter::format(const qreal value, const qreal tickStep) const
{
    if (tickLabel_.isCallable()) {
        const auto args = QList<QJSValue>{QJSValue(value)};
        const auto result = tickLabel_.call(args);
        if (!result.isError()) {
            return result.toString();
        }
    }
    return doFormat(value, tickStep);
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
