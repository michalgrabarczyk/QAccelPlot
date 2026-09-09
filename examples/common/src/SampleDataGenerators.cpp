//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "SampleDataGenerators.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>

namespace QAccelPlotExample {

std::vector<float> generateSinusoid(const int pointCount, const float phase, const float amplitude, const float xDomain, const float frequencyScale)
{
    if (pointCount <= 0) {
        return {};
    }

    std::vector<float> data(static_cast<size_t>(pointCount) * 2);
    const float xScale = xDomain / static_cast<float>(pointCount);
    const float yScale = 100.0f / static_cast<float>(pointCount);

    for (int i = 0; i < pointCount; ++i) {
        const float x = static_cast<float>(i) * xScale;
        const float y = amplitude * std::sin(static_cast<float>(i) * yScale * frequencyScale + phase);
        data[static_cast<size_t>(i) * 2] = x;
        data[static_cast<size_t>(i) * 2 + 1] = y;
    }
    return data;
}

std::vector<float> generateRectangles(const int count, const float amplitude, const float xDomain)
{
    if (count <= 0) {
        return {};
    }

    std::vector<float> rects(static_cast<size_t>(count) * 4);
    const float pointSpacing = xDomain / static_cast<float>(count);
    const float rectHalfW = pointSpacing * 5.0f;
    const float rectHalfH = amplitude * 0.04f;
    const float xScale = xDomain / static_cast<float>(count);
    const float yScale = 100.0f / static_cast<float>(count);

    for (int i = 0; i < count; ++i) {
        const float x = static_cast<float>(i) * xScale;
        const float y = amplitude * std::sin(static_cast<float>(i) * yScale);
        const size_t idx = static_cast<size_t>(i) * 4;
        rects[idx + 0] = x - rectHalfW;
        rects[idx + 1] = y - rectHalfH;
        rects[idx + 2] = x + rectHalfW;
        rects[idx + 3] = y + rectHalfH;
    }
    return rects;
}

std::vector<float> generateDiscreteWaveSamples(const bool useCosine, const int pointCount, const float frequency)
{
    if (pointCount <= 0) {
        return {};
    }

    std::vector<float> data(static_cast<size_t>(pointCount) * 2);
    for (auto i = 0; i < pointCount; i++) {
        const auto angle = static_cast<float>(i) * frequency;
        data[static_cast<size_t>(i) * 2] = static_cast<float>(i);
        data[static_cast<size_t>(i) * 2 + 1] = useCosine ? std::cos(angle) : std::sin(angle);
    }
    return data;
}

std::vector<float> generateContinuousWaveSamples(const bool useCosine, const int pointCount, const float xMax)
{
    if (pointCount <= 0) {
        return {};
    }

    std::vector<float> data(static_cast<size_t>(pointCount) * 2);
    for (auto i = 0; i < pointCount; i++) {
        const auto t = pointCount == 1 ? 0.0f : static_cast<float>(i) / static_cast<float>(pointCount - 1);
        const auto x = t * xMax;
        data[static_cast<size_t>(i) * 2] = x;
        data[static_cast<size_t>(i) * 2 + 1] = useCosine ? std::cos(x) : std::sin(x);
    }
    return data;
}

std::vector<float> generateAcousticData(
    const int pointCount, const float baseFrequency, const float noiseAmplitude, const unsigned int frameSeed)
{
    if (pointCount <= 0) {
        return {};
    }

    const auto seedf = static_cast<float>(frameSeed);

    auto pseudoRand = [seedf](unsigned int seed) -> float {
        const auto x = std::sin(seedf * 73.156f + static_cast<float>(seed) * 12.9898f) * 43758.5453f;
        return x - std::floor(x);
    };

    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    const auto freqVariation = 0.95f + 0.05f * pseudoRand(1);
    const auto variedBaseFrequency = baseFrequency * freqVariation;

    const auto modSpeed1 = 0.2f + 0.05f * pseudoRand(2);
    const auto modSpeed2 = 0.3f + 0.05f * pseudoRand(3);
    const auto modAmp = 0.85f + 0.08f * pseudoRand(4);

    const auto harm1BaseRatio = 1.4f + 0.08f * pseudoRand(5);
    const auto harm2BaseRatio = 2.4f + 0.1f * pseudoRand(6);

    std::vector<float> data(static_cast<size_t>(pointCount) * 2);
    const auto xScale = 10.0f / static_cast<float>(pointCount);

    for (auto i = 0; i < pointCount; i++) {
        const auto x = static_cast<float>(i) * xScale;
        const auto frequencyMod = 1.0f + modAmp * std::sin(x * modSpeed1);
        const auto modulatedFrequency = variedBaseFrequency * frequencyMod;

        const auto basePhase = pseudoRand(7) * 0.5f;
        const auto phaseShift = basePhase + 0.3f * std::sin(x * modSpeed2) + pseudoRand(8) * 0.15f;

        const auto carrier = std::sin(modulatedFrequency * x * 0.01f + phaseShift);
        const auto secondaryRatio = 0.88f + 0.1f * pseudoRand(9);
        const auto secondaryCarrier = 0.5f * std::cos(modulatedFrequency * secondaryRatio * x * 0.01f + phaseShift * 0.7f);

        const auto harmonic1Amp = 0.55f * (0.8f + 0.08f * pseudoRand(10));
        const auto harmonic1 = harmonic1Amp * std::sin(modulatedFrequency * harm1BaseRatio * x * 0.01f + phaseShift * 0.3f);

        const auto harmonic2Amp = 0.35f * (0.88f + 0.08f * pseudoRand(11));
        const auto harmonic2 = harmonic2Amp * std::sin(modulatedFrequency * harm2BaseRatio * x * 0.01f + phaseShift * 0.5f);

        const auto noise = dist(rng) * 4.0f * noiseAmplitude * (0.85f + 0.15f * pseudoRand(12));

        const auto envelopeBase = 0.87f + 0.06f * pseudoRand(13);
        const auto envelope = envelopeBase * (0.85f + 0.08f * std::sin(x * (0.3f + 0.05f * pseudoRand(14))));

        data[static_cast<size_t>(i) * 2] = x;
        data[static_cast<size_t>(i) * 2 + 1] = envelope * (carrier + secondaryCarrier + harmonic1 + harmonic2 + noise);
    }

    return data;
}

AnnotationScenarioData generateAnnotationScenarioData(const int pointCount)
{
    if (pointCount <= 0) {
        return {};
    }

    AnnotationScenarioData scenario;
    scenario.points.resize(static_cast<size_t>(pointCount) * 2);

    float peakY = std::numeric_limits<float>::lowest();
    float valleyY = std::numeric_limits<float>::max();
    const float xScale = 100.0f / static_cast<float>(std::max(1, pointCount - 1));

    for (int i = 0; i < pointCount; ++i) {
        const float x = static_cast<float>(i) * xScale;
        const float response = 0.25f * std::sin(0.35f * x)
            + 6.2f * std::exp(-std::pow((x - 24.0f) / 4.0f, 2.0f))
            - 4.8f * std::exp(-std::pow((x - 58.0f) / 5.0f, 2.0f))
            + 3.0f * std::exp(-std::pow((x - 79.0f) / 7.0f, 2.0f));
        const size_t index = static_cast<size_t>(i) * 2;
        scenario.points[index] = x;
        scenario.points[index + 1] = response;

        if (response > peakY) {
            scenario.peakX = x;
            scenario.peakY = response;
            peakY = response;
        }
        if (response < valleyY) {
            scenario.valleyX = x;
            scenario.valleyY = response;
            valleyY = response;
        }
    }

    return scenario;
}

std::vector<float> generateTerrainProfile(const int pointCount, const float distance)
{
    if (pointCount <= 0 || distance <= 0.0f) {
        return {};
    }

    std::vector<float> data(static_cast<size_t>(pointCount) * 2);
    const float xScale = distance / static_cast<float>(std::max(1, pointCount - 1));

    for (int i = 0; i < pointCount; ++i) {
        const float x = static_cast<float>(i) * xScale;
        const float elevation = 28.0f
            + 20.0f * std::exp(-std::pow((x - 240.0f) / 120.0f, 2.0f))
            - 11.0f * std::exp(-std::pow((x - 500.0f) / 75.0f, 2.0f))
            + 34.0f * std::exp(-std::pow((x - 720.0f) / 150.0f, 2.0f))
            + 1.8f * std::sin(6.28318530718f * x / 95.0f);
        const size_t index = static_cast<size_t>(i) * 2;
        data[index] = x;
        data[index + 1] = elevation;
    }

    return data;
}

} // namespace QAccelPlotExample
