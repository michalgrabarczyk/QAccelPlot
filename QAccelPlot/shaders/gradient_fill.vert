//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440

layout(location = 0) in vec2 position;
layout(location = 1) in float gradientCoordinate;

layout(location = 0) out float v_gradientCoordinate;

layout(std140, binding = 0) uniform buf {
    mat4 matrix;
    vec4 parameters;
} ubuf;

void main()
{
    v_gradientCoordinate = gradientCoordinate;
    gl_Position = ubuf.matrix * vec4(position, 0.0, 1.0);
}
