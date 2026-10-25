//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "SpectrumAnalyzer.hpp"

#include <QtGlobal>

#include <algorithm>
#include <cmath>
#include <numeric>

namespace QAccelPlotExample {

namespace {

constexpr auto kTwoPi = 6.283185307179586476925286766559;
// Level of an empty spectrogram cell; also the lowest level a cell reports.
constexpr auto kFloorDecibels = -100.0f;

int log2OfPowerOfTwo(const int value)
{
    auto bits = 0;
    while ((1 << bits) < value) {
        ++bits;
    }
    return bits;
}

} // namespace

SpectrumAnalyzer::SpectrumAnalyzer(const int fftSize)
    : fftSize_(fftSize)
    , window_(static_cast<std::size_t>(fftSize))
    , bitReversed_(static_cast<std::size_t>(fftSize))
    , twiddles_(static_cast<std::size_t>(fftSize / 2))
    , buffer_(static_cast<std::size_t>(fftSize))
{
    Q_ASSERT(fftSize >= 2 && (fftSize & (fftSize - 1)) == 0);
    for (auto i = 0; i < fftSize; ++i) {
        window_[static_cast<std::size_t>(i)] = static_cast<float>(0.5 * (1.0 - std::cos(kTwoPi * i / fftSize)));
    }
    // A single-sided amplitude spectrum doubles the bins and undoes the window's gain.
    amplitudeScale_ = 2.0f / std::accumulate(window_.begin(), window_.end(), 0.0f);

    const auto bits = log2OfPowerOfTwo(fftSize);
    for (auto i = 0; i < fftSize; ++i) {
        auto reversed = 0;
        for (auto bit = 0; bit < bits; ++bit) {
            reversed |= ((i >> bit) & 1) << (bits - 1 - bit);
        }
        bitReversed_[static_cast<std::size_t>(i)] = reversed;
    }
    for (auto k = 0; k < fftSize / 2; ++k) {
        twiddles_[static_cast<std::size_t>(k)] = std::polar(1.0f, static_cast<float>(-kTwoPi * k / fftSize));
    }
}

int SpectrumAnalyzer::fftSize() const
{
    return fftSize_;
}

void SpectrumAnalyzer::amplitudes(const float* samples, const int binCount, float* amplitudes)
{
    transform(samples);
    for (auto bin = 0; bin < binCount; ++bin) {
        amplitudes[bin] = std::abs(buffer_[static_cast<std::size_t>(bin)]) * amplitudeScale_;
    }
}

void SpectrumAnalyzer::averagedAmplitudes(const float* samples, const int count, const int hop, const int binCount, float* amplitudes)
{
    power_.assign(static_cast<std::size_t>(binCount), 0.0f);
    auto segments = 0;
    for (auto start = 0; start + fftSize_ <= count; start += hop) {
        transform(samples + start);
        for (auto bin = 0; bin < binCount; ++bin) {
            power_[static_cast<std::size_t>(bin)] += std::norm(buffer_[static_cast<std::size_t>(bin)]);
        }
        ++segments;
    }
    for (auto bin = 0; bin < binCount; ++bin) {
        const auto power = segments > 0 ? power_[static_cast<std::size_t>(bin)] / static_cast<float>(segments) : 0.0f;
        amplitudes[bin] = std::sqrt(power) * amplitudeScale_;
    }
}

void SpectrumAnalyzer::transform(const float* samples)
{
    for (auto i = 0; i < fftSize_; ++i) {
        const auto index = static_cast<std::size_t>(i);
        buffer_[static_cast<std::size_t>(bitReversed_[index])] = {samples[i] * window_[index], 0.0f};
    }
    // Iterative Cooley-Tukey butterflies over blocks of doubling size.
    for (auto size = 2; size <= fftSize_; size *= 2) {
        const auto half = size / 2;
        const auto twiddleStep = fftSize_ / size;
        for (auto start = 0; start < fftSize_; start += size) {
            for (auto k = 0; k < half; ++k) {
                const auto even = static_cast<std::size_t>(start) + static_cast<std::size_t>(k);
                const auto odd = even + static_cast<std::size_t>(half);
                const auto product = twiddles_[static_cast<std::size_t>(k) * static_cast<std::size_t>(twiddleStep)] * buffer_[odd];
                buffer_[odd] = buffer_[even] - product;
                buffer_[even] += product;
            }
        }
    }
}

Spectrogram::Spectrogram(const int fftSize, const int binCount, const int spectrumCount)
    : analyzer_(fftSize)
    , binCount_(binCount)
    , spectrumCount_(spectrumCount)
    , amplitudes_(static_cast<std::size_t>(binCount))
{
    clear();
}

int Spectrogram::binCount() const
{
    return binCount_;
}

int Spectrogram::spectrumCount() const
{
    return spectrumCount_;
}

void Spectrogram::appendSpectrum(const float* samples)
{
    newestSpectrum_ = (newestSpectrum_ + 1) % spectrumCount_;
    analyzer_.amplitudes(samples, binCount_, amplitudes_.data());
    auto* spectrum = levels_.data() + static_cast<std::ptrdiff_t>(newestSpectrum_) * binCount_;
    std::transform(amplitudes_.begin(), amplitudes_.end(), spectrum,
        [](const float amplitude) { return std::max(kFloorDecibels, 20.0f * std::log10(std::max(amplitude, 1e-9f))); });
}

void Spectrogram::clear()
{
    levels_.assign(static_cast<std::size_t>(binCount_) * static_cast<std::size_t>(spectrumCount_), kFloorDecibels);
    newestSpectrum_ = spectrumCount_ - 1;
}

void Spectrogram::copyGrid(std::vector<float>& values) const
{
    // The ring buffer already stores whole spectra, so unrolling it from the oldest one is enough.
    const auto oldest = static_cast<std::ptrdiff_t>((newestSpectrum_ + 1) % spectrumCount_) * binCount_;
    values.resize(levels_.size());
    std::rotate_copy(levels_.begin(), levels_.begin() + oldest, levels_.end(), values.begin());
}

} // namespace QAccelPlotExample
