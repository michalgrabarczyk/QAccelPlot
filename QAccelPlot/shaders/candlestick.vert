//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#version 440
#extension GL_GOOGLE_include_directive : require

// Each candle takes 18 vertices: a quad of two triangles for each of three parts. A candlestick
// draws its wick above and below the body, then its body; an OHLC bar draws its line, its open
// tick, and its close tick. Parts don't overlap, so a translucent candle blends each pixel once.
// The candle, part, and corner come from gl_VertexIndex. The scene
// graph requires at least one vertex attribute, so the vertex buffer holds this placeholder; its
// content is ignored.
layout(location = 0) in vec4 vertexPadding;

// Must match the inputs of error_bar.frag, which draws the parts.
layout(location = 0) out vec4 v_color;
// Pixel distances to the left, right, top, and bottom edges of the part; negative in the
// antialiasing margin outside it.
layout(location = 1) out vec4 v_edgeDistance;
layout(location = 2) flat out float v_antialiasing;
// Glow halo color and (radius, intensity, opacity), see glow.glsl; a zero radius draws none.
// The opacity scales the candle and its halo together.
layout(location = 3) flat out vec4 v_glowColor;
layout(location = 4) flat out vec3 v_glowParams;

// Must match CandlestickUbo in CandlestickMaterial.cpp.
layout(std140, binding = 0) uniform buf {
    mat4 matrix;          //   0-63
    vec4 color;           //  64-79: up candle color
    vec2 domainMin;       //  80-87
    vec2 domainMax;       //  88-95
    vec2 viewportSize;    //  96-103
    float logScaleX;      // 104-107
    float logScaleY;      // 108-111
    float useVertexColor; // 112-115: unused
    float opacity;        // 116-119: inherited item opacity
    float hoveredIndex;   // 120-123: -1 when no candle is highlighted
    float lineWidth;      // 124-127: pixels
    vec4 hoverColor;      // 128-143
    vec4 downColor;       // 144-159
    float halfWidth;      // 160-163: x data units
    float minimumWidth;   // 164-167: pixels
    float ohlc;           // 168-171: 1 to draw OHLC bars
    float candleCount;    // 172-175
    float antialiasing;   // 176-179: 1 to smooth the edges
    vec4 itemFadeGroups[28]; // item opacity, see item_opacity.glsl
    vec4 itemFadeTrail;
    vec4 itemFadeTrailEasing[4];
    vec4 glowColor;         // Glow effect of up candles, see glow.glsl
    vec4 glowParams;
    vec4 glowDownColor;     // Glow color of down candles
} ubuf;

layout(binding = 1) uniform sampler2D dataSampler;

#include "data_texture.glsl"
#include "math_utils.glsl"
#include "axis_pixel.glsl"
#include "item_opacity.glsl"
#include "glow.glsl"

const int kVerticesPerCandle = 18;
const int kValuesPerCandle = 5;
// Antialiasing margin around each quad, in pixels.
const float kFeather = 1.0;
// Added to the distance to an edge that joins another part of the candle, so the edge is drawn
// sharp and without a glow halo, and the joined parts leave no seam.
const float kJoinedEdgeDistance = 1.0e4;

void cull() {
    v_color = vec4(0.0);
    v_edgeDistance = vec4(0.0);
    v_antialiasing = 0.0;
    v_glowColor = vec4(0.0);
    v_glowParams = vec3(0.0);
    gl_Position = kCulledClipPosition;
}

void main() {
    // Reading the placeholder keeps it in the shader interface on every backend. UNorm values are
    // finite, so the product is always zero.
    int vertexIndex = gl_VertexIndex + int(vertexPadding.x * 0.0);
    int index = vertexIndex / kVerticesPerCandle;
    int part = (vertexIndex / 6) % 3;
    int corner = vertexIndex % 6;
    bool ohlc = ubuf.ohlc > 0.5;
    // Every part but a candlestick's body is a line.
    bool body = !ohlc && part == 2;
    if (float(index) >= ubuf.candleCount || (!body && ubuf.lineWidth <= 0.0)) {
        cull();
        return;
    }

    int base = index * kValuesPerCandle;
    uint xBits = fetchFloatBits(base);
    uint openBits = fetchFloatBits(base + 1);
    uint highBits = fetchFloatBits(base + 2);
    uint lowBits = fetchFloatBits(base + 3);
    uint closeBits = fetchFloatBits(base + 4);
    float x = uintBitsToFloat(xBits);
    if (!isFiniteBits(xBits) || isNaNBits(openBits) || isNaNBits(closeBits) || (ubuf.logScaleX > 0.5 && x <= 0.0)) {
        cull();
        return;
    }

    bool up = uintBitsToFloat(closeBits) >= uintBitsToFloat(openBits);
    // A NaN high or low leaves out that half of the wick, which then ends at the body.
    uint bodyTopBits = up ? closeBits : openBits;
    uint bodyBottomBits = up ? openBits : closeBits;
    uint wickHighBits = isNaNBits(highBits) ? bodyTopBits : highBits;
    uint wickLowBits = isNaNBits(lowBits) ? bodyBottomBits : lowBits;

    AxisMapping mapping = axisMapping(ubuf.domainMin, ubuf.domainMax, ubuf.viewportSize, bvec2(ubuf.logScaleX > 0.5, ubuf.logScaleY > 0.5));
    float center = axisPixel(mapping, xBits, 0);
    // A left edge outside a log axis' domain moves to x, as in CandlestickSeries' candleSpan().
    float left = x - ubuf.halfWidth;
    if (ubuf.logScaleX > 0.5 && left <= 0.0) {
        left = x;
    }
    vec2 candleSpan = widenedSpan(axisPixel(mapping, floatBitsToUint(left), 0), axisPixel(mapping, floatBitsToUint(x + ubuf.halfWidth), 0), ubuf.minimumWidth);
    float openPixel = axisPixel(mapping, openBits, 1);
    float closePixel = axisPixel(mapping, closeBits, 1);
    float halfLine = 0.5 * ubuf.lineWidth;

    // The part as spans along x and y, in pixels, and which of its left, right, top, and bottom
    // edges join another part.
    vec2 xSpan = vec2(center - halfLine, center + halfLine);
    vec2 ySpan;
    bvec4 joined = bvec4(false);
    vec2 bodySpan = widenedSpan(openPixel, closePixel, ubuf.lineWidth);
    float wickHigh = axisPixel(mapping, wickHighBits, 1);
    float wickLow = axisPixel(mapping, wickLowBits, 1);
    vec2 wickSpan = vec2(min(wickHigh, wickLow), max(wickHigh, wickLow));
    if (body) {
        // At least as wide as the wick it replaces, so a narrow candle still shows the wick.
        xSpan = vec2(min(candleSpan.x, xSpan.x), max(candleSpan.y, xSpan.y));
        ySpan = bodySpan;
    } else if (!ohlc) {
        // The wick on each side of the body, up to its edge.
        ySpan = part == 0 ? vec2(wickSpan.x, min(wickSpan.y, bodySpan.x)) : vec2(max(wickSpan.x, bodySpan.y), wickSpan.y);
        joined = bvec4(false, false, part == 1 && ySpan.x == bodySpan.y, part == 0 && ySpan.y == bodySpan.x);
    } else if (part == 0) {
        ySpan = wickSpan;
    } else {
        // The ticks end at the line.
        bool openTick = part == 1;
        xSpan = openTick ? vec2(candleSpan.x, center - halfLine) : vec2(center + halfLine, candleSpan.y);
        float tick = openTick ? openPixel : closePixel;
        ySpan = vec2(tick - halfLine, tick + halfLine);
        joined = bvec4(!openTick, openTick, false, false);
    }
    if (xSpan.y <= xSpan.x || ySpan.y <= ySpan.x) {
        cull();
        return;
    }

    // Corners of two triangles: (0,0), (1,0), (0,1) and (1,0), (1,1), (0,1).
    bool high0 = corner == 1 || corner == 3 || corner == 4;
    bool high1 = corner == 2 || corner == 4 || corner == 5;
    float margin = (ubuf.antialiasing > 0.5 ? kFeather : 0.0) + (glowActive(ubuf.glowParams.xy) ? ubuf.glowParams.x : 0.0);
    vec4 margins = margin * (1.0 - vec4(joined));
    float u = high0 ? xSpan.y + margins.y : xSpan.x - margins.x;
    float v = high1 ? ySpan.y + margins.w : ySpan.x - margins.z;
    v_edgeDistance = vec4(u - xSpan.x, xSpan.y - u, v - ySpan.x, ySpan.y - v) + vec4(joined) * kJoinedEdgeDistance;
    v_antialiasing = ubuf.antialiasing;

    vec4 candleColor = up ? ubuf.color : ubuf.downColor;
    v_color = index == int(ubuf.hoveredIndex) ? ubuf.hoverColor : candleColor;
    v_glowColor = up ? ubuf.glowColor : ubuf.glowDownColor;
    // The candle and its halo fade together.
    v_glowParams = vec3(glowActive(ubuf.glowParams.xy) ? ubuf.glowParams.xy : vec2(0.0), ubuf.opacity * itemOpacity(float(index)));
    gl_Position = ubuf.matrix * vec4(u, v, 0.0, 1.0);
}
