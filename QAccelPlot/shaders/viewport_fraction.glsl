//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Requires: an include of math_utils.glsl.

// Returns an edge's position as a fraction of the viewport from dMin (0) to dMax (1),
// clamped to one viewport beyond each side. Infinite edges map straight onto that margin,
// so they reach past the plot edge even when dMin and dMax are too far from the render
// origin to differ in single precision. dMin and dMax are in log10 on a log scale.
float viewportFraction(uint bits, float dMin, float dMax, bool logScale) {
    if (!isFiniteBits(bits)) {
        bool towardMin = ((bits & 0x80000000u) != 0u) == (dMax >= dMin);
        return towardMin ? -1.0 : 2.0;
    }
    float value = uintBitsToFloat(bits);
    if (logScale) {
        value = safeLog10(value);
    }
    return clamp((value - dMin) / (dMax - dMin), -1.0, 2.0);
}
