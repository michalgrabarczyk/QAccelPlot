//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440
#extension GL_GOOGLE_include_directive : require

layout(location = 0) out vec2 v_uv;        // +-1 is markerSize; the quad extends further with antialiasing
layout(location = 1) out float v_colorT;   // normalized colormap coordinate
layout(location = 2) out float v_useValue; // 1.0 when the colormap applies to this point

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
    float markerStrokeWidth;   // 144-147
    float markerFilled;        // 148-151
    float valueLogScale;       // 152-155
    float opacity;             // 156-159: inherited item opacity
} ubuf;

layout(binding = 1) uniform sampler2D dataSampler;

#include "data_texture.glsl"
#include "math_utils.glsl"
#include "point_shapes.glsl"
#include "marker_vertex.glsl"

// Collapses every vertex of a skipped point onto one position outside clip space,
// so the billboard has zero area and is clipped away.
void discardPoint() {
    v_uv = vec2(0.0);
    v_colorT = 0.0;
    v_useValue = 0.0;
    gl_Position = kCulledClipPosition;
}

void main() {
    int strideInt = int(ubuf.stride + 0.5);
    MarkerVertex marker = computeMarkerVertex(strideInt);
    if (!marker.valid) {
        discardPoint();
        return;
    }

    v_useValue = 0.0;
    v_colorT = 0.0;
    if (ubuf.useVertexColor > 0.5 && strideInt >= 3) {
        uint valueBits = fetchFloatBits(marker.base + 2);
        if (isFiniteBits(valueBits)) {
            float value = uintBitsToFloat(valueBits);
            float lo = ubuf.valueMin;
            float hi = ubuf.valueMax;
            bool mappable = true;
            if (ubuf.valueLogScale > 0.5) {
                // Log normalization is undefined at or below zero; those points keep the uniform color.
                mappable = value > 0.0 && lo > 0.0 && hi > 0.0;
                value = safeLog10(value);
                lo = safeLog10(lo);
                hi = safeLog10(hi);
            }
            float valueRange = hi - lo;
            if (mappable) {
                v_colorT = abs(valueRange) > 0.0 ? clamp((value - lo) / valueRange, 0.0, 1.0) : 0.0;
                v_useValue = 1.0;
            }
        }
    }

    v_uv = marker.uv;
    gl_Position = marker.position;
}
