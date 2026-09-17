//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 pos;      // data-space coordinate
layout(location = 1) in vec2 corner;   // quad corner: each component is -1 or +1
layout(location = 2) in vec4 vertexColor;

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec2 v_uv;    // forwarded to fragment shader for SDF shaping; +-1 is markerSize

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
} ubuf;

const int kPixelShape = 15; // LineCurve::PointShape::Pixel - 1

#include "math_utils.glsl"

void main() {
    v_color = mix(ubuf.color, vertexColor, ubuf.useVertexColor);

    // Invalid samples (non-finite, or not strictly positive on a log-scale
    // dimension) are not drawn. All six corners share the culled position, so
    // the quad has zero area. Log-scale dimensions are never origin-shifted.
    bool valid = floatIsFinite(pos.x) && floatIsFinite(pos.y)
        && (ubuf.logScaleX < 0.5 || pos.x > 0.0)
        && (ubuf.logScaleY < 0.5 || pos.y > 0.0);
    if (!valid) {
        v_uv = corner;
        gl_Position = kCulledClipPosition;
        return;
    }

    vec2 dMin = ubuf.domainMin;
    vec2 dMax = ubuf.domainMax;
    vec2 p    = pos;

    if (ubuf.logScaleX > 0.5) {
        dMin.x = safeLog10(dMin.x);
        dMax.x = safeLog10(dMax.x);
        p.x    = safeLog10(p.x);
    }

    if (ubuf.logScaleY > 0.5) {
        dMin.y = safeLog10(dMin.y);
        dMax.y = safeLog10(dMax.y);
        p.y    = safeLog10(p.y);
    }

    vec2 range   = dMax - dMin;
    vec2 p_local = vec2(
        (p.x - dMin.x) / range.x * ubuf.viewportSize.x,
        (1.0 - (p.y - dMin.y) / range.y) * ubuf.viewportSize.y
    );

    float halfExtent = ubuf.markerSize;
    if (ubuf.shapeType == kPixelShape) {
        // A Pixel marker covers exactly one pixel: snap the centre to the pixel grid
        // and ignore markerSize and antialiasing.
        p_local = floor(p_local) + 0.5;
        halfExtent = 0.5;
    } else if (ubuf.antialiasingEnabled > 0.5 && ubuf.antialiasingFeather > 0.0) {
        // With antialiasing, the quad grows by half the feather so the coverage ramp
        // centred on the shape edge is not clipped by the geometry.
        halfExtent += ubuf.antialiasingFeather * 0.5;
    }
    v_uv = corner * (halfExtent / max(ubuf.markerSize, 1e-4));

    // Expand the point centre into a billboard quad in item-local pixel space
    vec2 final_pos = p_local + corner * halfExtent;
    gl_Position = ubuf.matrix * vec4(final_pos, 0.0, 1.0);
}
