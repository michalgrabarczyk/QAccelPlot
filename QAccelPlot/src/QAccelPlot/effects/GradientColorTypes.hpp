//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QColor>

#include <optional>
#include <vector>

namespace QAccelPlot {

/// \brief A single color stop within a gradient definition.
struct GradientStopData {
    float position{0.0f};          ///< \brief Normalized stop position in [0, 1].
    QColor color{Qt::transparent}; ///< \brief Color at this stop.

    /// \brief Returns \c true if both stops have the same position and color.
    friend bool operator==(const GradientStopData& lhs, const GradientStopData& rhs)
    {
        return lhs.position == rhs.position && lhs.color == rhs.color;
    }

    /// \brief Returns \c true if the stops differ in position or color.
    friend bool operator!=(const GradientStopData& lhs, const GradientStopData& rhs)
    {
        return !(lhs == rhs);
    }
};

/// \brief Render-thread snapshot of gradient stroke (line-color) parameters.
///
/// Assembled on the main thread and consumed by the line renderer without
/// touching the QObject hierarchy.
struct GradientColorPayload {
    /// \brief Returns \c true if the payload has at least two stops and is enabled.
    bool isValid() const
    {
        return enabled && stops.size() >= 2;
    }

    bool enabled{false};                       ///< \brief \c true when gradient stroke is active.
    Qt::Orientation direction{Qt::Horizontal}; ///< \brief Gradient direction.
    std::vector<GradientStopData> stops;       ///< \brief Color stops in position order.
    /// \brief Fixed min for gradient normalisation; \c std::nullopt = derived from data range.
    std::optional<qreal> gradientValueMin;
    /// \brief Fixed max for gradient normalisation; \c std::nullopt = derived from data range.
    std::optional<qreal> gradientValueMax;
};

/// \brief Render-thread snapshot of gradient fill (area-under-curve) parameters.
///
/// Assembled on the main thread and consumed by the line renderer without
/// touching the QObject hierarchy.
struct GradientFillPayload {
    /// \brief Returns \c true if the payload has at least two stops, is enabled, and has non-zero opacity.
    bool isValid() const
    {
        return enabled && opacity > 0.0f && stops.size() >= 2;
    }

    bool enabled{false};                       ///< \brief \c true when gradient fill is active.
    Qt::Orientation direction{Qt::Horizontal}; ///< \brief Gradient direction.
    std::vector<GradientStopData> stops;       ///< \brief Color stops in position order.
    /// \brief Fixed min for gradient normalisation; \c std::nullopt = derived from data range.
    std::optional<qreal> gradientValueMin;
    /// \brief Fixed max for gradient normalisation; \c std::nullopt = derived from data range.
    std::optional<qreal> gradientValueMax;
    /// \brief Fixed baseline data value; \c std::nullopt = the smallest visible value of the Y axis.
    std::optional<qreal> baselineValue;
    float opacity{1.0f}; ///< \brief Overall opacity of the fill in [0, 1].
};

} // namespace QAccelPlot
