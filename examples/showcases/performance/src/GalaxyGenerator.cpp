//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "GalaxyGenerator.hpp"
#include "ParallelFor.hpp"
#include "XorShiftRandom.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
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

// Rotates \a count XY pairs from \a source into \a destination.
void rotatePoints(const float* source, float* destination, const int count, const float cosine, const float sine)
{
    for (auto i = 0; i < count; ++i) {
        const auto offset = static_cast<std::size_t>(i) * 2;
        const auto x = source[offset];
        const auto y = source[offset + 1];
        destination[offset] = cosine * x - sine * y;
        destination[offset + 1] = sine * x + cosine * y;
    }
}

// Sets the X of every n-th point of the galaxy to NaN; firstPoint is the galaxy index of the part's first point.
void invalidatePoints(GalaxyPart& part, const int firstPoint, const float invalidFraction)
{
    if (invalidFraction <= 0.0f) {
        return;
    }
    const auto stride = std::max(1, static_cast<int>(std::lround(1.0f / invalidFraction)));
    for (auto point = (stride - firstPoint % stride) % stride; point < part.count; point += stride) {
        part.floats[static_cast<std::size_t>(point) * 2] = std::numeric_limits<float>::quiet_NaN();
    }
}

} // namespace

std::size_t GalaxyPart::byteSize() const
{
    return SeriesPart::byteSize() + values.size() * sizeof(float);
}

std::size_t GalaxyBatch::byteSize() const
{
    return partsByteSize(parts);
}

bool operator==(const GalaxyParameters& a, const GalaxyParameters& b)
{
    return a.dataset == b.dataset && a.values == b.values && a.invalidFraction == b.invalidFraction;
}

void GalaxyGenerator::generate(Batch& batch, const Parameters& parameters, const double timeSeconds)
{
    const auto pointCount = parameters.dataset.count;
    if (pointCount != builtPointCount_) {
        rebuildGalaxy(pointCount);
    }

    resizeParts(batch.parts, parameters.dataset, 2);
    for (auto& part : batch.parts) {
        part.values.resize(parameters.values ? static_cast<std::size_t>(part.count) : 0);
    }
    const auto rotations = bandRotationsAt(timeSeconds);
    parallelFor(pointCount, kParallelChunk, [this, &batch, &rotations](const int begin, const int end) {
        forEachPartSlice(batch.parts, begin, end, [this, &rotations](GalaxyPart& part, const int first, const int last, const int partBegin) {
            writeSlice(part, first, last, partBegin, rotations);
        });
    });

    auto firstPoint = 0;
    for (auto& part : batch.parts) {
        invalidatePoints(part, firstPoint, parameters.invalidFraction);
        firstPoint += part.count;
    }
    applyPrecision(batch.parts, parameters.dataset);
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

GalaxyGenerator::BandRotations GalaxyGenerator::bandRotationsAt(const double timeSeconds)
{
    auto rotations = BandRotations{};
    for (auto band = 0; band < kBandCount; ++band) {
        const auto angle = bandAngleAt(band, timeSeconds);
        rotations[static_cast<std::size_t>(band)] = {static_cast<float>(std::cos(angle)), static_cast<float>(std::sin(angle))};
    }
    return rotations;
}

void GalaxyGenerator::writeSlice(GalaxyPart& part, const int first, const int last, const int partBegin, const BandRotations& rotations) const
{
    const auto beginPoint = partBegin + first;
    const auto endPoint = partBegin + last;
    if (!part.values.empty()) {
        std::copy(baseValues_.begin() + beginPoint, baseValues_.begin() + endPoint, part.values.begin() + first);
    }
    for (auto band = 0; band < kBandCount; ++band) {
        const auto begin = std::max(beginPoint, bandStarts_[static_cast<std::size_t>(band)]);
        const auto end = std::min(endPoint, bandStarts_[static_cast<std::size_t>(band) + 1]);
        if (begin >= end) {
            continue;
        }
        const auto& rotation = rotations[static_cast<std::size_t>(band)];
        rotatePoints(basePositions_.data() + static_cast<std::size_t>(begin) * 2, part.floats.data() + static_cast<std::size_t>(begin - partBegin) * 2,
            end - begin, rotation.cosine, rotation.sine);
    }
}

} // namespace QAccelPlotExample
