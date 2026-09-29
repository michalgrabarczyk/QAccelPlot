

# File RectUniforms.hpp

[**File List**](files.md) **>** [**internal**](dir_3c3be61dbf90c69b9ad6cd32d23f3e24.md) **>** [**RectUniforms.hpp**](RectUniforms_8hpp.md)

[Go to the documentation of this file](RectUniforms_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QSGMaterialShader>

namespace QAccelPlot {
class RectMaterial;
}

namespace QAccelPlot::Internal {

struct RectUbo {
    float matrix[16];      // 0–63
    float color[4];        // 64–79
    float borderColor[4];  // 80–95
    float hoverColor[4];   // 96–111
    float domainMin[2];    // 112–119
    float domainMax[2];    // 120–127
    float viewportSize[2]; // 128–135
    float minimumSize[2];  // 136–143
    float logScaleX;       // 144–147
    float logScaleY;       // 148–151
    float useVertexColor;  // 152–155
    float rectCount;       // 156–159
    float opacity;         // 160–163
    float borderWidth;     // 164–167
    float hoveredIndex;    // 168–171
};

static_assert(sizeof(RectUbo) == 172);

void writeRectUniforms(RectUbo& ubo, const QSGMaterialShader::RenderState& state, const RectMaterial& material);

} // namespace QAccelPlot::Internal
```


