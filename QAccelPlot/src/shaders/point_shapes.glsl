//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Marker shapes shared by LineCurve markers (point.*) and PointCloud (point_cloud.*).
// The billboard quad spans uv -1..+1; every SDF returns a value where <= 1.0 is inside.
// Shape ids: 0=Circle, 1=Square, 2=Diamond, 3=TriangleUp, 4=TriangleDown, 5=Cross.

float sdf_circle(vec2 uv) {
    return length(uv);
}

float sdf_square(vec2 uv) {
    return max(abs(uv.x), abs(uv.y));
}

float sdf_diamond(vec2 uv) {
    // Scale down uv to make the shape bigger within the billboard.
    uv *= 0.75;
    // Narrow the horizontal axis so the shape is taller than wide.
    // Horizontal corners at +-0.6, vertical corners at +-1.
    return abs(uv.x * (1.0 / 0.6)) + abs(uv.y);
}

float sdf_triangle_base(vec2 uv) {
    // Scale uv down so the triangle fills ~75% of the billboard (making it
    // visually comparable in weight to a circle of the same marker size).
    uv *= 0.72;
    // Equilateral triangle: apex (0, 1), base corners (+-sqrt(3)/2, -1/2).
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

float shapeSDF(vec2 uv, int shapeType) {
    if (shapeType == 1) { return sdf_square(uv); }
    if (shapeType == 2) { return sdf_diamond(uv); }
    if (shapeType == 3) { return sdf_triangle_up(uv); }
    if (shapeType == 4) { return sdf_triangle_down(uv); }
    if (shapeType == 5) { return sdf_cross(uv); }
    return sdf_circle(uv); // default: 0 = Circle
}

// Vertex stage: precomputes the anti-aliasing fade for markerAlpha().
// fadeStart >= 1.0 is the sentinel for disabled anti-aliasing.
void computeMarkerFade(float markerSize, float antialiasingEnabled, float antialiasingFeather, out float fadeStart, out float softness) {
    if (antialiasingEnabled > 0.5 && antialiasingFeather > 0.0) {
        float radius = max(markerSize, 1e-4);
        float featherNorm = max(antialiasingFeather / radius, 0.01);
        fadeStart = 1.0 - featherNorm;
        softness = featherNorm;
    } else {
        fadeStart = 2.0;
        softness = 1.0;
    }
}

// Fragment stage: coverage in [0, 1] for an SDF distance.
float markerAlpha(float dist, float fadeStart, float softness) {
    if (fadeStart < 1.0) {
        float baseAlpha = 1.0 - smoothstep(max(fadeStart, 0.0), 1.0, dist);
        return pow(baseAlpha, max(softness, 1.0));
    }
    return dist <= 1.0 ? 1.0 : 0.0;
}
