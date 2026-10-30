//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in float barId;
layout(location = 1) in float corner; // 0-5: which vertex of the two-triangle quad
layout(location = 2) in vec4 vertexColor;

// Same outputs as rect.vert, so bars share rect.frag.
layout(location = 0) out vec4 v_color;
layout(location = 1) out vec4 v_edgeDistance;
layout(location = 2) out vec4 v_borderColor;
layout(location = 3) out float v_borderWidth;
layout(location = 4) flat out float v_antialiasing;

// Must match BarUbo in BarMaterial.cpp: the rect.vert block followed by the bar geometry and colormap.
layout(std140, binding = 0) uniform buf {
    mat4 matrix;          //   0-63
    vec4 color;           //  64-79
    vec4 borderColor;     //  80-95
    vec4 hoverColor;      //  96-111
    vec2 domainMin;       // 112-119
    vec2 domainMax;       // 120-127
    vec2 viewportSize;    // 128-135
    vec2 minimumSize;     // 136-143: minimum drawn width and height in pixels
    float logScaleX;      // 144-147
    float logScaleY;      // 148-151
    float useVertexColor; // 152-155
    float rectCount;      // 156-159
    float opacity;        // 160-163: inherited item opacity
    float borderWidth;    // 164-167: outline width in pixels
    float hoveredIndex;   // 168-171: -1 when no bar is highlighted
    float antialiasing;   // 172-175: 1 to smooth the edges
    float barWidth;       // 176-179: position-axis data units
    float barOffset;      // 180-183: position-axis data units
    float baseline;       // 184-187: value-axis data units, may be -Inf
    float horizontal;     // 188-191: 1 when positions lie on the Y axis
    float ranged;         // 192-195: 1 when a bar is (from, to, value) instead of (position, value)
    float colorSource;    // 196-199: 0 no colormap, 1 the value, 2 the position is mapped
    float valueMin;       // 200-203: value at colormap position 0
    float valueMax;       // 204-207: value at colormap position 1
    float valueOrigin;    // 208-211: added to the data before the log normalization
    float valueLogScale;  // 212-215: 1 to normalize in log10
} ubuf;

layout(binding = 1) uniform sampler2D dataSampler;
layout(binding = 2) uniform sampler2D colorMapSampler;

#include "data_texture.glsl"
#include "math_utils.glsl"
#include "rect_geometry.glsl"

// Reads the extent of bar `index` along the position axis and its value, as float bit patterns.
// Returns false when the bar is not drawn.
bool readBar(int index, out uint startBits, out uint endBits, out uint valueBits) {
    if (ubuf.ranged > 0.5) {
        int base = index * 3;
        startBits = fetchFloatBits(base);
        endBits = fetchFloatBits(base + 1);
        valueBits = fetchFloatBits(base + 2);
        return !isNaNBits(startBits) && !isNaNBits(endBits) && !isNaNBits(valueBits);
    }
    int base = index * 2;
    uint positionBits = fetchFloatBits(base);
    valueBits = fetchFloatBits(base + 1);
    float center = uintBitsToFloat(positionBits) + ubuf.barOffset;
    startBits = floatBitsToUint(center - 0.5 * ubuf.barWidth);
    endBits = floatBitsToUint(center + 0.5 * ubuf.barWidth);
    return isFiniteBits(positionBits) && !isNaNBits(valueBits);
}

// Colormap position of an infinite value. Finite, so the middle of a ranged bar with one infinite
// edge stays beyond the colormap end.
const float kFarColormapPosition = 1.0e30;

// Places the origin-relative float bit pattern `bits` on the colormap, unclamped: 0 at valueMin
// and 1 at valueMax. Returns false when the normalization cannot place it.
bool colormapPosition(uint bits, out float position) {
    position = 0.0;
    bool logScale = ubuf.valueLogScale > 0.5;
    float lo = ubuf.valueMin;
    float hi = ubuf.valueMax;
    bool negative = (bits & 0x80000000u) != 0u;
    if (isNaNBits(bits) || (logScale && (lo <= 0.0 || hi <= 0.0))) {
        return false;
    }
    if (!isFiniteBits(bits)) {
        if (logScale && negative) {
            return false;
        }
        position = negative == (hi >= lo) ? -kFarColormapPosition : kFarColormapPosition;
        return true;
    }
    float value = uintBitsToFloat(bits) + ubuf.valueOrigin;
    if (logScale) {
        if (value <= 0.0) {
            return false;
        }
        value = safeLog10(value);
        lo = safeLog10(lo);
        hi = safeLog10(hi);
    }
    float range = hi - lo;
    if (abs(range) > 0.0) {
        position = clamp((value - lo) / range, -kFarColormapPosition, kFarColormapPosition);
    }
    return true;
}

// Returns where bar `index` lies on the colormap, from 0 to 1, or -1 when it keeps the rectangle color.
// A ranged bar's position is the middle of its edges on the colormap. Positions beyond the range
// clamp to its ends.
float barColormapPosition(int index) {
    if (ubuf.colorSource < 0.5) {
        return -1.0;
    }
    int stride = ubuf.ranged > 0.5 ? 3 : 2;
    int base = index * stride;
    float position;
    if (ubuf.colorSource < 1.5) {
        return colormapPosition(fetchFloatBits(base + stride - 1), position) ? clamp(position, 0.0, 1.0) : -1.0;
    }
    if (!colormapPosition(fetchFloatBits(base), position)) {
        return -1.0;
    }
    if (ubuf.ranged > 0.5) {
        float end;
        if (!colormapPosition(fetchFloatBits(base + 1), end)) {
            return -1.0;
        }
        position = 0.5 * (position + end);
    }
    return clamp(position, 0.0, 1.0);
}

// Returns the fill color of bar `index`: the hover color, its colormap color, or the rectangle color.
vec4 barFillColor(int index) {
    float t = index == int(ubuf.hoveredIndex) ? -1.0 : barColormapPosition(index);
    if (t < 0.0) {
        return rectFillColor(index, vertexColor);
    }
    vec4 color = textureLod(colorMapSampler, vec2(t, 0.5), 0.0);
    color.a *= ubuf.opacity;
    return color;
}

void main() {
    int index = int(barId);
    v_color = barFillColor(index);
    v_borderColor = vec4(ubuf.borderColor.rgb, ubuf.borderColor.a * ubuf.opacity);
    v_borderWidth = ubuf.borderWidth;
    v_antialiasing = ubuf.antialiasing;

    uint startBits;
    uint endBits;
    uint valueBits;
    if (!readBar(index, startBits, endBits, valueBits)) {
        gl_Position = kCulledClipPosition;
        v_edgeDistance = vec4(0.0);
        return;
    }
    uint baselineBits = floatBitsToUint(ubuf.baseline);

    vec2 xs;
    vec2 ys;
    if (ubuf.horizontal > 0.5) {
        rectPixelSpans(baselineBits, startBits, valueBits, endBits, xs, ys);
    } else {
        rectPixelSpans(startBits, baselineBits, endBits, valueBits, xs, ys);
    }
    vec2 p_local = rectVertex(xs, ys, int(corner), v_edgeDistance);
    gl_Position = ubuf.matrix * vec4(p_local, 0.0, 1.0);
}
