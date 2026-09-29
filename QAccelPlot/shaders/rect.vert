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

// Must match RectUbo in materials/internal/RectUniforms.hpp.
layout(std140, binding = 0) uniform buf {
    mat4 matrix;          //   0-63
    vec4 color;           //  64-79
    vec4 borderColor;     //  80-95
    vec4 hoverColor;      //  96-111
    vec2 domainMin;       // 112-119
    vec2 domainMax;       // 120-127
    vec2 viewportSize;    // 128-135
    vec2 minimumSize;     // 136-143: minimum drawn width and height in pixels
    float logScaleX;      // 144-147
    float logScaleY;      // 148-151
    float useVertexColor; // 152-155
    float rectCount;      // 156-159
    float opacity;        // 160-163: inherited item opacity
    float borderWidth;    // 164-167: outline width in pixels
    float hoveredIndex;   // 168-171: -1 when no rectangle is highlighted
} ubuf;

layout(binding = 1) uniform sampler2D dataSampler;

#include "data_texture.glsl"
#include "math_utils.glsl"
#include "rect_geometry.glsl"

void main() {
    int index = int(rectId);
    v_color = rectFillColor(index, vertexColor);
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

    vec2 xs;
    vec2 ys;
    rectPixelSpans(x1Bits, y1Bits, x2Bits, y2Bits, xs, ys);
    vec2 p_local = rectVertex(xs, ys, int(corner), v_edgeDistance);
    gl_Position = ubuf.matrix * vec4(p_local, 0.0, 1.0);
}
