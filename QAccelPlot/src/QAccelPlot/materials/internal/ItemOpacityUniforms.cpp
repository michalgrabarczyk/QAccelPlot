//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/internal/ItemOpacityUniforms.hpp"

#include <algorithm>

namespace QAccelPlot::Internal {

ItemOpacityUniforms itemOpacityUniforms(const ItemFade& fade)
{
    auto uniforms = ItemOpacityUniforms{};
    if (!fade.active()) {
        return uniforms;
    }
    uniforms.fadeRange[0] = static_cast<float>(fade.first);
    uniforms.fadeRange[1] = static_cast<float>(fade.count);
    uniforms.fadeRange[2] = fade.progress;
    uniforms.fadeRange[3] = fade.stagger;
    uniforms.fadeParams[0] = fade.exit ? 1.0f : 0.0f;
    std::copy(fade.easing.cbegin(), fade.easing.cend(), uniforms.fadeEasing);
    return uniforms;
}

} // namespace QAccelPlot::Internal
