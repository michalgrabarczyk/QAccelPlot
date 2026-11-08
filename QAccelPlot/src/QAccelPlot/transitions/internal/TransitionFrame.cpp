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
#include <optional>

namespace QAccelPlot::Internal {

namespace {

/// \brief A run of items written into the frame.
struct Group {
    const double* source{nullptr}; ///< \brief Values of the group's first item.
    int count{0};                  ///< \brief Number of items in the group.
    int first{0};                  ///< \brief Frame index of the group's first item.
    bool exit{false};              ///< \brief \c true for removed items, which play the movement backwards.
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

/// \brief Returns \a samples, evenly spaced over [0, 1], interpolated at \a position.
template <std::size_t N> double interpolate(const std::array<double, N>& samples, const double position)
{
    const auto scaled = std::clamp(position, 0.0, 1.0) * static_cast<double>(N - 1);
    const auto lower = std::min(static_cast<std::size_t>(scaled), N - 2);
    const auto a = samples[lower];
    const auto b = samples[lower + 1];
    return a + (scaled - static_cast<double>(lower)) * (b - a);
}

/// \brief The easing curve of a staggered group, sampled once per frame instead of evaluated once per item.
class EasingTable {
public:
    explicit EasingTable(const QEasingCurve& easing)
    {
        for (auto index = std::size_t{0}; index < kSamples; ++index) {
            samples_[index] = easing.valueForProgress(static_cast<double>(index) / (kSamples - 1));
        }
    }

    /// \brief Returns the eased value at \a progress in [0, 1], interpolated between the samples.
    double operator()(const double progress) const
    {
        return interpolate(samples_, progress);
    }

private:
    static constexpr auto kSamples = std::size_t{1025};
    std::array<double, kSamples> samples_{};
};

/// \brief Returns the eased progress of every item of a group, evaluating the curve once when they all move together.
class ItemEasing {
public:
    ItemEasing(const TransitionStep& step, const ItemClock& clock)
        : clock_(clock)
        , together_(clock.timing().spread <= 0.0 && !clock.timing().front)
        , shared_(step.easing.valueForProgress(clock.motion(0)))
    {
        if (!together_) {
            table_.emplace(step.easing);
        }
    }

    double operator()(const int order) const
    {
        return together_ ? shared_ : (*table_)(clock_.motion(order));
    }

private:
    const ItemClock& clock_;
    bool together_;
    double shared_;
    std::optional<EasingTable> table_;
};

void writeMorph(const TransitionStep& step, const double* from, const double* to, const int count, std::vector<double>& out, const int stride)
{
    const auto clock = ItemClock{step, count, false};
    const auto easing = ItemEasing{step, clock};
    const auto values = static_cast<std::size_t>(stride);
    for (auto item = 0; item < count; ++item) {
        const auto eased = easing(item);
        const auto offset = static_cast<std::size_t>(item) * values;
        for (auto value = std::size_t{0}; value < values; ++value) {
            out[offset + value] = MorphTransition::interpolateCoordinate(from[offset + value], to[offset + value], eased);
        }
    }
}

void writeGrow(const TransitionStep& step, const ItemClock& clock, const TransitionLayout& layout, const Group& group, std::vector<double>& out)
{
    const auto baseline = std::isnan(step.baseline) ? layout.baseline : step.baseline;
    const auto easing = ItemEasing{step, clock};
    copyItems(group.source, group.count, out, group.first, layout.stride);
    for (auto item = 0; item < group.count; ++item) {
        const auto eased = easing(staggerOrder(group, item));
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

/// \brief Returns the reveal front of a \c DrawTransition at a stagger of 1, in items shown.
double revealFront(const TransitionStep& step, const ItemTiming& timing, const Group& group)
{
    const auto time = std::clamp(step.progress, 0.0, 1.0);
    const auto count = static_cast<double>(group.count);
    if (!group.exit) {
        // The front moves over the spread; the opacity animations of the last items fill the rest.
        const auto eased = timing.spread > 0.0 ? step.easing.valueForProgress(std::clamp(time / timing.spread, 0.0, 1.0)) : 1.0;
        return std::clamp(eased * count, 0.0, count);
    }
    // Removed items leave once their opacity animations end.
    const auto leaving = time - timing.length;
    const auto eased = timing.spread > 0.0 ? step.easing.valueForProgress(std::clamp(leaving / timing.spread, 0.0, 1.0)) : (leaving >= 0.0 ? 1.0 : 0.0);
    return std::clamp((1.0 - eased) * count, 0.0, count);
}

/// \brief Returns how many items of \a group a \c DrawTransition shows, and the reveal front in items into \a front.
///
/// \a front is negative when items are shown whole. \a keepFirst shows the first item from the start.
int drawnCount(const TransitionStep& step, const ItemClock& clock, const Group& group, const bool keepFirst, double& front)
{
    if (clock.timing().front) {
        front = revealFront(step, clock.timing(), group);
        const auto shown = static_cast<int>(std::ceil(front));
        return keepFirst ? std::max(shown, std::min(group.count, 1)) : shown;
    }
    front = -1.0;
    const auto time = std::clamp(step.progress, 0.0, 1.0);
    const auto length = clock.timing().length;
    auto shown = 0;
    for (; shown < group.count; ++shown) {
        const auto start = clock.start(staggerOrder(group, shown));
        const auto visible = group.exit ? time < start + length : time > start || (shown == 0 && keepFirst);
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

int writeDraw(const TransitionStep& step, const ItemClock& clock, const TransitionLayout& layout, const Group& group, std::vector<double>& out)
{
    auto front = 0.0;
    const auto shown = drawnCount(step, clock, group, !group.exit && group.first == 0, front);
    copyItems(group.source, shown, out, group.first, layout.stride);
    const auto last = group.first + shown - 1;
    const auto fraction = front - (shown - 1);
    if (layout.continuous && front >= 0.0 && shown > 0 && last > 0 && fraction > 0.0 && fraction < 1.0) {
        revealPartway(out, last, fraction, layout.stride);
    }
    return shown;
}

/// \brief Returns how the shaders fade \a count items from frame index \a first under \a step.
ItemFadeGroup fadeGroup(const TransitionStep& step, const ItemClock& clock, const int first, const int count, const bool exit)
{
    if (step.opacity.empty() || step.duration <= 0.0 || count == 0) {
        return {};
    }
    const auto& timing = clock.timing();
    auto group = ItemFadeGroup{first, count, exit, timing.front, static_cast<float>(std::clamp(step.progress, 0.0, 1.0)), static_cast<float>(timing.spread),
        static_cast<float>(timing.length)};
    const auto lengthMs = timing.length * step.duration;
    for (auto index = std::size_t{0}; index < group.curve.size(); ++index) {
        const auto time = lengthMs * static_cast<double>(index) / static_cast<double>(group.curve.size() - 1);
        group.curve[index] = static_cast<float>(std::clamp(step.opacity.valueAt(time), 0.0, 1.0));
    }
    if (const auto* starts = clock.frontStarts()) {
        for (auto index = std::size_t{0}; index < group.starts.size(); ++index) {
            group.starts[index] = static_cast<float>((*starts)(static_cast<double>(index) / static_cast<double>(group.starts.size() - 1)));
        }
    }
    return group;
}

/// \brief Writes \a group as an \c EnterExitTransition moves it and returns the number of items shown.
int writeEnterExit(const TransitionStep& step, const TransitionLayout& layout, const Group& group, std::vector<double>& out, ItemFadeGroup& fade)
{
    const auto clock = ItemClock{step, group.count, group.exit};
    fade = fadeGroup(step, clock, group.first, group.count, group.exit);
    if (step.kind == TransitionKind::Grow) {
        writeGrow(step, clock, layout, group, out);
        return group.count;
    }
    return writeDraw(step, clock, layout, group, out);
}

const double* sourceItem(const TransitionItems& items, const int index, const int stride)
{
    return items.values + static_cast<std::size_t>(index) * static_cast<std::size_t>(stride);
}

} // namespace

ItemTiming itemTiming(const TransitionStep& step, const int itemCount)
{
    // A single item moves over the whole duration, except a line segment drawn partway.
    const auto front = step.stagger >= 1.0 && (itemCount > 1 || step.kind == TransitionKind::Draw);
    auto motion = 1.0 - step.stagger;
    if (front) {
        motion = 0.0;
    } else if (itemCount <= 1 || step.stagger <= 0.0) {
        motion = 1.0;
    }
    const auto opacity = step.opacity.empty() || step.duration <= 0.0 ? 0.0 : step.opacity.length() / step.duration;
    const auto length = std::clamp(std::max(motion, opacity), 0.0, 1.0);
    return ItemTiming{motion, length, 1.0 - length, front};
}

FrontStarts::FrontStarts(const QEasingCurve& easing)
{
    // The front only moves forward, so it reaches a share when the highest eased value so far does.
    auto reached = std::array<double, kSamples>{};
    auto highest = -std::numeric_limits<double>::infinity();
    for (auto index = std::size_t{0}; index < kSamples; ++index) {
        highest = std::max(highest, easing.valueForProgress(static_cast<double>(index) / (kSamples - 1)));
        reached[index] = highest;
    }
    auto sample = std::size_t{0};
    for (auto index = std::size_t{0}; index < kSamples; ++index) {
        const auto share = static_cast<double>(index) / (kSamples - 1);
        while (sample < kSamples && reached[sample] < share) {
            ++sample;
        }
        if (sample == 0) {
            starts_[index] = 0.0;
        } else if (sample == kSamples) {
            starts_[index] = 1.0;
        } else {
            const auto before = reached[sample - 1];
            const auto after = reached[sample];
            const auto fraction = after > before ? (share - before) / (after - before) : 0.0;
            starts_[index] = (static_cast<double>(sample - 1) + fraction) / (kSamples - 1);
        }
    }
}

double FrontStarts::operator()(const double share) const
{
    return interpolate(starts_, share);
}

ItemClock::ItemClock(const TransitionStep& step, const int itemCount, const bool exit)
    : time_(std::clamp(step.progress, 0.0, 1.0))
    , count_(itemCount)
    , exit_(exit)
    , timing_(itemTiming(step, itemCount))
    , startStep_(itemCount > 1 ? timing_.spread / (itemCount - 1) : 0.0)
{
    if (timing_.front) {
        starts_.emplace(step.easing);
    }
}

const ItemTiming& ItemClock::timing() const noexcept
{
    return timing_;
}

double ItemClock::start(const int order) const
{
    if (!starts_) {
        return startStep_ * order;
    }
    // A removed item starts so that it leaves when the front passes it.
    const auto share = static_cast<double>(order + (exit_ ? 1 : 0)) / std::max(count_, 1);
    return timing_.spread * (*starts_)(share);
}

double ItemClock::motion(const int order) const
{
    const auto start = this->start(order);
    if (timing_.motion <= 0.0) {
        return time_ >= start ? 1.0 : 0.0;
    }
    return std::clamp((time_ - start) / timing_.motion, 0.0, 1.0);
}

const FrontStarts* ItemClock::frontStarts() const noexcept
{
    return starts_ ? &*starts_ : nullptr;
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
    auto& updateFade = fade.groups[0];
    auto& enterExitFade = fade.groups[1];
    out.resize(static_cast<std::size_t>(std::max(fromCount, toCount)) * static_cast<std::size_t>(stride));

    auto count = 0;
    if (input.update && input.update->kind != TransitionKind::Morph) {
        count = writeEnterExit(*input.update, input.layout, Group{input.to.values, toCount, 0, false}, out, updateFade);
    } else {
        const auto kept = std::min(fromCount, toCount);
        if (input.update) {
            writeMorph(*input.update, input.from.values, input.to.values, kept, out, stride);
            updateFade = fadeGroup(*input.update, ItemClock{*input.update, kept, false}, 0, kept, false);
        } else {
            copyItems(input.to.values, kept, out, 0, stride);
        }
        count = kept;
        if (toCount > fromCount) {
            const auto added = Group{sourceItem(input.to, kept, stride), toCount - kept, kept, false};
            if (input.enter) {
                count += writeEnterExit(*input.enter, input.layout, added, out, enterExitFade);
            } else {
                copyItems(added.source, added.count, out, kept, stride);
                count = toCount;
            }
        } else if (fromCount > toCount && input.exit) {
            const auto removed = Group{sourceItem(input.from, kept, stride), fromCount - kept, kept, true};
            count += writeEnterExit(*input.exit, input.layout, removed, out, enterExitFade);
        }
    }
    out.resize(static_cast<std::size_t>(count) * static_cast<std::size_t>(stride));
    return count;
}

} // namespace QAccelPlot::Internal
