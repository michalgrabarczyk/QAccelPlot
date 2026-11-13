//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Vertex stage of the line ribbon shared by line.vert, line_dashed.vert, and the band edge
// shaders. Define LINE_SAMPLE_STRIDE and LINE_Y_BITS before including it to read other
// sample layouts, see line_geometry.glsl, and LINE_DASHED, see line_vertex_input.glsl.

#include "line_vertex_input.glsl"

layout(location = 0) out vec4 v_color;
layout(location = 1) out float v_edgeDistance; // signed distance from the line centre, in pixels
layout(location = 2) out float v_arcLength;
layout(location = 3) out float v_validWeight;

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
    float opacity;
    float stepMode;
    vec4 itemFadeGroups[28]; // item opacity, see item_opacity.glsl
    vec4 itemFadeTrail;
    vec4 itemFadeTrailEasing[4];
    vec4 glowColor;         // Glow effect, see glow.glsl
    vec4 glowParams;
} ubuf;

layout(binding = 1) uniform sampler2D dataSampler;

#include "data_texture.glsl"
#include "math_utils.glsl"
#include "line_geometry.glsl"
#include "item_opacity.glsl"

void main() {
    LineVertexInput vertexInput = lineVertexInput();
    v_color = ubuf.color;
    v_color.a *= itemOpacity(float(vertexInput.index));
    v_arcLength = vertexInput.arcLength;

    LineVertexResult vertex = computeLineVertex(vertexInput.index, vertexInput.corner, vertexInput.outgoing, vertexInput.side);
    v_edgeDistance = vertexInput.side * lineRibbonHalfExtent() * vertex.extentScale;
    v_validWeight = vertex.validWeight;
    gl_Position = vertex.position;
}
