//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "BenchmarkScenario.hpp"

#include <cmath>
#include <cstdint>

namespace QAccelPlot {

struct BenchmarkViewport {
    double minimum;
    double maximum;
};

inline int sourceBufferCount(const BenchmarkScenario::UpdateMode mode)
{
    return mode == BenchmarkScenario::UpdateMode::Static ? 1 : 2;
}

inline BenchmarkViewport staticPanViewport(const int pointCount, const std::int64_t elapsedMilliseconds)
{
    constexpr auto kTwoPi = 6.283185307179586;
    constexpr auto kPanPeriodMs = std::int64_t{2000};
    const auto phase = static_cast<double>(elapsedMilliseconds % kPanPeriodMs) / static_cast<double>(kPanPeriodMs) * kTwoPi;
    const auto panRange = static_cast<double>(pointCount) * 0.3;
    const auto offset = (std::sin(phase) * 0.5 + 0.5) * panRange;
    return {offset, offset + static_cast<double>(pointCount) * 0.7};
}

} // namespace QAccelPlot
