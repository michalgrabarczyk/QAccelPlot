//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/GradientColorTypes.hpp"

#include <vector>

class QObject;

namespace QAccelPlot {

/// \brief Reads the stops of a QML \c Gradient into position order, covering the full [0, 1] range.
///
/// Stops are read from the \c stops list property, falling back to child objects. A single stop is
/// duplicated, and the first and last colors are extended to positions 0 and 1. Returns an empty
/// vector when \a gradient is null or has no valid stops.
///
/// \sa GradientFill, GradientStroke, PointCloud
std::vector<GradientStopData> readGradientStops(QObject* gradient);

} // namespace QAccelPlot
