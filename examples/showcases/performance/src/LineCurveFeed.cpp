//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
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
