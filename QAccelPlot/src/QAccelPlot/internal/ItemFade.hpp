//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <array>

namespace QAccelPlot::Internal {

/// \brief Number of easing samples the shaders interpolate to fade items.
inline constexpr int kFadeEasingSamples = 16;

/// \brief The items of a frame whose opacity the shaders fade, as one transition group.
///
/// The shaders compute each item's opacity from these values, see shaders/item_opacity.glsl.
struct ItemFade {
    int first{0};                                   ///< \brief Index of the first item of the group in the drawn items.
    int count{0};                                   ///< \brief Number of items in the group; 0 when no item fades.
    float progress{0.0f};                           ///< \brief Progress of the group's transition, before easing.
    float stagger{0.0f};                            ///< \brief Stagger of the group's transition.
    bool exit{false};                               ///< \brief \c true when the items fade out, the last one first.
    std::array<float, kFadeEasingSamples> easing{}; ///< \brief The easing curve at evenly spaced progress values from 0 to 1.

    /// \brief Returns \c true when some item fades.
    [[nodiscard]] bool active() const noexcept
    {
        return count > 0;
    }
};

/// \brief Orders \a left and \a right for \c QSGMaterial::compare(): negative, 0, or positive.
[[nodiscard]] int compareItemFades(const ItemFade& left, const ItemFade& right) noexcept;

/// \brief How the shaders fade the oldest items under a \c FadeTrail effect.
struct ItemTrail {
    float length{0.0f};     ///< \brief Share of the items that fades, from the first; 0 fades none.
    float minOpacity{1.0f}; ///< \brief Opacity of the first item.
    int itemCount{0};       ///< \brief Number of drawn items.

    /// \brief Returns \c true when some item fades.
    [[nodiscard]] bool active() const noexcept
    {
        return length > 0.0f && minOpacity < 1.0f && itemCount > 1;
    }
};

/// \brief Orders \a left and \a right for \c QSGMaterial::compare(): negative, 0, or positive.
[[nodiscard]] int compareItemTrails(const ItemTrail& left, const ItemTrail& right) noexcept;

} // namespace QAccelPlot::Internal
