//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "DataDeliveryMetrics.hpp"
#include "SeriesFeeder.hpp"

#include <QElapsedTimer>
#include <QObject>

#include <array>
#include <memory>
#include <optional>

class QQuickWindow;

namespace QAccelPlotExample {

/// \brief Feeds the visible showcase page, one batch per frame.
///
/// Reads \c activePage and the per-page settings objects from the QML root. Only the active
/// page's feeder runs; switching pages stops the previous one.
class ShowcaseController final : public QObject {
public:
    /// \brief Showcase pages, each fed by its own feeder.
    enum class Page { LineCurve, PointCloud, RectangleSeries };

    /// \brief Connects to \a window's frames and looks up the pages below \a root.
    ///
    /// In \a screenshotMode every batch is generated at time zero.
    ShowcaseController(QQuickWindow* window, QObject* root, std::shared_ptr<DataDeliveryMetrics> deliveryMetrics, bool screenshotMode);
    ~ShowcaseController() override;

private:
    void onFrame();
    std::optional<Page> requestedPage() const;
    void activatePage(std::optional<Page> page, double timeSeconds);
    PageFeeder& feeder(Page page);

    QObject* root_;
    std::shared_ptr<DataDeliveryMetrics> deliveryMetrics_;
    std::array<std::unique_ptr<PageFeeder>, 3> feeders_;
    std::optional<Page> activePage_;
    QElapsedTimer elapsed_;
    bool screenshotMode_;
};

} // namespace QAccelPlotExample
