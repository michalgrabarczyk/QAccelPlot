//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
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
