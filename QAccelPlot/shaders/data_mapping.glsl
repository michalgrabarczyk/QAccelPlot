//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Data-to-pixel mapping shared by curve vertex shaders.
// Requires: a `ubuf` uniform block with domainMin, domainMax, viewportSize,
// logScaleX and logScaleY members, followed by an include of math_utils.glsl.

// Validity weight written to both vertices of an invalid sample. Valid vertices
// write 1.0. The fragment shader discards fragments whose interpolated weight is
// negative, which removes every triangle bridging a gap except for a sliver of
// 1e-6 of the bridged distance next to the valid vertex.
const float kInvalidVertexWeight = -1.0e6;

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
