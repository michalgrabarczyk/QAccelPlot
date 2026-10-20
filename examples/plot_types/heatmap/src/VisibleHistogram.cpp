//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "VisibleHistogram.hpp"

#include <QAccelPlot/axis/Axis.hpp>
#include <QAccelPlot/data/Histogram.hpp>
#include <QAccelPlot/series/BarSeries.hpp>
#include <QAccelPlot/series/HeatmapSeries.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace QAccelPlotExample {

VisibleHistogram::VisibleHistogram(QAccelPlot::HeatmapSeries* heatmap, QAccelPlot::BarSeries* bars, std::vector<float> values, const int binCount)
    : heatmap_(heatmap)
    , bars_(bars)
    , values_(std::move(values))
{
    const auto [lowest, highest] = std::minmax_element(values_.cbegin(), values_.cend());
    edges_ = QAccelPlot::Histogram::linearEdges(*lowest, *highest, binCount);

    recountTimer_.setSingleShot(true);
    recountTimer_.setInterval(0);
    QObject::connect(&recountTimer_, &QTimer::timeout, &recountTimer_, [this]() { recount(); });
    for (auto* axis : {heatmap_->xAxis(), heatmap_->yAxis()}) {
        QObject::connect(axis, &QAccelPlot::Axis::rangeChanged, &recountTimer_, [this]() { recountTimer_.start(); });
    }

    // Counted at once, so the first frame shows the bars and the amplitude axis fits the edges.
    recount();
    (bars_->orientation() == Qt::Horizontal ? bars_->yAxis() : bars_->xAxis())->rescaleToData();
}

std::pair<int, int> VisibleHistogram::visibleCells(const int count, const double min, const double max, const double viewportMin, const double viewportMax)
{
    // Cell i is centered at min + (i + 0.5) * cellSize.
    const auto cellSize = (max - min) / count;
    const auto first = std::ceil((std::min(viewportMin, viewportMax) - min) / cellSize - 0.5);
    const auto last = std::floor((std::max(viewportMin, viewportMax) - min) / cellSize - 0.5);
    return {static_cast<int>(std::clamp(first, 0.0, static_cast<double>(count))), static_cast<int>(std::clamp(last, -1.0, count - 1.0))};
}

void VisibleHistogram::recount()
{
    const auto columns = heatmap_->columns();
    const auto [firstColumn, lastColumn]
        = visibleCells(columns, heatmap_->xMin(), heatmap_->xMax(), heatmap_->xAxis()->viewportMin(), heatmap_->xAxis()->viewportMax());
    const auto [firstRow, lastRow]
        = visibleCells(heatmap_->rows(), heatmap_->yMin(), heatmap_->yMax(), heatmap_->yAxis()->viewportMin(), heatmap_->yAxis()->viewportMax());

    visibleValues_.clear();
    for (auto row = firstRow; row <= lastRow; ++row) {
        const auto rowStart = values_.cbegin() + static_cast<std::ptrdiff_t>(row) * columns;
        visibleValues_.insert(visibleValues_.end(), rowStart + firstColumn, rowStart + std::max(firstColumn, lastColumn + 1));
    }
    const auto histogram = QAccelPlot::Histogram::fromSamples(visibleValues_.data(), visibleValues_.size(), edges_);
    bars_->setRangedData(histogram.barData(), histogram.binCount());
}

} // namespace QAccelPlotExample
