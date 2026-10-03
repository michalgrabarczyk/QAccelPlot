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

/// \brief Inspection queries that read a series' own buffer whose records are ordered along one axis.
///
/// The \c order argument names the axis whose coordinates are finite and non-decreasing. Positions
/// along it are found by binary search, so no index is built and appended records are visible
/// immediately. Per-block extents and Y statistics are cached lazily to answer large region
/// summaries and to prune nearest-point searches.
class SourceInspection {
public:
    /// \brief Returns true when every coordinate of \a source along \a axis is finite and not smaller than its predecessor.
    [[nodiscard]] static bool isSorted(const InspectionSource& source, InspectionAxis axis);

    /// \brief Discards cached statistics after the records were replaced or their validity changed.
    void reset();
    /// \brief Updates cached statistics after one record was appended, giving \a count records.
    void appended(int count);
    /// \brief Returns the bytes held by cached statistics.
    [[nodiscard]] std::size_t storageBytes() const;

    /// \brief Returns the valid sample nearest to \a pixel along the ordered axis.
    [[nodiscard]] InspectionHit nearestAlong(
        const InspectionSource& source, const InspectionMetric& metric, InspectionAxis order, double pixel, double radius) const;
    [[nodiscard]] InspectionHit nearest(
        const InspectionSource& source, const InspectionMetric& metric, InspectionAxis order, const QPointF& position, double radius);
    /// \brief Returns the valid samples on either side of \a value on the ordered axis.
    [[nodiscard]] InspectionNeighbors neighbors(const InspectionSource& source, InspectionAxis order, double value) const;
    [[nodiscard]] SummaryAccumulator summarize(const InspectionSource& source, InspectionAxis order, const InspectionBounds& bounds);
    void collect(const InspectionSource& source, InspectionAxis order, const InspectionBounds& bounds, int offset, int limit, QList<int>& indices);

private:
    struct Block {
        SummaryAccumulator stats;
        double xMin{0.0};
        double xMax{0.0};
        bool computed{false};
    };

    enum class Coverage { None, Partial, Full };

    void syncBlocks(int count);
    const Block& block(const InspectionSource& source, int index);
    // Tells how the samples of a whole block inside the ordered-axis range relate to the bounds of the other axis.
    Coverage coverage(const InspectionSource& source, InspectionAxis order, const InspectionBounds& bounds, int first, int end);
    bool visitBlock(
        const InspectionSource& source, const InspectionMetric& metric, InspectionAxis order, const QPointF& position, int index, InspectionHit& best);

    std::vector<Block> blocks_;
};

/// \brief Inspection queries that scan every record; used for small series that are not ordered along the queried axis.
namespace InspectionScan {

/// \brief Returns the valid sample nearest to \a pixel along \a axis.
[[nodiscard]] InspectionHit nearestAlong(const InspectionSource& source, const InspectionMetric& metric, InspectionAxis axis, double pixel, double radius);
[[nodiscard]] InspectionHit nearest(const InspectionSource& source, const InspectionMetric& metric, const QPointF& position, double radius);
/// \brief Returns the valid samples on either side of \a value on \a axis.
[[nodiscard]] InspectionNeighbors neighbors(const InspectionSource& source, InspectionAxis axis, double value);
[[nodiscard]] SummaryAccumulator summarize(const InspectionSource& source, const InspectionBounds& bounds);
void collect(const InspectionSource& source, const InspectionBounds& bounds, int offset, int limit, QList<int>& indices);

} // namespace InspectionScan

} // namespace QAccelPlot
