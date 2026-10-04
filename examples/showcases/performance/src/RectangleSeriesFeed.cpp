//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "RectangleSeriesFeed.hpp"

#include <QAccelPlot/series/RectangleSeries.hpp>

#include <utility>

namespace QAccelPlotExample {

PlasmaParameters RectangleSeriesFeed::parameters(const PageScene& /*scene*/, const CommonOptions& options)
{
    auto parameters = PlasmaParameters{};
    parameters.dataset = datasetParameters(options);
    return parameters;
}

void RectangleSeriesFeed::apply(QAccelPlot::RectangleSeries& series, PlasmaPart& part, const CommonOptions& /*options*/)
{
    series.setDataFNoRange(std::move(part.floats), part.count);
}

} // namespace QAccelPlotExample
