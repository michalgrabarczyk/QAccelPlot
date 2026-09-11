

# File CurveRendererParams.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**renderers**](dir_d8b25a79bf6effba969f37c0067b810c.md) **>** [**CurveRendererParams.hpp**](CurveRendererParams_8hpp.md)

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

#include "axis/Axis.hpp"

#include <vector>

namespace QAccelPlot {

struct CurveChunk {
    int start;  
    int count;  
    float minX; 
    float maxX; 
    float minY; 
    float maxY; 
};

struct CurveHitTestParams {
    const std::vector<float>& data;        
    int pointCount;                        
    const std::vector<CurveChunk>& chunks; 
    Axis* xAxis;                           
    Axis* yAxis;                           
    qreal width;                           
    qreal height;                          
    qreal hitThreshold;                    
};

} // namespace QAccelPlot
```


