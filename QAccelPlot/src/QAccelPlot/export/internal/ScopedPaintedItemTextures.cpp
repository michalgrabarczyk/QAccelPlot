//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/export/internal/ScopedPaintedItemTextures.hpp"

#include <QtMath>

namespace QAccelPlot::Internal {

ScopedPaintedItemTextures::ScopedPaintedItemTextures(QQuickItem* root, const qreal pixelsPerLogicalPixel, const qreal devicePixelRatio)
{
    if (root && devicePixelRatio > 0.0) {
        // QQuickPaintedItem multiplies an explicit textureSize by the device pixel ratio.
        resize(root, pixelsPerLogicalPixel / devicePixelRatio);
    }
}

ScopedPaintedItemTextures::~ScopedPaintedItemTextures()
{
    for (const auto& saved : saved_) {
        if (saved.item) {
            saved.item->setTextureSize(saved.textureSize);
            saved.item->update();
        }
    }
}

void ScopedPaintedItemTextures::resize(QQuickItem* root, const qreal textureScale)
{
    auto pending = QList<QQuickItem*>{root};
    while (!pending.isEmpty()) {
        auto* item = pending.takeLast();
        auto* painted = qobject_cast<QQuickPaintedItem*>(item);
        if (painted && painted->width() > 0.0 && painted->height() > 0.0) {
            saved_.append({painted, painted->textureSize()});
            painted->setTextureSize(QSize(qCeil(painted->width() * textureScale), qCeil(painted->height() * textureScale)));
            painted->update();
        }
        pending.append(item->childItems());
    }
}

} // namespace QAccelPlot::Internal
