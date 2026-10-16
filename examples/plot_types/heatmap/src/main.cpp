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
// The grid edges, matching xMin, xMax, yMin, and yMax in main.qml.
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
    auto* heatmap = root ? root->findChild<QAccelPlot::HeatmapSeries*>(QStringLiteral("heatmap")) : nullptr;
    if (!heatmap) {
        qCritical() << "Unable to find the heatmap";
        return EXIT_FAILURE;
    }
    heatmap->setValues(interferencePattern(), kColumns, kRows);

    QAccelPlotExample::setupScreenshotHandler(app, engine);
    return app.exec();
}
