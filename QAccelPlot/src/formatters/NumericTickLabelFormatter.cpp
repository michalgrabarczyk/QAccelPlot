//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "formatters/NumericTickLabelFormatter.hpp"

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

NumericTickLabelFormatter::NumericTickLabelFormatter(QObject* parent)
    : TickLabelFormatter(parent)
{
}

QString NumericTickLabelFormatter::doFormat(const qreal value, const qreal tickStep) const
{
    // Bias added before ceil() so a tick step sitting exactly on a decade boundary
    // (e.g. 0.1 → log10 = -1.0) still yields one decimal place rather than zero.
    constexpr static auto kPrecisionRoundingBias = double{0.5};
    const auto precision = (tickStep > 0.0) ? std::max(0, static_cast<int>(std::ceil(-std::log10(tickStep) + kPrecisionRoundingBias))) : 1;
    auto text = QString::number(value, 'f', precision);
    // A tiny negative value accumulated from floating-point tick-position arithmetic
    // (e.g. loopStart + i*step) can round to zero at the display precision while
    // keeping its sign, printing a misleading "-0.00". Strip the sign in that case.
    if (text.startsWith(QLatin1Char('-')) && text.toDouble() == 0.0) {
        text.remove(0, 1);
    }
    return text;
}

} // namespace QAccelPlot
