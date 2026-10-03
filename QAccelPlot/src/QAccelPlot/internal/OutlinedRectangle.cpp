//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/internal/OutlinedRectangle.hpp"

#include <QQuickWindow>
#include <QSGRectangleNode>

#include <algorithm>
#include <array>

namespace QAccelPlot {

OutlinedRectangle::OutlinedRectangle(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::NoButton);
}

void OutlinedRectangle::setColors(const QColor& fill, const QColor& border)
{
    if (fill_ == fill && border_ == border) {
        return;
    }
    fill_ = fill;
    border_ = border;
    update();
}

QSGNode* OutlinedRectangle::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
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
        rectangle->setColor(i == 0 ? fill_ : border_);
        node = node->nextSibling();
    }
    return root;
}

} // namespace QAccelPlot
