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

/// \brief Reads the stops of a QML \c Gradient into position order, covering the full [0, 1] range.
///
/// Stops are read from the \c stops list property, falling back to child objects. A single stop is
/// duplicated, and the first and last colors are extended to positions 0 and 1. Returns an empty
/// vector when \a gradient is null or has no valid stops.
inline std::vector<GradientStopData> readGradientStops(QObject* gradient)
{
    auto stops = std::vector<GradientStopData>{};
    if (!gradient) {
        return stops;
    }

    const auto stopsVariant = QQmlProperty::read(gradient, QStringLiteral("stops"));
    if (stopsVariant.isValid()) {
        const auto stopsList = stopsVariant.toList();
        for (const auto& stopVariant : stopsList) {
            appendStopFromObject(stops, stopVariant.value<QObject*>());
        }
    }

    if (stops.empty()) {
        const auto stopObjects = gradient->children();
        stops.reserve(static_cast<std::size_t>(stopObjects.size()));
        for (auto* stopObject : stopObjects) {
            appendStopFromObject(stops, stopObject);
        }
    }

    std::sort(stops.begin(), stops.end(), [](const auto& lhs, const auto& rhs) { return lhs.position < rhs.position; });

    if (stops.empty()) {
        return stops;
    }

    if (stops.size() == 1) {
        auto duplicatedStop = stops.front();
        duplicatedStop.position = 1.0f;
        stops.push_back(duplicatedStop);
    }

    if (stops.front().position > 0.0f) {
        auto extendedStop = stops.front();
        extendedStop.position = 0.0f;
        stops.insert(stops.begin(), extendedStop);
    }

    if (stops.back().position < 1.0f) {
        auto extendedStop = stops.back();
        extendedStop.position = 1.0f;
        stops.push_back(extendedStop);
    }

    return stops;
}

} // namespace QAccelPlot
