//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
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
