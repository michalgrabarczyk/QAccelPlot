

# File PointSpatialIndex.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**PointSpatialIndex.hpp**](PointSpatialIndex_8hpp.md)

[Go to the documentation of this file](PointSpatialIndex_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QtGlobal>

#include <vector>

namespace QAccelPlot {

class PointSpatialIndex {
public:
    struct Mapping {
        bool logX{false}; 
        bool logY{false}; 

        bool operator==(const Mapping& other) const;
    };

    void build(const float* data, int pointCount, int stride, Mapping mapping);
    void build(const float* data, int pointCount, int stride = 2);
    void clear();

    bool isEmpty() const;
    int validPointCount() const;
    Mapping mapping() const;

    int nearest(double x, double y, double radiusX, double radiusY) const;

    static bool mapCoordinate(double value, bool logarithmic, double& mapped);

private:
    struct IndexedPoint {
        float x;
        float y;
        int sourceIndex;
    };

    void collectValidPoints(const float* data, int pointCount, int stride);
    void computeGridDimensions();
    void fillCells();
    int cellColumn(double x) const;
    int cellRow(double y) const;

    Mapping mapping_;
    double minX_{0.0};
    double minY_{0.0};
    double cellWidth_{1.0};
    double cellHeight_{1.0};
    int columns_{0};
    int rows_{0};
    std::vector<IndexedPoint> points_;
    std::vector<int> cellStart_;   // columns_ * rows_ + 1 offsets into cellEntries_
    std::vector<int> cellEntries_; // positions in points_, grouped by cell
};

} // namespace QAccelPlot
```


