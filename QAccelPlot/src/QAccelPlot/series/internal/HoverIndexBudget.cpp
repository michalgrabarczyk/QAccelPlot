//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/internal/HoverIndexBudget.hpp"

#include <algorithm>

namespace QAccelPlot::Internal {

namespace {
constexpr auto kMinimumRecordsToLearnFrom = 10'000;
// The least a scan or a build counts for, so a clock too coarse to time one still makes progress.
constexpr auto kShortestDuration = std::chrono::nanoseconds{1};
} // namespace

HoverIndexBudget::HoverIndexBudget(const CostPerRecord assumedBuildCostPerRecord)
    : buildCostPerRecord_(assumedBuildCostPerRecord)
{
}

void HoverIndexBudget::reset()
{
    scanCost_ = std::chrono::nanoseconds{0};
}

bool HoverIndexBudget::buildDue(const int recordCount) const
{
    return scanCost_ >= buildCostPerRecord_ * recordCount;
}

void HoverIndexBudget::addScan(const std::chrono::nanoseconds duration)
{
    scanCost_ += std::max(duration, kShortestDuration);
}

void HoverIndexBudget::addBuild(const std::chrono::nanoseconds duration, const int recordCount)
{
    if (recordCount < kMinimumRecordsToLearnFrom) {
        return;
    }
    buildCostPerRecord_ = CostPerRecord{std::max(duration, kShortestDuration)} / recordCount;
}

} // namespace QAccelPlot::Internal
