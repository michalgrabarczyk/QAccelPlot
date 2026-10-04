//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "RectangleSeriesFeed.hpp"

#include <QAccelPlot/series/RectangleSeries.hpp>

#include <algorithm>
#include <utility>

namespace QAccelPlotExample {

PlasmaParameters RectangleSeriesFeed::parameters(const PageScene& scene, const CommonOptions& options)
{
    auto parameters = PlasmaParameters{};
    parameters.dataset = datasetParameters(options);
    parameters.tileScale = scene.setting("tileScale").toFloat();
    // The raw-array setters take no categories.
    parameters.categoryCount = options.ingestion == Ingestion::FloatNoRangeCopy ? 0 : std::max(0, scene.setting("categoryCount").toInt());
    return parameters;
}

void RectangleSeriesFeed::apply(QAccelPlot::RectangleSeries& series, PlasmaPart& part, const CommonOptions& options)
{
    if (part.count == 0 || part.categories.empty()) {
        applyRecords(series, part, options.ingestion);
        return;
    }
    switch (options.ingestion) {
    case Ingestion::FloatNoRangeMove:
        series.setDataFNoRange(std::move(part.floats), std::move(part.categories), part.count);
        break;
    case Ingestion::FloatMove:
        series.setDataF(std::move(part.floats), std::move(part.categories), part.count);
        break;
    case Ingestion::FloatNoRangeCopy:
        applyRecords(series, part, options.ingestion);
        break;
    case Ingestion::DoubleMove:
        series.setData(std::move(part.doubles), std::move(part.categories), part.count);
        break;
    case Ingestion::FloatPost:
        series.postData(std::move(part.floats), std::move(part.categories), part.count);
        break;
    }
}

} // namespace QAccelPlotExample
