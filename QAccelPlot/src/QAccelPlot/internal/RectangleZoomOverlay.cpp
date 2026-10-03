//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/internal/RectangleZoomOverlay.hpp"
#include "QAccelPlot/PlotRectangleZoom.hpp"

namespace QAccelPlot {

RectangleZoomOverlay::RectangleZoomOverlay(QQuickItem* parent, PlotRectangleZoom* configuration)
    : OutlinedRectangle(parent)
    , configuration_(configuration)
{
    setZ(0.5);
    connect(configuration_, &PlotRectangleZoom::activeChanged, this, &RectangleZoomOverlay::syncSelection);
    connect(configuration_, &PlotRectangleZoom::selectionRectChanged, this, &RectangleZoomOverlay::syncSelection);
    connect(configuration_, &PlotRectangleZoom::fillColorChanged, this, &RectangleZoomOverlay::syncColors);
    connect(configuration_, &PlotRectangleZoom::borderColorChanged, this, &RectangleZoomOverlay::syncColors);
    syncSelection();
    syncColors();
}

void RectangleZoomOverlay::syncSelection()
{
    const auto rect = configuration_->selectionRect();
    setPosition(rect.topLeft());
    setSize(rect.size());
    setVisible(configuration_->active() && !rect.isEmpty());
    update();
}

void RectangleZoomOverlay::syncColors()
{
    setColors(configuration_->fillColor(), configuration_->borderColor());
}

} // namespace QAccelPlot
