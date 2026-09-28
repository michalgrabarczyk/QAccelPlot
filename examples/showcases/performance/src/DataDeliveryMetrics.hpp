//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <atomic>
#include <cstdint>

namespace QAccelPlotExample {

/// \brief Returns the current steady-clock time in nanoseconds.
std::int64_t steadyNanoseconds() noexcept;

/// \brief Tracks application rate and timing for generated datasets.
class DataDeliveryMetrics final {
public:
    /// \brief Metrics accumulated since the preceding snapshot.
    struct Snapshot {
        std::uint64_t appliedBatches{0};
        std::int64_t longestDataGapNanoseconds{0};
    };

    /// \brief Records a dataset application at the supplied steady-clock timestamp.
    void dataApplied(std::int64_t appliedAtNanoseconds) noexcept;

    /// \brief Forgets the last application time, so the next dataset does not record a gap.
    void restartGapTracking() noexcept;

    /// \brief Returns and resets the interval counters while preserving sequence history.
    [[nodiscard]] Snapshot takeSnapshot() noexcept;

private:
    std::atomic<std::int64_t> lastAppliedAtNanoseconds_{0};
    std::atomic<std::uint64_t> appliedBatches_{0};
    std::atomic<std::int64_t> longestDataGapNanoseconds_{0};
};

} // namespace QAccelPlotExample
