//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/inspection/internal/InspectionTypes.hpp"

#include <QList>

#include <vector>

namespace QAccelPlot {

/// \brief Inspection queries that read a series' own buffer whose X values are finite and non-decreasing.
///
/// Positions are found by binary search, so no index is built and appended records are visible
/// immediately. Per-block Y statistics are cached lazily to answer large range summaries and to
/// prune nearest-point searches.
class SourceInspection {
public:
    /// \brief Returns true when every X in \a source is finite and not smaller than its predecessor.
    [[nodiscard]] static bool isSortedX(const InspectionSource& source);

    /// \brief Discards cached statistics after the records were replaced or their validity changed.
    void reset();
    /// \brief Updates cached statistics after one record was appended, giving \a count records.
    void appended(int count);
    /// \brief Returns the bytes held by cached statistics.
    [[nodiscard]] std::size_t storageBytes() const;

    [[nodiscard]] InspectionHit nearestX(const InspectionSource& source, const InspectionMetric& metric, double pixelX, double radius) const;
    [[nodiscard]] InspectionHit nearest(const InspectionSource& source, const InspectionMetric& metric, const QPointF& position, double radius);
    [[nodiscard]] InspectionNeighbors neighbors(const InspectionSource& source, double x) const;
    [[nodiscard]] SummaryAccumulator summarize(const InspectionSource& source, const InspectionBounds& bounds);
    void collect(const InspectionSource& source, const InspectionBounds& bounds, int offset, int limit, QList<int>& indices);

private:
    struct Block {
        SummaryAccumulator stats;
        double xMin{0.0};
        double xMax{0.0};
        bool computed{false};
    };

    void syncBlocks(int count);
    const Block& block(const InspectionSource& source, int index);
    bool visitBlock(const InspectionSource& source, const InspectionMetric& metric, const QPointF& position, int index, InspectionHit& best);

    std::vector<Block> blocks_;
};

/// \brief Inspection queries that scan every record; used for small series with unordered X.
namespace InspectionScan {

[[nodiscard]] InspectionHit nearestX(const InspectionSource& source, const InspectionMetric& metric, double pixelX, double radius);
[[nodiscard]] InspectionHit nearest(const InspectionSource& source, const InspectionMetric& metric, const QPointF& position, double radius);
[[nodiscard]] InspectionNeighbors neighbors(const InspectionSource& source, double x);
[[nodiscard]] SummaryAccumulator summarize(const InspectionSource& source, const InspectionBounds& bounds);
void collect(const InspectionSource& source, const InspectionBounds& bounds, int offset, int limit, QList<int>& indices);

} // namespace InspectionScan

} // namespace QAccelPlot
