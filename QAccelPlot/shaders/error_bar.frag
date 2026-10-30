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
layout(location = 2) flat in float v_antialiasing;

layout(location = 0) out vec4 fragColor;

void main() {
    float alpha = v_color.a;
    if (v_antialiasing > 0.5) {
        // Pixel coverage of the part, from the distances to its edges.
        float across = clamp(min(v_edgeDistance.x, v_edgeDistance.y) + 0.5, 0.0, 1.0);
        float along = clamp(min(v_edgeDistance.z, v_edgeDistance.w) + 0.5, 0.0, 1.0);
        alpha *= across * along;
    }
    // Premultiplied alpha output
    fragColor = vec4(v_color.rgb * alpha, alpha);
}
