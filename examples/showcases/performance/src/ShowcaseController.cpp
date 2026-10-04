//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ShowcaseController.hpp"

#include "LineCurveFeed.hpp"
#include "PointCloudFeed.hpp"
#include "RectangleSeriesFeed.hpp"

#include <QAccelPlot/series/LineCurve.hpp>
#include <QAccelPlot/series/PointCloud.hpp>
#include <QAccelPlot/series/RectangleSeries.hpp>

#include <QQuickWindow>
#include <QString>

#include <cstddef>
#include <utility>

namespace QAccelPlotExample {

ShowcaseController::ShowcaseController(QQuickWindow* window, QObject* root, std::shared_ptr<DataDeliveryMetrics> deliveryMetrics, const bool screenshotMode)
    : QObject(window)
    , root_(root)
    , deliveryMetrics_(std::move(deliveryMetrics))
    , feeders_{std::make_unique<SeriesFeeder<LineCurveFeed>>(root), std::make_unique<SeriesFeeder<PointCloudFeed>>(root),
          std::make_unique<SeriesFeeder<RectangleSeriesFeed>>(root)}
    , screenshotMode_(screenshotMode)
{
    elapsed_.start();
    // Pulls at most one batch per frame, so the worker never builds datasets the display cannot show.
    connect(window, &QQuickWindow::afterAnimating, this, &ShowcaseController::onFrame);
}

ShowcaseController::~ShowcaseController() = default;

void ShowcaseController::onFrame()
{
    const auto timeSeconds = screenshotMode_ ? 0.0 : static_cast<double>(elapsed_.elapsed()) / 1000.0;
    const auto page = requestedPage();
    if (page != activePage_) {
        activatePage(page, timeSeconds);
    }
    if (activePage_ && feeder(*activePage_).update(timeSeconds)) {
        deliveryMetrics_->dataApplied(steadyNanoseconds());
    }
}

std::optional<ShowcaseController::Page> ShowcaseController::requestedPage() const
{
    const auto name = root_->property("activePage").toString();
    if (name == QLatin1String("lineCurve")) {
        return Page::LineCurve;
    }
    if (name == QLatin1String("pointCloud")) {
        return Page::PointCloud;
    }
    if (name == QLatin1String("rectangleSeries")) {
        return Page::RectangleSeries;
    }
    return std::nullopt;
}

void ShowcaseController::activatePage(const std::optional<Page> page, const double timeSeconds)
{
    if (activePage_) {
        feeder(*activePage_).stop();
    }
    activePage_ = page;
    if (activePage_) {
        feeder(*activePage_).start(timeSeconds);
    }
    deliveryMetrics_->restartGapTracking();
}

PageFeeder& ShowcaseController::feeder(const Page page)
{
    return *feeders_[static_cast<std::size_t>(page)];
}

} // namespace QAccelPlotExample
