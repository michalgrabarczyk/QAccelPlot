//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Shared ribbon geometry for line vertex shaders.
// Requires: a `ubuf` uniform block with matrix, domainMin, domainMax,
// viewportSize, lineWidth, logScaleX, logScaleY and pointCount members,
// followed by includes of data_texture.glsl and math_utils.glsl.

// Validity weight written to both vertices of an invalid sample. Valid vertices
// write 1.0. The fragment shader discards fragments whose interpolated weight is
// negative, which removes every triangle bridging a gap except for a sliver of
// 1e-6 of the bridged distance next to the valid vertex.
const float kInvalidVertexWeight = -1.0e6;

struct LineVertexResult {
    vec4 position;     // clip-space position
    vec2 dataPosition; // untransformed data-space position of the sample
    float validWeight; // 1.0 for valid samples, kInvalidVertexWeight otherwise
};

// Fetches sample `index` (clamped to the data range) and returns whether it is
// valid: both coordinates finite and strictly positive on log-scale dimensions.
// Log-scale dimensions are never origin-shifted, so the sign test is exact.
bool fetchSample(int index, out vec2 position) {
    index = clamp(index, 0, int(ubuf.pointCount) - 1);
    int base = index * 2; // X float at 2*index, Y float at 2*index+1
    uint xBits = fetchFloatBits(base);
    uint yBits = fetchFloatBits(base + 1);
    position = vec2(uintBitsToFloat(xBits), uintBitsToFloat(yBits));
    if (!isFiniteBits(xBits) || !isFiniteBits(yBits)) {
        return false;
    }
    return (ubuf.logScaleX < 0.5 || position.x > 0.0) && (ubuf.logScaleY < 0.5 || position.y > 0.0);
}

// Maps a valid data-space position to item-local pixel coordinates.
// Y is flipped: in Qt, y=0 is top, but in domain y increases upward.
vec2 dataToLocal(vec2 value) {
    vec2 dMin = ubuf.domainMin;
    vec2 dMax = ubuf.domainMax;

    if (ubuf.logScaleX > 0.5) {
        dMin.x = safeLog10(dMin.x);
        dMax.x = safeLog10(dMax.x);
        value.x = safeLog10(value.x);
    }

    if (ubuf.logScaleY > 0.5) {
        dMin.y = safeLog10(dMin.y);
        dMax.y = safeLog10(dMax.y);
        value.y = safeLog10(value.y);
    }

    vec2 range = dMax - dMin;
    return vec2(
        (value.x - dMin.x) / range.x * ubuf.viewportSize.x,
        (1.0 - (value.y - dMin.y) / range.y) * ubuf.viewportSize.y
    );
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
    vec2 p;
    vec2 pr;
    vec2 nx;
    bool pValid = fetchSample(index, p);
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
    vec2 offset = normal * (ubuf.lineWidth * 0.5) * miterLength * side;

    LineVertexResult result;
    result.position = ubuf.matrix * vec4(p_local + offset, 0.0, 1.0);
    result.dataPosition = p;
    result.validWeight = 1.0;
    return result;
}
