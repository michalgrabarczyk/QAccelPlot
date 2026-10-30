//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440
#extension GL_GOOGLE_include_directive : require

// Each error bar takes 18 vertices: a quad of two triangles for the line, the cap at low, and the
// cap at high. The error bar, part, and corner come from gl_VertexIndex. The scene graph requires at
// least one vertex attribute, so the vertex buffer holds this placeholder; its content is ignored.
layout(location = 0) in vec4 vertexPadding;

layout(location = 0) out vec4 v_color;
// Pixel distances to the two edges across the part, then to the two edges along it; negative in
// the antialiasing margin outside the part.
layout(location = 1) out vec4 v_edgeDistance;
layout(location = 2) flat out float v_antialiasing;

// Must match ErrorBarUbo in ErrorBarMaterial.cpp.
layout(std140, binding = 0) uniform buf {
    mat4 matrix;          //   0-63
    vec4 color;           //  64-79
    vec2 domainMin;       //  80-87
    vec2 domainMax;       //  88-95
    vec2 viewportSize;    //  96-103
    float logScaleX;      // 104-107
    float logScaleY;      // 108-111
    float useVertexColor; // 112-115: unused
    float opacity;        // 116-119: inherited item opacity
    float hoveredIndex;   // 120-123: -1 when no error bar is highlighted
    float lineWidth;      // 124-127: pixels
    vec4 hoverColor;      // 128-143
    float capLength;      // 144-147: pixels
    float offset;         // 148-151: data units across the bounds
    float horizontal;     // 152-155: 1 when the bounds lie on the X axis
    float errorBarCount;  // 156-159
    float antialiasing;   // 160-163: 1 to smooth the edges
} ubuf;

layout(binding = 1) uniform sampler2D dataSampler;

#include "data_texture.glsl"
#include "math_utils.glsl"
#include "viewport_fraction.glsl"

const int kVerticesPerErrorBar = 18;
const int kValuesPerErrorBar = 4;
// Antialiasing margin around each quad, in pixels.
const float kFeather = 1.0;

void cull() {
    v_color = vec4(0.0);
    v_edgeDistance = vec4(0.0);
    v_antialiasing = 0.0;
    gl_Position = kCulledClipPosition;
}

bool isLogAxis(int axis) {
    return (axis == 0 ? ubuf.logScaleX : ubuf.logScaleY) > 0.5;
}

// Maps the float bit pattern `bits` on `axis` (0 = X, 1 = Y) to an item pixel coordinate, with y
// growing downward. Coordinates beyond the plot edge are clamped one viewport past it.
float axisPixel(uint bits, int axis) {
    bool logScale = isLogAxis(axis);
    float dMin = axis == 0 ? ubuf.domainMin.x : ubuf.domainMin.y;
    float dMax = axis == 0 ? ubuf.domainMax.x : ubuf.domainMax.y;
    if (logScale) {
        dMin = safeLog10(dMin);
        dMax = safeLog10(dMax);
    }
    float fraction = viewportFraction(bits, dMin, dMax, logScale);
    return axis == 0 ? fraction * ubuf.viewportSize.x : (1.0 - fraction) * ubuf.viewportSize.y;
}

// Whether a cap is drawn at a bound: finite, and positive on a log axis.
bool capped(uint bits, int axis) {
    return isFiniteBits(bits) && !(isLogAxis(axis) && uintBitsToFloat(bits) <= 0.0);
}

void main() {
    // Reading the placeholder keeps it in the shader interface on every backend. UNorm values are
    // finite, so the product is always zero.
    int vertexIndex = gl_VertexIndex + int(vertexPadding.x * 0.0);
    int index = vertexIndex / kVerticesPerErrorBar;
    int part = (vertexIndex / 6) % 3;
    int corner = vertexIndex % 6;
    if (float(index) >= ubuf.errorBarCount || ubuf.lineWidth <= 0.0) {
        cull();
        return;
    }

    int base = index * kValuesPerErrorBar;
    uint xBits = fetchFloatBits(base);
    uint yBits = fetchFloatBits(base + 1);
    uint lowBits = fetchFloatBits(base + 2);
    uint highBits = fetchFloatBits(base + 3);

    bool horizontal = ubuf.horizontal > 0.5;
    int positionAxis = horizontal ? 1 : 0;
    int valueAxis = 1 - positionAxis;
    uint positionBits = horizontal ? yBits : xBits;
    uint valueBits = horizontal ? xBits : yBits;
    float position = uintBitsToFloat(positionBits) + ubuf.offset;
    if (!isFiniteBits(positionBits) || isNaNBits(valueBits) || (isLogAxis(positionAxis) && position <= 0.0)) {
        cull();
        return;
    }

    // A NaN bound leaves out its half: the line runs from the value instead, without a cap.
    bool lowCapped = capped(lowBits, valueAxis);
    bool highCapped = capped(highBits, valueAxis);
    uint startBits = isNaNBits(lowBits) ? valueBits : lowBits;
    uint endBits = isNaNBits(highBits) ? valueBits : highBits;

    float across = axisPixel(floatBitsToUint(position), positionAxis);
    float start = axisPixel(startBits, valueAxis);
    float end = axisPixel(endBits, valueAxis);
    float halfWidth = 0.5 * ubuf.lineWidth;

    // The part as spans across and along the bounds, in pixels.
    vec2 acrossSpan;
    vec2 alongSpan;
    if (part == 0) {
        acrossSpan = vec2(across - halfWidth, across + halfWidth);
        alongSpan = vec2(min(start, end), max(start, end));
    } else {
        bool isLow = part == 1;
        if (ubuf.capLength <= 0.0 || !(isLow ? lowCapped : highCapped)) {
            cull();
            return;
        }
        float halfLength = 0.5 * max(ubuf.capLength, ubuf.lineWidth);
        float bound = isLow ? start : end;
        acrossSpan = vec2(across - halfLength, across + halfLength);
        alongSpan = vec2(bound - halfWidth, bound + halfWidth);
    }

    // Corners of two triangles: (0,0), (1,0), (0,1) and (1,0), (1,1), (0,1).
    bool high0 = corner == 1 || corner == 3 || corner == 4;
    bool high1 = corner == 2 || corner == 4 || corner == 5;
    float margin = ubuf.antialiasing > 0.5 ? kFeather : 0.0;
    float u = high0 ? acrossSpan.y + margin : acrossSpan.x - margin;
    float v = high1 ? alongSpan.y + margin : alongSpan.x - margin;
    v_edgeDistance = vec4(u - acrossSpan.x, acrossSpan.y - u, v - alongSpan.x, alongSpan.y - v);
    v_antialiasing = ubuf.antialiasing;

    v_color = index == int(ubuf.hoveredIndex) ? ubuf.hoverColor : ubuf.color;
    v_color.a *= ubuf.opacity;
    vec2 pixel = horizontal ? vec2(v, u) : vec2(u, v);
    gl_Position = ubuf.matrix * vec4(pixel, 0.0, 1.0);
}
