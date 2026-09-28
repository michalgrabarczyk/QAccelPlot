//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "SineLookup.hpp"

#include <array>
#include <cmath>

namespace QAccelPlotExample {
namespace {

constexpr auto kTableSize = std::size_t{1} << 14;
constexpr auto kTwoPi = 6.283185307179586476925286766559;
constexpr auto kAccumulatorRange = 18446744073709551616.0L; // 2^64

const std::array<float, kTableSize>& sineTable()
{
    static const auto table = []() {
        auto values = std::array<float, kTableSize>{};
        for (auto i = std::size_t{0}; i < values.size(); ++i) {
            const auto angle = kTwoPi * static_cast<double>(i) / static_cast<double>(values.size());
            values[i] = static_cast<float>(std::sin(angle));
        }
        return values;
    }();
    return table;
}

// Maps an angle to a 64-bit fixed-point fraction of a full turn.
std::uint64_t toAccumulator(const double angle, const long double rounding)
{
    auto normalizedAngle = std::fmod(angle, kTwoPi);
    if (normalizedAngle < 0.0) {
        normalizedAngle += kTwoPi;
    }

    const auto scaled = static_cast<long double>(normalizedAngle) * (kAccumulatorRange / static_cast<long double>(kTwoPi)) + rounding;
    return scaled >= kAccumulatorRange ? 0 : static_cast<std::uint64_t>(scaled);
}

} // namespace

SineLookupCursor::SineLookupCursor(const double initialPhase, const double angleStep)
    : table_(sineTable().data())
    , accumulator_(toAccumulator(initialPhase, 0.0L))
    , step_(toAccumulator(angleStep, 0.5L))
{
    static_assert(kTableSize == std::size_t{1} << kTableBits);
}

} // namespace QAccelPlotExample
