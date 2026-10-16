//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440

// Item-local pixel position of a corner of the series' quad.
layout(location = 0) in vec2 position;

layout(location = 0) out vec2 v_cell; // fractional cell coordinate: (column, row)

layout(std140, binding = 0) uniform buf {
    mat4  matrix;
    vec2  cellScale;
    vec2  cellOffset;
    float valueMin;
    float valueMax;
    float valueLogScale;
    float opacity;
    int   columns;
    int   rows;
} ubuf;

void main()
{
    // The mapping is affine, so interpolating it per fragment is exact.
    v_cell = position * ubuf.cellScale + ubuf.cellOffset;
    gl_Position = ubuf.matrix * vec4(position, 0.0, 1.0);
}
