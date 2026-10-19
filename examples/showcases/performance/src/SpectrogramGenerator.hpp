//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "DatasetParts.hpp"
#include "XorShiftRandom.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace QAccelPlotExample {

/// \brief One series' share of a HeatmapSeries dataset: a band of grid rows, one float per cell.
struct SpectrogramPart : SeriesPart {
    /// \brief Number of cells per row.
    int columns{0};
    /// \brief Number of rows in the band.
    int rows{0};
    /// \brief Grid row of the band's first row, counted from the bottom.
    int firstRow{0};
};

/// \brief One HeatmapSeries dataset, split into consecutive bands of rows.
struct SpectrogramBatch {
    std::vector<SpectrogramPart> parts;
};

/// \brief Settings of a SpectrogramGenerator batch.
struct SpectrogramParameters {
    DatasetParameters dataset;
};

/// \brief Generates a scrolling spectrogram on a square grid: frequency along x, time along y.
///
/// The newest spectrum is the top row; older rows move down as time passes, and the whole grid
/// scrolls past in \c kScrollSeconds whatever its size. Each spectrum holds a noise floor, three
/// tones drifting in frequency, and a chirp sweeping the band. Values are in [0, 1], shaped like
/// decibels: a peak falls off as a parabola around its frequency. The generator keeps the spectra
/// in a ring buffer and synthesizes only the rows that are new since the previous batch, so a
/// batch mostly costs a copy of the grid.
class SpectrogramGenerator final {
public:
    using Batch = SpectrogramBatch;
    using Parameters = SpectrogramParameters;

    /// \brief Time, in seconds, a row takes to scroll from the top of the grid to the bottom.
    static constexpr auto kScrollSeconds = 10.0;

    /// \brief Fills \a batch with the spectrogram described by \a parameters at \a timeSeconds.
    void generate(Batch& batch, const Parameters& parameters, double timeSeconds);

    /// \brief Returns the number of rows and columns of the square grid for \a cellCount cells.
    static int gridSide(int cellCount);

private:
    void updateHistory(int side, std::int64_t newestLine);
    void writeLine(std::int64_t line);
    void copyRows(Batch& batch, const Parameters& parameters) const;
    std::size_t lineOffset(std::int64_t line) const;

    std::vector<float> history_; // ring buffer of side spectra, side values each
    int side_{0};
    std::int64_t newestLine_{0};
    XorShiftRandom random_;
};

} // namespace QAccelPlotExample
