//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "GalaxyGenerator.hpp"
#include "PageScene.hpp"

namespace QAccelPlot {
class PointCloud;
} // namespace QAccelPlot

namespace QAccelPlotExample {

/// \brief Connects the PointCloud page to its galaxy generator; see \c SeriesFeeder.
struct PointCloudFeed {
    using Generator = GalaxyGenerator;
    using Series = QAccelPlot::PointCloud;

    static constexpr auto kSettingsProperty = "pointCloudSettings";
    static constexpr auto kPlotName = "pointCloudPlot";

    /// \brief Returns the generator parameters for the page's settings.
    static GalaxyParameters parameters(const PageScene& scene, const CommonOptions& options);
    /// \brief Hands \a part to \a cloud.
    static void apply(QAccelPlot::PointCloud& cloud, GalaxyPart& part, const CommonOptions& options);
};

} // namespace QAccelPlotExample
