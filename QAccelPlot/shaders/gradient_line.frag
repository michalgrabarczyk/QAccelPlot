//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440

layout(location = 0) in vec4 v_color;
layout(location = 1) in float v_edgeDistance; // signed distance from the line centre, in pixels
layout(location = 2) in float v_arcLength;
layout(location = 3) in float v_gradientCoordinate;
layout(location = 4) in float v_validWeight;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    vec4 color;
    vec2 domainMin;
    vec2 domainMax;
    vec2 viewportSize;
    float lineWidth;
    float logScaleX;
    float logScaleY;
    float useVertexColor;
    float pointCount;
    float antialiasingEnabled;
    float antialiasingFeather;
    float dashPeriod;
    float dashOffset;
    int dashPatternSize;
    vec4 dashPattern[2];
    vec4 gradientParameters;
    float opacity;
} ubuf;

layout(binding = 2) uniform sampler2D gradientSampler;

void main() {
    // Gap discard: the fragment belongs to a triangle bridging an invalid sample.
    if (v_validWeight < 0.0) {
        discard;
    }

    if (ubuf.dashPatternSize > 0 && ubuf.dashPeriod > 0.0) {
        float t = mod(v_arcLength + ubuf.dashOffset, ubuf.dashPeriod);
        float cursor = 0.0;
        for (int i = 0; i < ubuf.dashPatternSize; ++i) {
            float segmentLength = ubuf.dashPattern[i / 4][i % 4];
            cursor += segmentLength;
            if (t < cursor) {
                if (i % 2 == 1) { discard; }
                break;
            }
        }
    }

    // Coverage ramps linearly across `antialiasingFeather` pixels centred on the
    // line edge. Lines thinner than 1 px are drawn 1 px wide with proportionally
    // reduced alpha, so they stay visible regardless of pixel alignment.
    float alpha = 1.0;
    if (ubuf.antialiasingEnabled > 0.5 && ubuf.antialiasingFeather > 0.0) {
        float coverageWidth = max(ubuf.lineWidth, 1.0);
        float edgeDistance = coverageWidth * 0.5 - abs(v_edgeDistance);
        alpha = clamp(edgeDistance / ubuf.antialiasingFeather + 0.5, 0.0, 1.0);
        alpha *= clamp(ubuf.lineWidth, 0.0, 1.0);
    }

    vec4 gradientColor = texture(gradientSampler, vec2(clamp(v_gradientCoordinate, 0.0, 1.0), 0.5));
    float outputAlpha = gradientColor.a * alpha * ubuf.opacity;
    fragColor = vec4(gradientColor.rgb * outputAlpha, outputAlpha);
}
