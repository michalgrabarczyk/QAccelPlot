

# File LineCurveLineRenderer.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**renderers**](dir_d8b25a79bf6effba969f37c0067b810c.md) **>** [**LineCurveLineRenderer.hpp**](LineCurveLineRenderer_8hpp.md)

[Go to the documentation of this file](LineCurveLineRenderer_8hpp.md)


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
#include "effects/GradientColorTypes.hpp"
#include "linestyles/LineStyle.hpp"
#include "renderers/CurveRendererParams.hpp"

#include <QColor>
#include <QPointF>
#include <QSGGeometry>
#include <QSGNode>
#include <QVector2D>

#include <vector>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot {

struct DashParameters;
class LineMaterial;

struct LineCurveRenderParams {
    QQuickWindow* window;                           
    const std::vector<float>& data;                 
    CurveDataView sourceData;                       
    int pointCount;                                 
    bool dataChanged;                               
    QColor color;                                   
    bool hovered;                                   
    qreal lineWidth;                                
    QVector2D domainMin;                            
    QVector2D domainMax;                            
    QVector2D viewportSize;                         
    Axis* xAxis;                                    
    Axis* yAxis;                                    
    bool logScaleX;                                 
    bool logScaleY;                                 
    bool antialiasingEnabled;                       
    qreal antialiasingFeather;                      
    const GradientColorPayload& gradientPayload;    
    const GradientFillPayload& gradientFillPayload; 
    const std::vector<char>* vertexCache;           
    const LineStyle* lineStyle;                     
};

struct LineVertex {
    float id;        
    float side;      
    unsigned char r; 
    unsigned char g; 
    unsigned char b; 
    unsigned char a; 
    float arcLength; 
};

class LineCurveLineRenderer {
public:
    void buildVertexCache(const std::vector<float>& data, int pointCount, std::vector<char>& cache) const;

    bool contains(const QPointF& point, const CurveHitTestParams& params) const;

    QSGNode* paint(QSGNode* oldNode, const LineCurveRenderParams& params) const;

private:
    void updateFillGeometry(QSGGeometryNode* fillNode, const LineCurveRenderParams& params) const;
    void updateLineMaterial(
        LineMaterial* material, const LineCurveRenderParams& params, const QColor& effectiveColor, bool useVertexColor, const DashParameters& dashParams) const;
    std::vector<float> computeArcLengths(const LineCurveRenderParams& params, const DashParameters& dashParams) const;
    void updateLineVertices(QSGGeometry* geometry, const LineCurveRenderParams& params, bool useVertexColor, const QColor& effectiveColor,
        const std::vector<float>& arcLengths) const;
};

} // namespace QAccelPlot
```


