//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 v_cell;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4  matrix;
    vec2  cellScale;
    vec2  cellOffset;
    float valueMin;
    float valueMax;
    float valueLogScale;
    float opacity;
    int   columns;
    int   rows;
} ubuf;

layout(binding = 1) uniform sampler2D dataSampler;
layout(binding = 2) uniform sampler2D colorMapSampler;

#include "data_texture.glsl"
#include "math_utils.glsl"

void main()
{
    vec2 cell = floor(v_cell);
    if (cell.x < 0.0 || cell.y < 0.0 || cell.x >= float(ubuf.columns) || cell.y >= float(ubuf.rows)) {
        discard;
    }

    uint valueBits = fetchFloatBits(int(cell.y) * ubuf.columns + int(cell.x));
    float value = uintBitsToFloat(valueBits);
    float lo = ubuf.valueMin;
    float hi = ubuf.valueMax;
    bool mappable = isFiniteBits(valueBits);
    if (ubuf.valueLogScale > 0.5) {
        // Log normalization is undefined at or below zero; those cells stay transparent.
        mappable = mappable && value > 0.0 && lo > 0.0 && hi > 0.0;
        value = safeLog10(value);
        lo = safeLog10(lo);
        hi = safeLog10(hi);
    }
    if (!mappable) {
        discard;
    }

    float valueRange = hi - lo;
    float t = abs(valueRange) > 0.0 ? clamp((value - lo) / valueRange, 0.0, 1.0) : 0.0;
    // An explicit level: implicit derivatives are undefined after the discards above.
    vec4 color = textureLod(colorMapSampler, vec2(t, 0.5), 0.0);
    float alpha = color.a * ubuf.opacity;
    fragColor = vec4(color.rgb * alpha, alpha);
}
