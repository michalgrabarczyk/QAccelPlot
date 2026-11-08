//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/internal/ItemFade.hpp"

namespace QAccelPlot::Internal {

/// \brief The std140 uniform members of one fading transition group, see \c ItemFadeGroup.
struct ItemFadeGroupUniforms {
    float range[4];                  ///< \brief First item, item count (0: no fade), 1 when the starts follow a front, 1 when the items exit.
    float timing[4];                 ///< \brief Progress, spread, and timeline length of each item; then padding.
    float curve[kFadeCurveSamples];  ///< \brief Opacity over the timeline of an item.
    float starts[kFadeStartSamples]; ///< \brief Start times along the front.
};

static_assert(sizeof(ItemFadeGroupUniforms) == 224);

/// \brief The std140 uniform members that shaders/item_opacity.glsl reads, at a 16-byte aligned offset.
struct ItemOpacityUniforms {
    ItemFadeGroupUniforms groups[kFadeGroupCount]; ///< \brief The fading groups: kept items, then added or removed ones.
    float trail[4];                                ///< \brief The \c FadeTrail length, minimum opacity, and item count; then padding.
};

static_assert(sizeof(ItemOpacityUniforms) == 464);

/// \brief Returns the uniforms that fade the items of \a fade and \a trail.
ItemOpacityUniforms itemOpacityUniforms(const ItemFade& fade, const ItemTrail& trail);

} // namespace QAccelPlot::Internal
