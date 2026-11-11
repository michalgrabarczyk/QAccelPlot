//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "InterferenceGenerator.hpp"
#include "ParallelFor.hpp"
#include "SineLookup.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace QAccelPlotExample {
namespace {

constexpr auto kTwoPi = 6.283185307179586;

struct Wave {
    double cycles; // across the whole dataset
    double speed;  // radians per second; the sign sets the drift direction
    float amplitude;
};

constexpr auto kWaves = std::array<Wave, 3>{{{3.0, 0.8, 0.26f}, {11.0, -1.3, 0.14f}, {37.0, 2.1, 0.08f}}};
// The amplitudes add up to 0.48, so values stay within kMinimumValue and kMaximumValue.
constexpr auto kMidValue = 0.5f;

SineLookupCursor cursorFor(const Wave& wave, const int barCount, const double timeSeconds)
{
    return SineLookupCursor{wave.speed * timeSeconds, kTwoPi * wave.cycles / static_cast<double>(barCount)};
}

// Writes bars [first, last) of part; partBegin is the index of the part's first bar, and the
// cursors start at bar 0.
void writeBars(InterferencePart& part, const int first, const int last, const int partBegin, const int categoryCount, std::array<SineLookupCursor, 3> waves)
{
    for (auto& wave : waves) {
        wave.skip(static_cast<std::uint64_t>(partBegin + first));
    }
    auto* bar = part.floats.data() + static_cast<std::size_t>(first) * 2;
    auto* category = categoryCount > 0 ? part.categories.data() + first : nullptr;
    for (auto i = first; i < last; ++i) {
        const auto value = kMidValue + kWaves[0].amplitude * waves[0].next() + kWaves[1].amplitude * waves[1].next() + kWaves[2].amplitude * waves[2].next();
        bar[0] = static_cast<float>(partBegin + i);
        bar[1] = value;
        bar += 2;
        if (category) {
            *category++ = std::min(categoryCount - 1, static_cast<int>(value * static_cast<float>(categoryCount)));
        }
    }
}

} // namespace

std::size_t InterferencePart::byteSize() const
{
    return SeriesPart::byteSize() + categories.size() * sizeof(int);
}

std::size_t InterferenceBatch::byteSize() const
{
    return partsByteSize(parts);
}

bool operator==(const InterferenceParameters& a, const InterferenceParameters& b)
{
    return a.dataset == b.dataset && a.categoryCount == b.categoryCount;
}

void InterferenceGenerator::generate(Batch& batch, const Parameters& parameters, const double timeSeconds)
{
    const auto barCount = parameters.dataset.count;
    const auto categoryCount = parameters.categoryCount;
    resizeParts(batch.parts, parameters.dataset, 2);
    for (auto& part : batch.parts) {
        part.categories.resize(categoryCount > 0 ? static_cast<std::size_t>(part.count) : 0);
    }

    const auto waves = std::array<SineLookupCursor, 3>{
        cursorFor(kWaves[0], barCount, timeSeconds), cursorFor(kWaves[1], barCount, timeSeconds), cursorFor(kWaves[2], barCount, timeSeconds)};
    parallelFor(barCount, kParallelChunk, [&batch, categoryCount, &waves](const int begin, const int end) {
        forEachPartSlice(batch.parts, begin, end, [categoryCount, &waves](InterferencePart& part, const int first, const int last, const int partBegin) {
            writeBars(part, first, last, partBegin, categoryCount, waves);
        });
    });
    applyPrecision(batch.parts, parameters.dataset);
}

float InterferenceGenerator::valueAt(const int index, const int barCount, const double timeSeconds)
{
    auto value = static_cast<double>(kMidValue);
    for (const auto& wave : kWaves) {
        value += static_cast<double>(wave.amplitude) * std::sin(wave.speed * timeSeconds + kTwoPi * wave.cycles * index / barCount);
    }
    return static_cast<float>(value);
}

} // namespace QAccelPlotExample
