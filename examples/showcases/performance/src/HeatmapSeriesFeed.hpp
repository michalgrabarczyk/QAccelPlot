//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "PageScene.hpp"
#include "SpectrogramGenerator.hpp"

namespace QAccelPlot {
class HeatmapSeries;
} // namespace QAccelPlot

namespace QAccelPlotExample {

/// \brief Connects the HeatmapSeries page to its spectrogram generator; see \c SeriesFeeder.
struct HeatmapSeriesFeed {
    using Generator = SpectrogramGenerator;
    using Series = QAccelPlot::HeatmapSeries;

    static constexpr auto kSettingsProperty = "heatmapSeriesSettings";
    static constexpr auto kPlotName = "heatmapSeriesPlot";

    /// \brief Returns the generator parameters for the page's settings.
    static SpectrogramParameters parameters(const PageScene& scene, const CommonOptions& options);
    /// \brief Places \a series on the rows of \a part and hands it the values.
    static void apply(QAccelPlot::HeatmapSeries& series, SpectrogramPart& part, const CommonOptions& options);
};

} // namespace QAccelPlotExample
