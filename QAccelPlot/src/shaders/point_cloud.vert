//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in float pointId;  // index of the point in the data texture
layout(location = 1) in float corner;   // 0-5: which vertex of the two-triangle billboard

layout(location = 0) out vec2 v_uv;        // -1..+1 across the billboard
layout(location = 1) out float v_colorT;   // normalized colormap coordinate
layout(location = 2) out float v_useValue; // 1.0 when the colormap applies to this point
layout(location = 3) out float v_fadeStart;
layout(location = 4) out float v_softness;

layout(std140, binding = 0) uniform buf {
    mat4  matrix;              //   0-63
    vec4  color;               //  64-79
    vec2  domainMin;           //  80-87
    vec2  domainMax;           //  88-95
    vec2  viewportSize;        //  96-103
    float logScaleX;           // 104-107
    float logScaleY;           // 108-111
    float useVertexColor;      // 112-115: 1.0 = ValueColor mode
    float markerSize;          // 116-119
    float antialiasingEnabled; // 120-123
    float antialiasingFeather; // 124-127
    float valueMin;            // 128-131
    float valueMax;            // 132-135
    float stride;              // 136-139: floats per point in the data texture (2 or 3)
    int   shapeType;           // 140-143
} ubuf;

layout(binding = 1) uniform sampler2D dataSampler;

#include "data_texture.glsl"
#include "math_utils.glsl"
#include "point_shapes.glsl"

vec2 cornerOffset(int c) {
    if (c == 0) return vec2(-1.0, -1.0);
    if (c == 1) return vec2( 1.0, -1.0);
    if (c == 2) return vec2( 1.0,  1.0);
    if (c == 3) return vec2(-1.0, -1.0);
    if (c == 4) return vec2( 1.0,  1.0);
    return vec2(-1.0, 1.0); // c == 5
}

// Collapses every vertex of a skipped point onto one position outside clip space
// (z = 2 is outside for both GL [-1, 1] and D3D/Metal/Vulkan [0, 1] depth ranges).
void discardPoint() {
    v_uv = vec2(0.0);
    v_colorT = 0.0;
    v_useValue = 0.0;
    v_fadeStart = 2.0;
    v_softness = 1.0;
    gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
}

void main() {
    int strideInt = int(ubuf.stride + 0.5);
    int base = int(pointId + 0.5) * strideInt;
    uint xBits = fetchFloatBits(base);
    uint yBits = fetchFloatBits(base + 1);
    if (isNonFiniteBits(xBits) || isNonFiniteBits(yBits)) {
        discardPoint();
        return;
    }

    vec2 p = vec2(uintBitsToFloat(xBits), uintBitsToFloat(yBits));
    vec2 dMin = ubuf.domainMin;
    vec2 dMax = ubuf.domainMax;

    if (ubuf.logScaleX > 0.5) {
        if (p.x <= 0.0) {
            discardPoint();
            return;
        }
        dMin.x = safeLog10(dMin.x);
        dMax.x = safeLog10(dMax.x);
        p.x = safeLog10(p.x);
    }

    if (ubuf.logScaleY > 0.5) {
        if (p.y <= 0.0) {
            discardPoint();
            return;
        }
        dMin.y = safeLog10(dMin.y);
        dMax.y = safeLog10(dMax.y);
        p.y = safeLog10(p.y);
    }

    v_useValue = 0.0;
    v_colorT = 0.0;
    if (ubuf.useVertexColor > 0.5 && strideInt >= 3) {
        uint valueBits = fetchFloatBits(base + 2);
        if (!isNonFiniteBits(valueBits)) {
            float valueRange = ubuf.valueMax - ubuf.valueMin;
            float value = uintBitsToFloat(valueBits);
            v_colorT = abs(valueRange) > 0.0 ? clamp((value - ubuf.valueMin) / valueRange, 0.0, 1.0) : 0.0;
            v_useValue = 1.0;
        }
    }

    vec2 offset = cornerOffset(int(corner + 0.5));
    v_uv = offset;
    computeMarkerFade(ubuf.markerSize, ubuf.antialiasingEnabled, ubuf.antialiasingFeather, v_fadeStart, v_softness);

    vec2 range = dMax - dMin;
    vec2 p_local = vec2(
        (p.x - dMin.x) / range.x * ubuf.viewportSize.x,
        (1.0 - (p.y - dMin.y) / range.y) * ubuf.viewportSize.y
    );

    // Expand the point center into a billboard quad in item-local pixel space.
    vec2 final_pos = p_local + offset * ubuf.markerSize;
    gl_Position = ubuf.matrix * vec4(final_pos, 0.0, 1.0);
}
