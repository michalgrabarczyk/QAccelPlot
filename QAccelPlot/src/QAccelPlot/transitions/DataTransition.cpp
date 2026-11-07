//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/transitions/DataTransition.hpp"

#include <QAbstractAnimation>

#include <algorithm>
#include <utility>

namespace QAccelPlot {

namespace {

class AnimationClock final : public QAbstractAnimation {
public:
    using QAbstractAnimation::QAbstractAnimation;

    int duration() const override
    {
        return -1;
    }

protected:
    void updateCurrentTime([[maybe_unused]] int currentTime) override
    {
    }
};

} // namespace

DataTransition::Run::~Run()
{
    end();
}

bool DataTransition::Run::active() const
{
    return transition_ != nullptr;
}

DataTransition* DataTransition::Run::transition() const
{
    return transition_;
}

void DataTransition::Run::end()
{
    if (auto* transition = std::exchange(transition_, nullptr)) {
        transition->removeRun(this);
    }
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

qreal DataTransition::stagger() const
{
    return stagger_;
}

void DataTransition::setStagger(const qreal stagger)
{
    const auto clamped = std::clamp(stagger, 0.0, 1.0);
    if (stagger_ == clamped) {
        return;
    }
    stagger_ = clamped;
    emit staggerChanged();
}

void DataTransition::start(Run& run)
{
    if (run.transition_ != this) {
        run.end();
        run.transition_ = this;
        runs_.append(&run);
    }
    setRunning(true);
    run.startTime_ = clock_->currentTime();
}

void DataTransition::cancel()
{
    for (auto* run : std::as_const(runs_)) {
        run->transition_ = nullptr;
    }
    runs_.clear();
    setRunning(false);
}

double DataTransition::progress(const Run& run) const
{
    if (run.transition_ != this || duration_ <= 0) {
        return 1.0;
    }
    const auto elapsed = clock_->currentTime() - run.startTime_;
    return std::clamp(static_cast<double>(elapsed) / duration_, 0.0, 1.0);
}

DataTransition::DataTransition(QObject* parent)
    : QObject(parent)
    , clock_(new AnimationClock(this))
{
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
    // Runs measure time from their start, so the clock restarts from zero for each running phase.
    if (running) {
        clock_->start();
    } else {
        clock_->stop();
    }
    emit runningChanged();
}

} // namespace QAccelPlot
