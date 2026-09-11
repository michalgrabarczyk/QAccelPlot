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

namespace {

QString superscriptExponent(const int exponent)
{
    // Superscript digits use U+2070, U+00B9, U+00B2, U+00B3, and U+2074–U+2079;
    // the superscript minus sign is U+207B.
    const auto superscriptDigits = QStringLiteral("\u2070\u00B9\u00B2\u00B3\u2074\u2075\u2076\u2077\u2078\u2079");
    const auto exponentText = QString::number(exponent);
    auto result = QString{};
    result.reserve(exponentText.size());

    for (const auto character : exponentText) {
        if (character == QLatin1Char('-')) {
            result += QStringLiteral("\u207B");
        } else {
            result += superscriptDigits.at(character.digitValue());
        }
    }
    return result;
}

}

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
        return QStringLiteral("10") + superscriptExponent(exp);
    }
    return QString::number(value, 'g', kLogNonPowerSignificantDigits);
}

} // namespace QAccelPlot
