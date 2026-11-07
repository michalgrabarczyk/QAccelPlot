//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/transitions/internal/TransitionFrame.hpp"

#include "QAccelPlot/transitions/MorphTransition.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace QAccelPlot::Internal {

namespace {

/// \brief A run of items written into the frame.
struct Group {
    const double* source{nullptr}; ///< \brief Values of the group's first item.
    int count{0};                  ///< \brief Number of items in the group.
    int first{0};                  ///< \brief Frame index of the group's first item.
    bool exit{false};              ///< \brief \c true for removed items, which play the transition backwards.
};

double* frameItem(std::vector<double>& out, const int index, const int stride)
{
    return out.data() + static_cast<std::size_t>(index) * static_cast<std::size_t>(stride);
}

void copyItems(const double* source, const int count, std::vector<double>& out, const int first, const int stride)
{
    std::copy_n(source, static_cast<std::size_t>(count) * static_cast<std::size_t>(stride), frameItem(out, first, stride));
}

/// \brief Returns the position of item \a index in the stagger order of \a group: removed items leave the last one first.
int staggerOrder(const Group& group, const int index)
{
    return group.exit ? group.count - 1 - index : index;
}

/// \brief Returns the eased progress of item \a index of \a group.
double easedItemProgress(const TransitionStep& step, const Group& group, const int index)
{
    return step.easing.valueForProgress(itemProgress(step.progress, staggerOrder(group, index), group.count, step.stagger));
}

void writeMorph(const TransitionStep& step, const double* from, const double* to, const int count, std::vector<double>& out, const int stride)
{
    const auto uniform = step.stagger <= 0.0;
    const auto shared = step.easing.valueForProgress(std::clamp(step.progress, 0.0, 1.0));
    const auto values = static_cast<std::size_t>(stride);
    for (auto item = 0; item < count; ++item) {
        const auto eased = uniform ? shared : step.easing.valueForProgress(itemProgress(step.progress, item, count, step.stagger));
        const auto offset = static_cast<std::size_t>(item) * values;
        for (auto value = std::size_t{0}; value < values; ++value) {
            out[offset + value] = MorphTransition::interpolateCoordinate(from[offset + value], to[offset + value], eased);
        }
    }
}

void writeGrow(const TransitionStep& step, const TransitionLayout& layout, const Group& group, std::vector<double>& out)
{
    const auto baseline = std::isnan(step.baseline) ? layout.baseline : step.baseline;
    const auto uniform = step.stagger <= 0.0;
    const auto shared = step.easing.valueForProgress(std::clamp(step.progress, 0.0, 1.0));
    copyItems(group.source, group.count, out, group.first, layout.stride);
    for (auto item = 0; item < group.count; ++item) {
        const auto eased = uniform ? shared : easedItemProgress(step, group, item);
        const auto scale = group.exit ? 1.0 - eased : eased;
        auto* values = frameItem(out, group.first + item, layout.stride);
        for (const auto column : layout.valueColumns) {
            const auto value = values[column];
            if (std::isfinite(value) && std::isfinite(baseline)) {
                values[column] = baseline + scale * (value - baseline);
            }
        }
    }
}

/// \brief Returns how many items of \a group a \c DrawTransition shows, and the reveal front in items into \a front.
///
/// \a front is negative when items are shown whole. \a keepFirst shows the first item from the start.
int drawnCount(const TransitionStep& step, const Group& group, const bool keepFirst, double& front)
{
    if (step.stagger >= 1.0) {
        const auto eased = step.easing.valueForProgress(std::clamp(step.progress, 0.0, 1.0));
        front = std::clamp((group.exit ? 1.0 - eased : eased) * group.count, 0.0, static_cast<double>(group.count));
        const auto shown = static_cast<int>(std::ceil(front));
        return keepFirst ? std::max(shown, std::min(group.count, 1)) : shown;
    }
    front = -1.0;
    auto shown = 0;
    for (; shown < group.count; ++shown) {
        const auto progress = itemProgress(step.progress, staggerOrder(group, shown), group.count, step.stagger);
        const auto visible = group.exit ? progress < 1.0 : progress > 0.0 || (shown == 0 && keepFirst);
        if (!visible) {
            break;
        }
    }
    return shown;
}

/// \brief Moves frame item \a index to the share \a fraction of the way from the item before it.
void revealPartway(std::vector<double>& out, const int index, const double fraction, const int stride)
{
    const auto* previous = frameItem(out, index - 1, stride);
    auto* item = frameItem(out, index, stride);
    for (auto value = 0; value < stride; ++value) {
        if (std::isfinite(previous[value]) && std::isfinite(item[value])) {
            item[value] = previous[value] + fraction * (item[value] - previous[value]);
        }
    }
}

int writeDraw(const TransitionStep& step, const TransitionLayout& layout, const Group& group, std::vector<double>& out)
{
    auto front = 0.0;
    const auto shown = drawnCount(step, group, !group.exit && group.first == 0, front);
    copyItems(group.source, shown, out, group.first, layout.stride);
    const auto last = group.first + shown - 1;
    const auto fraction = front - (shown - 1);
    if (layout.continuous && front >= 0.0 && shown > 0 && last > 0 && fraction > 0.0 && fraction < 1.0) {
        revealPartway(out, last, fraction, layout.stride);
    }
    return shown;
}

ItemFade groupFade(const TransitionStep& step, const Group& group)
{
    if (!step.fade || step.stagger >= 1.0 || group.count == 0) {
        return {};
    }
    return ItemFade{group.first, group.count, static_cast<float>(step.progress), static_cast<float>(step.stagger), group.exit, easingSamples(step.easing)};
}

/// \brief Writes \a group as an \c EnterExitTransition moves it and returns the number of items shown.
int writeEnterExit(const TransitionStep& step, const TransitionLayout& layout, const Group& group, std::vector<double>& out, ItemFade& fade)
{
    fade = groupFade(step, group);
    if (step.kind == TransitionKind::Grow) {
        writeGrow(step, layout, group, out);
        return group.count;
    }
    return writeDraw(step, layout, group, out);
}

const double* sourceItem(const TransitionItems& items, const int index, const int stride)
{
    return items.values + static_cast<std::size_t>(index) * static_cast<std::size_t>(stride);
}

} // namespace

double itemProgress(const double progress, const int item, const int itemCount, const double stagger)
{
    const auto clamped = std::clamp(progress, 0.0, 1.0);
    if (itemCount <= 1 || stagger <= 0.0) {
        return clamped;
    }
    const auto delay = stagger * item / (itemCount - 1);
    const auto span = 1.0 - stagger;
    if (span <= 0.0) {
        return clamped >= delay ? 1.0 : 0.0;
    }
    return std::clamp((clamped - delay) / span, 0.0, 1.0);
}

std::array<float, kFadeEasingSamples> easingSamples(const QEasingCurve& easing)
{
    auto samples = std::array<float, kFadeEasingSamples>{};
    for (auto index = 0; index < kFadeEasingSamples; ++index) {
        samples[static_cast<std::size_t>(index)] = static_cast<float>(easing.valueForProgress(static_cast<double>(index) / (kFadeEasingSamples - 1)));
    }
    return samples;
}

bool ItemFade::operator==(const ItemFade& other) const noexcept
{
    return first == other.first && count == other.count && progress == other.progress && stagger == other.stagger && exit == other.exit
        && easing == other.easing;
}

TransitionSlots plannedSlots(const int fromCount, const int toCount, const std::optional<TransitionKind> enter, const std::optional<TransitionKind> update,
    const std::optional<TransitionKind> exit)
{
    const auto redraw = update && *update != TransitionKind::Morph && fromCount > 0 && toCount > 0;
    if (redraw) {
        return TransitionSlots{false, true, false};
    }
    return TransitionSlots{
        enter.has_value() && toCount > fromCount, update.has_value() && std::min(fromCount, toCount) > 0, exit.has_value() && fromCount > toCount};
}

int writeTransitionFrame(const TransitionFrameInput& input, std::vector<double>& out, ItemFade& fade)
{
    const auto stride = input.layout.stride;
    const auto fromCount = input.from.count;
    const auto toCount = input.to.count;
    fade = {};
    out.resize(static_cast<std::size_t>(std::max(fromCount, toCount)) * static_cast<std::size_t>(stride));

    auto count = 0;
    if (input.update && input.update->kind != TransitionKind::Morph) {
        count = writeEnterExit(*input.update, input.layout, Group{input.to.values, toCount, 0, false}, out, fade);
    } else {
        const auto kept = std::min(fromCount, toCount);
        if (input.update) {
            writeMorph(*input.update, input.from.values, input.to.values, kept, out, stride);
        } else {
            copyItems(input.to.values, kept, out, 0, stride);
        }
        count = kept;
        if (toCount > fromCount) {
            const auto added = Group{sourceItem(input.to, kept, stride), toCount - kept, kept, false};
            if (input.enter) {
                count += writeEnterExit(*input.enter, input.layout, added, out, fade);
            } else {
                copyItems(added.source, added.count, out, kept, stride);
                count = toCount;
            }
        } else if (fromCount > toCount && input.exit) {
            count += writeEnterExit(*input.exit, input.layout, Group{sourceItem(input.from, kept, stride), fromCount - kept, kept, true}, out, fade);
        }
    }
    out.resize(static_cast<std::size_t>(count) * static_cast<std::size_t>(stride));
    return count;
}

} // namespace QAccelPlot::Internal
