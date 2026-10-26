//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "AcquisitionWorker.hpp"

#include <QAccelPlot/series/BandSeries.hpp>
#include <QAccelPlot/series/BarSeries.hpp>
#include <QAccelPlot/series/HeatmapSeries.hpp>
#include <QAccelPlot/series/LineCurve.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <numeric>
#include <utility>

namespace QAccelPlotExample {

namespace {

constexpr auto kChannelCount = VibrationModel::kChannelCount;
constexpr auto kSampleRate = VibrationModel::kSampleRate;
// Enough history to rebuild every spectrogram column after the selected channel changes.
constexpr auto kHistorySamples = std::int64_t{AcquisitionWorker::kWindowSamples} + AcquisitionWorker::kSpectrogramFftSize + AcquisitionWorker::kAnalysisHop;
// The buffer grows to twice the history before the newest history moves back to its start.
constexpr auto kHistoryCapacity = 2 * kHistorySamples;
constexpr auto kSpectrumBins = static_cast<int>(AcquisitionWorker::kMaxFrequencyHz * AcquisitionWorker::kSpectrumFftSize / kSampleRate) + 1;
constexpr auto kSpectrogramBins = static_cast<int>(AcquisitionWorker::kMaxFrequencyHz * AcquisitionWorker::kSpectrogramFftSize / kSampleRate) + 1;
// A channel's trace fills this much of the unit row height on each side of its center.
constexpr auto kTraceHalfHeight = 0.45;
// Acceleration in g that reaches kTraceHalfHeight, per channel.
constexpr auto kFullScale = std::array<double, kChannelCount>{0.5, 0.35, 0.6, 0.6, 3.5, 1.6, 0.3, 0.12};

// A channel's alarm envelope is the highest spectrum of the healthy machine over one period,
// taken every half second, widened by a few bins and raised by 6 dB.
constexpr auto kBaselineStep = kSampleRate / 2;
constexpr auto kAlarmWidenBins = 2;
constexpr auto kAlarmMargin = 2.0f;
// Upper edge of the alarm zone, far above the spectrum plot.
constexpr auto kAlarmZoneTop = 1000.0f;

std::vector<float> raisedEnvelope(const std::vector<float>& highest)
{
    const auto size = static_cast<int>(highest.size());
    auto envelope = std::vector<float>(highest.size());
    for (auto bin = 0; bin < size; ++bin) {
        const auto first = highest.begin() + std::max(0, bin - kAlarmWidenBins);
        const auto last = highest.begin() + std::min(size, bin + kAlarmWidenBins + 1);
        envelope[static_cast<std::size_t>(bin)] = kAlarmMargin * *std::max_element(first, last);
    }
    return envelope;
}

std::int64_t floorDivide(const std::int64_t value, const std::int64_t divisor)
{
    const auto quotient = value / divisor;
    return (value % divisor != 0 && value < 0) ? quotient - 1 : quotient;
}

} // namespace

AcquisitionWorker::AcquisitionWorker(const AcquisitionClock& clock, AcquisitionTargets targets, FrameCallback frameDelivered)
    : clock_(clock)
    , targets_(targets)
    , frameDelivered_(std::move(frameDelivered))
    , spectrumAnalyzer_(kSpectrumFftSize)
    , spectrogram_(kSpectrogramFftSize, kSpectrogramBins, kSpectrogramSpectra)
    , spectrumAmplitudes_(static_cast<std::size_t>(kSpectrumBins))
{
    for (auto& samples : history_) {
        samples.resize(static_cast<std::size_t>(kHistoryCapacity));
    }
}

AcquisitionWorker::~AcquisitionWorker()
{
    stop();
}

void AcquisitionWorker::start()
{
    const auto lock = std::lock_guard<std::mutex>{mutex_};
    if (running_) {
        return;
    }
    running_ = true;
    thread_ = std::thread([this]() { run(); });
}

void AcquisitionWorker::stop()
{
    {
        const auto lock = std::lock_guard<std::mutex>{mutex_};
        running_ = false;
    }
    wake_.notify_all();
    if (thread_.joinable()) {
        thread_.join();
    }
}

void AcquisitionWorker::requestFrame()
{
    {
        const auto lock = std::lock_guard<std::mutex>{mutex_};
        frameRequested_ = true;
    }
    wake_.notify_all();
}

void AcquisitionWorker::acknowledgeFrame()
{
    {
        const auto lock = std::lock_guard<std::mutex>{mutex_};
        frameInFlight_ = false;
    }
    wake_.notify_all();
}

void AcquisitionWorker::setSelectedChannel(const int channel)
{
    selectedChannel_.store(std::clamp(channel, 0, kChannelCount - 1), std::memory_order_relaxed);
}

double AcquisitionWorker::channelOffset(const int channel)
{
    return static_cast<double>(kChannelCount - 1 - channel);
}

double AcquisitionWorker::acceleration(const int channel, const double plotY)
{
    const auto index = static_cast<std::size_t>(std::clamp(channel, 0, kChannelCount - 1));
    return (plotY - channelOffset(channel)) / kTraceHalfHeight * kFullScale.at(index);
}

void AcquisitionWorker::run()
{
    computeAlarmEnvelopes();
    while (true) {
        {
            auto lock = std::unique_lock<std::mutex>{mutex_};
            wake_.wait(lock, [this]() { return !running_ || (frameRequested_ && !frameInFlight_); });
            if (!running_) {
                return;
            }
            frameRequested_ = false;
            frameInFlight_ = true;
        }
        produceFrame();
    }
}

void AcquisitionWorker::computeAlarmEnvelopes()
{
    // The healthy machine's spectra are taken like the live one: one second of samples each.
    auto samples = std::vector<float>(static_cast<std::size_t>(VibrationModel::kPeriodSamples + kSampleRate));
    auto spectrum = std::vector<float>(static_cast<std::size_t>(kSpectrumBins));
    for (auto channel = 0; channel < kChannelCount; ++channel) {
        model_.generate(channel, 0, static_cast<int>(samples.size()), samples.data(), VibrationModel::Condition::Healthy);
        auto highest = std::vector<float>(static_cast<std::size_t>(kSpectrumBins), 0.0f);
        for (auto start = std::int64_t{0}; start < VibrationModel::kPeriodSamples; start += kBaselineStep) {
            spectrumAnalyzer_.averagedAmplitudes(samples.data() + start, kSampleRate, kSpectrumFftSize / 2, kSpectrumBins, spectrum.data());
            std::transform(highest.begin(), highest.end(), spectrum.begin(), highest.begin(), [](const float a, const float b) { return std::max(a, b); });
        }
        alarmEnvelopes_.at(static_cast<std::size_t>(channel)) = raisedEnvelope(highest);
    }
}

void AcquisitionWorker::postAlarmEnvelope(const int channel) const
{
    const auto& envelope = alarmEnvelopes_.at(static_cast<std::size_t>(channel));
    const auto binHz = static_cast<float>(kSampleRate) / static_cast<float>(kSpectrumFftSize);
    // (frequency, limit, top) per bin: the band fills everything above the limit.
    auto band = std::vector<float>(envelope.size() * 3);
    for (auto bin = std::size_t{0}; bin < envelope.size(); ++bin) {
        band[bin * 3] = static_cast<float>(bin) * binHz;
        band[bin * 3 + 1] = envelope[bin];
        band[bin * 3 + 2] = kAlarmZoneTop;
    }
    targets_.alarmEnvelope->postData(std::move(band), static_cast<int>(envelope.size()));
}

void AcquisitionWorker::produceFrame()
{
    const auto endSample = static_cast<std::int64_t>(std::floor(clock_.now() * kSampleRate));
    const auto firstFrame = analyzedChannel_ < 0;
    const auto advanced = firstFrame || endSample != historyStart_ + historyCount_;
    acquire(endSample);
    if (advanced) {
        postWaveforms();
    }

    const auto channel = selectedChannel_.load(std::memory_order_relaxed);
    const auto tick = floorDivide(endSample, kAnalysisHop);
    if (firstFrame || channel != analyzedChannel_ || tick != analyzedTick_) {
        const auto rebuild = firstFrame || channel != analyzedChannel_ || tick < analyzedTick_ || tick - analyzedTick_ >= kSpectrogramSpectra;
        if (channel != analyzedChannel_) {
            postAlarmEnvelope(channel);
        }
        updateSpectrogram(channel, tick, rebuild);
        postAnalysis(channel, tick);
        analyzedTick_ = tick;
        analyzedChannel_ = channel;
    }
    frameDelivered_(static_cast<double>(endSample) / kSampleRate);
}

void AcquisitionWorker::acquire(const std::int64_t endSample)
{
    const auto historyEnd = historyStart_ + historyCount_;
    const auto newSamples = endSample - historyEnd;
    if (historyCount_ == 0 || newSamples < 0 || newSamples > kHistorySamples) {
        historyStart_ = endSample - kHistorySamples;
        historyCount_ = kHistorySamples;
        for (auto channel = 0; channel < kChannelCount; ++channel) {
            model_.generate(channel, historyStart_, static_cast<int>(kHistorySamples), history_.at(static_cast<std::size_t>(channel)).data());
        }
        return;
    }
    if (historyCount_ + newSamples > kHistoryCapacity) {
        const auto dropped = historyCount_ + newSamples - kHistorySamples;
        for (auto& samples : history_) {
            std::memmove(samples.data(), samples.data() + dropped, static_cast<std::size_t>(historyCount_ - dropped) * sizeof(float));
        }
        historyStart_ += dropped;
        historyCount_ -= dropped;
    }
    for (auto channel = 0; channel < kChannelCount; ++channel) {
        auto* destination = history_.at(static_cast<std::size_t>(channel)).data() + historyCount_;
        model_.generate(channel, historyStart_ + historyCount_, static_cast<int>(newSamples), destination);
    }
    historyCount_ += newSamples;
}

const float* AcquisitionWorker::recentSamples(const int channel, const std::int64_t endSample, const int count) const
{
    return history_.at(static_cast<std::size_t>(channel)).data() + (endSample - count - historyStart_);
}

void AcquisitionWorker::postWaveforms() const
{
    const auto endSample = historyStart_ + historyCount_;
    const auto sampleSeconds = 1.0f / static_cast<float>(kSampleRate);
    for (auto channel = 0; channel < kChannelCount; ++channel) {
        const auto* samples = recentSamples(channel, endSample, kWindowSamples);
        const auto offset = static_cast<float>(channelOffset(channel));
        const auto scale = static_cast<float>(kTraceHalfHeight / kFullScale.at(static_cast<std::size_t>(channel)));
        // X is the time relative to the newest sample, so the X values stay the same as the trace scrolls.
        auto points = std::vector<float>(static_cast<std::size_t>(kWindowSamples) * 2);
        for (auto i = 0; i < kWindowSamples; ++i) {
            points[static_cast<std::size_t>(i) * 2] = static_cast<float>(i - kWindowSamples) * sampleSeconds;
            points[static_cast<std::size_t>(i) * 2 + 1] = offset + scale * samples[i];
        }
        targets_.waveforms.at(static_cast<std::size_t>(channel))->postData(std::move(points), kWindowSamples);
    }
}

void AcquisitionWorker::updateSpectrogram(const int channel, const std::int64_t tick, const bool rebuild)
{
    if (rebuild) {
        spectrogram_.clear();
    }
    const auto firstTick = rebuild ? tick - kSpectrogramSpectra + 1 : analyzedTick_ + 1;
    for (auto spectrumTick = firstTick; spectrumTick <= tick; ++spectrumTick) {
        spectrogram_.appendSpectrum(recentSamples(channel, spectrumTick * kAnalysisHop, kSpectrogramFftSize));
    }
}

void AcquisitionWorker::postAnalysis(const int channel, const std::int64_t tick)
{
    const auto endSample = tick * kAnalysisHop;
    spectrumAnalyzer_.averagedAmplitudes(
        recentSamples(channel, endSample, kSampleRate), kSampleRate, kSpectrumFftSize / 2, kSpectrumBins, spectrumAmplitudes_.data());
    const auto binHz = static_cast<float>(kSampleRate) / static_cast<float>(kSpectrumFftSize);
    auto spectrum = std::vector<float>(static_cast<std::size_t>(kSpectrumBins) * 2);
    for (auto bin = 0; bin < kSpectrumBins; ++bin) {
        spectrum[static_cast<std::size_t>(bin) * 2] = static_cast<float>(bin) * binHz;
        spectrum[static_cast<std::size_t>(bin) * 2 + 1] = spectrumAmplitudes_[static_cast<std::size_t>(bin)];
    }
    targets_.spectrum->postData(std::move(spectrum), kSpectrumBins);

    auto grid = std::vector<float>{};
    spectrogram_.copyGrid(grid);
    // Frequency runs along X and time along Y: one heatmap row per spectrum, oldest at the bottom.
    targets_.spectrogram->postValues(std::move(grid), spectrogram_.binCount(), spectrogram_.spectrumCount());

    // RMS over the last second, as (position, value) bars at the channel offsets.
    auto bars = std::vector<float>(static_cast<std::size_t>(kChannelCount) * 2);
    for (auto bar = 0; bar < kChannelCount; ++bar) {
        const auto* samples = recentSamples(bar, endSample, kSampleRate);
        const auto sumOfSquares = std::inner_product(samples, samples + kSampleRate, samples, 0.0);
        bars[static_cast<std::size_t>(bar) * 2] = static_cast<float>(channelOffset(bar));
        bars[static_cast<std::size_t>(bar) * 2 + 1] = static_cast<float>(std::sqrt(sumOfSquares / kSampleRate));
    }
    targets_.rms->postData(std::move(bars), kChannelCount);
}

} // namespace QAccelPlotExample
