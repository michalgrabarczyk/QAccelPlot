//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
// The halo of a Glow effect, drawn in the same pass as the shape it surrounds.
// `color` is the halo color, `params` holds its radius in pixels and its intensity,
// matching QAccelPlot::Internal::GlowUniforms; a zero radius draws no halo.

bool glowActive(vec2 params) {
    return params.x > 0.0 && params.y > 0.0;
}

// Returns `shape`, a premultiplied color, over the halo at `outsideDistance` pixels outside the
// shape's edge. `opacity` scales the halo with the shape, for example by the item opacity.
vec4 withGlow(vec4 shape, float outsideDistance, vec4 color, vec2 params, float opacity) {
    if (!glowActive(params)) {
        return shape;
    }
    float falloff = 1.0 - clamp(max(outsideDistance, 0.0) / params.x, 0.0, 1.0);
    float alpha = color.a * params.y * falloff * falloff * opacity;
    return shape + vec4(color.rgb * alpha, alpha) * (1.0 - shape.a);
}
