

# File LineCurvePointRenderer.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**renderers**](dir_a5593d4bbe882811c43c55c842f746b7.md) **>** [**LineCurvePointRenderer.hpp**](LineCurvePointRenderer_8hpp.md)

[Go to the documentation of this file](LineCurvePointRenderer_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/GradientColorTypes.hpp"
#include "QAccelPlot/renderers/CurveRendererParams.hpp"

#include <QColor>
#include <QPointF>
#include <QSGNode>
#include <QVector2D>

#include <vector>

namespace QAccelPlot {

struct PointCurveRenderParams {
    const std::vector<float>& data;              
    CurveDataView sourceData;                    
    int pointCount;                              
    bool dataChanged;                            
    QColor color;                                
    bool hovered;                                
    qreal markerSize;                            
    QVector2D domainMin;                         
    QVector2D domainMax;                         
    QVector2D viewportSize;                      
    bool logScaleX;                              
    bool logScaleY;                              
    bool antialiasingEnabled;                    
    qreal antialiasingFeather;                   
    const GradientColorPayload& gradientPayload; 
    const std::vector<char>* vertexCache;        
    int shapeType;                               
};

struct PointVertex {
    float x;       
    float y;       
    float cornerX; 
    float cornerY; 
    float r;       
    float g;       
    float b;       
    float a;       
};

class LineCurvePointRenderer {
public:
    void buildVertexCache(const std::vector<float>& data, int pointCount, std::vector<char>& cache) const;

    bool contains(const QPointF& point, const CurveHitTestParams& params) const;

    QSGNode* paint(QSGNode* oldNode, const PointCurveRenderParams& params) const;
};

} // namespace QAccelPlot
```


