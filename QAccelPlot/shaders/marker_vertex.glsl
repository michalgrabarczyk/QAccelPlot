//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Vertex stage of the marker quads shared by point.vert and point_cloud.vert. Each
// point is two unindexed triangles that read the point from the data texture.
// Requires dataSampler, data_texture.glsl, math_utils.glsl, point_shapes.glsl, and a
// ubuf with matrix, domainMin, domainMax, viewportSize, logScaleX, logScaleY,
// markerSize, shapeType, antialiasingEnabled, and antialiasingFeather.

// The point index and quad corner come from gl_VertexIndex. The scene graph requires at least
// one vertex attribute, so the vertex buffer holds this placeholder; its content is ignored. It is
// stored as 4 unsigned bytes (UNormByte4, 4 bytes per vertex) and read as a normalized vec4.
layout(location = 0) in vec4 vertexPadding;

struct MarkerVertex {
    int base;          // data texture index of the point's X; Y follows it
    bool valid;        // false for invalid samples, which are not drawn
    vec2 dataPosition; // untransformed data-space position of the point
    vec2 uv;           // +-1 is markerSize; the quad extends further with antialiasing
    vec4 position;
};

vec2 markerCornerOffset(int corner) {
    if (corner == 0) return vec2(-1.0, -1.0);
    if (corner == 1) return vec2( 1.0, -1.0);
    if (corner == 2) return vec2( 1.0,  1.0);
    if (corner == 3) return vec2(-1.0, -1.0);
    if (corner == 4) return vec2( 1.0,  1.0);
    return vec2(-1.0, 1.0); // corner == 5
}

// Returns this vertex of the marker quad of a point stored as `floatsPerPoint` floats,
// X and Y first. Invalid samples (non-finite, or not strictly positive on a log-scale
// dimension) collapse onto one position outside clip space, so the quad has zero area.
MarkerVertex computeMarkerVertex(int floatsPerPoint) {
    // Reading the placeholder keeps it in the shader interface on every backend. UNorm values are
    // finite, so the product is always zero.
    int vertexIndex = gl_VertexIndex + int(vertexPadding.x * 0.0);

    MarkerVertex result;
    result.base = (vertexIndex / 6) * floatsPerPoint;
    result.valid = false;
    result.dataPosition = vec2(0.0);
    result.uv = vec2(0.0);
    result.position = kCulledClipPosition;

    uint xBits = fetchFloatBits(result.base);
    uint yBits = fetchFloatBits(result.base + 1);
    if (!isFiniteBits(xBits) || !isFiniteBits(yBits)) {
        return result;
    }
    vec2 p = vec2(uintBitsToFloat(xBits), uintBitsToFloat(yBits));
    if ((ubuf.logScaleX > 0.5 && p.x <= 0.0) || (ubuf.logScaleY > 0.5 && p.y <= 0.0)) {
        return result;
    }
    result.valid = true;
    result.dataPosition = p;

    vec2 dMin = ubuf.domainMin;
    vec2 dMax = ubuf.domainMax;
    if (ubuf.logScaleX > 0.5) {
        dMin.x = safeLog10(dMin.x);
        dMax.x = safeLog10(dMax.x);
        p.x = safeLog10(p.x);
    }
    if (ubuf.logScaleY > 0.5) {
        dMin.y = safeLog10(dMin.y);
        dMax.y = safeLog10(dMax.y);
        p.y = safeLog10(p.y);
    }

    vec2 range = dMax - dMin;
    vec2 pLocal = vec2(
        (p.x - dMin.x) / range.x * ubuf.viewportSize.x,
        (1.0 - (p.y - dMin.y) / range.y) * ubuf.viewportSize.y
    );

    float halfExtent = ubuf.markerSize;
    if (ubuf.shapeType == kPixel) {
        // A Pixel marker covers exactly one pixel: snap the centre to the pixel grid
        // and ignore markerSize and antialiasing.
        pLocal = floor(pLocal) + 0.5;
        halfExtent = 0.5;
    } else if (ubuf.antialiasingEnabled > 0.5 && ubuf.antialiasingFeather > 0.0) {
        // With antialiasing, the quad grows by half the feather so the coverage ramp
        // centred on the shape edge is not clipped by the geometry.
        halfExtent += ubuf.antialiasingFeather * 0.5;
    }

    vec2 offset = markerCornerOffset(vertexIndex % 6);
    result.uv = offset * (halfExtent / max(ubuf.markerSize, 1e-4));
    // Expand the point centre into a billboard quad in item-local pixel space.
    result.position = ubuf.matrix * vec4(pLocal + offset * halfExtent, 0.0, 1.0);
    return result;
}
