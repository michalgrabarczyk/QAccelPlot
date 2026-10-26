//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "AcquisitionClock.hpp"
#include "AcquisitionWorker.hpp"

#include <QObject>
#include <QStringList>
#include <QTimer>

#include <functional>
#include <memory>

class QQuickWindow;

namespace QAccelPlotExample {

/// \brief Connects the dashboard QML to the acquisition: channel facts, selection, hold, and frame timing.
///
/// The QML receives it as its \c monitor property. With a real-time clock, every animation frame of
/// the window requests a new acquisition frame. With a stepped clock, frames are shown one at a time
/// through \c showFrameAt(), for recordings and screenshots.
class MonitorController final : public QObject {
    Q_OBJECT
    /// \brief Read-only constant: channel names, from the motor drive end to the base plate.
    Q_PROPERTY(QStringList channelNames READ channelNames CONSTANT)
    /// \brief Read-only constant: samples per second of every channel.
    Q_PROPERTY(int sampleRate READ sampleRate CONSTANT)
    /// \brief Read-only constant: length of the scrolling waveforms in seconds.
    Q_PROPERTY(int windowSeconds READ windowSeconds CONSTANT)
    /// \brief Read-only constant: number of waveform points replaced every frame.
    Q_PROPERTY(int pointCount READ pointCount CONSTANT)
    /// \brief Read-only constant: index of the channel with the bearing fault.
    Q_PROPERTY(int faultChannel READ faultChannel CONSTANT)
    /// \brief Read-only constant: ball pass frequency of the faulty bearing's outer race, in hertz.
    Q_PROPERTY(double bpfoHz READ bpfoHz CONSTANT)
    /// \brief Read-only constant: highest frequency of the spectrum and the spectrogram, in hertz.
    Q_PROPERTY(double maxFrequencyHz READ maxFrequencyHz CONSTANT)
    /// \brief Read-only constant: width of a spectrogram frequency bin, in hertz.
    Q_PROPERTY(double spectrogramBinHz READ spectrogramBinHz CONSTANT)
    /// \brief Read-only constant: whether time advances in fixed steps, for a recording or a screenshot.
    Q_PROPERTY(bool recording READ recording CONSTANT)
    /// \brief Read-only constant: whether the scripted demo drives the cursor and the zoom.
    Q_PROPERTY(bool demo READ demo CONSTANT)
    /// \brief Channel whose spectrum and spectrogram are shown.
    Q_PROPERTY(int selectedChannel READ selectedChannel WRITE setSelectedChannel NOTIFY selectedChannelChanged)
    /// \brief Whether the acquisition is on hold. Applies to the real-time clock only.
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
    /// \brief Read-only: acquisition time in seconds of the data on screen.
    Q_PROPERTY(double dataTime READ dataTime NOTIFY dataTimeChanged)
    /// \brief Read-only: frames per second that brought new data, measured over the last second.
    Q_PROPERTY(int framesPerSecond READ framesPerSecond NOTIFY framesPerSecondChanged)

public:
    /// \brief How acquisition time advances.
    enum class ClockMode {
        RealTime, ///< \brief Time follows the wall clock and can be paused.
        Stepped   ///< \brief Time moves only through \c showFrameAt().
    };

    /// \brief Creates a controller whose time advances as \a clockMode says; \a demo runs the scripted demo.
    MonitorController(ClockMode clockMode, bool demo, QObject* parent = nullptr);
    /// \brief Stops the acquisition.
    ~MonitorController() override;

    /// \brief Finds the dashboard series in \a window and starts the acquisition. Returns false when a series is missing.
    bool attach(QQuickWindow* window);
    /// \brief Stops the acquisition thread. The series keep their last data.
    void stop();
    /// \brief Acquires up to \a seconds and calls \a ready once the window holds that frame. Stepped clock only.
    void showFrameAt(double seconds, std::function<void()> ready);

    /// \brief Returns the acceleration in g that the waveform plot's Y value \a plotY shows for \a channel.
    Q_INVOKABLE double acceleration(int channel, double plotY) const;
    /// \brief Returns the waveform plot's Y value at the center of \a channel.
    Q_INVOKABLE double channelOffset(int channel) const;

    QStringList channelNames() const;
    int sampleRate() const;
    int windowSeconds() const;
    int pointCount() const;
    int faultChannel() const;
    double bpfoHz() const;
    double maxFrequencyHz() const;
    double spectrogramBinHz() const;
    bool recording() const;
    bool demo() const;

    int selectedChannel() const;
    /// \brief Selects \a channel; out-of-range values are clamped.
    void setSelectedChannel(int channel);
    bool paused() const;
    /// \brief Holds or resumes the real-time clock.
    void setPaused(bool paused);
    double dataTime() const;
    int framesPerSecond() const;

signals:
    void selectedChannelChanged();
    void pausedChanged();
    void dataTimeChanged();
    void framesPerSecondChanged();

private:
    const AcquisitionClock& clock() const;
    void onFrameDelivered(double dataTime);
    void updateFramesPerSecond();

    ClockMode clockMode_;
    bool demo_;
    RealTimeClock realTimeClock_;
    SteppedClock steppedClock_;
    std::unique_ptr<AcquisitionWorker> worker_;
    std::function<void()> frameReady_;
    QTimer framesPerSecondTimer_;
    int selectedChannel_{VibrationModel::kFaultChannel};
    bool paused_{false};
    double dataTime_{0.0};
    int framesPerSecond_{0};
    int framesThisSecond_{0};
};

} // namespace QAccelPlotExample
