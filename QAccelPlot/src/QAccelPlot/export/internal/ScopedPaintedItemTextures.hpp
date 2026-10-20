//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QList>
#include <QPointer>
#include <QQuickPaintedItem>
#include <QSize>

namespace QAccelPlot::Internal {

/// \brief Rasterizes the painted items under a root item at a capture scale until destroyed.
///
/// A \c QQuickPaintedItem rasterizes into a texture sized for its window's device pixel ratio, so
/// an item grab at a higher scale would magnify, and blur, that texture. This raises every painted
/// descendant's texture to \a pixelsPerLogicalPixel and restores the previous texture sizes on
/// destruction.
class ScopedPaintedItemTextures {
public:
    /// \brief Resizes the textures of \a root and its painted descendants rendered at \a devicePixelRatio.
    ScopedPaintedItemTextures(QQuickItem* root, qreal pixelsPerLogicalPixel, qreal devicePixelRatio);
    /// \brief Restores the previous texture sizes of the items that still exist.
    ~ScopedPaintedItemTextures();

    ScopedPaintedItemTextures(const ScopedPaintedItemTextures&) = delete;
    ScopedPaintedItemTextures& operator=(const ScopedPaintedItemTextures&) = delete;
    ScopedPaintedItemTextures(ScopedPaintedItemTextures&&) = delete;
    ScopedPaintedItemTextures& operator=(ScopedPaintedItemTextures&&) = delete;

private:
    struct SavedTexture {
        QPointer<QQuickPaintedItem> item;
        QSize textureSize;
    };

    void resize(QQuickItem* root, qreal textureScale);

    QList<SavedTexture> saved_;
};

} // namespace QAccelPlot::Internal
