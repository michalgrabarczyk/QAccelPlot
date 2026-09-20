

# File SpatialGrid.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**SpatialGrid.hpp**](SpatialGrid_8hpp.md)

[Go to the documentation of this file](SpatialGrid_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <vector>

namespace QAccelPlot {

class SpatialGrid {
public:
    void build(const float* data, int itemCount, int floatsPerItem = 4);
    int query(float x, float y) const;

private:
    struct ItemBounds {
        float minX;
        float minY;
        float maxX;
        float maxY;

        bool contains(float x, float y) const;
    };

    void computeDataBounds(const float* data, int itemCount, int floatsPerItem);
    void computeGridDimensions(int itemCount);
    void fillSpatialGrid(int itemCount);

    float minX_{0.0f};
    float minY_{0.0f};
    float maxX_{1.0f};
    float maxY_{1.0f};
    int cols_{0};
    int rows_{0};
    float cellW_{1.0f};
    float cellH_{1.0f};
    std::vector<ItemBounds> itemBounds_;
    std::vector<std::vector<int>> cells_;
    std::vector<int> largeItems_;
};

} // namespace QAccelPlot
```


