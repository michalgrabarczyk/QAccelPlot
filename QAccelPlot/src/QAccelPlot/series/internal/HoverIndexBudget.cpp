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
}

HoverIndexBudget::HoverIndexBudget(const double assumedBuildNanosecondsPerRecord)
    : buildNanosecondsPerRecord_(assumedBuildNanosecondsPerRecord)
{
}

void HoverIndexBudget::reset()
{
    scanNanoseconds_ = 0;
}

bool HoverIndexBudget::buildDue(const int recordCount) const
{
    return static_cast<double>(scanNanoseconds_) >= buildNanosecondsPerRecord_ * static_cast<double>(recordCount);
}

void HoverIndexBudget::addScan(const std::chrono::nanoseconds duration)
{
    // At least one nanosecond, so a clock too coarse to time a scan still leads to a build.
    scanNanoseconds_ += std::max<qint64>(duration.count(), 1);
}

void HoverIndexBudget::addBuild(const std::chrono::nanoseconds duration, const int recordCount)
{
    if (recordCount < kMinimumRecordsToLearnFrom) {
        return;
    }
    buildNanosecondsPerRecord_ = static_cast<double>(std::max<qint64>(duration.count(), 1)) / static_cast<double>(recordCount);
}

} // namespace QAccelPlot::Internal
