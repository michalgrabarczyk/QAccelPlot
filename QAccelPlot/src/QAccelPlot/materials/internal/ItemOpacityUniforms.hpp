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

/// \brief The std140 uniform members that shaders/item_opacity.glsl reads, at a 16-byte aligned offset.
struct ItemOpacityUniforms {
    float fadeRange[4];                   ///< \brief First item, item count, progress, and stagger of the fading group.
    float fadeParams[4];                  ///< \brief 1 when the items exit, then the \c FadeTrail length, minimum opacity, and item count.
    float fadeEasing[kFadeEasingSamples]; ///< \brief The easing curve of the fading group.
};

static_assert(sizeof(ItemOpacityUniforms) == 96);

/// \brief Returns the uniforms that fade the items of \a fade and \a trail.
ItemOpacityUniforms itemOpacityUniforms(const ItemFade& fade, const ItemTrail& trail);

} // namespace QAccelPlot::Internal
