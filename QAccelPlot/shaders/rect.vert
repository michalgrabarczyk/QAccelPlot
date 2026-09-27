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
// Pixel distances to the left, right, top, and bottom edges, for the outline.
layout(location = 1) out vec4 v_edgeDistance;
// The outline settings pass through varyings, so only this stage declares ubuf: shader
// compilers trim unused trailing block members per stage, and OpenGL rejects blocks that differ.
layout(location = 2) out vec4 v_borderColor;
layout(location = 3) out float v_borderWidth;

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

// Widens the pixel span from a to b around its center to at least minSize, keeping a and b in order.
vec2 widenedSpan(float a, float b, float minSize) {
    float grow = 0.5 * max(minSize - abs(b - a), 0.0);
    float direction = a <= b ? 1.0 : -1.0;
    return vec2(a - direction * grow, b + direction * grow);
}

void main() {
    int index = int(rectId);
    v_color = index == int(ubuf.hoveredIndex) ? ubuf.hoverColor : mix(ubuf.color, vertexColor, ubuf.useVertexColor);
    v_color.a *= ubuf.opacity;
    v_borderColor = vec4(ubuf.borderColor.rgb, ubuf.borderColor.a * ubuf.opacity);
    v_borderWidth = ubuf.borderWidth;

    int base = index * 4;
    uint x1Bits = fetchFloatBits(base);
    uint y1Bits = fetchFloatBits(base + 1);
    uint x2Bits = fetchFloatBits(base + 2);
    uint y2Bits = fetchFloatBits(base + 3);
    if (isNaNBits(x1Bits) || isNaNBits(y1Bits) || isNaNBits(x2Bits) || isNaNBits(y2Bits)) {
        gl_Position = kCulledClipPosition;
        v_edgeDistance = vec4(0.0);
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

    // Edges in item-local pixels, with y growing downward.
    vec2 size = ubuf.viewportSize;
    vec2 xs = widenedSpan(viewportFraction(x1Bits, dMin.x, dMax.x, logX) * size.x,
                          viewportFraction(x2Bits, dMin.x, dMax.x, logX) * size.x, ubuf.minimumSize.x);
    vec2 ys = widenedSpan((1.0 - viewportFraction(y1Bits, dMin.y, dMax.y, logY)) * size.y,
                          (1.0 - viewportFraction(y2Bits, dMin.y, dMax.y, logY)) * size.y, ubuf.minimumSize.y);

    vec2 uv = cornerUV(int(corner));
    vec2 p_local = vec2(uv.x < 0.5 ? xs.x : xs.y, uv.y < 0.5 ? ys.x : ys.y);
    v_edgeDistance = vec4(p_local.x - min(xs.x, xs.y), max(xs.x, xs.y) - p_local.x,
                          p_local.y - min(ys.x, ys.y), max(ys.x, ys.y) - p_local.y);

    gl_Position = ubuf.matrix * vec4(p_local, 0.0, 1.0);
}
