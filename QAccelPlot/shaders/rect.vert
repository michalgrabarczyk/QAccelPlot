//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in float rectId;
layout(location = 1) in float corner; // 0-5: which vertex of the two-triangle quad
layout(location = 2) in vec4 vertexColor;

layout(location = 0) out vec4 v_color;

layout(std140, binding = 0) uniform buf {
    mat4 matrix;       //   0-63
    vec4 color;        //  64-79
    vec2 domainMin;    //  80-87
    vec2 domainMax;    //  88-95
    vec2 viewportSize; //  96-103
    float logScaleX;   // 104-107
    float logScaleY;   // 108-111
    float useVertexColor; // 112-115
    float rectCount;   // 116-119
} ubuf;

layout(binding = 1) uniform sampler2D dataSampler;

#include "data_texture.glsl"
#include "math_utils.glsl"

// Corner UVs for 2-triangle quad (CCW winding):
//   Triangle 1: (0,0), (1,0), (0,1)
//   Triangle 2: (1,0), (1,1), (0,1)
vec2 cornerUV(int c) {
    // Using step functions to avoid array indexing
    if (c == 0) return vec2(0.0, 0.0);
    if (c == 1) return vec2(1.0, 0.0);
    if (c == 2) return vec2(0.0, 1.0);
    if (c == 3) return vec2(1.0, 0.0);
    if (c == 4) return vec2(1.0, 1.0);
    return vec2(0.0, 1.0); // c == 5
}

void main() {
    v_color = mix(ubuf.color, vertexColor, ubuf.useVertexColor);

    int idx = int(rectId);
    int base = idx * 4;
    float x1 = fetchFloat(base);
    float y1 = fetchFloat(base + 1);
    float x2 = fetchFloat(base + 2);
    float y2 = fetchFloat(base + 3);

    vec2 uv = cornerUV(int(corner));
    float px = mix(x1, x2, uv.x);
    float py = mix(y1, y2, uv.y);

    // Apply log scale if active
    vec2 dMin = ubuf.domainMin;
    vec2 dMax = ubuf.domainMax;

    if (ubuf.logScaleX > 0.5) {
        dMin.x = safeLog10(dMin.x);
        dMax.x = safeLog10(dMax.x);
        px = safeLog10(px);
    }

    if (ubuf.logScaleY > 0.5) {
        dMin.y = safeLog10(dMin.y);
        dMax.y = safeLog10(dMax.y);
        py = safeLog10(py);
    }

    // Map domain coordinates to item-local pixel coordinates
    vec2 range = dMax - dMin;
    vec2 p_local = vec2(
        (px - dMin.x) / range.x * ubuf.viewportSize.x,
        (1.0 - (py - dMin.y) / range.y) * ubuf.viewportSize.y
    );

    gl_Position = ubuf.matrix * vec4(p_local, 0.0, 1.0);
}
