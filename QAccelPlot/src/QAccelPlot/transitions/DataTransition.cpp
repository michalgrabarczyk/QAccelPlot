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
    return toData_;
}

int DataTransition::Run::targetPointCount() const
{
    return toPointCount_;
}

bool DataTransition::Run::finish(std::vector<double>& outData, int& outPointCount)
{
    if (!pending_) {
        return false;
    }
    outData = std::move(toData_);
    outPointCount = toPointCount_;
    cancel();
    return true;
}

void DataTransition::Run::cancel()
{
    // Cleared before detaching, so runningChanged handlers see the run as ended.
    pending_ = false;
    fromData_.clear();
    toData_.clear();
    fromPointCount_ = 0;
    toPointCount_ = 0;
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
    Run& run, const std::vector<double>& currentData, const int currentPointCount, std::vector<double>&& newData, const int newPointCount)
{
    if (run.transition_ != this) {
        run.detach();
        run.transition_ = this;
        runs_.append(&run);
    }
    run.pending_ = true;
    run.fromData_ = currentData;
    run.fromPointCount_ = currentPointCount;
    run.toData_ = std::move(newData);
    run.toPointCount_ = newPointCount;
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

    interpolate(easing_.valueForProgress(progress), run.fromData_, run.fromPointCount_, run.toData_, run.toPointCount_, outData, outPointCount);
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
