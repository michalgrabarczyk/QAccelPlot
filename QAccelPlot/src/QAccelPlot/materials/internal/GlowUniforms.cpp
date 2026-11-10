//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
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
