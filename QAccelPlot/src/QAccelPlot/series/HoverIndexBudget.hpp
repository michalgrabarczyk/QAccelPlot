//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QtGlobal>

#include <chrono>
#include <utility>

namespace QAccelPlot {

/// \brief Decides when queried data is worth a hover index.
///
/// A scan answers a query on any data at the cost of a pass over it. An index answers almost
/// for free, but must be rebuilt after every data change. Queries therefore scan until the
/// scans since the last data change have cost as much as a build would. That keeps the total
/// within about twice the cheaper choice, whether the data changes every frame or never.
class HoverIndexBudget {
public:
    /// \brief Creates a budget that assumes \a assumedBuildNanosecondsPerRecord until a build has been timed.
    explicit HoverIndexBudget(double assumedBuildNanosecondsPerRecord);

    /// \brief Forgets the scans paid so far. Call when the index stops matching the data.
    void reset();
    /// \brief Returns \c true once the scans have cost as much as indexing \a recordCount records.
    bool buildDue(int recordCount) const;
    /// \brief Adds the cost of one scan.
    void addScan(std::chrono::nanoseconds duration);
    /// \brief Replaces the assumed build cost with that of a build of \a recordCount records.
    ///
    /// Builds of few records are ignored: their fixed overhead would overstate the cost per record.
    void addBuild(std::chrono::nanoseconds duration, int recordCount);

    /// \brief Runs \a scan, adds its duration, and returns its result.
    template <typename Scan> auto timeScan(Scan&& scan)
    {
        const auto start = std::chrono::steady_clock::now();
        auto result = std::forward<Scan>(scan)();
        addScan(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start));
        return result;
    }

    /// \brief Runs \a build and records its duration as the cost of indexing \a recordCount records.
    template <typename Build> void timeBuild(const int recordCount, Build&& build)
    {
        const auto start = std::chrono::steady_clock::now();
        std::forward<Build>(build)();
        addBuild(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start), recordCount);
    }

private:
    double buildNanosecondsPerRecord_;
    qint64 scanNanoseconds_{0};
};

} // namespace QAccelPlot
