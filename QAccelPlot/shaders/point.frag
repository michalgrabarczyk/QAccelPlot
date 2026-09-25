//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec2 v_uv;     // +-1 is markerSize; the quad extends further with antialiasing

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4  matrix;
    vec4  color;
    vec2  domainMin;
    vec2  domainMax;
    vec2  viewportSize;
    float markerSize;
    float logScaleX;
    float logScaleY;
    float useVertexColor;
    float antialiasingEnabled;
    float antialiasingFeather;
    int   shapeType;
    float markerStrokeWidth;
    float markerFilled;
    float opacity;
} ubuf;

#include "point_shapes.glsl"

void main() {
    if (ubuf.shapeType == kPixel) {
        // point.vert sizes the quad to exactly one pixel.
        float pixelAlpha = v_color.a * ubuf.opacity;
        fragColor = vec4(v_color.rgb * pixelAlpha, pixelAlpha);
        return;
    }

    bool antialiasing = ubuf.antialiasingEnabled > 0.5 && ubuf.antialiasingFeather > 0.0;
    float distancePx = shapeSDF(v_uv, ubuf.shapeType, ubuf.markerSize) * ubuf.markerSize;
    float alpha = markerAlpha(distancePx, ubuf.shapeType, antialiasing, ubuf.antialiasingFeather,
                              ubuf.markerStrokeWidth, ubuf.markerFilled >= 0.5);

    if (alpha <= 0.0) {
        discard;
    }

    float a = v_color.a * alpha * ubuf.opacity;
    fragColor = vec4(v_color.rgb * a, a);
}
