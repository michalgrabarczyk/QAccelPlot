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

layout(location = 0) out vec2 v_cell;     // fractional cell coordinate (column, row) of the affine mappings
layout(location = 1) out vec2 v_position; // item-local pixel position, for the non-affine mappings

layout(std140, binding = 0) uniform buf {
    mat4  matrix;
    vec2  cellScale;
    vec2  cellOffset;
    vec2  cellRate;
    vec2  cellShift;
    float valueMin;
    float valueMax;
    float valueLogScale;
    float opacity;
    int   columns;
    int   rows;
    int   columnMapping;
    int   rowMapping;
} ubuf;

void main()
{
    // An affine mapping interpolates exactly per fragment.
    v_cell = position * ubuf.cellScale + ubuf.cellOffset;
    v_position = position;
    gl_Position = ubuf.matrix * vec4(position, 0.0, 1.0);
}
