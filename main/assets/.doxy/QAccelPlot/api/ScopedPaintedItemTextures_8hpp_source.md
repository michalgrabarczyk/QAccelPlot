

# File ScopedPaintedItemTextures.hpp

[**File List**](files.md) **>** [**export**](dir_165621f1d3f543afcb149fc22fafadb8.md) **>** [**internal**](dir_502a5035d8e883b896944d94c324e4a9.md) **>** [**ScopedPaintedItemTextures.hpp**](ScopedPaintedItemTextures_8hpp.md)

[Go to the documentation of this file](ScopedPaintedItemTextures_8hpp.md)


```C++
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

class ScopedPaintedItemTextures {
public:
    ScopedPaintedItemTextures(QQuickItem* root, qreal pixelsPerLogicalPixel, qreal devicePixelRatio);
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
```


