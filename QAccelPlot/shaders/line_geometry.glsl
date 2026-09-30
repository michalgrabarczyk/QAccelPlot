//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Shared ribbon geometry for line vertex shaders.
// Requires: a `ubuf` uniform block with matrix, domainMin, domainMax,
// viewportSize, lineWidth, logScaleX, logScaleY, pointCount,
// antialiasingEnabled and antialiasingFeather members,
// followed by includes of data_texture.glsl and math_utils.glsl.
//
// Samples are read as LINE_SAMPLE_STRIDE floats each, X first. LINE_Y_BITS(base)
// returns the bits of the drawn Y for the sample starting at float `base`. Define
// both before including this file to read other layouts.

#ifndef LINE_SAMPLE_STRIDE
#define LINE_SAMPLE_STRIDE 2
#endif
#ifndef LINE_Y_BITS
#define LINE_Y_BITS(base) fetchFloatBits((base) + 1)
#endif

#include "data_mapping.glsl"

struct LineVertexResult {
    vec4 position;     // clip-space position
    vec2 dataPosition; // untransformed data-space position of the sample
    float validWeight; // 1.0 for valid samples, kInvalidVertexWeight otherwise
};

// Returns true when fragment antialiasing applies to the ribbon.
bool lineAntialiased() {
    return ubuf.antialiasingEnabled > 0.5 && ubuf.antialiasingFeather > 0.0;
}

// Half-width of the rasterized ribbon in pixels. With antialiasing, the ribbon
// is at least 1 px wide and grows by half the feather on each side, so the
// coverage ramp centred on the line edge is never clipped by the geometry.
float lineRibbonHalfExtent() {
    if (!lineAntialiased()) {
        return ubuf.lineWidth * 0.5;
    }
    return max(ubuf.lineWidth, 1.0) * 0.5 + ubuf.antialiasingFeather * 0.5;
}

// Fetches sample `index` (clamped to the data range) and returns whether it is
// valid: both coordinates finite and strictly positive on log-scale dimensions.
// Log-scale dimensions are never origin-shifted, so the sign test is exact.
bool fetchSample(int index, out vec2 position) {
    index = clamp(index, 0, int(ubuf.pointCount) - 1);
    int base = index * LINE_SAMPLE_STRIDE;
    uint xBits = fetchFloatBits(base);
    uint yBits = LINE_Y_BITS(base);
    position = vec2(uintBitsToFloat(xBits), uintBitsToFloat(yBits));
    if (!isFiniteBits(xBits) || !isFiniteBits(yBits)) {
        return false;
    }
    return (ubuf.logScaleX < 0.5 || position.x > 0.0) && (ubuf.logScaleY < 0.5 || position.y > 0.0);
}

// Places both ribbon vertices of an invalid sample on the centre of an adjacent
// valid sample, or outside the view volume inside an invalid run. This keeps
// triangles within a gap degenerate and avoids NaN clip positions.
LineVertexResult invalidLineVertex(vec2 position, bool previousValid, vec2 previous, bool nextValid, vec2 next) {
    LineVertexResult result;
    result.dataPosition = position;
    result.validWeight = kInvalidVertexWeight;
    if (previousValid) {
        result.position = ubuf.matrix * vec4(dataToLocal(previous), 0.0, 1.0);
    } else if (nextValid) {
        result.position = ubuf.matrix * vec4(dataToLocal(next), 0.0, 1.0);
    } else {
        result.position = kCulledClipPosition;
    }
    return result;
}

// Computes the miter-joined ribbon vertex on `side` (+1 or -1) of sample `index`.
// A valid sample next to an invalid neighbor is treated as a line endpoint.
LineVertexResult computeLineVertex(int index, float side) {
    vec2 p = vec2(0.0);
    vec2 pr;
    vec2 nx;
    // Vertices past the last sample, reserved for appended data, collapse onto the last sample.
    bool pValid = index < int(ubuf.pointCount) && fetchSample(index, p);
    bool prValid = fetchSample(index - 1, pr);
    bool nxValid = fetchSample(index + 1, nx);

    if (!pValid) {
        return invalidLineVertex(p, prValid, pr, nxValid, nx);
    }
    if (!prValid) pr = p;
    if (!nxValid) nx = p;

    vec2 p_local = dataToLocal(p);
    vec2 prev_local = dataToLocal(pr);
    vec2 next_local = dataToLocal(nx);

    // Direction vectors for the miter join
    vec2 dir1 = p_local - prev_local;
    vec2 dir2 = next_local - p_local;
    float len1 = length(dir1);
    float len2 = length(dir2);
    if (len1 > 0.0001) dir1 /= len1; else dir1 = vec2(0.0);
    if (len2 > 0.0001) dir2 /= len2; else dir2 = vec2(0.0);

    // Handle endpoints where prev==pos or next==pos
    if (len1 <= 0.0001) dir1 = dir2;
    if (len2 <= 0.0001) dir2 = dir1;

    // Tangent and miter normal
    vec2 tangent = dir1 + dir2;
    if (length(tangent) > 0.0001) {
        tangent = normalize(tangent);
    } else {
        tangent = vec2(1.0, 0.0); // Safe fallback to prevent NaN
    }
    vec2 normal = vec2(-tangent.y, tangent.x);

    // Miter length
    vec2 n1 = vec2(-dir1.y, dir1.x);
    float miterDot = dot(normal, n1);
    float miterLength = 1.0;
    if (abs(miterDot) > 0.15) {
        miterLength = 1.0 / abs(miterDot);
    }

    // Offset in item-local pixel space, then use Qt's matrix to project
    vec2 offset = normal * lineRibbonHalfExtent() * miterLength * side;

    LineVertexResult result;
    result.position = ubuf.matrix * vec4(p_local + offset, 0.0, 1.0);
    result.dataPosition = p;
    result.validWeight = 1.0;
    return result;
}
