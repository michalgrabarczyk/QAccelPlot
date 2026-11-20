//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/inspection/internal/InspectionTypes.hpp"

#include <QList>

#include <atomic>
#include <vector>

namespace QAccelPlot::Internal {

/// \brief Immutable k-d tree over the valid data points of a series, with their order along each axis.
class InspectionIndex {
public:
    struct Point {
        double x;
        double y;
        int index;
    };

    /// \brief Builds the tree from \a points; returns false when \a cancelled was set meanwhile.
    bool build(std::vector<Point>&& points, bool logX, bool logY, const std::atomic_bool& cancelled);
    [[nodiscard]] std::size_t storageBytes() const;
    /// \brief Moves the indexed data point with source \a index to \a x, \a y in place.
    ///
    /// Node bounds and summaries along the point's path are recomputed, so queries stay exact; the
    /// tree may only get less balanced. Returns false when the data point is not indexed.
    bool movePoint(int index, double x, double y);
    /// \brief Returns the data point nearest to \a pixel along \a axis.
    [[nodiscard]] InspectionHit nearestAlong(const InspectionMetric& metric, InspectionAxis axis, double pixel, double radius) const;
    [[nodiscard]] InspectionHit nearest(const InspectionMetric& metric, const QPointF& position, double radius) const;
    /// \brief Returns the data points on either side of \a value on \a axis.
    [[nodiscard]] InspectionNeighbors neighbors(InspectionAxis axis, double value) const;
    [[nodiscard]] SummaryAccumulator summarize(const InspectionBounds& bounds) const;
    void collect(const InspectionBounds& bounds, int offset, int limit, QList<int>& indices) const;

private:
    struct Node {
        InspectionBounds bounds;
        SummaryAccumulator summary;
        int first;
        int last;
        int left{-1};
        int right{-1};
        int maxIndex{-1};
    };

    // Widens node's bounds, maximum index, and summary by point.
    static void include(Node& node, const Point& point);

    void buildNodes(const std::atomic_bool& cancelled);
    int createNode(int first, int last, const std::atomic_bool& cancelled);
    // Moves the entry for the point at position in order to where value sorts, before the point gets value.
    void reorder(InspectionAxis axis, std::vector<int>& order, int position, double value);
    // Recomputes the nodes containing position, from its leaf up to the root.
    void refreshNodes(int position);
    // Fills order with the positions in points_ sorted by their coordinate along axis, then by source index.
    void sortOrder(InspectionAxis axis, std::vector<int>& order) const;
    const std::vector<int>& order(InspectionAxis axis) const;
    // Returns the coordinate along axis of the point at a position in points_.
    double key(InspectionAxis axis, int position) const;
    double distanceToBounds(const InspectionBounds& bounds, const InspectionMetric& metric, const QPointF& position) const;
    double mappedX(double value) const;
    double mappedY(double value) const;

    bool logX_{false};
    bool logY_{false};
    std::vector<Point> points_;
    std::vector<int> xOrder_;
    std::vector<int> yOrder_;
    std::vector<Node> nodes_;
};

} // namespace QAccelPlot::Internal
