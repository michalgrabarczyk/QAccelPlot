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
layout(location = 1) in float side;
layout(location = 2) in vec4 vertexColor;
layout(location = 3) in float arcLength;

layout(location = 0) out vec4 v_color;
layout(location = 1) out float v_edgeDistance;
layout(location = 2) out float v_fadeStart;
layout(location = 3) out float v_softness;
layout(location = 4) out float v_arcLength;
layout(location = 5) out float v_gradientCoordinate;

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
} ubuf;

layout(binding = 1) uniform sampler2D dataSampler;

#include "data_texture.glsl"
#include "math_utils.glsl"

vec2 fetchPos(int index) {
    index = clamp(index, 0, int(ubuf.pointCount) - 1);
    int base = index * 2;
    return vec2(fetchFloat(base), fetchFloat(base + 1));
}

void main() {
    v_color = mix(ubuf.color, vertexColor, ubuf.useVertexColor);
    v_edgeDistance = side;
    v_arcLength = arcLength;

    if (ubuf.antialiasingEnabled > 0.5 && ubuf.antialiasingFeather > 0.0) {
        float halfWidth = max(ubuf.lineWidth * 0.5, 1e-4);
        float featherNorm = max(ubuf.antialiasingFeather / halfWidth, 0.01);
        v_fadeStart = 1.0 - featherNorm;
        v_softness = featherNorm;
    } else {
        v_fadeStart = 2.0;
        v_softness = 1.0;
    }

    vec2 dMin = ubuf.domainMin;
    vec2 dMax = ubuf.domainMax;

    int index = int(id);
    vec2 p = fetchPos(index);
    vec2 pr = fetchPos(index - 1);
    vec2 nx = fetchPos(index + 1);

    // Evaluate the palette coordinate from the untransformed data value. This
    // matches GradientFill and QML Gradient semantics: horizontal runs left to
    // right, while vertical position 0 is the top (largest visible Y value).
    float gradientValue = ubuf.gradientParameters.x < 0.5 ? p.x : p.y;
    float gradientRange = max(ubuf.gradientParameters.z - ubuf.gradientParameters.y, 1e-6);
    // Preserve values outside [0, 1] through raster interpolation. Clamping at
    // vertices would stretch the palette across an entire segment that crosses
    // a configured gradient boundary. The fragment shader clamps before sampling.
    float normalizedGradient = (gradientValue - ubuf.gradientParameters.y) / gradientRange;
    v_gradientCoordinate = ubuf.gradientParameters.x < 0.5 ? normalizedGradient : 1.0 - normalizedGradient;

    if (ubuf.logScaleX > 0.5) {
        dMin.x = safeLog10(dMin.x);
        dMax.x = safeLog10(dMax.x);
        p.x = safeLog10(p.x);
        pr.x = safeLog10(pr.x);
        nx.x = safeLog10(nx.x);
    }

    if (ubuf.logScaleY > 0.5) {
        dMin.y = safeLog10(dMin.y);
        dMax.y = safeLog10(dMax.y);
        p.y = safeLog10(p.y);
        pr.y = safeLog10(pr.y);
        nx.y = safeLog10(nx.y);
    }

    vec2 range = dMax - dMin;
    vec2 p_local = vec2(
        (p.x - dMin.x) / range.x * ubuf.viewportSize.x,
        (1.0 - (p.y - dMin.y) / range.y) * ubuf.viewportSize.y
    );
    vec2 prev_local = vec2(
        (pr.x - dMin.x) / range.x * ubuf.viewportSize.x,
        (1.0 - (pr.y - dMin.y) / range.y) * ubuf.viewportSize.y
    );
    vec2 next_local = vec2(
        (nx.x - dMin.x) / range.x * ubuf.viewportSize.x,
        (1.0 - (nx.y - dMin.y) / range.y) * ubuf.viewportSize.y
    );

    vec2 dir1 = p_local - prev_local;
    vec2 dir2 = next_local - p_local;
    float len1 = length(dir1);
    float len2 = length(dir2);
    if (len1 > 0.0001) dir1 /= len1; else dir1 = vec2(0.0);
    if (len2 > 0.0001) dir2 /= len2; else dir2 = vec2(0.0);
    if (len1 <= 0.0001) dir1 = dir2;
    if (len2 <= 0.0001) dir2 = dir1;

    vec2 tangent = dir1 + dir2;
    if (length(tangent) > 0.0001) tangent = normalize(tangent);
    else tangent = vec2(1.0, 0.0);
    vec2 normal = vec2(-tangent.y, tangent.x);
    vec2 n1 = vec2(-dir1.y, dir1.x);
    float miterDot = dot(normal, n1);
    float miterLength = 1.0;
    if (abs(miterDot) > 0.15) miterLength = 1.0 / abs(miterDot);

    vec2 offset = normal * (ubuf.lineWidth * 0.5) * miterLength * side;
    gl_Position = ubuf.matrix * vec4(p_local + offset, 0.0, 1.0);
}
