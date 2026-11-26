//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/formatters/NumericTickLabelFormatter.hpp"
#include "QAccelPlot/formatters/internal/NumericLabel.hpp"

namespace QAccelPlot {

NumericTickLabelFormatter::NumericTickLabelFormatter(QObject* parent)
    : TickLabelFormatter(parent)
{
}

QString NumericTickLabelFormatter::doFormat(const qreal value, const qreal tickStep) const
{
    return Internal::numericLabel(value, tickStep);
}

QString NumericTickLabelFormatter::doFormatLogTick(const qreal value) const
{
    return Internal::logTickLabel(value);
}

} // namespace QAccelPlot
