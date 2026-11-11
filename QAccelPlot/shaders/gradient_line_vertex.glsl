//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Vertex stage of the gradient line ribbon shared by gradient_line.vert and
// gradient_line_dashed.vert; define LINE_DASHED first, see line_vertex_input.glsl.

#include "line_vertex_input.glsl"

layout(location = 0) out vec4 v_color;
layout(location = 1) out float v_edgeDistance; // signed distance from the line centre, in pixels
layout(location = 2) out float v_arcLength;
layout(location = 3) out float v_gradientCoordinate;
layout(location = 4) out float v_validWeight;
layout(location = 5) out float v_itemOpacity;

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
    vec4 gradientParameters;
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
    v_itemOpacity = itemOpacity(float(vertexInput.index));
    v_arcLength = vertexInput.arcLength;

    LineVertexResult vertex = computeLineVertex(vertexInput.index, vertexInput.corner, vertexInput.side);
    v_edgeDistance = vertexInput.side * lineRibbonHalfExtent() * vertex.extentScale;
    v_validWeight = vertex.validWeight;
    gl_Position = vertex.position;

    if (vertex.validWeight < 0.0) {
        // Invalid samples have no meaningful palette coordinate; every fragment
        // interpolating from this vertex is discarded.
        v_gradientCoordinate = 0.0;
        return;
    }

    // Evaluate the palette coordinate from the untransformed data value. This
    // matches GradientFill and QML Gradient semantics: horizontal runs left to
    // right, while vertical position 0 is the top (largest visible Y value).
    vec2 p = vertex.dataPosition;
    float gradientValue = ubuf.gradientParameters.x < 0.5 ? p.x : p.y;
    float gradientRange = max(ubuf.gradientParameters.z - ubuf.gradientParameters.y, 1e-6);
    // Preserve values outside [0, 1] through raster interpolation. Clamping at
    // vertices would stretch the palette across an entire segment that crosses
    // a configured gradient boundary. The fragment shader clamps before sampling.
    float normalizedGradient = (gradientValue - ubuf.gradientParameters.y) / gradientRange;
    v_gradientCoordinate = ubuf.gradientParameters.x < 0.5 ? normalizedGradient : 1.0 - normalizedGradient;
}
