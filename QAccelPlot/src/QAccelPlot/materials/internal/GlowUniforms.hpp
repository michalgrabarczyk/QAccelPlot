//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/internal/GlowStyle.hpp"

namespace QAccelPlot::Internal {

/// \brief The std140 uniform members of a \c Glow halo, \c glowColor and \c glowParams, at a 16-byte aligned offset.
struct GlowUniforms {
    float color[4];  ///< \brief Halo color, not premultiplied.
    float params[4]; ///< \brief Halo radius in pixels and intensity; a zero radius draws no halo.
};

static_assert(sizeof(GlowUniforms) == 32);

/// \brief Returns the uniforms that draw \a style.
GlowUniforms glowUniforms(const GlowStyle& style);

} // namespace QAccelPlot::Internal
