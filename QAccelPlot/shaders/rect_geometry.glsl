//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
// Shared quad geometry for vertex shaders that draw one rectangle per data item.
// Requires: a `ubuf` uniform block with color, hoverColor, domainMin, domainMax,
// viewportSize, minimumSize, logScaleX, logScaleY, useVertexColor, opacity, hoveredIndex and
// antialiasing members, followed by an include of math_utils.glsl.

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

#include "axis_pixel.glsl"

// Returns the fill color of rectangle `index`, without the item opacity.
vec4 rectFillColor(int index, vec4 vertexColor) {
    return index == int(ubuf.hoveredIndex) ? ubuf.hoverColor : mix(ubuf.color, vertexColor, ubuf.useVertexColor);
}

// Maps the edges of a rectangle, given as float bit patterns, to item-local pixel spans with
// y growing downward. Each span is widened to the minimum size.
void rectPixelSpans(uint x1Bits, uint y1Bits, uint x2Bits, uint y2Bits, out vec2 xs, out vec2 ys) {
    AxisMapping mapping = axisMapping(ubuf.domainMin, ubuf.domainMax, ubuf.viewportSize, bvec2(ubuf.logScaleX > 0.5, ubuf.logScaleY > 0.5));
    xs = widenedSpan(axisPixel(mapping, x1Bits, 0), axisPixel(mapping, x2Bits, 0), ubuf.minimumSize.x);
    ys = widenedSpan(axisPixel(mapping, y1Bits, 1), axisPixel(mapping, y2Bits, 1), ubuf.minimumSize.y);
}

// Pixels the quad extends past each edge with antialiasing, so edge pixels get partial coverage.
const float kRectAntialiasingMargin = 0.5;

// Returns the item-local position of quad corner `corner` (0-5) of the rectangle spanning
// pixel spans xs and ys, and its pixel distances to the left, right, top, and bottom edges.
// With antialiasing, the quad extends past the rectangle, where the distances are negative. It
// extends `glowRadius` pixels further for the halo of a glow.
vec2 rectVertex(vec2 xs, vec2 ys, int corner, float glowRadius, out vec4 edgeDistance) {
    vec2 uv = cornerUV(corner);
    float margin = (ubuf.antialiasing > 0.5 ? kRectAntialiasingMargin : 0.0) + glowRadius;
    vec2 lo = vec2(min(xs.x, xs.y), min(ys.x, ys.y)) - margin;
    vec2 hi = vec2(max(xs.x, xs.y), max(ys.x, ys.y)) + margin;
    vec2 p = vec2(uv.x < 0.5 ? lo.x : hi.x, uv.y < 0.5 ? lo.y : hi.y);
    edgeDistance = vec4(p.x - min(xs.x, xs.y), max(xs.x, xs.y) - p.x,
                        p.y - min(ys.x, ys.y), max(ys.x, ys.y) - p.y);
    return p;
}
