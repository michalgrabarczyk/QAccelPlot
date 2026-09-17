//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Decode one IEEE 754 float from an RGBA8888 texel.
// QImage::Format_RGBA8888 stores [R=b0, G=b1, B=b2, A=b3] in memory.
// GL_RGBA8 (non-sRGB) uploads these bytes verbatim; texelFetch returns them
// normalised to [0,1]. Multiply by 255 and round to recover the original bytes.
// Requires: sampler2D dataSampler declared before this include.
uint fetchFloatBits(int floatIndex) {
    int w = 2048; // Must match texWidth in DataTextureMaterial.cpp
    vec4 texel = texelFetch(dataSampler, ivec2(floatIndex % w, floatIndex / w), 0);
    uint b0 = uint(round(texel.r * 255.0));
    uint b1 = uint(round(texel.g * 255.0));
    uint b2 = uint(round(texel.b * 255.0));
    uint b3 = uint(round(texel.a * 255.0));
    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

float fetchFloat(int floatIndex) {
    return uintBitsToFloat(fetchFloatBits(floatIndex));
}
