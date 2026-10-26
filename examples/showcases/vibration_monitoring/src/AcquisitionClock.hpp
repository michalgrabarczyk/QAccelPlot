//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <atomic>
#include <chrono>
#include <mutex>

namespace QAccelPlotExample {

/// \brief Time source of the acquisition, in seconds since it started. Thread-safe.
class AcquisitionClock {
public:
    AcquisitionClock() = default;
    virtual ~AcquisitionClock() = default;
    AcquisitionClock(const AcquisitionClock&) = delete;
    AcquisitionClock& operator=(const AcquisitionClock&) = delete;

    /// \brief Returns the current acquisition time in seconds.
    virtual double now() const = 0;
};

/// \brief Wall-clock time that can be held: while paused, time stands still and later resumes where it stopped.
class RealTimeClock final : public AcquisitionClock {
public:
    /// \brief Starts the clock at zero.
    RealTimeClock();

    /// \brief Returns the seconds elapsed since construction, minus the time spent paused.
    double now() const override;
    /// \brief Holds or resumes the clock.
    void setPaused(bool paused);

private:
    using Clock = std::chrono::steady_clock;

    double runningSeconds(Clock::time_point now) const;

    mutable std::mutex mutex_;
    Clock::time_point resumedAt_;
    double secondsBeforeResume_{0.0};
    bool paused_{false};
};

/// \brief Time that only moves when set, for frame-by-frame recording with a fixed step per frame.
class SteppedClock final : public AcquisitionClock {
public:
    /// \brief Returns the time set last; zero initially.
    double now() const override;
    /// \brief Sets the time to \a seconds.
    void setTime(double seconds);

private:
    std::atomic<double> seconds_{0.0};
};

} // namespace QAccelPlotExample
