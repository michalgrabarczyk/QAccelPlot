//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#version 440
#extension GL_GOOGLE_include_directive : require

// The dashed upper edge line of a BandSeries: the larger of low and high at each (x, low, high) data point.
#define LINE_SAMPLE_STRIDE 3
#define LINE_Y_BITS(base) bandEdgeBits(base)
#define LINE_DASHED 1
#define BAND_EDGE_UPPER 1

uint bandEdgeBits(int base);

#include "line_vertex.glsl"
#include "band_edge.glsl"
