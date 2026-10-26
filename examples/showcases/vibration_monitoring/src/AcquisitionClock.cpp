//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "AcquisitionClock.hpp"

namespace QAccelPlotExample {

RealTimeClock::RealTimeClock()
    : resumedAt_(Clock::now())
{
}

double RealTimeClock::now() const
{
    const auto lock = std::lock_guard<std::mutex>{mutex_};
    return paused_ ? secondsBeforeResume_ : runningSeconds(Clock::now());
}

void RealTimeClock::setPaused(const bool paused)
{
    const auto lock = std::lock_guard<std::mutex>{mutex_};
    if (paused == paused_) {
        return;
    }
    const auto now = Clock::now();
    if (paused) {
        secondsBeforeResume_ = runningSeconds(now);
    } else {
        resumedAt_ = now;
    }
    paused_ = paused;
}

double RealTimeClock::runningSeconds(const Clock::time_point now) const
{
    return secondsBeforeResume_ + std::chrono::duration<double>(now - resumedAt_).count();
}

double SteppedClock::now() const
{
    return seconds_.load(std::memory_order_acquire);
}

void SteppedClock::setTime(const double seconds)
{
    seconds_.store(seconds, std::memory_order_release);
}

} // namespace QAccelPlotExample
