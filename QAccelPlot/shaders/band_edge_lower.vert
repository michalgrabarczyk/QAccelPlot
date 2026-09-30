//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440
#extension GL_GOOGLE_include_directive : require

// The lower edge line of a BandSeries: the smaller of low and high at each (x, low, high) sample.
#define LINE_SAMPLE_STRIDE 3
#define LINE_Y_BITS(base) bandEdgeBits(base)
#define BAND_EDGE_UPPER 0

uint bandEdgeBits(int base);

#include "line_vertex.glsl"
#include "band_edge.glsl"
