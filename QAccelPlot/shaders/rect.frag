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
layout(location = 4) flat in float v_antialiasing;

layout(location = 0) out vec4 fragColor;

void main() {
    // Opposite edge distances add up to the rectangle's size. The outline shrinks on a narrow
    // axis so at least one pixel of fill stays visible.
    vec2 size = vec2(v_edgeDistance.x + v_edgeDistance.y, v_edgeDistance.z + v_edgeDistance.w);
    vec2 border = min(vec2(v_borderWidth), max(0.5 * (size - 1.0), 0.0));
    // Distance to the nearest edge per axis; the antialiasing margin counts as the edge.
    vec2 distance = max(vec2(min(v_edgeDistance.x, v_edgeDistance.y), min(v_edgeDistance.z, v_edgeDistance.w)), 0.0);
    // Premultiplied alpha output
    vec4 fill = vec4(v_color.rgb * v_color.a, v_color.a);
    vec4 outline = vec4(v_borderColor.rgb * v_borderColor.a, v_borderColor.a);
    if (v_antialiasing < 0.5) {
        fragColor = any(lessThan(distance, border)) ? outline : fill;
        return;
    }

    // Box-filtered pixel shares: inside the outline band, and inside the rectangle, per axis.
    vec2 band = mix(vec2(0.0), clamp(border - distance + 0.5, 0.0, 1.0), greaterThan(border, vec2(0.0)));
    vec4 edge = clamp(v_edgeDistance + 0.5, 0.0, 1.0);
    vec2 coverage = clamp(edge.xz + edge.yw - 1.0, 0.0, 1.0);
    fragColor = mix(fill, outline, max(band.x, band.y)) * (coverage.x * coverage.y);
}
