//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440

layout(location = 0) in vec4 v_color;

layout(location = 0) out vec4 fragColor;

void main() {
    // Premultiplied alpha output
    fragColor = vec4(v_color.rgb * v_color.a, v_color.a);
}
