//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <complex>
#include <vector>

namespace QAccelPlotExample {

/// \brief Amplitude spectra of real signals from a Hann-windowed radix-2 FFT.
///
/// A sine of amplitude A centered on a bin reads A in that bin. Not thread-safe: use one analyzer per thread.
class SpectrumAnalyzer {
public:
    /// \brief Creates an analyzer for \a fftSize samples, which must be a power of two of at least 2.
    explicit SpectrumAnalyzer(int fftSize);

    /// \brief Returns the number of samples per transform.
    int fftSize() const;

    /// \brief Writes the amplitudes of the first \a binCount bins of the \c fftSize() samples at \a samples.
    void amplitudes(const float* samples, int binCount, float* amplitudes);

    /// \brief Like \c amplitudes(), averaged over the segments that start every \a hop samples within \a count samples.
    ///
    /// Averages power, as Welch's method does, which smooths the noise floor.
    void averagedAmplitudes(const float* samples, int count, int hop, int binCount, float* amplitudes);

private:
    void transform(const float* samples);

    int fftSize_;
    std::vector<float> window_;
    std::vector<int> bitReversed_;
    std::vector<std::complex<float>> twiddles_;
    std::vector<std::complex<float>> buffer_;
    std::vector<float> power_;
    float amplitudeScale_;
};

/// \brief Short-time spectra of one signal, kept as a scrolling grid of levels in decibels.
///
/// Each spectrum covers one block of samples. The grid holds the newest \c spectrumCount()
/// spectra in a ring buffer; appending a spectrum drops the oldest.
class Spectrogram {
public:
    /// \brief Creates a grid of \a spectrumCount spectra of \a binCount frequency bins, from blocks of \a fftSize samples.
    Spectrogram(int fftSize, int binCount, int spectrumCount);

    /// \brief Returns the number of frequency bins per spectrum.
    int binCount() const;
    /// \brief Returns the number of spectra.
    int spectrumCount() const;

    /// \brief Appends the spectrum of the \c fftSize samples at \a samples as the newest one.
    void appendSpectrum(const float* samples);
    /// \brief Sets every cell to the floor level.
    void clear();
    /// \brief Writes the grid to \a values one spectrum after another, oldest first, each from the lowest frequency up.
    void copyGrid(std::vector<float>& values) const;

private:
    SpectrumAnalyzer analyzer_;
    int binCount_;
    int spectrumCount_;
    int newestSpectrum_{-1};
    // Spectrum after spectrum, binCount_ levels each.
    std::vector<float> levels_;
    std::vector<float> amplitudes_;
};

} // namespace QAccelPlotExample
