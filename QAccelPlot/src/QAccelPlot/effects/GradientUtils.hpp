//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/GradientColorTypes.hpp"

#include <QVariantList>

#include <vector>

class QObject;

namespace QAccelPlot {

class Colormap;

/// \brief Reads the stops of a QML \c Gradient into position order, covering the full [0, 1] range.
///
/// Stops are read from the \c stops list property, falling back to child objects. A single stop is
/// duplicated, and the first and last colors are extended to positions 0 and 1. Returns an empty
/// vector when \a gradient is null or has no valid stops.
///
/// \sa GradientFill, GradientStroke, PointCloud
std::vector<GradientStopData> readGradientStops(QObject* gradient);

/// \brief Reads a list of stop objects, each exposing \c position and \c color, into position order.
///
/// Normalized the same way as \c readGradientStops(): a single stop is duplicated and the end colors
/// are extended to 0 and 1. Entries that are not stop-like are skipped.
///
/// \sa Colormap
std::vector<GradientStopData> readGradientStopList(const QVariantList& stopObjects);

/// \brief Returns the color stops of a gradient effect: the \a colormap ramp when set, otherwise
/// the stops of \a gradient.
///
/// Returns an empty vector when both are null.
///
/// \sa GradientFill, GradientStroke
std::vector<GradientStopData> readEffectStops(const Colormap* colormap, QObject* gradient);

} // namespace QAccelPlot
