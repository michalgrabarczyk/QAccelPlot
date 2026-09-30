//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot::Internal {

/// \brief Returns whether series accept hover events: \c false only when \c QACCELPLOT_HOVER_ENABLED is 0.
bool hoverEnabled();

/// \brief Returns \c true when \a window renders through a hardware scene graph backend that runs custom shaders.
bool supportsCustomShaderRendering(const QQuickWindow* window);

/// \brief Returns the largest texture width and height, in pixels, that \a window's GPU supports.
///
/// Falls back to 8192, supported by every target GPU, when the RHI cannot be queried (Qt older
/// than 6.6, or a build without the private Qt API). \c QACCELPLOT_MAX_TEXTURE_SIZE lowers the
/// result, for example to exercise capacity limits in tests.
int maxTextureSize(QQuickWindow* window);

} // namespace QAccelPlot::Internal
