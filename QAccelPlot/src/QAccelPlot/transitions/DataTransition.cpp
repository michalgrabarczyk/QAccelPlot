//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/transitions/DataTransition.hpp"

#include <algorithm>
#include <utility>

namespace QAccelPlot {

DataTransition::Run::~Run()
{
    detach();
}

bool DataTransition::Run::active() const
{
    return transition_ != nullptr;
}

bool DataTransition::Run::pending() const
{
    return pending_;
}

const std::vector<double>& DataTransition::Run::targetData() const
{
    return to_.values;
}

int DataTransition::Run::targetPointCount() const
{
    return to_.count;
}

bool DataTransition::Run::finish(std::vector<double>& outData, int& outPointCount)
{
    if (!pending_) {
        return false;
    }
    outData = std::move(to_.values);
    outPointCount = to_.count;
    cancel();
    return true;
}

void DataTransition::Run::cancel()
{
    // Cleared before detaching, so runningChanged handlers see the run as ended.
    pending_ = false;
    from_.values.clear();
    to_.values.clear();
    from_.count = 0;
    to_.count = 0;
    detach();
}

void DataTransition::Run::detach()
{
    if (auto* transition = std::exchange(transition_, nullptr)) {
        transition->removeRun(this);
    }
}

DataTransition::DataTransition(QObject* parent)
    : QObject(parent)
{
}

DataTransition::~DataTransition()
{
    for (auto* run : std::as_const(runs_)) {
        run->transition_ = nullptr;
    }
}

int DataTransition::duration() const
{
    return duration_;
}

void DataTransition::setDuration(const int duration)
{
    if (duration_ == duration) {
        return;
    }
    duration_ = duration;
    emit durationChanged();
}

QEasingCurve DataTransition::easing() const
{
    return easing_;
}

void DataTransition::setEasing(const QEasingCurve& easing)
{
    if (easing_ == easing) {
        return;
    }
    easing_ = easing;
    emit easingChanged();
}

bool DataTransition::enabled() const
{
    return enabled_;
}

void DataTransition::setEnabled(const bool enabled)
{
    if (enabled_ == enabled) {
        return;
    }
    enabled_ = enabled;
    emit enabledChanged();
}

bool DataTransition::running() const
{
    return running_;
}

void DataTransition::start(
    Run& run, const std::vector<double>& currentData, const int currentPointCount, std::vector<double>&& newData, const int newPointCount, const int stride)
{
    Q_ASSERT(stride > 0);
    if (run.transition_ != this) {
        run.detach();
        run.transition_ = this;
        runs_.append(&run);
    }
    run.pending_ = true;
    run.from_.values = currentData;
    run.from_.count = currentPointCount;
    run.from_.stride = stride;
    run.to_.values = std::move(newData);
    run.to_.count = newPointCount;
    run.to_.stride = stride;
    run.timer_.start();
    setRunning(true);
}

void DataTransition::cancel()
{
    for (auto* run : std::as_const(runs_)) {
        run->transition_ = nullptr;
    }
    runs_.clear();
    setRunning(false);
}

bool DataTransition::advance(Run& run, std::vector<double>& outData, int& outPointCount)
{
    if (run.transition_ != this) {
        return false;
    }

    const auto elapsed = run.timer_.elapsed();
    const auto dur = std::max(duration_, 1);
    const auto progress = std::clamp(static_cast<qreal>(elapsed) / dur, 0.0, 1.0);
    if (progress >= 1.0) {
        run.finish(outData, outPointCount);
        return false;
    }

    // The frame borrows the caller's buffer, so its capacity is reused from frame to frame.
    auto frame = Dataset{std::move(outData), outPointCount, run.to_.stride};
    interpolate(easing_.valueForProgress(progress), run.from_, run.to_, frame);
    outData = std::move(frame.values);
    outPointCount = frame.count;
    return true;
}

void DataTransition::removeRun(Run* run)
{
    runs_.removeOne(run);
    setRunning(!runs_.isEmpty());
}

void DataTransition::setRunning(const bool running)
{
    if (running_ == running) {
        return;
    }
    running_ = running;
    emit runningChanged();
}

} // namespace QAccelPlot
