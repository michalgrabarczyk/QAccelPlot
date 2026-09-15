//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Smallest positive normal float. Lower values may be treated as zero by GPUs
// that don't support denormals, which would make log() return -inf.
const float kMinPositiveNormalFloat = 1.175494e-38;

// Guards against log(0) by clamping to the smallest positive normal float.
float safeLog10(float val) {
    return log(max(val, kMinPositiveNormalFloat)) / log(10.0);
}

// Returns true when the IEEE 754 bit pattern encodes a finite value.
// NaN and +/-Inf have every exponent bit set. Testing the integer bits keeps
// the check reliable when shader compilers assume floats are never NaN or Inf
// (fast-math), which is allowed to fold isnan()/isinf() to false.
bool isFiniteBits(uint bits) {
    return (bits & 0x7F800000u) != 0x7F800000u;
}

bool floatIsFinite(float value) {
    return isFiniteBits(floatBitsToUint(value));
}

// Clip-space position outside the view volume for geometry that must not be drawn.
// Placing every vertex of a primitive here produces a zero-area, clipped primitive.
const vec4 kCulledClipPosition = vec4(-2.0, -2.0, 0.0, 1.0);
