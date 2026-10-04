//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "PlasmaGenerator.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace QAccelPlotExample {
namespace {

constexpr auto kTwoPi = 6.283185307179586;
constexpr auto kTermsPerLine = std::size_t{5};

// Wave numbers (radians per data unit) and drift speeds (radians per second).
constexpr auto kXWave = kTwoPi / 60.0;
constexpr auto kXSpeed = 0.7;
constexpr auto kYWave = kTwoPi / 45.0;
constexpr auto kYSpeed = 0.9;
constexpr auto kRisingWave = kTwoPi / 80.0;  // along x + y
constexpr auto kRisingSpeed = 0.5;
constexpr auto kFallingWave = kTwoPi / 70.0; // along x - y
constexpr auto kFallingSpeed = 0.6;

// Tile side as a multiple of the grid pitch, from trough to crest. Above 1, crest tiles overlap.
constexpr auto kMinimumSide = 0.4f;
constexpr auto kMaximumSide = 2.0f;
// Extra crest side in data units (about 3 px at the default window size), so crests still cover
// more pixels than troughs when the grid pitch is below a pixel.
constexpr auto kCrestBoost = 0.5f;

// Maps the sum of four waves, in [-4, 4], to [0, 1] with extra contrast.
float shape(const float waveSum)
{
    const auto value = std::clamp((waveSum + 4.0f) / 8.0f, 0.0f, 1.0f);
    return value * value * (3.0f - 2.0f * value);
}

} // namespace

void PlasmaGenerator::generate(Batch& batch, const Parameters& parameters, const double timeSeconds)
{
    const auto rectangleCount = parameters.dataset.count;
    const auto columns = std::max(1, static_cast<int>(std::ceil(std::sqrt(rectangleCount * kDomainWidth / kDomainHeight))));
    const auto rows = (rectangleCount + columns - 1) / columns;
    const auto pitch = std::min(kDomainWidth / static_cast<float>(columns), kDomainHeight / static_cast<float>(rows));
    const auto originX = (kDomainWidth - static_cast<float>(columns) * pitch + pitch) * 0.5f;
    const auto originY = (kDomainHeight - static_cast<float>(rows) * pitch + pitch) * 0.5f;
    const auto grid = Grid{columns, rows, rectangleCount, pitch, originX, originY};

    updateWaveTerms(grid, timeSeconds);
    resizeParts(batch.parts, parameters.dataset, 4);
    for (auto& part : batch.parts) {
        part.categories.resize(parameters.categoryCount > 0 ? static_cast<std::size_t>(part.count) : 0);
    }
    writeTiles(batch, grid, parameters);
    applyPrecision(batch.parts, parameters.dataset);
}

float PlasmaGenerator::valueAt(const float x, const float y, const double timeSeconds)
{
    const auto waveSum = std::sin(kXWave * x + kXSpeed * timeSeconds) + std::sin(kYWave * y + kYSpeed * timeSeconds)
        + std::sin(kRisingWave * (x + y) + kRisingSpeed * timeSeconds) + std::sin(kFallingWave * (x - y) + kFallingSpeed * timeSeconds);
    return shape(static_cast<float>(waveSum));
}

float PlasmaGenerator::tileSide(const float value, const float pitch)
{
    return pitch * (kMinimumSide + (kMaximumSide - kMinimumSide) * value) + kCrestBoost * value;
}

void PlasmaGenerator::updateWaveTerms(const Grid& grid, const double timeSeconds)
{
    // Each diagonal wave splits into an x part and a y part: sin(a + b) = sin a cos b + cos a sin b.
    columnTerms_.resize(static_cast<std::size_t>(grid.columns) * kTermsPerLine);
    for (auto column = 0; column < grid.columns; ++column) {
        const auto x = static_cast<double>(grid.originX + static_cast<float>(column) * grid.pitch);
        auto* terms = columnTerms_.data() + static_cast<std::size_t>(column) * kTermsPerLine;
        terms[0] = static_cast<float>(std::sin(kXWave * x + kXSpeed * timeSeconds));
        terms[1] = static_cast<float>(std::sin(kRisingWave * x + kRisingSpeed * timeSeconds));
        terms[2] = static_cast<float>(std::cos(kRisingWave * x + kRisingSpeed * timeSeconds));
        terms[3] = static_cast<float>(std::sin(kFallingWave * x + kFallingSpeed * timeSeconds));
        terms[4] = static_cast<float>(std::cos(kFallingWave * x + kFallingSpeed * timeSeconds));
    }
    rowTerms_.resize(static_cast<std::size_t>(grid.rows) * kTermsPerLine);
    for (auto row = 0; row < grid.rows; ++row) {
        const auto y = static_cast<double>(grid.originY + static_cast<float>(row) * grid.pitch);
        auto* terms = rowTerms_.data() + static_cast<std::size_t>(row) * kTermsPerLine;
        terms[0] = static_cast<float>(std::sin(kYWave * y + kYSpeed * timeSeconds));
        terms[1] = static_cast<float>(std::sin(kRisingWave * y));
        terms[2] = static_cast<float>(std::cos(kRisingWave * y));
        terms[3] = static_cast<float>(std::sin(kFallingWave * y));
        terms[4] = static_cast<float>(std::cos(kFallingWave * y));
    }
}

void PlasmaGenerator::writeTiles(Batch& batch, const Grid& grid, const Parameters& parameters) const
{
    const auto categoryCount = parameters.categoryCount;
    auto part = batch.parts.begin();
    auto* rect = part->floats.data();
    auto* category = part->categories.data();
    auto partRemaining = part->count;
    auto remaining = grid.tileCount;
    for (auto row = 0; row < grid.rows; ++row) {
        const auto* yTerms = rowTerms_.data() + static_cast<std::size_t>(row) * kTermsPerLine;
        const auto y = grid.originY + static_cast<float>(row) * grid.pitch;
        const auto rowColumns = std::min(grid.columns, remaining);
        for (auto column = 0; column < rowColumns; ++column) {
            // The parts hold tileCount tiles in total, so a next part exists while tiles remain.
            while (partRemaining == 0) {
                ++part;
                rect = part->floats.data();
                category = part->categories.data();
                partRemaining = part->count;
            }
            const auto* xTerms = columnTerms_.data() + static_cast<std::size_t>(column) * kTermsPerLine;
            const auto waveSum = xTerms[0] + yTerms[0] + (xTerms[1] * yTerms[2] + xTerms[2] * yTerms[1]) + (xTerms[3] * yTerms[4] - xTerms[4] * yTerms[3]);
            const auto value = shape(waveSum);
            const auto halfSide = 0.5f * parameters.tileScale * tileSide(value, grid.pitch);
            const auto x = grid.originX + static_cast<float>(column) * grid.pitch;
            rect[0] = x - halfSide;
            rect[1] = y - halfSide;
            rect[2] = x + halfSide;
            rect[3] = y + halfSide;
            rect += 4;
            if (categoryCount > 0) {
                *category++ = std::min(categoryCount - 1, static_cast<int>(value * static_cast<float>(categoryCount)));
            }
            --partRemaining;
        }
        remaining -= rowColumns;
    }
}

} // namespace QAccelPlotExample
