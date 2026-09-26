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
    float opacity;     // 120-123: inherited item opacity
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

bool isNaNBits(uint bits) {
    return (bits & 0x7FFFFFFFu) > 0x7F800000u;
}

// Returns an edge's position as a fraction of the viewport from dMin (0) to dMax (1),
// clamped to one viewport beyond each side. Infinite edges map straight onto that margin,
// so they reach past the plot edge even when dMin and dMax are too far from the render
// origin to differ in single precision.
float viewportFraction(uint bits, float dMin, float dMax, bool logScale) {
    if (!isFiniteBits(bits)) {
        bool towardMin = ((bits & 0x80000000u) != 0u) == (dMax >= dMin);
        return towardMin ? -1.0 : 2.0;
    }
    float value = uintBitsToFloat(bits);
    if (logScale) {
        value = safeLog10(value);
    }
    return clamp((value - dMin) / (dMax - dMin), -1.0, 2.0);
}

void main() {
    v_color = mix(ubuf.color, vertexColor, ubuf.useVertexColor);
    v_color.a *= ubuf.opacity;

    int base = int(rectId) * 4;
    uint x1Bits = fetchFloatBits(base);
    uint y1Bits = fetchFloatBits(base + 1);
    uint x2Bits = fetchFloatBits(base + 2);
    uint y2Bits = fetchFloatBits(base + 3);
    if (isNaNBits(x1Bits) || isNaNBits(y1Bits) || isNaNBits(x2Bits) || isNaNBits(y2Bits)) {
        gl_Position = kCulledClipPosition;
        return;
    }

    bool logX = ubuf.logScaleX > 0.5;
    bool logY = ubuf.logScaleY > 0.5;
    vec2 dMin = ubuf.domainMin;
    vec2 dMax = ubuf.domainMax;
    if (logX) {
        dMin.x = safeLog10(dMin.x);
        dMax.x = safeLog10(dMax.x);
    }
    if (logY) {
        dMin.y = safeLog10(dMin.y);
        dMax.y = safeLog10(dMax.y);
    }

    vec2 uv = cornerUV(int(corner));
    float fx = viewportFraction(uv.x < 0.5 ? x1Bits : x2Bits, dMin.x, dMax.x, logX);
    float fy = viewportFraction(uv.y < 0.5 ? y1Bits : y2Bits, dMin.y, dMax.y, logY);

    // Map viewport fractions to item-local pixel coordinates
    vec2 p_local = vec2(fx * ubuf.viewportSize.x, (1.0 - fy) * ubuf.viewportSize.y);

    gl_Position = ubuf.matrix * vec4(p_local, 0.0, 1.0);
}
