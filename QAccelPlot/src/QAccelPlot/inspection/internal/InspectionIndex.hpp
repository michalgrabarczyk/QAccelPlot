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

#include <atomic>
#include <vector>

namespace QAccelPlot {

/// \brief Immutable k-d tree over the valid samples of a series whose X values are unordered.
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
    [[nodiscard]] InspectionHit nearestX(const InspectionMetric& metric, double pixelX, double radius) const;
    [[nodiscard]] InspectionHit nearest(const InspectionMetric& metric, const QPointF& position, double radius) const;
    [[nodiscard]] InspectionNeighbors neighbors(double x) const;
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

    void buildNodes(const std::atomic_bool& cancelled);
    int createNode(int first, int last, const std::atomic_bool& cancelled);
    double distanceToBounds(const InspectionBounds& bounds, const InspectionMetric& metric, const QPointF& position) const;
    double mappedX(double value) const;
    double mappedY(double value) const;

    bool logX_{false};
    bool logY_{false};
    std::vector<Point> points_;
    std::vector<int> xOrder_;
    std::vector<Node> nodes_;
};

} // namespace QAccelPlot
