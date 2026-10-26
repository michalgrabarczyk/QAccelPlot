//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "LevelHistogram.hpp"

#include <QAccelPlot/axis/Axis.hpp>
#include <QAccelPlot/data/Histogram.hpp>
#include <QAccelPlot/series/BarSeries.hpp>
#include <QAccelPlot/series/HeatmapSeries.hpp>

#include <algorithm>
#include <cmath>

namespace QAccelPlotExample {

namespace {

// 2 dB bands from the spectrogram's -100 dB floor up to 0 dB.
constexpr auto kLowestLevelDecibels = -100.0;
constexpr auto kHighestLevelDecibels = 0.0;
constexpr auto kLevelBands = 50;

} // namespace

LevelHistogram::LevelHistogram(QAccelPlot::HeatmapSeries* spectrogram, QAccelPlot::BarSeries* bars, QObject* parent)
    : QObject(parent)
    , spectrogram_(spectrogram)
    , bars_(bars)
{
    recountTimer_.setSingleShot(true);
    recountTimer_.setInterval(0);
    connect(&recountTimer_, &QTimer::timeout, this, &LevelHistogram::recount);
    connect(spectrogram_, &QAccelPlot::PlotSeries::dataRevisionChanged, &recountTimer_, qOverload<>(&QTimer::start));
    for (auto* axis : {spectrogram_->xAxis(), spectrogram_->yAxis()}) {
        connect(axis, &QAccelPlot::Axis::rangeChanged, &recountTimer_, qOverload<>(&QTimer::start));
    }
}

std::pair<int, int> LevelHistogram::visibleCells(const int count, const double min, const double max, const double viewportMin, const double viewportMax)
{
    // Cell i is centered at min + (i + 0.5) * cellSize.
    const auto cellSize = (max - min) / count;
    const auto first = std::ceil((std::min(viewportMin, viewportMax) - min) / cellSize - 0.5);
    const auto last = std::floor((std::max(viewportMin, viewportMax) - min) / cellSize - 0.5);
    return {static_cast<int>(std::clamp(first, 0.0, static_cast<double>(count))), static_cast<int>(std::clamp(last, -1.0, count - 1.0))};
}

void LevelHistogram::recount()
{
    const auto columns = spectrogram_->columns();
    const auto rows = spectrogram_->rows();
    if (columns <= 0 || rows <= 0) {
        return;
    }
    const auto* xAxis = spectrogram_->xAxis();
    const auto* yAxis = spectrogram_->yAxis();
    const auto [firstColumn, lastColumn] = visibleCells(columns, spectrogram_->xMin(), spectrogram_->xMax(), xAxis->viewportMin(), xAxis->viewportMax());
    const auto [firstRow, lastRow] = visibleCells(rows, spectrogram_->yMin(), spectrogram_->yMax(), yAxis->viewportMin(), yAxis->viewportMax());

    visibleLevels_.clear();
    for (auto row = firstRow; row <= lastRow; ++row) {
        for (auto column = firstColumn; column <= lastColumn; ++column) {
            visibleLevels_.push_back(static_cast<float>(spectrogram_->valueAt(row, column)));
        }
    }
    const auto histogram
        = QAccelPlot::Histogram::fromSamples(visibleLevels_.data(), visibleLevels_.size(), kLevelBands, kLowestLevelDecibels, kHighestLevelDecibels);
    bars_->setRangedData(histogram.barData(), histogram.binCount());
}

} // namespace QAccelPlotExample
