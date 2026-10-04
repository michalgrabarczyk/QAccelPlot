//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "PageScene.hpp"
#include "SineWaveGenerator.hpp"

namespace QAccelPlot {
class LineCurve;
} // namespace QAccelPlot

namespace QAccelPlotExample {

/// \brief Connects the LineCurve page to its sine wave generator; see \c SeriesFeeder.
struct LineCurveFeed {
    using Generator = SineWaveGenerator;
    using Series = QAccelPlot::LineCurve;

    static constexpr auto kSettingsProperty = "lineCurveSettings";
    static constexpr auto kPlotName = "lineCurvePlot";

    /// \brief Returns the generator parameters for the page's settings.
    static SineWaveParameters parameters(const PageScene& scene, const CommonOptions& options);
    /// \brief Hands \a part to \a curve.
    static void apply(QAccelPlot::LineCurve& curve, SineWavePart& part, const CommonOptions& options);
};

} // namespace QAccelPlotExample
