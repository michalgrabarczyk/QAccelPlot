//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in float id;
layout(location = 1) in float edge; // 0.0: lower edge, 1.0: upper edge

layout(location = 0) out vec4 v_color;
layout(location = 1) out float v_validWeight;

// Only this stage declares ubuf, so both stages never disagree on a trimmed block (see rect.vert).
// Must match BandUbo in BandMaterial.cpp.
layout(std140, binding = 0) uniform buf {
    mat4 matrix;          //   0-63
    vec4 color;           //  64-79
    vec2 domainMin;       //  80-87
    vec2 domainMax;       //  88-95
    vec2 viewportSize;    //  96-103
    float logScaleX;      // 104-107
    float logScaleY;      // 108-111
    float sampleCount;    // 112-115
    float opacity;        // 116-119: inherited item opacity
} ubuf;

layout(binding = 1) uniform sampler2D dataSampler;

#include "data_texture.glsl"
#include "math_utils.glsl"
#include "data_mapping.glsl"

// Fetches sample `index` as (x, low, high) and returns whether all three values are valid:
// finite and strictly positive on log-scale dimensions.
bool fetchBandSample(int index, out vec3 bandSample) {
    int base = index * 3;
    uint xBits = fetchFloatBits(base);
    uint lowBits = fetchFloatBits(base + 1);
    uint highBits = fetchFloatBits(base + 2);
    bandSample = vec3(uintBitsToFloat(xBits), uintBitsToFloat(lowBits), uintBitsToFloat(highBits));
    if (!isFiniteBits(xBits) || !isFiniteBits(lowBits) || !isFiniteBits(highBits)) {
        return false;
    }
    return (ubuf.logScaleX < 0.5 || bandSample.x > 0.0) && (ubuf.logScaleY < 0.5 || (bandSample.y > 0.0 && bandSample.z > 0.0));
}

// Pixel position of this vertex's edge. Each sample draws min(low, high) on the lower edge, so
// swapped or crossing values never fold the strip over itself and blend twice.
vec2 edgeLocal(vec3 bandSample) {
    float y = edge < 0.5 ? min(bandSample.y, bandSample.z) : max(bandSample.y, bandSample.z);
    return dataToLocal(vec2(bandSample.x, y));
}

void main() {
    v_color = vec4(ubuf.color.rgb, ubuf.color.a * ubuf.opacity);

    int index = int(id);
    vec3 bandSample;
    if (fetchBandSample(index, bandSample)) {
        v_validWeight = 1.0;
        gl_Position = ubuf.matrix * vec4(edgeLocal(bandSample), 0.0, 1.0);
        return;
    }

    // An invalid sample collapses onto the same edge of an adjacent valid sample, so the
    // triangles reaching into the gap have no area and every fragment near it is discarded.
    v_validWeight = kInvalidVertexWeight;
    vec3 neighbor;
    if (index > 0 && fetchBandSample(index - 1, neighbor)) {
        gl_Position = ubuf.matrix * vec4(edgeLocal(neighbor), 0.0, 1.0);
    } else if (index + 1 < int(ubuf.sampleCount) && fetchBandSample(index + 1, neighbor)) {
        gl_Position = ubuf.matrix * vec4(edgeLocal(neighbor), 0.0, 1.0);
    } else {
        gl_Position = kCulledClipPosition;
    }
}
