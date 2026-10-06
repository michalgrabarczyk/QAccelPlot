//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "InterferenceGenerator.hpp"
#include "SineLookup.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

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

} // namespace

void InterferenceGenerator::generate(Batch& batch, const Parameters& parameters, const double timeSeconds)
{
    const auto barCount = parameters.dataset.count;
    const auto categoryCount = parameters.categoryCount;
    resizeParts(batch.parts, parameters.dataset, 2);

    auto slow = cursorFor(kWaves[0], barCount, timeSeconds);
    auto medium = cursorFor(kWaves[1], barCount, timeSeconds);
    auto fast = cursorFor(kWaves[2], barCount, timeSeconds);
    auto index = 0;
    for (auto& part : batch.parts) {
        part.categories.resize(categoryCount > 0 ? static_cast<std::size_t>(part.count) : 0);
        auto* bar = part.floats.data();
        auto* category = part.categories.data();
        for (auto i = 0; i < part.count; ++i, ++index) {
            const auto value = kMidValue + kWaves[0].amplitude * slow.next() + kWaves[1].amplitude * medium.next() + kWaves[2].amplitude * fast.next();
            bar[0] = static_cast<float>(index);
            bar[1] = value;
            bar += 2;
            if (categoryCount > 0) {
                *category++ = std::min(categoryCount - 1, static_cast<int>(value * static_cast<float>(categoryCount)));
            }
        }
    }
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
