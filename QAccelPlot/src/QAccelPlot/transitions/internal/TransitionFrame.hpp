//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/internal/ItemFade.hpp"
#include "QAccelPlot/transitions/DataTransition.hpp"
#include "QAccelPlot/transitions/internal/OpacityTimeline.hpp"

#include <QEasingCurve>

#include <array>
#include <limits>
#include <optional>
#include <vector>

namespace QAccelPlot::Internal {

/// \brief How a series' items animate.
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
    double baseline{std::numeric_limits<double>::quiet_NaN()}; ///< \brief \c GrowTransition baseline; NaN for the layout's.
    OpacityTimeline opacity;                                   ///< \brief Opacity of each item over its own time; empty for none.
    double duration{0.0};                                      ///< \brief Duration of the run in milliseconds, which times \c opacity.
};

/// \brief The timing of the items of one group, as shares of the transition duration.
struct ItemTiming {
    double motion{1.0}; ///< \brief Time each item takes to move.
    double length{1.0}; ///< \brief Time each item animates: its movement or its opacity animations, whichever is longer.
    double spread{0.0}; ///< \brief Time over which the start times of the items spread.
    bool front{false};  ///< \brief Items start as an eased reveal front reaches them, at a stagger of 1.
};

/// \brief Returns the timing of a group of \a itemCount items under \a step.
ItemTiming itemTiming(const TransitionStep& step, int itemCount);

/// \brief When an eased reveal front reaches each share of the items.
class FrontStarts {
public:
    /// \brief Samples the front that \a easing moves.
    explicit FrontStarts(const QEasingCurve& easing);

    /// \brief Returns the progress in [0, 1] at which the eased progress first reaches \a share.
    [[nodiscard]] double operator()(double share) const;

private:
    static constexpr auto kSamples = 1025;
    std::array<double, kSamples> starts_{};
};

/// \brief The start time and movement of each item of one group at the frame.
class ItemClock {
public:
    /// \brief Times a group of \a itemCount items under \a step; removed items when \a exit.
    ItemClock(const TransitionStep& step, int itemCount, bool exit);

    /// \brief Returns the timing of the group.
    [[nodiscard]] const ItemTiming& timing() const noexcept;
    /// \brief Returns the start time of the item at position \a order of the stagger order.
    [[nodiscard]] double start(int order) const;
    /// \brief Returns the progress of the movement of the item at position \a order, before easing.
    [[nodiscard]] double motion(int order) const;
    /// \brief Returns where the front starts the items, or \c nullptr without a front.
    [[nodiscard]] const FrontStarts* frontStarts() const noexcept;

private:
    double time_;
    int count_;
    bool exit_;
    ItemTiming timing_;
    double startStep_;
    std::optional<FrontStarts> starts_;
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

/// \brief Returns \c true when an item below both counts of \a from and \a to, \a stride values each, changes.
///
/// Invalid values that stay invalid do not count as a change.
bool keptItemsChanged(const TransitionItems& from, const TransitionItems& to, int stride);

/// \brief Returns the slots that animate a change from \a fromCount to \a toCount items.
///
/// Each kind is that of the slot's enabled transition, or \c std::nullopt without one. The update
/// slot runs only when \a keptChanged: a change that only adds or removes items leaves the kept ones
/// alone. A \c DrawTransition or \c GrowTransition in the update slot redraws all items, so only
/// the update slot runs.
TransitionSlots plannedSlots(int fromCount, int toCount, bool keptChanged, std::optional<TransitionKind> enter, std::optional<TransitionKind> update,
    std::optional<TransitionKind> exit);

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
/// The frame holds the kept items, then the shown added or removed items. The first group of
/// \a fade describes the update slot, the second the enter or exit slot.
/// \return The number of items in the frame.
int writeTransitionFrame(const TransitionFrameInput& input, std::vector<double>& out, ItemFade& fade);

} // namespace QAccelPlot::Internal
