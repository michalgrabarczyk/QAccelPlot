//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot::Internal {

/// \brief Returns the largest texture width and height, in pixels, that \a window's GPU supports.
///
/// Falls back to 8192 when the RHI cannot be queried (Qt older than 6.6, or a build without the
/// private Qt API). \c QACCELPLOT_MAX_TEXTURE_SIZE lowers the result, for example to exercise
/// capacity limits in tests.
int maxTextureSize(QQuickWindow* window);

} // namespace QAccelPlot::Internal
