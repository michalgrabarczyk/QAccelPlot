//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/formatters/NumericTickLabelFormatter.hpp"

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

namespace {

// Returns "10" followed by exponent in superscript digits, such as "10²" or "10⁻³".
QString powerOfTen(const int exponent)
{
    // Superscript digits use U+2070, U+00B9, U+00B2, U+00B3, and U+2074–U+2079;
    // the superscript minus sign is U+207B.
    const auto superscriptDigits = QStringLiteral("\u2070\u00B9\u00B2\u00B3\u2074\u2075\u2076\u2077\u2078\u2079");
    const auto exponentText = QString::number(exponent);
    auto result = QStringLiteral("10");
    result.reserve(result.size() + exponentText.size());

    for (const auto character : exponentText) {
        if (character == QLatin1Char('-')) {
            result += QStringLiteral("\u207B");
        } else {
            result += superscriptDigits.at(character.digitValue());
        }
    }
    return result;
}

// Returns the number of decimal places needed to write tickStep exactly, so labels
// spaced by that step are distinguishable without trailing zeros: 1, 2, 5 and 10
// get none, 0.5 and 0.1 get one, 0.25 and 0.05 get two. Non-finite or
// non-positive steps fall back to one decimal place.
int decimalsForStep(const qreal tickStep)
{
    if (!(tickStep > 0.0) || !std::isfinite(tickStep)) {
        return 1;
    }
    // Relative tolerance for deciding that step * 10^d is a whole number. It absorbs
    // floating-point noise such as 0.1 + 0.2 - 0.2 == 0.10000000000000003 without
    // accepting a genuinely finer step.
    constexpr static auto kRelativeTolerance = qreal{1e-9};
    // Decimal places needed to reach the step's leading significant digit. The
    // tolerance keeps a decade step whose log10 lands just below an integer
    // (e.g. -1.0000000000000002 for 0.1) from gaining an extra place.
    const auto leadingDigitDecimals = std::max(0, static_cast<int>(-std::floor(std::log10(tickStep) + kRelativeTolerance)));
    // Extra digits allowed beyond the leading one, enough for steps such as 0.25 or
    // 0.125. Steps that never terminate (e.g. 1/3) are cut off at this limit.
    constexpr static auto kMaxExtraDecimals = 2;
    const auto maxDecimals = leadingDigitDecimals + kMaxExtraDecimals;
    auto scale = qreal{1.0};
    for (auto decimals = 0; decimals < maxDecimals; ++decimals, scale *= 10.0) {
        const auto scaled = tickStep * scale;
        if (std::abs(scaled - std::round(scaled)) <= scaled * kRelativeTolerance) {
            return decimals;
        }
    }
    return maxDecimals;
}

} // namespace

NumericTickLabelFormatter::NumericTickLabelFormatter(QObject* parent)
    : TickLabelFormatter(parent)
{
}

QString NumericTickLabelFormatter::doFormat(const qreal value, const qreal tickStep) const
{
    const auto precision = decimalsForStep(tickStep);
    auto text = QString::number(value, 'f', precision);
    // A tiny negative value accumulated from floating-point tick-position arithmetic
    // (e.g. loopStart + i*step) can round to zero at the display precision while
    // keeping its sign, printing a misleading "-0.00". Strip the sign in that case.
    if (text.startsWith(QLatin1Char('-')) && text.toDouble() == 0.0) {
        text.remove(0, 1);
    }
    return text;
}

QString NumericTickLabelFormatter::doFormatLogTick(const qreal value) const
{
    // Maximum deviation of log10(value) from an integer for the value to count as a decade.
    constexpr static auto kDecadeTolerance = qreal{1e-9};
    if (!(value > 0.0) || !std::isfinite(value)) {
        return doFormat(value, value);
    }
    const auto logValue = std::log10(value);
    const auto exponent = std::round(logValue);
    if (std::abs(logValue - exponent) > kDecadeTolerance) {
        return doFormat(value, value);
    }
    return powerOfTen(static_cast<int>(exponent));
}

} // namespace QAccelPlot
