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
layout(location = 1) in vec2 v_position;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4  matrix;
    vec2  cellScale;
    vec2  cellOffset;
    vec2  cellRate;
    vec2  cellShift;
    float valueMin;
    float valueMax;
    float valueLogScale;
    float opacity;
    int   columns;
    int   rows;
    int   columnMapping;
    int   rowMapping;
} ubuf;

layout(binding = 1) uniform sampler2D dataSampler;
layout(binding = 2) uniform sampler2D colorMapSampler;

#include "data_texture.glsl"
#include "math_utils.glsl"

// Cell mappings of Internal::HeatmapShaderMapping::Mode.
const int kExponentialMapping = 1;
const int kLogarithmicMapping = 2;

// exp(x) - 1, accurate near 0 where exp(x) - 1.0 cancels.
float expMinusOne(float x) {
    if (abs(x) < 0.5) {
        return x * (1.0 + x / 2.0 * (1.0 + x / 3.0 * (1.0 + x / 4.0 * (1.0 + x / 5.0 * (1.0 + x / 6.0 * (1.0 + x / 7.0 * (1.0 + x / 8.0)))))));
    }
    return exp(x) - 1.0;
}

// log(1 + x), accurate near 0 where log(1.0 + x) cancels, through log(1 + x) = 2 atanh(x / (2 + x)).
float logOnePlus(float x) {
    if (abs(x) < 0.25) {
        float z = x / (2.0 + x);
        float z2 = z * z;
        return 2.0 * z * (1.0 + z2 * (1.0 / 3.0 + z2 * (1.0 / 5.0 + z2 * (1.0 / 7.0 + z2 / 9.0))));
    }
    return log(1.0 + x);
}

// Returns the fractional cell coordinate along one dimension at item-local pixel position p.
float cellCoordinate(int mapping, float affineCell, float p, float scale, float offset, float rate, float shift) {
    if (mapping == kExponentialMapping) {
        return offset + scale * expMinusOne(rate * p);
    }
    if (mapping == kLogarithmicMapping) {
        float ratio = shift + rate * p;
        // A value at or below zero lies in no log-spaced cell.
        return ratio > -1.0 ? offset + scale * logOnePlus(ratio) : -1.0;
    }
    return affineCell;
}

void main()
{
    vec2 fractionalCell = vec2(
        cellCoordinate(ubuf.columnMapping, v_cell.x, v_position.x, ubuf.cellScale.x, ubuf.cellOffset.x, ubuf.cellRate.x, ubuf.cellShift.x),
        cellCoordinate(ubuf.rowMapping, v_cell.y, v_position.y, ubuf.cellScale.y, ubuf.cellOffset.y, ubuf.cellRate.y, ubuf.cellShift.y));
    if (!floatIsFinite(fractionalCell.x) || !floatIsFinite(fractionalCell.y)) {
        discard;
    }
    vec2 cell = floor(fractionalCell);
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
