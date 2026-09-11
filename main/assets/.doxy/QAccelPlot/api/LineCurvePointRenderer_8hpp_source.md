

# File LineCurvePointRenderer.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**renderers**](dir_d8b25a79bf6effba969f37c0067b810c.md) **>** [**LineCurvePointRenderer.hpp**](LineCurvePointRenderer_8hpp.md)

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

#include "effects/GradientColorTypes.hpp"
#include "renderers/CurveRendererParams.hpp"

#include <QColor>
#include <QPointF>
#include <QSGNode>
#include <QVector2D>

#include <vector>

namespace QAccelPlot {

struct PointCurveRenderParams {
    const std::vector<float>& data;              
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


