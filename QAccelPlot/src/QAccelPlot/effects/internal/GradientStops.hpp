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
} // namespace QAccelPlot

namespace QAccelPlot::Internal {

/// \brief Reads a list of stop objects, each exposing \c position and \c color, into position order.
///
/// A single stop is duplicated, and the first and last colors are extended to positions 0 and 1.
/// Entries that are not stop-like are skipped.
///
/// \sa Colormap
std::vector<GradientStopData> readGradientStopList(const QVariantList& stopObjects);

/// \brief Returns the color stops of a gradient effect: the \a colormap ramp when set, otherwise
/// the stops of the QML \c Gradient \a gradient.
///
/// \a gradient stops are read from its \c stops list property, falling back to child objects, and
/// normalized like \c readGradientStopList(). Returns an empty vector when both are null.
///
/// \sa GradientFill, GradientStroke
std::vector<GradientStopData> readEffectStops(const Colormap* colormap, QObject* gradient);

} // namespace QAccelPlot::Internal
