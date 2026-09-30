//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Drawn Y of a band edge line, included after line_vertex.glsl by the band edge shaders.
// Requires: BAND_EDGE_UPPER defined as 0 (lower edge) or 1 (upper edge).

// Returns the bits of the smaller (lower edge) or larger (upper edge) of low and high for the
// (x, low, high) sample starting at float `base`, matching the band fill. Returns NaN when either
// value is invalid, so the edge line breaks where the fill does.
uint bandEdgeBits(int base) {
    uint lowBits = fetchFloatBits(base + 1);
    uint highBits = fetchFloatBits(base + 2);
    float low = uintBitsToFloat(lowBits);
    float high = uintBitsToFloat(highBits);
    bool invalid = !isFiniteBits(lowBits) || !isFiniteBits(highBits) || (ubuf.logScaleY > 0.5 && (low <= 0.0 || high <= 0.0));
    if (invalid) {
        return 0x7FC00000u;
    }
    return floatBitsToUint(BAND_EDGE_UPPER != 0 ? max(low, high) : min(low, high));
}
