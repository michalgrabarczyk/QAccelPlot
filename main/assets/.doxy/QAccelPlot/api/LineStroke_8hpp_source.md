

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
#include <cstdint>
#include <optional>
#include <vector>

namespace QAccelPlot {

class LineMaterial;

struct LineVertex {
    static constexpr int kMaximumIndex = (1 << 28) - 1;
    static constexpr int kMaximumCorner = 7;

    std::uint32_t packed;

    static constexpr LineVertex make(const int index, const int corner, const bool negativeSide) noexcept
    {
        return {static_cast<std::uint32_t>(index) | (static_cast<std::uint32_t>(corner) << 28U) | (negativeSide ? 1U << 31U : 0U)};
    }

    constexpr int index() const noexcept
    {
        return static_cast<int>(packed & 0x0FFFFFFFU);
    }

    constexpr int corner() const noexcept
    {
        return static_cast<int>((packed >> 28U) & 7U);
    }

    constexpr bool negativeSide() const noexcept
    {
        return (packed >> 31U) != 0U;
    }
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

void writeVertices(LineVertex* vertices, int pointCount, int cornerCount, int firstPoint = 0);

void writeDashedVertices(DashedLineVertex* vertices, int pointCount, int cornerCount, const std::vector<float>& arcLengths);

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


