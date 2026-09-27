//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#version 440
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec4 v_edgeDistance;

layout(location = 0) out vec4 fragColor;

#include "rect_uniforms.glsl"

void main() {
    vec4 color = v_color;
    float edgeDistance = min(min(v_edgeDistance.x, v_edgeDistance.y), min(v_edgeDistance.z, v_edgeDistance.w));
    if (edgeDistance < ubuf.borderWidth) {
        color = vec4(ubuf.borderColor.rgb, ubuf.borderColor.a * ubuf.opacity);
    }
    // Premultiplied alpha output
    fragColor = vec4(color.rgb * color.a, color.a);
}
