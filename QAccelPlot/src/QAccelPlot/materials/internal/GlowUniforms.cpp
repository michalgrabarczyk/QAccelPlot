//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/materials/internal/GlowUniforms.hpp"

namespace QAccelPlot::Internal {

GlowUniforms glowUniforms(const GlowStyle& style)
{
    auto uniforms = GlowUniforms{};
    if (!style.active()) {
        return uniforms;
    }
    uniforms.color[0] = static_cast<float>(style.color.redF());
    uniforms.color[1] = static_cast<float>(style.color.greenF());
    uniforms.color[2] = static_cast<float>(style.color.blueF());
    uniforms.color[3] = static_cast<float>(style.color.alphaF());
    uniforms.params[0] = style.radius;
    uniforms.params[1] = style.intensity;
    return uniforms;
}

} // namespace QAccelPlot::Internal
