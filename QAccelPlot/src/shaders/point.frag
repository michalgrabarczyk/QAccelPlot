//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec2 v_uv;     // -1..+1 across the billboard quad
layout(location = 2) in float v_fadeStart;
layout(location = 3) in float v_softness;

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
} ubuf;

// --- SDF functions --- each returns a value where <= 1.0 is inside the shape ---

float sdf_circle(vec2 uv) {
    return length(uv);
}

float sdf_square(vec2 uv) {
    return max(abs(uv.x), abs(uv.y));
}

float sdf_diamond(vec2 uv) {
    // Scale down uv to make the shape bigger within the billboard.
    uv *= 0.75;
    // Narrow the horizontal axis so the shape is taller than wide (♦).
    // Horizontal corners at ±0.6, vertical corners at ±1.
    return abs(uv.x * (1.0 / 0.6)) + abs(uv.y);
}

float sdf_triangle_base(vec2 uv) {
    // Scale uv down so the triangle fills ~75% of the billboard (making it
    // visually comparable in weight to a circle of the same markerSize).
    uv *= 0.72;
    // Equilateral triangle: apex (0, 1), base corners (±√3/2, -½).
    const float k = sqrt(3.0);
    float d1 = uv.y + 0.5;
    float d2 = 1.0 - k * uv.x - uv.y;
    float d3 = 1.0 + k * uv.x - uv.y;
    float inside = min(d1, min(d2, d3));
    return 1.0 - inside * 2.0;
}

float sdf_triangle_up(vec2 uv) {
    // Screen Y is flipped vs data Y, so negate to make apex point up in data space.
    return sdf_triangle_base(vec2(uv.x, -uv.y));
}

float sdf_triangle_down(vec2 uv) {
    return sdf_triangle_base(uv);
}

float sdf_cross(vec2 uv) {
    // Plus-sign: union of a horizontal bar and a vertical bar.
    // arm = half-width of each bar.
    const float arm = 0.35;
    float hbar = min(1.0 - abs(uv.x), arm - abs(uv.y)); // inside horiz bar
    float vbar = min(1.0 - abs(uv.y), arm - abs(uv.x)); // inside vert bar
    float inside = max(hbar, vbar);    // union: positive if inside either bar
    return 1.0 - inside / arm;         // dist=0 at centre, dist=1 at bar edge
}

float shapeSDF(vec2 uv) {
    if (ubuf.shapeType == 1) { return sdf_square(uv); }
    if (ubuf.shapeType == 2) { return sdf_diamond(uv); }
    if (ubuf.shapeType == 3) { return sdf_triangle_up(uv); }
    if (ubuf.shapeType == 4) { return sdf_triangle_down(uv); }
    if (ubuf.shapeType == 5) { return sdf_cross(uv); }
    return sdf_circle(uv); // default: 0 = Circle
}

void main() {
    float dist  = shapeSDF(v_uv);
    float alpha;
    if (v_fadeStart < 1.0) {
        float baseAlpha = 1.0 - smoothstep(max(v_fadeStart, 0.0), 1.0, dist);
        alpha = pow(baseAlpha, max(v_softness, 1.0));
    } else {
        alpha = dist <= 1.0 ? 1.0 : 0.0;
    }

    if (alpha <= 0.0) {
        discard;
    }

    float a = v_color.a * alpha;
    fragColor = vec4(v_color.rgb * a, a);
}
