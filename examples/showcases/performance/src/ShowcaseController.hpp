//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "DataDeliveryMetrics.hpp"
#include "GalaxyGenerator.hpp"
#include "GenerationWorker.hpp"
#include "PlasmaGenerator.hpp"
#include "SineWaveGenerator.hpp"

#include <QElapsedTimer>
#include <QObject>
#include <QPointer>

#include <memory>
#include <optional>

class QQuickWindow;

namespace QAccelPlot {
class LineCurve;
class PointCloud;
class RectangleList;
} // namespace QAccelPlot

namespace QAccelPlotExample {

/// \brief Feeds the visible showcase page from its generation worker, one batch per frame.
///
/// Reads \c activePage and the per-page count properties from the QML root. Only the active
/// page's worker runs; switching pages stops the previous worker.
class ShowcaseController final : public QObject {
public:
    /// \brief Showcase pages, each fed by its own worker.
    enum class Page { LineCurve, PointCloud, RectangleList };

    /// \brief Connects to \a window's frames and looks up the series below \a root.
    ///
    /// In \a screenshotMode every batch is generated at time zero.
    ShowcaseController(QQuickWindow* window, QObject* root, std::shared_ptr<DataDeliveryMetrics> deliveryMetrics, bool screenshotMode);
    ~ShowcaseController() override;

private:
    void onFrame();
    std::optional<Page> requestedPage() const;
    void activatePage(std::optional<Page> page, double timeSeconds);
    void setWorkerRunning(Page page, bool running, double timeSeconds);
    int count(Page page) const;
    void updateLineCurve(double timeSeconds);
    void updatePointCloud(double timeSeconds);
    void updateRectangleList(double timeSeconds);
    void dataApplied();

    QObject* root_;
    QPointer<QAccelPlot::LineCurve> lineCurve_;
    QPointer<QAccelPlot::PointCloud> pointCloud_;
    QPointer<QAccelPlot::RectangleList> rectangleList_;
    std::shared_ptr<DataDeliveryMetrics> deliveryMetrics_;
    GenerationWorker<SineWaveGenerator> lineCurveWorker_;
    GenerationWorker<GalaxyGenerator> pointCloudWorker_;
    GenerationWorker<PlasmaGenerator> rectangleListWorker_;
    std::optional<Page> activePage_;
    QElapsedTimer elapsed_;
    bool screenshotMode_;
};

} // namespace QAccelPlotExample
