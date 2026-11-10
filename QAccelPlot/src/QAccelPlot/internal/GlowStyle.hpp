//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>

namespace QAccelPlot::Internal {

/// \brief How the shaders draw a \c Glow halo; see shaders/glow.glsl.
struct GlowStyle {
    QColor color;          ///< \brief Halo color.
    float radius{0.0f};    ///< \brief Halo width in pixels; 0 draws no halo.
    float intensity{0.0f}; ///< \brief Halo opacity next to the shape.

    /// \brief Returns \c true when the style draws a halo.
    [[nodiscard]] bool active() const noexcept
    {
        return radius > 0.0f && intensity > 0.0f;
    }
};

/// \brief Orders \a left and \a right for \c QSGMaterial::compare(): negative, 0, or positive.
[[nodiscard]] int compareGlowStyles(const GlowStyle& left, const GlowStyle& right) noexcept;

} // namespace QAccelPlot::Internal
