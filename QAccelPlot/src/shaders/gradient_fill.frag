//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440

layout(location = 0) in float v_gradientCoordinate;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    vec4 parameters;
} ubuf;

layout(binding = 1) uniform sampler2D gradientSampler;

void main()
{
    vec4 color = texture(gradientSampler, vec2(clamp(v_gradientCoordinate, 0.0, 1.0), 0.5));
    float alpha = color.a * ubuf.parameters.x;
    fragColor = vec4(color.rgb * alpha, alpha);
}
