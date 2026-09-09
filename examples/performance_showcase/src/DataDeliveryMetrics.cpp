//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "DataDeliveryMetrics.hpp"

namespace QAccelPlotExample {
namespace {

template<typename T>
void storeMaximum(std::atomic<T>& destination, const T candidate) noexcept
{
    auto current = destination.load(std::memory_order_relaxed);
    while (candidate > current && !destination.compare_exchange_weak(current, candidate, std::memory_order_relaxed)) {
    }
}

} // namespace

void DataDeliveryMetrics::dataApplied(const std::int64_t appliedAtNanoseconds) noexcept
{
    appliedBatches_.fetch_add(1, std::memory_order_relaxed);

    const auto previousAppliedAt = lastAppliedAtNanoseconds_.exchange(appliedAtNanoseconds, std::memory_order_relaxed);
    if (previousAppliedAt > 0 && appliedAtNanoseconds > previousAppliedAt) {
        storeMaximum(longestDataGapNanoseconds_, appliedAtNanoseconds - previousAppliedAt);
    }
}

DataDeliveryMetrics::Snapshot DataDeliveryMetrics::takeSnapshot() noexcept
{
    return {
        appliedBatches_.exchange(0, std::memory_order_relaxed),
        longestDataGapNanoseconds_.exchange(0, std::memory_order_relaxed),
    };
}

} // namespace QAccelPlotExample
