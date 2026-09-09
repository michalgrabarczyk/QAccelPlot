//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "effects/GradientColorTypes.hpp"

#include <QColor>
#include <QObject>
#include <QQmlProperty>

#include <algorithm>
#include <vector>

namespace QAccelPlot {

/// \brief Reads a QML gradient stop object and appends it to \a outStops.
///
/// The stop object is expected to expose \c position (real) and \c color (color)
/// properties via the QML meta-object system.
inline void appendStopFromObject(std::vector<GradientStopData>& outStops, QObject* stopObject)
{
    if (!stopObject) {
        return;
    }

    const auto positionVariant = QQmlProperty::read(stopObject, QStringLiteral("position"));
    const auto colorVariant = QQmlProperty::read(stopObject, QStringLiteral("color"));

    if (!positionVariant.isValid() || !colorVariant.isValid()) {
        return;
    }

    const auto color = colorVariant.value<QColor>();
    if (!color.isValid()) {
        return;
    }

    const auto unclampedPosition = static_cast<float>(positionVariant.toReal());
    auto stopData = GradientStopData{};
    stopData.position = std::clamp(unclampedPosition, 0.0f, 1.0f);
    stopData.color = color;
    outStops.push_back(stopData);
}

} // namespace QAccelPlot
