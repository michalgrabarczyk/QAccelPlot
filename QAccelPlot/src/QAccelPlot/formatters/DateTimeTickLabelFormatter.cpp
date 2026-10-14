//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/formatters/DateTimeTickLabelFormatter.hpp"

#include <QDateTime>

#include <cmath>
#include <limits>

namespace QAccelPlot {

DateTimeTickLabelFormatter::DateTimeTickLabelFormatter(QObject* parent)
    : TickLabelFormatter(parent)
{
}

QString DateTimeTickLabelFormatter::dateTimeFormat() const
{
    return dateTimeFormat_;
}

void DateTimeTickLabelFormatter::setDateTimeFormat(const QString& format)
{
    if (dateTimeFormat_ == format) {
        return;
    }
    dateTimeFormat_ = format;
    emit dateTimeFormatChanged();
    emit formatChanged();
}

QString DateTimeTickLabelFormatter::doFormat(const qreal value, [[maybe_unused]] const qreal tickStep) const
{
    // qint64's maximum rounds up to 2^63 as a double, so the upper bound is exclusive.
    constexpr auto minimum = static_cast<qreal>(std::numeric_limits<qint64>::min());
    if (!std::isfinite(value) || value < minimum || value >= -minimum) {
        return {};
    }
    return QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(value)).toString(dateTimeFormat_);
}

} // namespace QAccelPlot
