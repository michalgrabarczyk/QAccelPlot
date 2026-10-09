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

GalaxyParameters PointCloudFeed::parameters(const PageScene& scene, const CommonOptions& options)
{
    auto parameters = GalaxyParameters{};
    parameters.dataset = datasetParameters(options);
    // The raw-array setters take no values.
    parameters.values = scene.setting("values").toBool() && options.ingestion != Ingestion::FloatCopy;
    parameters.invalidFraction = scene.setting("invalidFraction").toFloat();
    return parameters;
}

void PointCloudFeed::apply(QAccelPlot::PointCloud& cloud, GalaxyPart& part, const CommonOptions& options)
{
    if (part.count == 0) {
        cloud.clearData();
        return;
    }
    switch (options.ingestion) {
    case Ingestion::FloatMove:
        cloud.setDataF(std::move(part.floats), std::move(part.values), part.count);
        break;
    case Ingestion::FloatCopy:
        applyRecords(cloud, part, options.ingestion);
        break;
    case Ingestion::DoubleMove:
        cloud.setData(std::move(part.doubles), std::move(part.values), part.count);
        break;
    case Ingestion::FloatPost:
        cloud.postData(std::move(part.floats), std::move(part.values), part.count);
        break;
    }
}

} // namespace QAccelPlotExample
