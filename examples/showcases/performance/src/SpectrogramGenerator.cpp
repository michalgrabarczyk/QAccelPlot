//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "SpectrogramGenerator.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace QAccelPlotExample {
namespace {

// The noise floor is uniform in [kNoiseFloor, kNoiseFloor + kNoiseSpread].
constexpr auto kNoiseFloor = 0.08f;
constexpr auto kNoiseSpread = 0.12f;

struct Tone {
    float frequency; // center, as a share of the band
    float drift;     // amplitude of the frequency wobble, as a share of the band
    double rate;     // wobble speed, radians per second
    double phase;
    float level;
};

constexpr auto kTones = std::array<Tone, 3>{{{0.18f, 0.04f, 0.9, 0.0, 0.92f}, {0.47f, 0.08f, 0.35, 1.3, 0.8f}, {0.74f, 0.025f, 1.7, 2.1, 0.7f}}};

constexpr auto kChirpPeriodSeconds = 4.0;
constexpr auto kChirpStart = 0.05f;
constexpr auto kChirpEnd = 0.95f;
constexpr auto kChirpLevel = 0.85f;

// Distance from its center, as a share of the band, at which a peak falls to zero.
constexpr auto kPeakHalfWidth = 0.012f;
// Keeps peaks visible on small grids.
constexpr auto kMinimumPeakHalfWidthBins = 1.5f;

// Raises the bins of row around center, a share of the band, to a parabolic peak of level.
void addPeak(float* row, const int side, const float center, const float level)
{
    const auto bins = static_cast<float>(side);
    const auto halfWidth = std::max(kPeakHalfWidth, kMinimumPeakHalfWidthBins / bins);
    const auto first = std::max(0, static_cast<int>(std::ceil((center - halfWidth) * bins - 0.5f)));
    const auto last = std::min(side - 1, static_cast<int>(std::floor((center + halfWidth) * bins - 0.5f)));
    for (auto bin = first; bin <= last; ++bin) {
        const auto offset = ((static_cast<float>(bin) + 0.5f) / bins - center) / halfWidth;
        row[bin] = std::max(row[bin], level * (1.0f - offset * offset));
    }
}

} // namespace

void SpectrogramGenerator::generate(Batch& batch, const Parameters& parameters, const double timeSeconds)
{
    const auto side = gridSide(parameters.dataset.count);
    updateHistory(side, static_cast<std::int64_t>(std::floor(timeSeconds * static_cast<double>(side) / kScrollSeconds)));
    copyRows(batch, parameters);
    applyPrecision(batch.parts, parameters.dataset);
}

int SpectrogramGenerator::gridSide(const int cellCount)
{
    return std::max(1, static_cast<int>(std::lround(std::sqrt(static_cast<double>(cellCount)))));
}

void SpectrogramGenerator::updateHistory(const int side, const std::int64_t newestLine)
{
    auto firstLine = newestLine_ + 1;
    // A new shape, or a time jump past the whole grid, regenerates every row.
    if (side != side_ || newestLine < newestLine_ || newestLine - newestLine_ >= side) {
        side_ = side;
        history_.resize(static_cast<std::size_t>(side) * static_cast<std::size_t>(side));
        firstLine = newestLine - side + 1;
    }
    for (auto line = firstLine; line <= newestLine; ++line) {
        writeLine(line);
    }
    newestLine_ = newestLine;
}

void SpectrogramGenerator::writeLine(const std::int64_t line)
{
    auto* row = history_.data() + lineOffset(line);
    for (auto bin = 0; bin < side_; ++bin) {
        row[bin] = kNoiseFloor + kNoiseSpread * static_cast<float>(random_.uniform());
    }

    const auto timeSeconds = static_cast<double>(line) * kScrollSeconds / static_cast<double>(side_);
    for (const auto& tone : kTones) {
        const auto center = tone.frequency + tone.drift * static_cast<float>(std::sin(tone.rate * timeSeconds + tone.phase));
        addPeak(row, side_, center, tone.level);
    }
    const auto cycles = timeSeconds / kChirpPeriodSeconds;
    const auto sweep = static_cast<float>(cycles - std::floor(cycles));
    addPeak(row, side_, kChirpStart + (kChirpEnd - kChirpStart) * sweep, kChirpLevel);
}

void SpectrogramGenerator::copyRows(Batch& batch, const Parameters& parameters) const
{
    const auto seriesCount = parameters.dataset.seriesCount;
    const auto columns = static_cast<std::size_t>(side_);
    // Row 0 is the oldest line, at the bottom of the grid.
    const auto oldestLine = newestLine_ - side_ + 1;
    batch.parts.resize(static_cast<std::size_t>(seriesCount));
    auto firstRow = 0;
    for (auto index = 0; index < seriesCount; ++index) {
        auto& part = batch.parts[static_cast<std::size_t>(index)];
        part.columns = side_;
        part.rows = partCount(side_, seriesCount, index);
        part.firstRow = firstRow;
        part.count = part.rows * side_;
        part.floats.resize(static_cast<std::size_t>(part.count));
        part.doubles.clear();
        for (auto row = 0; row < part.rows; ++row) {
            std::copy_n(history_.data() + lineOffset(oldestLine + firstRow + row), columns, part.floats.data() + static_cast<std::size_t>(row) * columns);
        }
        firstRow += part.rows;
    }
}

std::size_t SpectrogramGenerator::lineOffset(const std::int64_t line) const
{
    const auto slot = ((line % side_) + side_) % side_;
    return static_cast<std::size_t>(slot) * static_cast<std::size_t>(side_);
}

} // namespace QAccelPlotExample
