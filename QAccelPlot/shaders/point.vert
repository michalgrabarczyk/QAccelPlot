//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440
#extension GL_GOOGLE_include_directive : require

layout(location = 0) out vec2 v_uv;                  // +-1 is markerSize; the quad extends further with antialiasing
layout(location = 1) out float v_gradientCoordinate; // unclamped gradient coordinate of the marker

layout(std140, binding = 0) uniform buf {
    mat4  matrix;              //   0-63
    vec4  color;               //  64-79
    vec2  domainMin;           //  80-87
    vec2  domainMax;           //  88-95
    vec2  viewportSize;        //  96-103
    float logScaleX;           // 104-107
    float logScaleY;           // 108-111
    float useVertexColor;      // 112-115: 1.0 = gradient color
    float markerSize;          // 116-119
    float antialiasingEnabled; // 120-123
    float antialiasingFeather; // 124-127
    int   shapeType;           // 128-131
    float markerStrokeWidth;   // 132-135
    float markerFilled;        // 136-139
    float gradientDirection;   // 140-143: 0.0 = horizontal, 1.0 = vertical
    float gradientValueMin;    // 144-147
    float gradientValueMax;    // 148-151
    float opacity;             // 152-155: inherited item opacity
} ubuf;

layout(binding = 1) uniform sampler2D dataSampler;

#include "data_texture.glsl"
#include "math_utils.glsl"
#include "point_shapes.glsl"
#include "marker_vertex.glsl"

void main() {
    MarkerVertex marker = computeMarkerVertex(2);
    v_uv = marker.uv;
    gl_Position = marker.position;

    // Same palette coordinate as gradient_line.vert: horizontal runs left to right,
    // while vertical position 0 is the top (largest Y value).
    float gradientValue = ubuf.gradientDirection < 0.5 ? marker.dataPosition.x : marker.dataPosition.y;
    float gradientRange = max(ubuf.gradientValueMax - ubuf.gradientValueMin, 1e-6);
    float normalizedGradient = (gradientValue - ubuf.gradientValueMin) / gradientRange;
    v_gradientCoordinate = ubuf.gradientDirection < 0.5 ? normalizedGradient : 1.0 - normalizedGradient;
}
