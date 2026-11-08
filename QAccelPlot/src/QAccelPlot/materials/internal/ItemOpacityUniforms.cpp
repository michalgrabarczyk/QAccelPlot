//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/internal/ItemOpacityUniforms.hpp"

#include <algorithm>
#include <cstddef>

namespace QAccelPlot::Internal {

namespace {

ItemFadeGroupUniforms groupUniforms(const ItemFadeGroup& group)
{
    auto uniforms = ItemFadeGroupUniforms{};
    if (!group.active()) {
        return uniforms;
    }
    uniforms.range[0] = static_cast<float>(group.first);
    uniforms.range[1] = static_cast<float>(group.count);
    uniforms.range[2] = group.front ? 1.0f : 0.0f;
    uniforms.range[3] = group.exit ? 1.0f : 0.0f;
    uniforms.timing[0] = group.time;
    uniforms.timing[1] = group.spread;
    uniforms.timing[2] = group.length;
    std::copy(group.curve.cbegin(), group.curve.cend(), uniforms.curve);
    std::copy(group.starts.cbegin(), group.starts.cend(), uniforms.starts);
    return uniforms;
}

} // namespace

ItemOpacityUniforms itemOpacityUniforms(const ItemFade& fade, const ItemTrail& trail)
{
    auto uniforms = ItemOpacityUniforms{};
    for (auto index = std::size_t{0}; index < fade.groups.size(); ++index) {
        uniforms.groups[index] = groupUniforms(fade.groups[index]);
    }
    if (trail.active()) {
        uniforms.trail[0] = trail.length;
        uniforms.trail[1] = trail.minOpacity;
        uniforms.trail[2] = static_cast<float>(trail.itemCount);
    }
    return uniforms;
}

} // namespace QAccelPlot::Internal
