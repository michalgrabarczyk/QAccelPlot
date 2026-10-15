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

/// \brief Returns how many records a series draws that reads them from the data texture by a float vertex id.
///
/// A record takes \a floatsPerRecord floats of the data texture and \a vertexBytesPerRecord bytes
/// of vertices. The count is limited by the texture size of \a window's GPU, by the 2^24 ids a
/// float holds exactly, and by \c QSGGeometry, which sizes its vertex buffer in \c int bytes.
int recordCapacity(QQuickWindow* window, int floatsPerRecord, int vertexBytesPerRecord);

/// \brief Returns whether \a recordCount exceeds \a capacity, and warns when it newly does.
///
/// Pass the previous result as \a wasOverCapacity. A series that stays over capacity, for example
/// while streaming, warns once; one whose data fits again and then outgrows the capacity warns
/// again. \a seriesType and \a records name the series and its records in the warning.
bool warnIfOutgrowsCapacity(bool wasOverCapacity, int recordCount, int capacity, const char* seriesType, const char* records);

} // namespace QAccelPlot::Internal
