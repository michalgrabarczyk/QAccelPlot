//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//

// Uniform block shared by rect.vert and rect.frag. Must match RectUbo in RectMaterial.cpp.
layout(std140, binding = 0) uniform buf {
    mat4 matrix;          //   0-63
    vec4 color;           //  64-79
    vec2 domainMin;       //  80-87
    vec2 domainMax;       //  88-95
    vec2 viewportSize;    //  96-103
    float logScaleX;      // 104-107
    float logScaleY;      // 108-111
    float useVertexColor; // 112-115
    float rectCount;      // 116-119
    float opacity;        // 120-123: inherited item opacity
    vec2 minimumSize;     // 128-135: minimum drawn width and height in pixels
    float borderWidth;    // 136-139: outline width in pixels
    vec4 borderColor;     // 144-159
    vec4 hoverColor;      // 160-175
    float hoveredIndex;   // 176-179: -1 when no rectangle is highlighted
} ubuf;
