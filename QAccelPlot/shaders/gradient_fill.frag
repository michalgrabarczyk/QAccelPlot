//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#version 440
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in float v_gradientCoordinate;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    vec4 parameters;
    vec4 logGradient; // x: the coordinate is log10(value / reference); y: scale; z: offset
} ubuf;

layout(binding = 1) uniform sampler2D gradientSampler;

#include "gradient_log.glsl"

void main()
{
    float coordinate = ubuf.logGradient.x > 0.5 ? logGradientCoordinate(v_gradientCoordinate, ubuf.logGradient.y, ubuf.logGradient.z) : v_gradientCoordinate;
    vec4 color = texture(gradientSampler, vec2(clamp(coordinate, 0.0, 1.0), 0.5));
    // parameters.x: fill opacity; parameters.y: inherited item opacity.
    float alpha = color.a * ubuf.parameters.x * ubuf.parameters.y;
    fragColor = vec4(color.rgb * alpha, alpha);
}
