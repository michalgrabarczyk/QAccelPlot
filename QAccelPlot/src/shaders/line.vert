//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in float id;
layout(location = 1) in float side; // 1.0 or -1.0
layout(location = 2) in vec4 vertexColor;
layout(location = 3) in float arcLength;

layout(location = 0) out vec4 v_color;
layout(location = 1) out float v_edgeDistance;
layout(location = 2) out float v_fadeStart;
layout(location = 3) out float v_softness;
layout(location = 4) out float v_arcLength;
layout(location = 5) out float v_validWeight;

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    vec4 color;
    vec2 domainMin;
    vec2 domainMax;
    vec2 viewportSize;
    float lineWidth;
    float logScaleX;
    float logScaleY;
    float useVertexColor;
    float pointCount;
    float antialiasingEnabled;
    float antialiasingFeather;
    float dashPeriod;
    float dashOffset;
    int dashPatternSize;
    vec4 dashPattern[2];
} ubuf;

layout(binding = 1) uniform sampler2D dataSampler;

#include "data_texture.glsl"
#include "math_utils.glsl"
#include "line_geometry.glsl"

void main() {
    v_color = mix(ubuf.color, vertexColor, ubuf.useVertexColor);
    v_edgeDistance = side;
    v_arcLength = arcLength;

    // Precompute AA fade threshold for the fragment shader.
    if (ubuf.antialiasingEnabled > 0.5 && ubuf.antialiasingFeather > 0.0) {
        float halfWidth = max(ubuf.lineWidth * 0.5, 1e-4);
        float featherNorm = max(ubuf.antialiasingFeather / halfWidth, 0.01);
        v_fadeStart = 1.0 - featherNorm;
        v_softness = featherNorm;
    } else {
        v_fadeStart = 2.0; // sentinel: AA disabled
        v_softness = 1.0;
    }

    LineVertexResult vertex = computeLineVertex(int(id), side);
    v_validWeight = vertex.validWeight;
    gl_Position = vertex.position;
}
