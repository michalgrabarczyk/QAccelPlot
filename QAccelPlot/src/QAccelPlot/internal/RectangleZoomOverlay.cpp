//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/internal/RectangleZoomOverlay.hpp"
#include "QAccelPlot/PlotRectangleZoom.hpp"

#include <QQuickWindow>
#include <QSGRectangleNode>

#include <algorithm>
#include <array>

namespace QAccelPlot {

RectangleZoomOverlay::RectangleZoomOverlay(QQuickItem* parent, PlotRectangleZoom* configuration)
    : QQuickItem(parent)
    , configuration_(configuration)
{
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::NoButton);
    setZ(0.5);
    connect(configuration_, &PlotRectangleZoom::activeChanged, this, &RectangleZoomOverlay::syncSelection);
    connect(configuration_, &PlotRectangleZoom::selectionRectChanged, this, &RectangleZoomOverlay::syncSelection);
    connect(configuration_, &PlotRectangleZoom::fillColorChanged, this, &RectangleZoomOverlay::update);
    connect(configuration_, &PlotRectangleZoom::borderColorChanged, this, &RectangleZoomOverlay::update);
    syncSelection();
}

QSGNode* RectangleZoomOverlay::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    auto* root = oldNode ? oldNode : new QSGNode;
    if (root->childCount() == 0) {
        for (auto i = 0; i < 5; ++i) {
            root->appendChildNode(window()->createRectangleNode());
        }
    }
    const auto outline = std::min({qreal{1}, width() / 2, height() / 2});
    const auto rects = std::array<QRectF, 5>{QRectF{outline, outline, width() - 2 * outline, height() - 2 * outline}, QRectF{0, 0, width(), outline},
        QRectF{0, height() - outline, width(), outline}, QRectF{0, outline, outline, height() - 2 * outline},
        QRectF{width() - outline, outline, outline, height() - 2 * outline}};
    auto* node = root->firstChild();
    for (size_t i = 0; i < rects.size(); ++i) {
        auto* rectangle = static_cast<QSGRectangleNode*>(node);
        rectangle->setRect(rects[i]);
        rectangle->setColor(i == 0 ? configuration_->fillColor() : configuration_->borderColor());
        node = node->nextSibling();
    }
    return root;
}

void RectangleZoomOverlay::syncSelection()
{
    const auto rect = configuration_->selectionRect();
    setPosition(rect.topLeft());
    setSize(rect.size());
    setVisible(configuration_->active() && !rect.isEmpty());
    update();
}

} // namespace QAccelPlot
