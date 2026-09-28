//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <cstddef>
#include <cstdint>

namespace QAccelPlotExample {

/// \brief Steps through sin(initialPhase + k × angleStep) for k = 0, 1, 2, …
///
/// Values come from a 16K-entry table indexed by a 64-bit fixed-point phase accumulator, so the
/// table stays in cache and no trigonometry runs per value. The absolute error is below 0.0002.
class SineLookupCursor final {
public:
    /// \brief Starts at \a initialPhase and advances by \a angleStep per value; both in radians.
    SineLookupCursor(double initialPhase, double angleStep);

    /// \brief Returns the current value and advances by one step.
    float next() noexcept
    {
        const auto index = static_cast<std::size_t>((accumulator_ + kIndexRounding) >> kIndexShift);
        accumulator_ += step_;
        return table_[index];
    }

private:
    static constexpr auto kTableBits = 14;
    static constexpr auto kIndexShift = 64 - kTableBits;
    static constexpr auto kIndexRounding = std::uint64_t{1} << (kIndexShift - 1);

    const float* table_;
    std::uint64_t accumulator_;
    std::uint64_t step_;
};

} // namespace QAccelPlotExample
