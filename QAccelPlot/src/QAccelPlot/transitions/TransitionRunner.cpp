//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/transitions/TransitionRunner.hpp"

#include <QQuickItem>
#include <QQuickWindow>

#include <utility>

namespace QAccelPlot {

TransitionRunner::TransitionRunner(QQuickItem* item)
{
    connect(item, &QQuickItem::windowChanged, this, &TransitionRunner::connectWindow);
    // A parent already in a window adds the item to it from the QQuickItem constructor.
    connectWindow(item->window());
}

TransitionRunner::~TransitionRunner()
{
    // Ending the run emits runningChanged, which must not reach this partly destroyed object.
    if (transition_) {
        disconnect(transition_, nullptr, this, nullptr);
    }
    run_.cancel();
}

DataTransition* TransitionRunner::transition() const
{
    return transition_;
}

bool TransitionRunner::setTransition(DataTransition* transition)
{
    if (transition_ == transition) {
        return false;
    }
    interrupt();
    if (transition_) {
        disconnect(transition_, &QObject::destroyed, this, &TransitionRunner::onTransitionDestroyed);
        disconnect(transition_, &DataTransition::runningChanged, this, &TransitionRunner::onRunningChanged);
    }
    transition_ = transition;
    if (transition_) {
        connect(transition_, &QObject::destroyed, this, &TransitionRunner::onTransitionDestroyed);
        connect(transition_, &DataTransition::runningChanged, this, &TransitionRunner::onRunningChanged);
    }
    return true;
}

bool TransitionRunner::enabled() const
{
    return transition_ && transition_->enabled();
}

bool TransitionRunner::active() const
{
    return run_.active();
}

bool TransitionRunner::pending() const
{
    return run_.pending();
}

const std::vector<double>& TransitionRunner::targetData() const
{
    return run_.targetData();
}

int TransitionRunner::targetPointCount() const
{
    return run_.targetPointCount();
}

void TransitionRunner::start(
    const std::vector<double>& currentData, const int currentPointCount, std::vector<double>&& newData, const int newPointCount, const int stride)
{
    Q_ASSERT(transition_);
    if (!transition_) {
        return;
    }
    transition_->start(run_, currentData, currentPointCount, std::move(newData), newPointCount, stride);
    requestFrame();
}

bool TransitionRunner::advance(std::vector<double>& outData, int& outPointCount)
{
    return transition_ && transition_->advance(run_, outData, outPointCount);
}

bool TransitionRunner::finish(std::vector<double>& outData, int& outPointCount)
{
    return run_.finish(outData, outPointCount);
}

void TransitionRunner::cancel()
{
    run_.cancel();
}

void TransitionRunner::interrupt()
{
    if (!run_.pending()) {
        return;
    }
    emit interrupted();
    run_.cancel();
}

void TransitionRunner::onTransitionDestroyed()
{
    interrupt();
    emit transitionDestroyed();
}

void TransitionRunner::onRunningChanged()
{
    // DataTransition::cancel() ends every run but leaves its data pending for the host.
    if (!run_.active()) {
        interrupt();
    }
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
    if (!transition_ || !run_.active()) {
        return;
    }
    emit frameDue();
    // The item is still dirty from this frame, so its update() alone does not schedule the next one.
    // A runningChanged handler may have started a new run, so the run is checked again here.
    requestFrame();
}

void TransitionRunner::requestFrame()
{
    if (run_.active() && window_) {
        window_->update();
    }
}

} // namespace QAccelPlot
