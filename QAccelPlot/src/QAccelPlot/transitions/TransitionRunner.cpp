//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/transitions/TransitionRunner.hpp"

#include "QAccelPlot/transitions/GrowTransition.hpp"

#include <QQuickItem>
#include <QQuickWindow>

#include <algorithm>
#include <cstddef>
#include <utility>

namespace QAccelPlot {

namespace {

std::size_t slotIndex(const TransitionRunner::Slot slot)
{
    return static_cast<std::size_t>(slot);
}

constexpr auto kSlots = std::array{TransitionRunner::Slot::Enter, TransitionRunner::Slot::Update, TransitionRunner::Slot::Exit};

} // namespace

TransitionRunner::TransitionRunner(QQuickItem* item)
{
    connect(item, &QQuickItem::windowChanged, this, &TransitionRunner::connectWindow);
    // A parent already in a window adds the item to it from the QQuickItem constructor.
    connectWindow(item->window());
}

TransitionRunner::~TransitionRunner()
{
    // Ending the runs emits runningChanged, which must not reach this partly destroyed object.
    for (const auto& connection : runningConnections_) {
        disconnect(connection);
    }
    for (const auto& connection : destroyedConnections_) {
        disconnect(connection);
    }
    pending_ = false;
    endRuns();
}

DataTransition* TransitionRunner::transition(const Slot slot) const
{
    return transitions_[slotIndex(slot)];
}

bool TransitionRunner::setTransition(const Slot slot, DataTransition* transition)
{
    const auto index = slotIndex(slot);
    if (transitions_[index] == transition) {
        return false;
    }
    if (slot != Slot::Update && transition && !qobject_cast<EnterExitTransition*>(transition)) {
        return false;
    }
    interrupt();
    disconnect(destroyedConnections_[index]);
    disconnect(runningConnections_[index]);
    transitions_[index] = transition;
    if (transition) {
        destroyedConnections_[index] = connect(transition, &QObject::destroyed, this, [this, slot] { onTransitionDestroyed(slot); });
        runningConnections_[index] = connect(transition, &DataTransition::runningChanged, this, &TransitionRunner::onRunningChanged);
    }
    return true;
}

bool TransitionRunner::enabled() const
{
    return std::any_of(kSlots.cbegin(), kSlots.cend(), [this](const Slot slot) { return enabledKind(slot).has_value(); });
}

bool TransitionRunner::active() const
{
    return active_;
}

bool TransitionRunner::pending() const
{
    return pending_;
}

const std::vector<double>& TransitionRunner::targetData() const
{
    return to_;
}

int TransitionRunner::targetPointCount() const
{
    return toCount_;
}

int TransitionRunner::frameCapacity() const
{
    return std::max(fromCount_, toCount_);
}

bool TransitionRunner::start(
    const std::vector<double>& currentData, const int currentPointCount, std::vector<double>&& newData, const int newPointCount, const Layout& layout)
{
    Q_ASSERT(layout.stride > 0);
    // Items still leaving from the previous run follow its target items, and are dropped.
    const auto fromCount = pending_ ? std::min(currentPointCount, toCount_) : currentPointCount;
    // Ended without interrupting: the new run replaces the old one.
    pending_ = false;
    endRuns();
    fade_ = {};
    const auto planned = Internal::plannedSlots(fromCount, newPointCount, enabledKind(Slot::Enter), enabledKind(Slot::Update), enabledKind(Slot::Exit));
    if (!planned.any()) {
        cancel();
        return false;
    }
    layout_ = layout;
    from_.assign(currentData.cbegin(), currentData.cbegin() + static_cast<std::ptrdiff_t>(fromCount) * layout.stride);
    fromCount_ = fromCount;
    to_ = std::move(newData);
    toCount_ = newPointCount;
    pending_ = true;
    const auto used = std::array{planned.enter, planned.update, planned.exit};
    for (const auto slot : kSlots) {
        if (used[slotIndex(slot)]) {
            transitions_[slotIndex(slot)]->start(runs_[slotIndex(slot)]);
        }
    }
    // Set after all runs started, so runningChanged handlers don't see a used slot without its run.
    used_ = used;
    updateActive();
    requestFrame();
    return true;
}

bool TransitionRunner::advance(std::vector<double>& outData, int& outPointCount)
{
    if (!pending_) {
        return false;
    }
    if (finished()) {
        finish(outData, outPointCount);
        return false;
    }
    const auto input = Internal::TransitionFrameInput{Internal::TransitionItems{from_.data(), fromCount_}, Internal::TransitionItems{to_.data(), toCount_},
        layout_, step(Slot::Enter), step(Slot::Update), step(Slot::Exit)};
    outPointCount = Internal::writeTransitionFrame(input, outData, fade_);
    return true;
}

const Internal::ItemFade& TransitionRunner::fade() const
{
    return fade_;
}

bool TransitionRunner::finish(std::vector<double>& outData, int& outPointCount)
{
    if (!pending_) {
        return false;
    }
    outData = std::move(to_);
    outPointCount = toCount_;
    cancel();
    return true;
}

void TransitionRunner::cancel()
{
    // Cleared before the runs end, so runningChanged handlers see the run as ended.
    pending_ = false;
    endRuns();
    from_ = std::vector<double>{};
    to_ = std::vector<double>{};
    fromCount_ = 0;
    toCount_ = 0;
    fade_ = {};
    updateActive();
}

std::optional<Internal::TransitionKind> TransitionRunner::enabledKind(const Slot slot) const
{
    const auto* transition = transitions_[slotIndex(slot)].data();
    if (!transition || !transition->enabled()) {
        return std::nullopt;
    }
    return transition->kind();
}

std::optional<Internal::TransitionStep> TransitionRunner::step(const Slot slot) const
{
    const auto index = slotIndex(slot);
    const auto* transition = transitions_[index].data();
    if (!used_[index] || !transition) {
        return std::nullopt;
    }
    auto result = Internal::TransitionStep{transition->kind(), transition->progress(runs_[index]), transition->stagger(), transition->easing()};
    result.opacity = transition->opacityTimeline();
    result.duration = transition->duration();
    if (const auto* grow = qobject_cast<const GrowTransition*>(transition)) {
        result.baseline = grow->baseline();
    }
    return result;
}

bool TransitionRunner::finished() const
{
    return std::all_of(kSlots.cbegin(), kSlots.cend(), [this](const Slot slot) {
        const auto index = slotIndex(slot);
        return !used_[index] || !transitions_[index] || transitions_[index]->progress(runs_[index]) >= 1.0;
    });
}

void TransitionRunner::endRuns()
{
    for (auto& run : runs_) {
        run.end();
    }
    used_ = {};
}

void TransitionRunner::interrupt()
{
    if (!pending_) {
        return;
    }
    emit interrupted();
    cancel();
}

void TransitionRunner::onTransitionDestroyed(const Slot slot)
{
    if (used_[slotIndex(slot)]) {
        interrupt();
    }
    emit transitionDestroyed(slot);
}

void TransitionRunner::onRunningChanged()
{
    // DataTransition::cancel() ends every run but leaves its data pending for the host.
    for (const auto slot : kSlots) {
        const auto index = slotIndex(slot);
        if (used_[index] && !runs_[index].active()) {
            interrupt();
            return;
        }
    }
}

void TransitionRunner::updateActive()
{
    const auto active = pending_ && std::any_of(runs_.cbegin(), runs_.cend(), [](const DataTransition::Run& run) { return run.active(); });
    if (active_ == active) {
        return;
    }
    active_ = active;
    emit activeChanged();
}

void TransitionRunner::connectWindow(QQuickWindow* window)
{
    if (window_ == window) {
        return;
    }
    if (window_) {
        disconnect(window_, &QQuickWindow::afterAnimating, this, &TransitionRunner::onAfterAnimating);
    }
    window_ = window;
    if (window) {
        connect(window, &QQuickWindow::afterAnimating, this, &TransitionRunner::onAfterAnimating);
    }
    requestFrame();
}

void TransitionRunner::onAfterAnimating()
{
    if (!active_) {
        return;
    }
    emit frameDue();
    // The item is still dirty from this frame, so its update() alone does not schedule the next one.
    // A runningChanged handler may have started a new run, so the run is checked again here.
    requestFrame();
}

void TransitionRunner::requestFrame()
{
    if (active_ && window_) {
        window_->update();
    }
}

} // namespace QAccelPlot
