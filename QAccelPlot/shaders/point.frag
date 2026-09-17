//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440

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
} ubuf;

// Shape indices: LineCurve::PointShape value - 1.
const int kCircle        = 0;
const int kSquare        = 1;
const int kDiamond       = 2;
const int kTriangleUp    = 3;
const int kTriangleDown  = 4;
const int kTriangleLeft  = 5;
const int kTriangleRight = 6;
const int kCross         = 7;
const int kXCross        = 8;
const int kHLine         = 9;
const int kVLine         = 10;
const int kStar          = 11;
const int kAsterisk      = 12;
const int kPixel         = 13;
const int kHexagon       = 14;
const int kPentagon      = 15;

// Half-width of the bars of Cross and XCross, and of the thinner Asterisk and line bars, in marker radii.
const float kBarHalfWidth = 0.35;
const float kThinBarHalfWidth = 0.2;

// --- Signed distance functions ---
// Each returns the signed distance to the shape outline in marker radii, negative inside.
// p is y-up with +-1 at markerSize, and every shape stays within that square so the quad
// never clips it. Formulas follow https://iquilezles.org/articles/distfunctions2d/.

float sdCircle(vec2 p) {
    return length(p) - 1.0;
}

float sdBox(vec2 p, vec2 halfSize) {
    vec2 d = abs(p) - halfSize;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

float sdRhombus(vec2 p, vec2 halfSize) {
    p = abs(p);
    vec2 b = halfSize;
    float h = clamp(((b.x - 2.0 * p.x) * b.x - (b.y - 2.0 * p.y) * b.y) / dot(b, b), -1.0, 1.0);
    float d = length(p - 0.5 * b * vec2(1.0 - h, 1.0 + h));
    return d * sign(p.x * b.y + p.y * b.x - b.x * b.y);
}

float sdTriangleUp(vec2 p) {
    // Equilateral triangle with side 2: apex (0, 1), base corners (+-1, 1 - sqrt(3)).
    const float k = sqrt(3.0);
    p.y += 2.0 / k - 1.0; // move the centroid to the origin
    p.x = abs(p.x) - 1.0;
    p.y = p.y + 1.0 / k;
    if (p.x + k * p.y > 0.0) {
        p = vec2(p.x - k * p.y, -k * p.x - p.y) / 2.0;
    }
    p.x -= clamp(p.x, -2.0, 0.0);
    return -length(p) * sign(p.y);
}

float barHalfWidth(float halfWidth) {
    // Keep bars at least 1 px wide so small markers stay solid.
    return max(halfWidth, 0.5 / max(ubuf.markerSize, 1e-4));
}

float sdPlus(vec2 p, float halfWidth) {
    float w = barHalfWidth(halfWidth);
    return min(sdBox(p, vec2(1.0, w)), sdBox(p, vec2(w, 1.0)));
}

vec2 rotate45(vec2 p) {
    return vec2(p.x + p.y, p.y - p.x) * 0.70710678;
}

float sdStar(vec2 p) {
    // Five points on the unit circle, first point up; inner vertices at half the outer radius.
    const vec2 k1 = vec2(0.809016994375, -0.587785252292);
    const vec2 k2 = vec2(-k1.x, k1.y);
    const float innerRatio = 0.5;
    p.x = abs(p.x);
    p -= 2.0 * max(dot(k1, p), 0.0) * k1;
    p -= 2.0 * max(dot(k2, p), 0.0) * k2;
    p.x = abs(p.x);
    p.y -= 1.0;
    vec2 ba = innerRatio * vec2(-k1.y, k1.x) - vec2(0.0, 1.0);
    float h = clamp(dot(p, ba) / dot(ba, ba), 0.0, 1.0);
    return length(p - ba * h) * sign(p.y * ba.x - p.x * ba.y);
}

float sdPentagon(vec2 p) {
    // Regular pentagon with circumradius 1, vertex up.
    const vec3 k = vec3(0.809016994, 0.587785252, 0.726542528);
    const float apothem = 0.809016994;
    p.y = -p.y;
    p.x = abs(p.x);
    p -= 2.0 * min(dot(vec2(-k.x, k.y), p), 0.0) * vec2(-k.x, k.y);
    p -= 2.0 * min(dot(vec2(k.x, k.y), p), 0.0) * vec2(k.x, k.y);
    p -= vec2(clamp(p.x, -apothem * k.z, apothem * k.z), apothem);
    return length(p) * sign(p.y);
}

float sdHexagon(vec2 p) {
    // Regular hexagon with circumradius 1, vertex up.
    const vec3 k = vec3(-0.866025404, 0.5, 0.577350269);
    const float apothem = 0.866025404;
    p = abs(p.yx);
    p -= 2.0 * min(dot(k.xy, p), 0.0) * k.xy;
    p -= vec2(clamp(p.x, -k.z * apothem, k.z * apothem), apothem);
    return length(p) * sign(p.y);
}

bool isLineShape(int shape) {
    return shape == kCross || shape == kXCross || shape == kAsterisk || shape == kHLine || shape == kVLine;
}

float shapeSDF(vec2 uv) {
    // v_uv is y-down like the item; the shapes are defined y-up.
    vec2 p = vec2(uv.x, -uv.y);
    int shape = ubuf.shapeType;
    if (shape == kSquare)        { return sdBox(p, vec2(1.0)); }
    if (shape == kDiamond)       { return sdRhombus(p, vec2(0.8, 1.0)); }
    if (shape == kTriangleUp)    { return sdTriangleUp(p); }
    if (shape == kTriangleDown)  { return sdTriangleUp(vec2(p.x, -p.y)); }
    if (shape == kTriangleLeft)  { return sdTriangleUp(vec2(p.y, -p.x)); }
    if (shape == kTriangleRight) { return sdTriangleUp(vec2(p.y, p.x)); }
    if (shape == kCross)         { return sdPlus(p, kBarHalfWidth); }
    if (shape == kXCross)        { return sdPlus(rotate45(p), kBarHalfWidth); }
    if (shape == kStar)          { return sdStar(p); }
    if (shape == kAsterisk)      { return min(sdPlus(p, kThinBarHalfWidth), sdPlus(rotate45(p), kThinBarHalfWidth)); }
    if (shape == kPentagon)      { return sdPentagon(p); }
    if (shape == kHexagon)       { return sdHexagon(p); }
    if (shape == kHLine)         { return sdBox(p, vec2(1.0, barHalfWidth(kThinBarHalfWidth))); }
    if (shape == kVLine)         { return sdBox(p, vec2(barHalfWidth(kThinBarHalfWidth), 1.0)); }
    return sdCircle(p);
}

void main() {
    if (ubuf.shapeType == kPixel) {
        // point.vert sizes the quad to exactly one pixel.
        fragColor = vec4(v_color.rgb * v_color.a, v_color.a);
        return;
    }

    bool antialiasing = ubuf.antialiasingEnabled > 0.5 && ubuf.antialiasingFeather > 0.0;
    float distancePx = shapeSDF(v_uv) * ubuf.markerSize;
    float coverage = 1.0;

    if (ubuf.markerFilled < 0.5 && !isLineShape(ubuf.shapeType)) {
        // Hollow: a ring of markerStrokeWidth inside the outline, so the marker keeps its size.
        // Like lines, strokes thinner than 1 px draw 1 px wide with proportionally reduced alpha.
        float strokeWidth = max(ubuf.markerStrokeWidth, 0.0);
        float drawnWidth = antialiasing ? max(strokeWidth, 1.0) : strokeWidth;
        distancePx = abs(distancePx + drawnWidth * 0.5) - drawnWidth * 0.5;
        if (antialiasing) {
            coverage = clamp(strokeWidth, 0.0, 1.0);
        }
    }

    float alpha;
    if (antialiasing) {
        // Coverage ramps linearly across `antialiasingFeather` pixels centred on the shape edge.
        alpha = clamp(-distancePx / ubuf.antialiasingFeather + 0.5, 0.0, 1.0) * coverage;
    } else {
        alpha = distancePx <= 0.0 ? 1.0 : 0.0;
    }

    if (alpha <= 0.0) {
        discard;
    }

    float a = v_color.a * alpha;
    fragColor = vec4(v_color.rgb * a, a);
}
