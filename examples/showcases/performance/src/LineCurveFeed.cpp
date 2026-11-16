//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "LineCurveFeed.hpp"

#include <QAccelPlot/series/LineCurve.hpp>

namespace QAccelPlotExample {

SineWaveParameters LineCurveFeed::parameters(const PageScene& scene, const CommonOptions& options)
{
    auto parameters = SineWaveParameters{};
    parameters.dataset = datasetParameters(options);
    parameters.gapFraction = scene.setting("gapFraction").toFloat();
    return parameters;
}

void LineCurveFeed::apply(QAccelPlot::LineCurve& curve, SineWavePart& part, const CommonOptions& options)
{
    applyRecords(curve, part, options.ingestion);
}

} // namespace QAccelPlotExample
