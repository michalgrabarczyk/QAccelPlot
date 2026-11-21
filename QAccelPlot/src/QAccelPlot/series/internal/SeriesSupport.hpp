//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/transitions/internal/TransitionFrame.hpp"

#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot {
class Axis;
}

namespace QAccelPlot::Internal {

/// \brief Returns whether series accept hover events: \c false only when \c QACCELPLOT_HOVER_ENABLED is 0.
bool hoverEnabled();

/// \brief Returns \c true when \a window renders through a hardware scene graph backend that runs custom shaders.
bool supportsCustomShaderRendering(const QQuickWindow* window);

/// \brief Returns how many records a series draws that reads them from the data texture by a float vertex id.
///
/// A record takes \a floatsPerRecord floats of the data texture and \a vertexBytesPerRecord bytes
/// of vertices. The count is limited by the texture size of \a window's GPU, by the 2^24 ids a
/// float holds exactly, and by \c QSGGeometry, which sizes its vertex buffer in \c int bytes.
int recordCapacity(QQuickWindow* window, int floatsPerRecord, int vertexBytesPerRecord);

/// \brief Returns \a feather clamped to the [0, 10] pixel range of the series' \c antialiasingFeather properties.
qreal clampAntialiasingFeather(qreal feather);

/// \brief Stores \a requested, clamped to 0 or more, in \a member and returns \c true when that changes it.
///
/// A non-finite \a requested, or one nearly equal to \a member, leaves \a member unchanged.
bool assignNonNegative(qreal& member, qreal requested);

/// \brief Returns the viewport of \a valueAxis that places grow baselines; without an axis, both edges are 0.
GrowViewport growViewport(const Axis* valueAxis);

/// \brief Returns the value that items drawn from \a baseline on \a valueAxis grow from.
///
/// It is \a baseline, or the viewport edge the items are drawn from when \a valueAxis cannot place
/// it: an infinite baseline, or one at or below 0 on a log axis. Without an axis, an infinite
/// baseline gives 0.
double growBaseline(double baseline, const Axis* valueAxis);

} // namespace QAccelPlot::Internal
