//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
// Edge-to-pixel mapping shared by vertex shaders that draw quads from data edges.
// Requires: an include of math_utils.glsl.

#include "viewport_fraction.glsl"

// The plot domain and viewport of the X (.x) and Y (.y) axes. Build it with axisMapping().
struct AxisMapping {
    vec2 domainMin; // log10 on a log axis
    vec2 domainMax; // log10 on a log axis
    vec2 viewportSize;
    bvec2 logScale;
};

// Returns the mapping of the domain from domainMin to domainMax, given in data units, onto viewportSize.
AxisMapping axisMapping(vec2 domainMin, vec2 domainMax, vec2 viewportSize, bvec2 logScale) {
    AxisMapping mapping;
    mapping.domainMin = domainMin;
    mapping.domainMax = domainMax;
    mapping.viewportSize = viewportSize;
    mapping.logScale = logScale;
    if (logScale.x) {
        mapping.domainMin.x = safeLog10(domainMin.x);
        mapping.domainMax.x = safeLog10(domainMax.x);
    }
    if (logScale.y) {
        mapping.domainMin.y = safeLog10(domainMin.y);
        mapping.domainMax.y = safeLog10(domainMax.y);
    }
    return mapping;
}

// Maps the float bit pattern `bits` on `axis` (0 = X, 1 = Y) to an item pixel coordinate, with y
// growing downward. Coordinates beyond the plot edge are clamped one viewport past it.
float axisPixel(AxisMapping mapping, uint bits, int axis) {
    if (axis == 0) {
        return viewportFraction(bits, mapping.domainMin.x, mapping.domainMax.x, mapping.logScale.x) * mapping.viewportSize.x;
    }
    return (1.0 - viewportFraction(bits, mapping.domainMin.y, mapping.domainMax.y, mapping.logScale.y)) * mapping.viewportSize.y;
}

// Returns the pixel span between a and b, in increasing order, widened around its center to at
// least minSize. Matches QAccelPlot::Internal::widenedSpan(), so hit tests use the drawn size.
vec2 widenedSpan(float a, float b, float minSize) {
    float low = min(a, b);
    float high = max(a, b);
    float grow = 0.5 * max(minSize - (high - low), 0.0);
    return vec2(low - grow, high + grow);
}
