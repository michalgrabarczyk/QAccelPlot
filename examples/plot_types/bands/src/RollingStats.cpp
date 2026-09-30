//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "RollingStats.hpp"

#include <algorithm>
#include <cmath>
#include <random>

namespace {

constexpr auto kDurationSeconds = 600.0;
constexpr auto kWindowSeconds = 10.0;
constexpr auto kPi = 3.14159265358979323846;

double gaussian(const double t, const double center, const double width)
{
    const auto z = (t - center) / width;
    return std::exp(-z * z);
}

// Slow drift of the sensor's operating point.
double baseline(const double t)
{
    return 0.8 * std::sin(2.0 * kPi * t / 240.0) + 0.3 * std::sin(2.0 * kPi * t / 37.0) + 1.2 * gaussian(t, 380.0, 30.0);
}

// Standard deviation of the vibration; two bearing faults raise it for a while.
double noiseLevel(const double t)
{
    return 0.12 + 0.45 * gaussian(t, 170.0, 20.0) + 0.3 * gaussian(t, 470.0, 12.0);
}

std::vector<double> simulateSensor(const int sampleCount)
{
    auto rng = std::mt19937{20260929};
    auto noise = std::normal_distribution<double>{};
    auto values = std::vector<double>(static_cast<std::size_t>(sampleCount));
    for (auto i = 0; i < sampleCount; ++i) {
        const auto t = kDurationSeconds * i / (sampleCount - 1);
        values[static_cast<std::size_t>(i)] = baseline(t) + noiseLevel(t) * noise(rng);
    }
    return values;
}

} // namespace

// The window sums are updated incrementally as the window slides.
RollingStats computeRollingStats(const int sampleCount)
{
    const auto values = simulateSensor(sampleCount);
    const auto count = static_cast<std::size_t>(sampleCount);
    const auto halfWindow = std::max<std::size_t>(1, static_cast<std::size_t>(kWindowSeconds * sampleCount / kDurationSeconds / 2.0));

    auto stats = RollingStats{};
    stats.samples.reserve(count * 2);
    stats.mean.reserve(count * 2);
    stats.band.reserve(count * 3);
    auto sum = 0.0;
    auto sumOfSquares = 0.0;
    auto first = std::size_t{0}; // window is [first, last)
    auto last = std::size_t{0};
    for (auto i = std::size_t{0}; i < count; ++i) {
        for (; last < std::min(count, i + halfWindow + 1); ++last) {
            sum += values[last];
            sumOfSquares += values[last] * values[last];
        }
        for (; first + halfWindow < i; ++first) {
            sum -= values[first];
            sumOfSquares -= values[first] * values[first];
        }
        const auto n = static_cast<double>(last - first);
        const auto mean = sum / n;
        const auto deviation = std::sqrt(std::max(sumOfSquares / n - mean * mean, 0.0));
        const auto x = static_cast<float>(kDurationSeconds * static_cast<double>(i) / static_cast<double>(count - 1));
        stats.samples.insert(stats.samples.end(), {x, static_cast<float>(values[i])});
        stats.mean.insert(stats.mean.end(), {x, static_cast<float>(mean)});
        stats.band.insert(stats.band.end(), {x, static_cast<float>(mean - 2.0 * deviation), static_cast<float>(mean + 2.0 * deviation)});
    }
    return stats;
}
