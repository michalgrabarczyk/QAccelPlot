//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "HeatmapSeriesFeed.hpp"

#include <QAccelPlot/series/HeatmapSeries.hpp>

#include <utility>

namespace QAccelPlotExample {

SpectrogramParameters HeatmapSeriesFeed::parameters(const PageScene& /*scene*/, const CommonOptions& options)
{
    auto parameters = SpectrogramParameters{};
    parameters.dataset = datasetParameters(options);
    return parameters;
}

void HeatmapSeriesFeed::apply(QAccelPlot::HeatmapSeries& series, SpectrogramPart& part, const CommonOptions& options)
{
    series.setXMin(0.0);
    series.setXMax(part.columns);
    series.setYMin(part.firstRow);
    series.setYMax(part.firstRow + part.rows);
    if (part.count == 0) {
        series.clearData();
        return;
    }
    switch (options.ingestion) {
    case Ingestion::FloatMove:
        series.setValues(std::move(part.floats), part.columns, part.rows);
        break;
    case Ingestion::FloatPost:
        series.postValues(std::move(part.floats), part.columns, part.rows);
        break;
    case Ingestion::FloatCopy:
    case Ingestion::DoubleMove:
        // The PlotSeries setters keep the current columns.
        series.setColumns(part.columns);
        applyRecords(series, part, options.ingestion);
        break;
    }
}

} // namespace QAccelPlotExample
