//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "ShowcaseController.hpp"

#include <QAccelPlot/series/LineCurve.hpp>
#include <QAccelPlot/series/PointCloud.hpp>
#include <QAccelPlot/series/RectangleList.hpp>

#include <QQuickWindow>
#include <QString>

#include <utility>

using namespace QAccelPlot;

namespace QAccelPlotExample {
namespace {

template <typename Worker> void setRunning(Worker& worker, const bool running, const int count, const double timeSeconds)
{
    if (!running) {
        worker.requestStop();
        return;
    }
    worker.setCount(count);
    worker.setTime(timeSeconds);
    worker.start();
}

template <typename Worker> bool takeBatch(Worker& worker, const int count, const double timeSeconds, typename Worker::Batch& batch)
{
    worker.setCount(count);
    worker.setTime(timeSeconds);
    return worker.tryConsume(batch);
}

} // namespace

ShowcaseController::ShowcaseController(QQuickWindow* window, QObject* root, std::shared_ptr<DataDeliveryMetrics> deliveryMetrics, const bool screenshotMode)
    : QObject(window)
    , root_(root)
    , lineCurve_(root->findChild<LineCurve*>(QStringLiteral("lineCurve")))
    , pointCloud_(root->findChild<PointCloud*>(QStringLiteral("pointCloud")))
    , rectangleList_(root->findChild<RectangleList*>(QStringLiteral("rectangleList")))
    , deliveryMetrics_(std::move(deliveryMetrics))
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
    if (!activePage_) {
        return;
    }

    switch (*activePage_) {
    case Page::LineCurve:
        updateLineCurve(timeSeconds);
        break;
    case Page::PointCloud:
        updatePointCloud(timeSeconds);
        break;
    case Page::RectangleList:
        updateRectangleList(timeSeconds);
        break;
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
    if (name == QLatin1String("rectangleList")) {
        return Page::RectangleList;
    }
    return std::nullopt;
}

void ShowcaseController::activatePage(const std::optional<Page> page, const double timeSeconds)
{
    if (activePage_) {
        setWorkerRunning(*activePage_, false, timeSeconds);
    }
    activePage_ = page;
    if (activePage_) {
        setWorkerRunning(*activePage_, true, timeSeconds);
    }
    deliveryMetrics_->restartGapTracking();
}

void ShowcaseController::setWorkerRunning(const Page page, const bool running, const double timeSeconds)
{
    switch (page) {
    case Page::LineCurve:
        setRunning(lineCurveWorker_, running, count(page), timeSeconds);
        break;
    case Page::PointCloud:
        setRunning(pointCloudWorker_, running, count(page), timeSeconds);
        break;
    case Page::RectangleList:
        setRunning(rectangleListWorker_, running, count(page), timeSeconds);
        break;
    }
}

int ShowcaseController::count(const Page page) const
{
    switch (page) {
    case Page::LineCurve:
        return root_->property("lineCurveCount").toInt();
    case Page::PointCloud:
        return root_->property("pointCloudCount").toInt();
    case Page::RectangleList:
        return root_->property("rectangleCount").toInt();
    }
    return 0;
}

void ShowcaseController::updateLineCurve(const double timeSeconds)
{
    auto batch = SineWaveBatch{};
    if (!takeBatch(lineCurveWorker_, count(Page::LineCurve), timeSeconds, batch) || !lineCurve_) {
        return;
    }
    if (batch.vertexCache.empty()) {
        lineCurve_->setDataFNoRange(std::move(batch.xy), batch.pointCount);
    } else {
        lineCurve_->setDataFNoRangeWithCache(std::move(batch.xy), batch.pointCount, std::move(batch.vertexCache));
    }
    dataApplied();
}

void ShowcaseController::updatePointCloud(const double timeSeconds)
{
    auto batch = GalaxyBatch{};
    if (!takeBatch(pointCloudWorker_, count(Page::PointCloud), timeSeconds, batch) || !pointCloud_) {
        return;
    }
    pointCloud_->setDataFNoRange(std::move(batch.xy), std::move(batch.values), batch.pointCount);
    dataApplied();
}

void ShowcaseController::updateRectangleList(const double timeSeconds)
{
    auto batch = PlasmaBatch{};
    if (!takeBatch(rectangleListWorker_, count(Page::RectangleList), timeSeconds, batch) || !rectangleList_) {
        return;
    }
    rectangleList_->setDataFNoRange(std::move(batch.rects), batch.rectangleCount);
    dataApplied();
}

void ShowcaseController::dataApplied()
{
    deliveryMetrics_->dataApplied(steadyNanoseconds());
}

} // namespace QAccelPlotExample
