//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "AcquisitionClock.hpp"
#include "SpectrumAnalyzer.hpp"
#include "VibrationModel.hpp"

#include <array>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace QAccelPlot {
class BandSeries;
class BarSeries;
class HeatmapSeries;
class LineCurve;
} // namespace QAccelPlot

namespace QAccelPlotExample {

/// \brief Series that the acquisition feeds. They must outlive the worker's thread.
struct AcquisitionTargets {
    std::array<QAccelPlot::LineCurve*, VibrationModel::kChannelCount> waveforms{}; ///< \brief One scrolling waveform per channel.
    QAccelPlot::LineCurve* spectrum{nullptr};                                      ///< \brief Spectrum of the selected channel.
    QAccelPlot::HeatmapSeries* spectrogram{nullptr};                               ///< \brief Spectrogram of the selected channel.
    QAccelPlot::BarSeries* rms{nullptr};                                           ///< \brief RMS level of every channel.
    QAccelPlot::BandSeries* alarmEnvelope{nullptr};                                ///< \brief Spectral alarm limits of the selected channel.
};

/// \brief Acquires the channels on a background thread and hands the series their data.
///
/// For every requested frame the worker reads the clock, generates the samples acquired since the
/// previous frame, and posts the last \c kWindowSeconds of every channel to its waveform. At each
/// analysis tick, ten per second, it also posts the spectrum and spectrogram of the selected channel
/// and the RMS levels of all channels. The data goes through \c postData() and \c postValues(), so
/// the worker never touches an item; \a frameDelivered then runs on the worker thread with the time
/// the posted data shows. A new frame starts only after \c acknowledgeFrame(), so at most one frame
/// of data waits in the UI thread's event queue.
///
/// Before the first frame, the worker derives each channel's spectral alarm envelope from the
/// healthy machine: the highest one-second spectrum over a period, widened by a few bins and raised
/// by 6 dB. It posts the selected channel's envelope whenever the selection changes.
class AcquisitionWorker {
public:
    /// \brief Length of the scrolling waveforms.
    static constexpr int kWindowSeconds = 10;
    /// \brief Samples per waveform.
    static constexpr int kWindowSamples = kWindowSeconds * VibrationModel::kSampleRate;
    /// \brief Samples between analysis ticks.
    static constexpr int kAnalysisHop = VibrationModel::kSampleRate / 10;
    /// \brief Highest frequency of the spectrum and the spectrogram.
    static constexpr double kMaxFrequencyHz = 1000.0;
    /// \brief Samples per spectrum segment; the spectrum averages the segments of the last second.
    static constexpr int kSpectrumFftSize = 8192;
    /// \brief Samples per spectrogram column.
    static constexpr int kSpectrogramFftSize = 4096;
    /// \brief Spectra in the spectrogram: one per analysis tick over the waveform window.
    static constexpr int kSpectrogramSpectra = kWindowSamples / kAnalysisHop;

    /// \brief Called on the worker thread after a frame's data is posted, with the time the data shows in seconds.
    using FrameCallback = std::function<void(double dataTime)>;

    /// \brief Creates a stopped worker that reads \a clock and feeds \a targets.
    AcquisitionWorker(const AcquisitionClock& clock, AcquisitionTargets targets, FrameCallback frameDelivered);
    /// \brief Stops the worker.
    ~AcquisitionWorker();
    AcquisitionWorker(const AcquisitionWorker&) = delete;
    AcquisitionWorker& operator=(const AcquisitionWorker&) = delete;

    /// \brief Starts the background thread.
    void start();
    /// \brief Stops and joins the background thread.
    void stop();
    /// \brief Asks for a frame at the clock's current time. Requests made while a frame is under way are merged. Thread-safe.
    void requestFrame();
    /// \brief Reports that the UI thread applied the last delivered frame. Thread-safe.
    void acknowledgeFrame();
    /// \brief Selects the channel whose spectrum and spectrogram are posted. Thread-safe.
    void setSelectedChannel(int channel);

    /// \brief Returns the waveform plot's Y value at the center of \a channel; the first channel is on top.
    static double channelOffset(int channel);
    /// \brief Returns the acceleration in g that the waveform plot's Y value \a plotY shows for \a channel.
    static double acceleration(int channel, double plotY);

private:
    void run();
    void computeAlarmEnvelopes();
    void postAlarmEnvelope(int channel) const;
    void produceFrame();
    void acquire(std::int64_t endSample);
    const float* recentSamples(int channel, std::int64_t endSample, int count) const;
    void postWaveforms() const;
    void updateSpectrogram(int channel, std::int64_t tick, bool rebuild);
    void postAnalysis(int channel, std::int64_t tick);

    const AcquisitionClock& clock_;
    AcquisitionTargets targets_;
    FrameCallback frameDelivered_;
    VibrationModel model_;
    SpectrumAnalyzer spectrumAnalyzer_;
    Spectrogram spectrogram_;
    std::vector<float> spectrumAmplitudes_;
    // Alarm limit per spectrum bin, for each channel.
    std::array<std::vector<float>, VibrationModel::kChannelCount> alarmEnvelopes_;

    // The newest samples of each channel; history_[c][0] is sample historyStart_.
    std::array<std::vector<float>, VibrationModel::kChannelCount> history_;
    std::int64_t historyStart_{0};
    std::int64_t historyCount_{0};
    std::int64_t analyzedTick_{0};
    int analyzedChannel_{-1};

    std::mutex mutex_;
    std::condition_variable wake_;
    std::thread thread_;
    bool running_{false};
    bool frameRequested_{false};
    bool frameInFlight_{false};
    std::atomic<int> selectedChannel_{VibrationModel::kFaultChannel};
};

} // namespace QAccelPlotExample
