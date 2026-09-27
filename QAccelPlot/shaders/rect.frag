//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec4 v_edgeDistance;
layout(location = 2) in vec4 v_borderColor;
layout(location = 3) in float v_borderWidth;

layout(location = 0) out vec4 fragColor;

void main() {
    float edgeDistance = min(min(v_edgeDistance.x, v_edgeDistance.y), min(v_edgeDistance.z, v_edgeDistance.w));
    vec4 color = edgeDistance < v_borderWidth ? v_borderColor : v_color;
    // Premultiplied alpha output
    fragColor = vec4(color.rgb * color.a, color.a);
}
