//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "LineCurveFeed.hpp"

#include <QAccelPlot/series/LineCurve.hpp>

#include <QString>

#include <utility>

namespace QAccelPlotExample {
namespace {

// A curve takes a prebuilt vertex cache only for a solid line without markers or gradient effects.
bool takesVertexCache(const PageScene& scene)
{
    return scene.setting("lineStyle").toString() == QLatin1String("solid")
        && scene.setting("markerShape").toInt() == static_cast<int>(QAccelPlot::PlotSeries::MarkerShape::None)
        && scene.setting("effect").toString() == QLatin1String("none");
}

} // namespace

SineWaveParameters LineCurveFeed::parameters(const PageScene& scene, const CommonOptions& options)
{
    auto parameters = SineWaveParameters{};
    parameters.dataset = datasetParameters(options);
    parameters.frequencyScale = scene.setting("frequencyScale").toFloat();
    parameters.gapFraction = scene.setting("gapFraction").toFloat();
    parameters.vertexCache = takesVertexCache(scene);
    return parameters;
}

void LineCurveFeed::apply(QAccelPlot::LineCurve& curve, SineWavePart& part, const CommonOptions& options)
{
    // Only the no-range float setters take a prebuilt vertex cache.
    if (part.vertexCache.empty()) {
        applyRecords(curve, part, options.ingestion);
    } else if (options.ingestion == Ingestion::FloatNoRangeMove) {
        curve.setDataFNoRangeWithCache(std::move(part.floats), part.count, std::move(part.vertexCache));
    } else if (options.ingestion == Ingestion::FloatNoRangeCopy) {
        curve.setDataFNoRangeWithCache(part.floats.data(), part.count, std::move(part.vertexCache));
    } else {
        applyRecords(curve, part, options.ingestion);
    }
}

} // namespace QAccelPlotExample
