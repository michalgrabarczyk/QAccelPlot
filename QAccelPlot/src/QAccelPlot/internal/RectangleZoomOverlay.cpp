//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/internal/RectangleZoomOverlay.hpp"
#include "QAccelPlot/PlotRectangleZoom.hpp"

namespace QAccelPlot::Internal {

RectangleZoomOverlay::RectangleZoomOverlay(QQuickItem* parent, PlotRectangleZoom* configuration)
    : OutlinedRectangle(parent)
    , configuration_(configuration)
{
    setZ(0.5);
    connect(configuration_, &PlotRectangleZoom::activeChanged, this, &RectangleZoomOverlay::syncSelection);
    connect(configuration_, &PlotRectangleZoom::selectionRectChanged, this, &RectangleZoomOverlay::syncSelection);
    connect(configuration_, &PlotRectangleZoom::visibleChanged, this, &RectangleZoomOverlay::syncSelection);
    connect(configuration_, &PlotRectangleZoom::fillColorChanged, this, &RectangleZoomOverlay::syncStyle);
    connect(configuration_->border(), &Border::colorChanged, this, &RectangleZoomOverlay::syncStyle);
    connect(configuration_->border(), &Border::widthChanged, this, &RectangleZoomOverlay::syncStyle);
    syncSelection();
    syncStyle();
}

void RectangleZoomOverlay::syncSelection()
{
    const auto rect = configuration_->selectionRect();
    setPosition(rect.topLeft());
    setSize(rect.size());
    setVisible(configuration_->isVisible() && configuration_->active() && !rect.isEmpty());
    update();
}

void RectangleZoomOverlay::syncStyle()
{
    setStyle(configuration_->fillColor(), configuration_->border()->color(), configuration_->border()->width());
}

} // namespace QAccelPlot::Internal
