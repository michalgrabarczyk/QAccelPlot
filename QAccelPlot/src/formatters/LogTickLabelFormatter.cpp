//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "formatters/LogTickLabelFormatter.hpp"

#include <cmath>

namespace QAccelPlot {

LogTickLabelFormatter::LogTickLabelFormatter(QObject* parent)
    : TickLabelFormatter(parent)
{
}

QString LogTickLabelFormatter::doFormat(const qreal value, const qreal tickStep) const
{
    // Maximum fractional deviation from an integer exponent before switching to 'g' notation.
    constexpr static auto kLogExponentTolerance = double{0.01};
    // Significant digits used to format non-power-of-ten values.
    constexpr static auto kLogNonPowerSignificantDigits = int{3};

    const auto logValue = std::log10(value);
    if (std::abs(logValue - std::round(logValue)) < kLogExponentTolerance) {
        const auto exp = static_cast<int>(std::round(logValue));
        return QString("1e%1").arg(exp);
    }
    return QString::number(value, 'g', kLogNonPowerSignificantDigits);
}

} // namespace QAccelPlot
