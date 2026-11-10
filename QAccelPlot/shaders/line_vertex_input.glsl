//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// Vertex inputs of the line ribbon (LineVertex and DashedLineVertex in LineStroke.hpp).
// Define LINE_DASHED as 1 before including it for dashed lines, whose vertices also
// carry a screen-space arc length.

#ifndef LINE_DASHED
#define LINE_DASHED 0
#endif

// The four bytes of a packed vertex: bits 0-27 hold the data point index, bits 28-30
// the path corner, and bit 31 marks the negative ribbon side.
layout(location = 0) in vec4 packedVertex;
#if LINE_DASHED
layout(location = 1) in float arcLength;
#endif

struct LineVertexInput {
    int index;
    int corner;
    float side; // +1 or -1
    float arcLength;
};

LineVertexInput lineVertexInput() {
    uvec4 bytes = uvec4(packedVertex * 255.0 + 0.5);
    LineVertexInput vertexInput;
    vertexInput.index = int(bytes.x | (bytes.y << 8u) | (bytes.z << 16u) | ((bytes.w & 15u) << 24u));
    vertexInput.corner = int((bytes.w >> 4u) & 7u);
    vertexInput.side = (bytes.w & 128u) != 0u ? -1.0 : 1.0;
#if LINE_DASHED
    vertexInput.arcLength = arcLength;
#else
    vertexInput.arcLength = 0.0;
#endif
    return vertexInput;
}
