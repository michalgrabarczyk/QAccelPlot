

# File InspectionIndex.hpp

[**File List**](files.md) **>** [**inspection**](dir_7c3af00b227ed418fdf47d7cd69e8a77.md) **>** [**internal**](dir_4814785cc4b3fb9ee4963645259310b2.md) **>** [**InspectionIndex.hpp**](InspectionIndex_8hpp.md)

[Go to the documentation of this file](InspectionIndex_8hpp.md)


```C++
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

class InspectionIndex {
public:
    struct Point {
        double x;
        double y;
        int index;
    };

    bool build(std::vector<Point>&& points, bool logX, bool logY, const std::atomic_bool& cancelled);
    [[nodiscard]] std::size_t storageBytes() const;
    [[nodiscard]] InspectionHit nearestAlong(const InspectionMetric& metric, InspectionAxis axis, double pixel, double radius) const;
    [[nodiscard]] InspectionHit nearest(const InspectionMetric& metric, const QPointF& position, double radius) const;
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

    void buildNodes(const std::atomic_bool& cancelled);
    int createNode(int first, int last, const std::atomic_bool& cancelled);
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

} // namespace QAccelPlot
```


