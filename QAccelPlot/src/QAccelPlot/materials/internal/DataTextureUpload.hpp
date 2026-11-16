//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QtCore/qglobal.h>

#include <memory>

QT_FORWARD_DECLARE_CLASS(QImage)
QT_FORWARD_DECLARE_CLASS(QQuickWindow)
QT_FORWARD_DECLARE_CLASS(QSGTexture)

namespace QAccelPlot::Internal {

/// \brief Uploads an image to the live-data texture using the configured Qt API path.
void uploadDataTexture(std::unique_ptr<QSGTexture>& texture, QQuickWindow* window, const QImage& image);

} // namespace QAccelPlot::Internal
