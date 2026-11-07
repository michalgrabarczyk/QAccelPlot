//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/transitions/DataTransition.hpp"

#include <QEasingCurve>

#include <array>
#include <limits>
#include <optional>
#include <vector>

namespace QAccelPlot::Internal {

/// \brief Number of easing samples the shaders interpolate to fade items.
inline constexpr int kFadeEasingSamples = 16;

/// \brief Returns the progress of item \a item of \a itemCount at transition progress \a progress, before easing.
///
/// The start times of the items are spread over the share \a stagger of the duration, in item order.
double itemProgress(double progress, int item, int itemCount, double stagger);

/// \brief Returns \a easing at \c kFadeEasingSamples evenly spaced progress values from 0 to 1.
std::array<float, kFadeEasingSamples> easingSamples(const QEasingCurve& easing);

/// \brief The items of a frame whose opacity the shaders fade, as one transition group.
struct ItemFade {
    int first{0};                                   ///< \brief Frame index of the first item of the group.
    int count{0};                                   ///< \brief Number of items in the group; 0 when no item fades.
    float progress{0.0f};                           ///< \brief Progress of the group's transition, before easing.
    float stagger{0.0f};                            ///< \brief Stagger of the group's transition.
    bool exit{false};                               ///< \brief \c true when the items fade out, the last one first.
    std::array<float, kFadeEasingSamples> easing{}; ///< \brief Easing samples, see \c easingSamples().

    /// \brief Returns \c true when some item fades.
    [[nodiscard]] bool active() const noexcept
    {
        return count > 0;
    }

    /// \brief Returns \c true when \a other fades the same items the same way.
    [[nodiscard]] bool operator==(const ItemFade& other) const noexcept;

    /// \brief Returns \c true when \a other differs.
    [[nodiscard]] bool operator!=(const ItemFade& other) const noexcept
    {
        return !(*this == other);
    }
};

/// \brief How the items of a series animate.
struct TransitionLayout {
    int stride{2};                    ///< \brief Values per item.
    std::vector<int> valueColumns{1}; ///< \brief Columns a \c GrowTransition grows.
    double baseline{0.0};             ///< \brief Value items grow from when the \c GrowTransition has no baseline.
    bool continuous{false};           ///< \brief A \c DrawTransition reveals the last item partway, as a line segment.
};

/// \brief The state of one slot's transition at the frame.
struct TransitionStep {
    TransitionKind kind{TransitionKind::Morph};                ///< \brief How the transition moves items.
    double progress{1.0};                                      ///< \brief Progress of the run, before easing.
    double stagger{0.0};                                       ///< \brief Stagger of the transition.
    QEasingCurve easing;                                       ///< \brief Easing of each item's progress.
    bool fade{false};                                          ///< \brief Whether items fade.
    double baseline{std::numeric_limits<double>::quiet_NaN()}; ///< \brief \c GrowTransition baseline; NaN for the layout's.
};

/// \brief Items on one side of a transition, \c TransitionLayout::stride values each.
struct TransitionItems {
    const double* values{nullptr}; ///< \brief The values, item after item.
    int count{0};                  ///< \brief Number of items.
};

/// \brief Slots a data change animates.
struct TransitionSlots {
    bool enter{false};  ///< \brief Added items run the enter transition.
    bool update{false}; ///< \brief Kept items, or all items when \c redraw, run the update transition.
    bool exit{false};   ///< \brief Removed items run the exit transition.

    /// \brief Returns \c true when some slot runs.
    [[nodiscard]] bool any() const noexcept
    {
        return enter || update || exit;
    }
};

/// \brief Returns the slots that animate a change from \a fromCount to \a toCount items.
///
/// Each kind is that of the slot's enabled transition, or \c std::nullopt without one. A
/// \c DrawTransition or \c GrowTransition in the update slot redraws all items, so only the update slot runs.
TransitionSlots plannedSlots(
    int fromCount, int toCount, std::optional<TransitionKind> enter, std::optional<TransitionKind> update, std::optional<TransitionKind> exit);

/// \brief The two sides of a transition and the steps of its slots at one frame.
struct TransitionFrameInput {
    TransitionItems from;                 ///< \brief Items drawn when the change happened.
    TransitionItems to;                   ///< \brief The new items.
    TransitionLayout layout;              ///< \brief How the items animate.
    std::optional<TransitionStep> enter;  ///< \brief Step of the enter slot, when it runs.
    std::optional<TransitionStep> update; ///< \brief Step of the update slot, when it runs.
    std::optional<TransitionStep> exit;   ///< \brief Step of the exit slot, when it runs.
};

/// \brief Writes the frame of \a input into \a out and the items it fades into \a fade.
///
/// The frame holds the kept items, then the shown added or removed items.
/// \return The number of items in the frame.
int writeTransitionFrame(const TransitionFrameInput& input, std::vector<double>& out, ItemFade& fade);

} // namespace QAccelPlot::Internal
