//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "transitions/DataTransition.hpp"

#include <algorithm>

namespace QAccelPlot {

DataTransition::DataTransition(QObject* parent)
    : QObject(parent)
{
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

void DataTransition::start(const std::vector<float>& currentData, const int currentPointCount, std::vector<float>&& newData, const int newPointCount)
{
    fromData_ = currentData;
    fromPointCount_ = currentPointCount;
    toData_ = std::move(newData);
    toPointCount_ = newPointCount;

    setRunning(true);
    animTimer_.start();
}

void DataTransition::cancel()
{
    fromData_.clear();
    toData_.clear();
    fromPointCount_ = 0;
    toPointCount_ = 0;
    setRunning(false);
}

bool DataTransition::advance(std::vector<float>& outData, int& outPointCount)
{
    if (!running_) {
        return false;
    }

    const auto elapsed = animTimer_.elapsed();
    const auto dur = std::max(duration_, 1);
    const auto progress = std::clamp(static_cast<qreal>(elapsed) / dur, 0.0, 1.0);
    const auto easedProgress = static_cast<float>(easing_.valueForProgress(progress));

    interpolate(easedProgress, fromData_, fromPointCount_, toData_, toPointCount_, outData, outPointCount);

    if (progress >= 1.0) {
        outData = toData_;
        outPointCount = toPointCount_;
        fromData_.clear();
        toData_.clear();
        setRunning(false);
        return false;
    }

    return true;
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
