//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440
#extension GL_GOOGLE_include_directive : require

// The upper edge line of a BandSeries, reading (x, low, high) samples.
#define LINE_SAMPLE_STRIDE 3
#define LINE_Y_COMPONENT 2

#include "line_vertex.glsl"
