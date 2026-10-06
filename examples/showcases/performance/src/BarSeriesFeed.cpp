//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "BarSeriesFeed.hpp"

#include <QAccelPlot/series/BarSeries.hpp>

#include <algorithm>

namespace QAccelPlotExample {

InterferenceParameters BarSeriesFeed::parameters(const PageScene& scene, const CommonOptions& options)
{
    auto parameters = InterferenceParameters{};
    parameters.dataset = datasetParameters(options);
    // The raw-array setters take no categories.
    parameters.categoryCount = options.ingestion == Ingestion::FloatNoRangeCopy ? 0 : std::max(0, scene.setting("categoryCount").toInt());
    return parameters;
}

void BarSeriesFeed::apply(QAccelPlot::BarSeries& series, InterferencePart& part, const CommonOptions& options)
{
    applyCategorizedRecords(series, part, options.ingestion);
}

} // namespace QAccelPlotExample
