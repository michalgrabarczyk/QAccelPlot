//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ExampleUtils.hpp"

#include <QAccelPlot/series/HeatmapSeries.hpp>

#include <QDebug>
#include <QGuiApplication>
#include <QLocale>
#include <QQmlApplicationEngine>

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <utility>
#include <vector>

namespace {

constexpr auto kColumns = 400;
constexpr auto kRows = 300;
// The grid edges, matching xMin, xMax, yMin, and yMax in InterferencePage.qml.
constexpr auto kXMin = -10.0;
constexpr auto kXMax = 10.0;
constexpr auto kYMin = -7.5;
constexpr auto kYMax = 7.5;

constexpr auto kWavelength = 1.6;
constexpr auto kSourceX = 3.0;

// Amplitude at (x, y) of a circular wave from a source at (sourceX, 0). A 2D wave decays with the
// square root of the distance.
double wave(const double x, const double y, const double sourceX)
{
    constexpr auto kTwoPi = 6.283185307179586;
    const auto distance = std::hypot(x - sourceX, y);
    return std::cos(kTwoPi * distance / kWavelength) / std::sqrt(1.0 + distance);
}

// Two in-phase sources at x = ±kSourceX, sampled at the center of each cell, row 0 at kYMin.
std::vector<float> interferencePattern()
{
    auto values = std::vector<float>{};
    values.reserve(static_cast<std::size_t>(kColumns) * kRows);
    const auto cellWidth = (kXMax - kXMin) / kColumns;
    const auto cellHeight = (kYMax - kYMin) / kRows;
    for (auto row = 0; row < kRows; ++row) {
        const auto y = kYMin + (row + 0.5) * cellHeight;
        for (auto column = 0; column < kColumns; ++column) {
            const auto x = kXMin + (column + 0.5) * cellWidth;
            values.push_back(static_cast<float>(wave(x, y, -kSourceX) + wave(x, y, kSourceX)));
        }
    }
    return values;
}

constexpr auto kSweepColumns = 240;
constexpr auto kSweepRows = 180;
// The grid edges, matching LogBinsPage.qml.
constexpr auto kDuration = 10.0;
constexpr auto kLowestFrequency = 20.0;
constexpr auto kHighestFrequency = 20000.0;

// Level in dB of a bin centered at frequency, with a constant-Q response a twelfth of an octave
// wide, for a tone at toneFrequency with its second and third harmonics.
double binLevel(const double frequency, const double toneFrequency)
{
    constexpr auto kBandwidthOctaves = 1.0 / 12.0;
    constexpr auto kNoiseFloor = 1e-4; // -80 dB
    auto amplitude = kNoiseFloor;
    for (auto harmonic = 1; harmonic <= 3; ++harmonic) {
        const auto octaves = std::log2(frequency / (harmonic * toneFrequency)) / kBandwidthOctaves;
        amplitude += std::exp(-0.5 * octaves * octaves) / (harmonic * harmonic);
    }
    return 20.0 * std::log10(amplitude);
}

// An exponential sweep from 50 Hz to 5 kHz in bins of equal frequency ratio, row 0 at
// kLowestFrequency. Each bin is sampled at its time center and geometric frequency center.
std::vector<float> frequencySweep()
{
    auto values = std::vector<float>{};
    values.reserve(static_cast<std::size_t>(kSweepColumns) * kSweepRows);
    for (auto row = 0; row < kSweepRows; ++row) {
        const auto frequency = kLowestFrequency * std::pow(kHighestFrequency / kLowestFrequency, (row + 0.5) / kSweepRows);
        for (auto column = 0; column < kSweepColumns; ++column) {
            const auto time = kDuration * (column + 0.5) / kSweepColumns;
            values.push_back(static_cast<float>(binLevel(frequency, 50.0 * std::pow(100.0, time / kDuration))));
        }
    }
    return values;
}

} // namespace

int main(int argc, char* argv[])
{
    QLocale::setDefault(QLocale::English);
    QAccelPlotExample::configureGraphicsApi();

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QAccelPlotExample::setupEngineFailureHandler(app, engine);

    engine.load(QUrl(QStringLiteral("qrc:/app/qml/main.qml")));
    auto* root = engine.rootObjects().value(0);
    auto* interference = root ? root->findChild<QAccelPlot::HeatmapSeries*>(QStringLiteral("interferenceHeatmap")) : nullptr;
    auto* logBins = root ? root->findChild<QAccelPlot::HeatmapSeries*>(QStringLiteral("logBinsHeatmap")) : nullptr;
    if (!interference || !logBins) {
        qCritical() << "Unable to find the heatmaps";
        return EXIT_FAILURE;
    }
    interference->setValues(interferencePattern(), kColumns, kRows);
    logBins->setValues(frequencySweep(), kSweepColumns, kSweepRows);

    QAccelPlotExample::setupScreenshotHandler(app, engine);
    return app.exec();
}
