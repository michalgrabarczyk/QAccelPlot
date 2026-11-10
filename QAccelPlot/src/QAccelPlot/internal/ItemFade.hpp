//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <array>
#include <vector>

namespace QAccelPlot::Internal {

/// \brief Number of opacity samples over the timeline of an item.
inline constexpr int kFadeCurveSamples = 32;
/// \brief Number of samples of the start times of the items along an eased reveal front.
inline constexpr int kFadeStartSamples = 16;
/// \brief Most transition groups that fade at once: the kept items, and the added or removed ones.
inline constexpr int kFadeGroupCount = 2;
/// \brief Number of samples of the \c FadeTrail easing curve.
inline constexpr int kTrailEasingSamples = 16;

/// \brief The items of one transition group whose opacity the shaders animate.
///
/// The shaders compute each item's opacity from these values, see shaders/item_opacity.glsl.
/// Times are shares of the transition duration.
struct ItemFadeGroup {
    int first{0};       ///< \brief Index of the first item of the group in the drawn items.
    int count{0};       ///< \brief Number of items in the group; 0 when the group does not fade.
    bool exit{false};   ///< \brief \c true for removed items, which start from the last one.
    bool front{false};  ///< \brief The start times follow \c starts rather than spreading evenly.
    float time{0.0f};   ///< \brief Progress of the transition.
    float spread{0.0f}; ///< \brief Time over which the start times of the items spread.
    float length{0.0f}; ///< \brief Length of the timeline of each item.
    /// \brief Opacity at evenly spaced times over the timeline of an item, from its start to \c length.
    std::array<float, kFadeCurveSamples> curve{};
    /// \brief When \c front, the start time over \c spread of the item at evenly spaced shares of the items.
    std::array<float, kFadeStartSamples> starts{};

    /// \brief Returns \c true when the group fades.
    [[nodiscard]] bool active() const noexcept
    {
        return count > 0;
    }
};

/// \brief The transition groups of a frame whose opacity the shaders animate.
struct ItemFade {
    std::array<ItemFadeGroup, kFadeGroupCount> groups{}; ///< \brief The groups; inactive ones have no items.

    /// \brief Returns \c true when some group fades.
    [[nodiscard]] bool active() const noexcept
    {
        return groups[0].active() || groups[1].active();
    }
};

/// \brief Returns \a group after some of its items are left out of the drawn items; it starts at drawn index \a first.
///
/// \a kept lists, in order, the positions in \a group of the remaining items. They keep their
/// start times through \c starts, which holds them at \c kFadeStartSamples evenly spaced shares
/// of the remaining items; items between two samples start at an interpolated time.
[[nodiscard]] ItemFadeGroup compactedFadeGroup(const ItemFadeGroup& group, int first, const std::vector<int>& kept);

/// \brief Orders \a left and \a right for \c QSGMaterial::compare(): negative, 0, or positive.
[[nodiscard]] int compareItemFades(const ItemFade& left, const ItemFade& right) noexcept;

/// \brief How the shaders fade the oldest items under a \c FadeTrail effect.
struct ItemTrail {
    float length{0.0f};     ///< \brief Share of the items that fades, from the first; 0 fades none.
    float minOpacity{1.0f}; ///< \brief Opacity of the first item.
    int itemCount{0};       ///< \brief Number of drawn items.
    /// \brief How far the opacity has risen from \c minOpacity to 1 at evenly spaced shares of the fading items.
    std::array<float, kTrailEasingSamples> easing{};

    /// \brief Returns \c true when some item fades.
    [[nodiscard]] bool active() const noexcept
    {
        return length > 0.0f && minOpacity < 1.0f && itemCount > 1;
    }
};

/// \brief Orders \a left and \a right for \c QSGMaterial::compare(): negative, 0, or positive.
[[nodiscard]] int compareItemTrails(const ItemTrail& left, const ItemTrail& right) noexcept;

} // namespace QAccelPlot::Internal
