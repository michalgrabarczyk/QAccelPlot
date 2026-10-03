//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/inspection/internal/SelectionRectangle.hpp"

#include "QAccelPlot/inspection/SelectionTool.hpp"
#include "QAccelPlot/internal/OutlinedRectangle.hpp"

namespace QAccelPlot {

SelectionRectangle::SelectionRectangle(SelectionTool& tool)
    : tool_(tool)
    , rectangle_(new OutlinedRectangle(this))
{
    setParent(&tool);
    setClip(true);
    connect(&tool_, &SelectionTool::plotChanged, this, &SelectionRectangle::attach);
    connect(&tool_, &SelectionTool::pixelRectChanged, this, &SelectionRectangle::sync);
    connect(&tool_, &SelectionTool::fillColorChanged, this, &SelectionRectangle::sync);
    connect(&tool_, &SelectionTool::borderColorChanged, this, &SelectionRectangle::sync);
    connect(&tool_, &SelectionTool::rectangleVisibleChanged, this, &SelectionRectangle::sync);
    attach();
}

void SelectionRectangle::attach()
{
    const auto* plot = tool_.plot();
    auto* overlay = plot ? plot->overlay() : nullptr;
    setParentItem(overlay);
    if (overlay && overlay->childItems().constFirst() != this) {
        // A selection is a backdrop for the other overlays, such as an inspector's tooltip.
        stackBefore(overlay->childItems().constFirst());
    }
    sync();
}

void SelectionRectangle::sync()
{
    const auto* plot = tool_.plot();
    const auto rect = tool_.pixelRect();
    setVisible(plot && tool_.rectangleVisible() && !rect.isEmpty());
    if (!plot) {
        return;
    }
    const auto area = plot->plotRect();
    setPosition(area.topLeft());
    setSize(area.size());
    rectangle_->setPosition(rect.topLeft() - area.topLeft());
    rectangle_->setSize(rect.size());
    rectangle_->setColors(tool_.fillColor(), tool_.borderColor());
}

} // namespace QAccelPlot
