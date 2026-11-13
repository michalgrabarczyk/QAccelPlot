

# File LineStroke.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**renderers**](dir_a5593d4bbe882811c43c55c842f746b7.md) **>** [**LineStroke.hpp**](LineStroke_8hpp.md)

[Go to the documentation of this file](LineStroke_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/linestyles/LineStyle.hpp"

#include <QColor>
#include <QPointF>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QVector2D>

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace QAccelPlot {

class LineMaterial;

struct LineVertex {
    static constexpr int kMaximumIndex = (1 << 28) - 1;
    static constexpr int kMaximumCorner = 3;

    std::uint32_t packed;

    static LineVertex make(int index, int corner, bool negativeSide, bool outgoing = false) noexcept;

    int index() const noexcept;

    int corner() const noexcept;

    bool outgoing() const noexcept;

    bool negativeSide() const noexcept;
};

static_assert(sizeof(LineVertex) == 4);

struct DashedLineVertex {
    LineVertex vertex; 
    float arcLength;   
};

static_assert(sizeof(DashedLineVertex) == 8);

namespace LineStroke {

struct Uniforms {
    QColor color;              
    qreal lineWidth;           
    QVector2D domainMin;       
    QVector2D domainMax;       
    QVector2D viewportSize;    
    bool logScaleX;            
    bool logScaleY;            
    int pointCount;            
    bool antialiasingEnabled;  
    qreal antialiasingFeather; 
    DashParameters dash;       
};

const QSGGeometry::AttributeSet& vertexAttributes(bool dashed);

QSGGeometryNode* createNode(int vertexCount, LineMaterial* material);

bool setMaterial(QSGGeometryNode* node, LineMaterial* material);

int verticesPerPathPoint(bool splitJoins) noexcept;

void writeVertices(LineVertex* vertices, int pointCount, int cornerCount, bool splitJoins, int firstPoint = 0);

void writeDashedVertices(DashedLineVertex* vertices, int pointCount, int cornerCount, bool splitJoins, const std::vector<float>& arcLengths);

void applyUniforms(LineMaterial& material, const Uniforms& uniforms);

std::vector<float> arcLengths(int pointCount, const std::function<std::optional<QPointF>(int)>& pixelAt);

} // namespace LineStroke

} // namespace QAccelPlot
```


