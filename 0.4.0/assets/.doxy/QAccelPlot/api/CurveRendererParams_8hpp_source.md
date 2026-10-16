

# File CurveRendererParams.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**renderers**](dir_a5593d4bbe882811c43c55c842f746b7.md) **>** [**CurveRendererParams.hpp**](CurveRendererParams_8hpp.md)

[Go to the documentation of this file](CurveRendererParams_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/axis/Axis.hpp"

#include <vector>

namespace QAccelPlot {

struct CurveChunk {
    int start;  
    int count;  
    qreal minX; 
    qreal maxX; 
    qreal minY; 
    qreal maxY; 
};

struct CurveDataView {
    const float* floatData{nullptr};
    const double* doubleData{nullptr};

    qreal x(int index) const
    {
        const auto offset = static_cast<std::size_t>(index) * 2;
        return doubleData ? static_cast<qreal>(doubleData[offset]) : static_cast<qreal>(floatData[offset]);
    }

    qreal y(int index) const
    {
        const auto offset = static_cast<std::size_t>(index) * 2 + 1;
        return doubleData ? static_cast<qreal>(doubleData[offset]) : static_cast<qreal>(floatData[offset]);
    }
};

struct CurveHitTestParams {
    CurveDataView data;                    
    int pointCount;                        
    const std::vector<CurveChunk>& chunks; 
    Axis* xAxis;                           
    Axis* yAxis;                           
    qreal width;                           
    qreal height;                          
    qreal hitThreshold;                    
    bool nonPositiveXInvalid;              
    bool nonPositiveYInvalid;              
};

inline bool isEmptyChunk(const CurveChunk& chunk)
{
    return chunk.minX > chunk.maxX || chunk.minY > chunk.maxY;
}

} // namespace QAccelPlot
```


