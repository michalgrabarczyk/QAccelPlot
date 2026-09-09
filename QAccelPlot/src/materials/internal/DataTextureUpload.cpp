//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "materials/internal/DataTextureUpload.hpp"

#include <QImage>
#include <QQuickWindow>
#include <QSGTexture>

#if QACCELPLOT_USE_QT_PRIVATE_API
#include <QtQuick/private/qsgplaintexture_p.h>
#endif

namespace QAccelPlot::Internal {

void uploadDataTexture(std::unique_ptr<QSGTexture>& texture, QQuickWindow* window, const QImage& image)
{
    if (!window || image.isNull()) {
        return;
    }

#if QACCELPLOT_USE_QT_PRIVATE_API
    if (texture) {
        static_cast<QSGPlainTexture*>(texture.get())->setImage(image);
        return;
    }
#endif

    texture.reset(window->createTextureFromImage(image));
}

} // namespace QAccelPlot::Internal
