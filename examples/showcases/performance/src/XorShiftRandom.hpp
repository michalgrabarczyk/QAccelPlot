//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace QAccelPlotExample {

/// \brief Small, fast xorshift64* generator with a fixed seed, so generated scenes are reproducible.
class XorShiftRandom final {
public:
    /// \brief Returns a uniform value in [0, 1).
    double uniform()
    {
        state_ ^= state_ >> 12;
        state_ ^= state_ << 25;
        state_ ^= state_ >> 27;
        return static_cast<double>((state_ * 0x2545F4914F6CDD1DULL) >> 11) * 0x1.0p-53;
    }

    /// \brief Returns a standard normal value.
    double gaussian()
    {
        constexpr auto kTwoPi = 6.283185307179586;
        const auto u = std::max(uniform(), 1e-12);
        return std::sqrt(-2.0 * std::log(u)) * std::cos(kTwoPi * uniform());
    }

private:
    std::uint64_t state_{0x9E3779B97F4A7C15ULL};
};

} // namespace QAccelPlotExample
