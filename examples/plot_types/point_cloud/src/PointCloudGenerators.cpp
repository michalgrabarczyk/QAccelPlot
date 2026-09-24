//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "PointCloudGenerators.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <random>

namespace QAccelPlotExample {

namespace {

constexpr auto kTwoPi = 6.283185307179586;
constexpr auto kNoiseFraction = 0.05;
constexpr auto kNoiseHalfWidth = 9.0f;
constexpr auto kNoiseHalfHeight = 7.0f;
constexpr auto kValueRadiusScale = 3.0f; // 3 sigma maps to value 1

struct ClusterSpec {
    float centerX;
    float centerY;
    float sigmaMajor;
    float sigmaMinor;
    float angleRadians;
    float spinRate; // relative angular speed while animating
    float weight;   // share of non-noise points
};

constexpr auto kClusters = std::array<ClusterSpec, 3>{{
    {-4.0f, 2.0f, 2.0f, 0.7f, 0.6f, 1.0f, 0.45f},
    {3.5f, 3.5f, 1.1f, 1.0f, 0.0f, -1.6f, 0.25f},
    {1.5f, -3.0f, 2.6f, 0.45f, -0.35f, 0.7f, 0.30f},
}};

// std::mt19937 output is specified by the standard; the distributions are not.
class PortableRandom {
public:
    explicit PortableRandom(const unsigned int seed)
        : engine_(seed)
    {
    }

    double uniform()
    {
        // (k + 0.5) / 2^32 is strictly inside (0, 1), safe for log().
        return (static_cast<double>(engine_()) + 0.5) / 4294967296.0;
    }

    double uniform(const double min, const double max)
    {
        return min + (max - min) * uniform();
    }

    double normal()
    {
        if (hasSpare_) {
            hasSpare_ = false;
            return spare_;
        }
        const auto radius = std::sqrt(-2.0 * std::log(uniform()));
        const auto angle = kTwoPi * uniform();
        spare_ = radius * std::sin(angle);
        hasSpare_ = true;
        return radius * std::cos(angle);
    }

private:
    std::mt19937 engine_;
    double spare_{0.0};
    bool hasSpare_{false};
};

int clusterIndexFor(const double selector)
{
    auto accumulated = 0.0;
    for (auto i = 0; i < static_cast<int>(kClusters.size()); ++i) {
        accumulated += kClusters[static_cast<std::size_t>(i)].weight;
        if (selector < accumulated) {
            return i;
        }
    }
    return static_cast<int>(kClusters.size()) - 1;
}

void writeClusterSample(PortableRandom& random, const ClusterSpec& spec, ClusterCloud& cloud, const std::size_t index)
{
    const auto major = random.normal();
    const auto minor = random.normal();
    const auto cosAngle = std::cos(spec.angleRadians);
    const auto sinAngle = std::sin(spec.angleRadians);
    const auto localX = static_cast<float>(major * spec.sigmaMajor * cosAngle - minor * spec.sigmaMinor * sinAngle);
    const auto localY = static_cast<float>(major * spec.sigmaMajor * sinAngle + minor * spec.sigmaMinor * cosAngle);
    const auto normalizedRadius = static_cast<float>(std::sqrt(major * major + minor * minor)) / kValueRadiusScale;

    cloud.localXy[index * 2] = localX;
    cloud.localXy[index * 2 + 1] = localY;
    cloud.centers[index * 2] = spec.centerX;
    cloud.centers[index * 2 + 1] = spec.centerY;
    cloud.spinRates[index] = spec.spinRate;
    cloud.values[index] = std::min(normalizedRadius, 1.0f);
}

void writeNoiseSample(PortableRandom& random, ClusterCloud& cloud, const std::size_t index)
{
    cloud.localXy[index * 2] = static_cast<float>(random.uniform(-kNoiseHalfWidth, kNoiseHalfWidth));
    cloud.localXy[index * 2 + 1] = static_cast<float>(random.uniform(-kNoiseHalfHeight, kNoiseHalfHeight));
    cloud.centers[index * 2] = 0.0f;
    cloud.centers[index * 2 + 1] = 0.0f;
    cloud.spinRates[index] = 0.0f;
    cloud.values[index] = 1.0f;
}

} // namespace

ClusterCloud generateClusterCloud(const int pointCount, const unsigned int seed)
{
    auto cloud = ClusterCloud{};
    const auto count = std::max(pointCount, 0);
    const auto size = static_cast<std::size_t>(count);
    cloud.pointCount = count;
    cloud.localXy.resize(size * 2);
    cloud.centers.resize(size * 2);
    cloud.spinRates.resize(size);
    cloud.values.resize(size);

    // Background noise comes first: points draw in index order, so the clusters paint over it.
    const auto noiseCount = static_cast<std::size_t>(std::lround(static_cast<double>(count) * kNoiseFraction));
    auto random = PortableRandom{seed};
    for (auto i = std::size_t{0}; i < size; ++i) {
        if (i < noiseCount) {
            writeNoiseSample(random, cloud, i);
        } else {
            const auto clusterIndex = clusterIndexFor(random.uniform());
            writeClusterSample(random, kClusters[static_cast<std::size_t>(clusterIndex)], cloud, i);
        }
    }
    return cloud;
}

void composeClusterFrame(const ClusterCloud& cloud, const float phaseRadians, PointCloudData& frame)
{
    const auto size = static_cast<std::size_t>(cloud.pointCount);
    frame.pointCount = cloud.pointCount;
    frame.xy.resize(size * 2);
    frame.values.assign(cloud.values.begin(), cloud.values.end());

    for (auto i = std::size_t{0}; i < size; ++i) {
        const auto angle = phaseRadians * cloud.spinRates[i];
        const auto cosAngle = std::cos(angle);
        const auto sinAngle = std::sin(angle);
        const auto localX = cloud.localXy[i * 2];
        const auto localY = cloud.localXy[i * 2 + 1];
        frame.xy[i * 2] = cloud.centers[i * 2] + localX * cosAngle - localY * sinAngle;
        frame.xy[i * 2 + 1] = cloud.centers[i * 2 + 1] + localX * sinAngle + localY * cosAngle;
    }
}

std::vector<float> generatePowerLawScatter(const int pointCount, const unsigned int seed)
{
    const auto validCount = std::max(pointCount, 0);
    const auto totalCount = static_cast<std::size_t>(validCount + kPowerLawInvalidPointCount);
    auto xy = std::vector<float>(totalCount * 2);
    auto random = PortableRandom{seed};

    // x spans 1..10^4 uniformly in log space; y ~ 3 * x^0.75 with log-normal scatter.
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(validCount); ++i) {
        const auto x = std::pow(10.0, random.uniform(0.0, 4.0));
        const auto y = 3.0 * std::pow(x, 0.75) * std::exp(0.35 * random.normal());
        xy[i * 2] = static_cast<float>(x);
        xy[i * 2 + 1] = static_cast<float>(y);
    }

    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();
    const auto invalid = std::array<float, kPowerLawInvalidPointCount * 2>{0.0f, 10.0f, 50.0f, -3.0f, nan, 100.0f, 200.0f, nan};
    std::copy(invalid.begin(), invalid.end(), xy.begin() + static_cast<std::ptrdiff_t>(validCount) * 2);
    return xy;
}

} // namespace QAccelPlotExample
