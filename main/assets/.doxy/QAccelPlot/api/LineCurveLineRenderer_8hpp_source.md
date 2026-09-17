

# File LineCurveLineRenderer.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**renderers**](dir_a5593d4bbe882811c43c55c842f746b7.md) **>** [**LineCurveLineRenderer.hpp**](LineCurveLineRenderer_8hpp.md)

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

#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/GradientColorTypes.hpp"
#include "QAccelPlot/linestyles/LineStyle.hpp"
#include "QAccelPlot/renderers/CurveRendererParams.hpp"
#include "QAccelPlot/series/LineCurveGapFilter.hpp"

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
    // Valid-sample runs used to break the gradient fill at gaps, cached across frames.
    struct FillRunCache {
        const void* data{nullptr};
        int pointCount{-1};
        bool logScaleX{false};
        bool logScaleY{false};
        std::vector<SampleRun> runs;
    };

    void updateFillGeometry(QSGGeometryNode* fillNode, const LineCurveRenderParams& params) const;
    const std::vector<SampleRun>& validRuns(const LineCurveRenderParams& params) const;
    void updateLineMaterial(
        LineMaterial* material, const LineCurveRenderParams& params, const QColor& effectiveColor, bool useVertexColor, const DashParameters& dashParams) const;
    std::vector<float> computeArcLengths(const LineCurveRenderParams& params, const DashParameters& dashParams) const;
    void updateLineVertices(QSGGeometry* geometry, const LineCurveRenderParams& params, bool useVertexColor, const QColor& effectiveColor,
        const std::vector<float>& arcLengths) const;

    // Render-thread state touched only by paint(); workers that call buildVertexCache() never read it.
    mutable FillRunCache fillRunCache_;
};

} // namespace QAccelPlot
```


