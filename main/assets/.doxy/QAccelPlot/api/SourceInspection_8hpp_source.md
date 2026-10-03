

# File SourceInspection.hpp

[**File List**](files.md) **>** [**inspection**](dir_7c3af00b227ed418fdf47d7cd69e8a77.md) **>** [**internal**](dir_4814785cc4b3fb9ee4963645259310b2.md) **>** [**SourceInspection.hpp**](SourceInspection_8hpp.md)

[Go to the documentation of this file](SourceInspection_8hpp.md)


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

#include <vector>

namespace QAccelPlot {

class SourceInspection {
public:
    [[nodiscard]] static bool isSorted(const InspectionSource& source, InspectionAxis axis);

    void reset();
    void appended(int count);
    [[nodiscard]] std::size_t storageBytes() const;

    [[nodiscard]] InspectionHit nearestAlong(
        const InspectionSource& source, const InspectionMetric& metric, InspectionAxis order, double pixel, double radius) const;
    [[nodiscard]] InspectionHit nearest(
        const InspectionSource& source, const InspectionMetric& metric, InspectionAxis order, const QPointF& position, double radius);
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

namespace InspectionScan {

[[nodiscard]] InspectionHit nearestAlong(const InspectionSource& source, const InspectionMetric& metric, InspectionAxis axis, double pixel, double radius);
[[nodiscard]] InspectionHit nearest(const InspectionSource& source, const InspectionMetric& metric, const QPointF& position, double radius);
[[nodiscard]] InspectionNeighbors neighbors(const InspectionSource& source, InspectionAxis axis, double value);
[[nodiscard]] SummaryAccumulator summarize(const InspectionSource& source, const InspectionBounds& bounds);
void collect(const InspectionSource& source, const InspectionBounds& bounds, int offset, int limit, QList<int>& indices);

} // namespace InspectionScan

} // namespace QAccelPlot
```


