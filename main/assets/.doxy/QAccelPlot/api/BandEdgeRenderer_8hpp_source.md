

# File BandEdgeRenderer.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**renderers**](dir_a5593d4bbe882811c43c55c842f746b7.md) **>** [**BandEdgeRenderer.hpp**](BandEdgeRenderer_8hpp.md)

[Go to the documentation of this file](BandEdgeRenderer_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/materials/BandEdgeMaterial.hpp"
#include "QAccelPlot/renderers/LineStroke.hpp"

#include <QSGGeometryNode>
#include <QVector2D>

#include <memory>
#include <optional>
#include <vector>

namespace QAccelPlot {

class Axis;
class DataTexture;

struct BandSamples {
    const float* floatData{nullptr};   
    const double* doubleData{nullptr}; 
    int count{0};                      

    qreal value(const int index, const int component) const
    {
        const auto offset = static_cast<std::size_t>(index) * 3 + static_cast<std::size_t>(component);
        return doubleData ? static_cast<qreal>(doubleData[offset]) : static_cast<qreal>(floatData[offset]);
    }
};

struct BandEdgeRenderParams {
    std::shared_ptr<DataTexture> dataTexture; 
    BandSamples samples;                      
    LineStroke::Uniforms uniforms;            
    Axis* xAxis;                              
    Axis* yAxis;                              
    bool dataChanged;                         
    int reservedSampleCount;
};

class BandEdgeRenderer {
public:
    explicit BandEdgeRenderer(BandEdgeMaterial::Edge edge);

    QSGGeometryNode* paint(QSGGeometryNode* oldNode, const BandEdgeRenderParams& params) const;

private:
    // The zoom state dash arc lengths depend on. Panning leaves it unchanged; spans are in log
    // space on log-scale axes.
    struct ArcLengthScale {
        qreal xSpan;
        qreal ySpan;
        QVector2D viewportSize;
        bool logScaleX;
        bool logScaleY;

        bool matches(const ArcLengthScale& other) const;
    };

    static ArcLengthScale arcLengthScale(const BandEdgeRenderParams& params);
    std::vector<float> arcLengths(const BandEdgeRenderParams& params) const;

    BandEdgeMaterial::Edge edge_;
    // Render-thread state touched only by paint(): the scale of the arc lengths in the vertex
    // buffer, or nothing when it holds none.
    mutable std::optional<ArcLengthScale> arcLengthScale_;
};

} // namespace QAccelPlot
```


