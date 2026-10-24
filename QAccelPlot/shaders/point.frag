//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 v_uv;     // +-1 is markerSize; the quad extends further with antialiasing
layout(location = 1) in float v_gradientCoordinate;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4  matrix;
    vec4  color;
    vec2  domainMin;
    vec2  domainMax;
    vec2  viewportSize;
    float logScaleX;
    float logScaleY;
    float useVertexColor;
    float markerSize;
    float antialiasingEnabled;
    float antialiasingFeather;
    int   shapeType;
    float markerStrokeWidth;
    float markerFilled;
    float gradientDirection;
    float gradientValueMin;
    float gradientValueMax;
    float opacity;
} ubuf;

layout(binding = 2) uniform sampler2D gradientSampler;

#include "point_shapes.glsl"

void main() {
    float alpha;
    if (ubuf.shapeType == kPixel) {
        // point.vert sizes the quad to exactly one pixel.
        alpha = 1.0;
    } else {
        bool antialiasing = ubuf.antialiasingEnabled > 0.5 && ubuf.antialiasingFeather > 0.0;
        float distancePx = shapeSDF(v_uv, ubuf.shapeType, ubuf.markerSize) * ubuf.markerSize;
        alpha = markerAlpha(distancePx, ubuf.shapeType, antialiasing, ubuf.antialiasingFeather,
                            ubuf.markerStrokeWidth, ubuf.markerFilled >= 0.5);
    }

    if (alpha <= 0.0) {
        discard;
    }

    // Sample the gradient unconditionally (keeps the sampler live on every backend).
    vec4 gradientColor = texture(gradientSampler, vec2(clamp(v_gradientCoordinate, 0.0, 1.0), 0.5));
    vec4 c = mix(ubuf.color, gradientColor, step(0.5, ubuf.useVertexColor));
    float a = c.a * alpha * ubuf.opacity;
    fragColor = vec4(c.rgb * a, a);
}
