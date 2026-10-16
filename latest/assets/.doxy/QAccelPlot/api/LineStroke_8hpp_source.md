

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

#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

namespace QAccelPlot {

class LineMaterial;

struct LineVertex {
    float id;        
    float side;      
    unsigned char r; 
    unsigned char g; 
    unsigned char b; 
    unsigned char a; 
    float arcLength; 
};

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

const QSGGeometry::AttributeSet& vertexAttributes();

QSGGeometryNode* createNode(int vertexCount, QSGMaterial* material);

void writeVertices(LineVertex* vertices, int pointCount, const QColor& color, const std::vector<float>& arcLengths);

void applyUniforms(LineMaterial& material, const Uniforms& uniforms);

template <typename PixelAt> std::vector<float> arcLengths(const int pointCount, const PixelAt& pixelAt)
{
    auto lengths = std::vector<float>(static_cast<std::size_t>(std::max(pointCount, 0)));
    auto length = 0.0f;
    auto previousX = 0.0f;
    auto previousY = 0.0f;
    auto previousValid = false;
    for (auto i = 0; i < pointCount; ++i) {
        const auto current = std::optional<QPointF>{pixelAt(i)};
        if (current) {
            const auto x = static_cast<float>(current->x());
            const auto y = static_cast<float>(current->y());
            if (previousValid) {
                const auto dx = x - previousX;
                const auto dy = y - previousY;
                length += std::sqrt(dx * dx + dy * dy);
            }
            previousX = x;
            previousY = y;
        }
        previousValid = current.has_value();
        lengths[static_cast<std::size_t>(i)] = length;
    }
    return lengths;
}

} // namespace LineStroke

} // namespace QAccelPlot
```


