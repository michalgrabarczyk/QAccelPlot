//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "InterferenceGenerator.hpp"
#include "PageScene.hpp"

namespace QAccelPlot {
class BarSeries;
} // namespace QAccelPlot

namespace QAccelPlotExample {

/// \brief Connects the BarSeries page to its interference generator; see \c SeriesFeeder.
struct BarSeriesFeed {
    using Generator = InterferenceGenerator;
    using Series = QAccelPlot::BarSeries;

    static constexpr auto kSettingsProperty = "barSeriesSettings";
    static constexpr auto kPlotName = "barSeriesPlot";

    /// \brief Returns the generator parameters for the page's settings.
    static InterferenceParameters parameters(const PageScene& scene, const CommonOptions& options);
    /// \brief Hands \a part to \a series.
    static void apply(QAccelPlot::BarSeries& series, InterferencePart& part, const CommonOptions& options);
};

} // namespace QAccelPlotExample
