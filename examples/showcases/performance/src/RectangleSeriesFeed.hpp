//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "PageScene.hpp"
#include "PlasmaGenerator.hpp"

namespace QAccelPlot {
class RectangleSeries;
} // namespace QAccelPlot

namespace QAccelPlotExample {

/// \brief Connects the RectangleSeries page to its plasma generator; see \c SeriesFeeder.
struct RectangleSeriesFeed {
    using Generator = PlasmaGenerator;
    using Series = QAccelPlot::RectangleSeries;

    static constexpr auto kSettingsProperty = "rectangleSeriesSettings";
    static constexpr auto kPlotName = "rectangleSeriesPlot";

    /// \brief Returns the generator parameters for the page's settings.
    static PlasmaParameters parameters(const PageScene& scene, const CommonOptions& options);
    /// \brief Hands \a part to \a series.
    static void apply(QAccelPlot::RectangleSeries& series, PlasmaPart& part, const CommonOptions& options);
};

} // namespace QAccelPlotExample
