//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "GalaxyGenerator.hpp"
#include "XorShiftRandom.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>

namespace QAccelPlotExample {
namespace {

constexpr auto kTwoPi = 6.283185307179586;

constexpr auto kArmCount = 3;
constexpr auto kArmWinding = 1.9;
constexpr auto kArmScatter = 0.18;
constexpr auto kCoreRadius = 0.06;
constexpr auto kBulgeFraction = 0.08;
constexpr auto kHaloFraction = 0.02;

// Negative, so the arms trail the rotation.
constexpr auto kSpinRadiansPerSecond = -0.22;
constexpr auto kWobbleRadians = 0.12;

// Share of arm points drawn as star-forming regions, and the value they get.
constexpr auto kStarFormingFraction = 0.04;
constexpr auto kStarFormingValue = 0.3;

using Random = XorShiftRandom;

struct GalaxyPoint {
    float x;
    float y;
    float value;
    int band;
};

// Returns the polar radius and angle of one point of the bulge, the halo, or a spiral arm.
std::pair<double, double> polarPosition(Random& random)
{
    const auto kind = random.uniform();
    if (kind < kBulgeFraction) {
        return {std::abs(random.gaussian()) * 0.1, kTwoPi * random.uniform()};
    }
    if (kind < kBulgeFraction + kHaloFraction) {
        return {1.05 * std::sqrt(random.uniform()), kTwoPi * random.uniform()};
    }

    const auto u = random.uniform();
    const auto radius = kCoreRadius + (1.0 - kCoreRadius) * u * std::sqrt(u);
    const auto arm = std::floor(random.uniform() * kArmCount);
    const auto spiralAngle = arm * kTwoPi / kArmCount + kArmWinding * std::log(radius / kCoreRadius);
    const auto scatter = random.gaussian() * kArmScatter * (1.0 - 0.5 * radius);
    return {radius * (1.0 + 0.05 * random.gaussian()), spiralAngle + scatter};
}

GalaxyPoint makePoint(Random& random)
{
    const auto [radius, angle] = polarPosition(random);
    const auto starForming = radius > 0.25 && radius < 0.9 && random.uniform() < kStarFormingFraction;
    const auto value = starForming ? kStarFormingValue : std::clamp(radius + 0.05 * random.gaussian(), 0.0, 1.0);
    const auto band = std::clamp(static_cast<int>(radius / GalaxyGenerator::kRadius * GalaxyGenerator::kBandCount), 0, GalaxyGenerator::kBandCount - 1);
    return {static_cast<float>(radius * std::cos(angle)), static_cast<float>(radius * std::sin(angle)), static_cast<float>(value), band};
}

void rotatePoints(const float* source, float* destination, const int begin, const int end, const float cosine, const float sine)
{
    for (auto i = begin; i < end; ++i) {
        const auto offset = static_cast<std::size_t>(i) * 2;
        const auto x = source[offset];
        const auto y = source[offset + 1];
        destination[offset] = cosine * x - sine * y;
        destination[offset + 1] = sine * x + cosine * y;
    }
}

} // namespace

void GalaxyGenerator::generate(Batch& batch, const int pointCount, const double timeSeconds)
{
    if (pointCount != builtPointCount_) {
        rebuildGalaxy(pointCount);
    }

    batch.xy.resize(static_cast<std::size_t>(pointCount) * 2);
    batch.values = baseValues_;
    batch.pointCount = pointCount;

    for (auto band = 0; band < kBandCount; ++band) {
        const auto angle = bandAngleAt(band, timeSeconds);
        rotatePoints(basePositions_.data(), batch.xy.data(), bandStarts_[static_cast<std::size_t>(band)], bandStarts_[static_cast<std::size_t>(band) + 1],
            static_cast<float>(std::cos(angle)), static_cast<float>(std::sin(angle)));
    }
}

double GalaxyGenerator::bandAngleAt(const int band, const double timeSeconds)
{
    return kSpinRadiansPerSecond * timeSeconds + kWobbleRadians * std::sin(1.1 * timeSeconds - 0.35 * band);
}

const std::vector<float>& GalaxyGenerator::basePositions() const
{
    return basePositions_;
}

const std::array<int, GalaxyGenerator::kBandCount + 1>& GalaxyGenerator::bandStarts() const
{
    return bandStarts_;
}

void GalaxyGenerator::rebuildGalaxy(const int pointCount)
{
    auto random = Random{};
    auto points = std::vector<GalaxyPoint>(static_cast<std::size_t>(pointCount));
    auto bandCounts = std::array<int, kBandCount>{};
    for (auto& point : points) {
        point = makePoint(random);
        ++bandCounts[static_cast<std::size_t>(point.band)];
    }

    bandStarts_[0] = 0;
    for (auto band = 0; band < kBandCount; ++band) {
        bandStarts_[static_cast<std::size_t>(band) + 1] = bandStarts_[static_cast<std::size_t>(band)] + bandCounts[static_cast<std::size_t>(band)];
    }

    // Counting sort by band, so each band is one contiguous run.
    basePositions_.resize(static_cast<std::size_t>(pointCount) * 2);
    baseValues_.resize(static_cast<std::size_t>(pointCount));
    auto next = bandStarts_;
    for (const auto& point : points) {
        const auto index = static_cast<std::size_t>(next[static_cast<std::size_t>(point.band)]++);
        basePositions_[index * 2] = point.x;
        basePositions_[index * 2 + 1] = point.y;
        baseValues_[index] = point.value;
    }
    builtPointCount_ = pointCount;
}

} // namespace QAccelPlotExample
