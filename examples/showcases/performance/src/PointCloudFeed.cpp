//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "PointCloudFeed.hpp"

#include <QAccelPlot/series/PointCloud.hpp>

#include <utility>

namespace QAccelPlotExample {

GalaxyParameters PointCloudFeed::parameters(const PageScene& /*scene*/, const CommonOptions& options)
{
    auto parameters = GalaxyParameters{};
    parameters.dataset = datasetParameters(options);
    return parameters;
}

void PointCloudFeed::apply(QAccelPlot::PointCloud& cloud, GalaxyPart& part, const CommonOptions& /*options*/)
{
    cloud.setDataFNoRange(std::move(part.floats), std::move(part.values), part.count);
}

} // namespace QAccelPlotExample
