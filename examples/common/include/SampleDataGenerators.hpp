//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <vector>

namespace QAccelPlotExample {

struct AnnotationScenarioData {
    std::vector<float> points;
    float peakX{0.0f};
    float peakY{0.0f};
    float valleyX{0.0f};
    float valleyY{0.0f};
};

std::vector<float> generateSinusoid(
    int pointCount, float phase = 0.0f, float amplitude = 8.0f, float xDomain = 1000.0f, float frequencyScale = 1.0f);
std::vector<float> generateRectangles(int count, float amplitude = 8.0f, float xDomain = 1000.0f);
std::vector<float> generateDiscreteWaveSamples(bool useCosine, int pointCount = 10, float frequency = 0.8f);
std::vector<float> generateContinuousWaveSamples(bool useCosine, int pointCount = 200, float xMax = 9.0f);
std::vector<float> generateAcousticData(int pointCount, float baseFrequency = 200.0f, float noiseAmplitude = 0.3f, unsigned int frameSeed = 0);
AnnotationScenarioData generateAnnotationScenarioData(int pointCount = 1001);
std::vector<float> generateTerrainProfile(int pointCount = 1001, float distance = 1000.0f);

} // namespace QAccelPlotExample
