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
layout(location = 2) flat in float v_antialiasing;
layout(location = 3) flat in vec4 v_glowColor;
layout(location = 4) flat in vec3 v_glowParams;

layout(location = 0) out vec4 fragColor;

#include "glow.glsl"

void main() {
    // Distance outside the part, rounded at its corners; the quad extends there for a glow halo.
    vec2 outside = max(-vec2(min(v_edgeDistance.x, v_edgeDistance.y), min(v_edgeDistance.z, v_edgeDistance.w)), 0.0);
    float outsideDistance = length(outside);
    float alpha = v_color.a;
    if (v_antialiasing < 0.5 && outsideDistance > 0.0) {
        alpha = 0.0;
    } else if (v_antialiasing > 0.5) {
        // Pixel coverage of the part, from the distances to its edges.
        float across = clamp(min(v_edgeDistance.x, v_edgeDistance.y) + 0.5, 0.0, 1.0);
        float along = clamp(min(v_edgeDistance.z, v_edgeDistance.w) + 0.5, 0.0, 1.0);
        alpha *= across * along;
    }
    // Premultiplied alpha output
    fragColor = withGlow(vec4(v_color.rgb * alpha, alpha), outsideDistance, v_glowColor, v_glowParams.xy, v_glowParams.z);
}
