//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Shared quad geometry for vertex shaders that draw one rectangle per data item.
// Requires: a `ubuf` uniform block with color, hoverColor, domainMin, domainMax,
// viewportSize, minimumSize, logScaleX, logScaleY, useVertexColor, opacity and
// hoveredIndex members, followed by an include of math_utils.glsl.

// Corner UVs for 2-triangle quad (CCW winding):
//   Triangle 1: (0,0), (1,0), (0,1)
//   Triangle 2: (1,0), (1,1), (0,1)
vec2 cornerUV(int c) {
    // Using step functions to avoid array indexing
    if (c == 0) return vec2(0.0, 0.0);
    if (c == 1) return vec2(1.0, 0.0);
    if (c == 2) return vec2(0.0, 1.0);
    if (c == 3) return vec2(1.0, 0.0);
    if (c == 4) return vec2(1.0, 1.0);
    return vec2(0.0, 1.0); // c == 5
}

// Returns an edge's position as a fraction of the viewport from dMin (0) to dMax (1),
// clamped to one viewport beyond each side. Infinite edges map straight onto that margin,
// so they reach past the plot edge even when dMin and dMax are too far from the render
// origin to differ in single precision.
float viewportFraction(uint bits, float dMin, float dMax, bool logScale) {
    if (!isFiniteBits(bits)) {
        bool towardMin = ((bits & 0x80000000u) != 0u) == (dMax >= dMin);
        return towardMin ? -1.0 : 2.0;
    }
    float value = uintBitsToFloat(bits);
    if (logScale) {
        value = safeLog10(value);
    }
    return clamp((value - dMin) / (dMax - dMin), -1.0, 2.0);
}

// Widens the pixel span from a to b around its center to at least minSize, keeping a and b in order.
vec2 widenedSpan(float a, float b, float minSize) {
    float grow = 0.5 * max(minSize - abs(b - a), 0.0);
    float direction = a <= b ? 1.0 : -1.0;
    return vec2(a - direction * grow, b + direction * grow);
}

// Returns the fill color of rectangle `index` with the item opacity applied.
vec4 rectFillColor(int index, vec4 vertexColor) {
    vec4 color = index == int(ubuf.hoveredIndex) ? ubuf.hoverColor : mix(ubuf.color, vertexColor, ubuf.useVertexColor);
    color.a *= ubuf.opacity;
    return color;
}

// Maps the edges of a rectangle, given as float bit patterns, to item-local pixel spans with
// y growing downward. Each span is widened to the minimum size.
void rectPixelSpans(uint x1Bits, uint y1Bits, uint x2Bits, uint y2Bits, out vec2 xs, out vec2 ys) {
    bool logX = ubuf.logScaleX > 0.5;
    bool logY = ubuf.logScaleY > 0.5;
    vec2 dMin = ubuf.domainMin;
    vec2 dMax = ubuf.domainMax;
    if (logX) {
        dMin.x = safeLog10(dMin.x);
        dMax.x = safeLog10(dMax.x);
    }
    if (logY) {
        dMin.y = safeLog10(dMin.y);
        dMax.y = safeLog10(dMax.y);
    }

    vec2 size = ubuf.viewportSize;
    xs = widenedSpan(viewportFraction(x1Bits, dMin.x, dMax.x, logX) * size.x,
                     viewportFraction(x2Bits, dMin.x, dMax.x, logX) * size.x, ubuf.minimumSize.x);
    ys = widenedSpan((1.0 - viewportFraction(y1Bits, dMin.y, dMax.y, logY)) * size.y,
                     (1.0 - viewportFraction(y2Bits, dMin.y, dMax.y, logY)) * size.y, ubuf.minimumSize.y);
}

// Returns the item-local position of quad corner `corner` (0-5) of the rectangle spanning
// pixel spans xs and ys, and its pixel distances to the left, right, top, and bottom edges.
vec2 rectVertex(vec2 xs, vec2 ys, int corner, out vec4 edgeDistance) {
    vec2 uv = cornerUV(corner);
    vec2 p = vec2(uv.x < 0.5 ? xs.x : xs.y, uv.y < 0.5 ? ys.x : ys.y);
    edgeDistance = vec4(p.x - min(xs.x, xs.y), max(xs.x, xs.y) - p.x,
                        p.y - min(ys.x, ys.y), max(ys.x, ys.y) - p.y);
    return p;
}
