//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>
#include <QtQml/qqmlregistration.h>

#include <optional>
#include <vector>

namespace QAccelPlot {

/// \brief Namespace exposing the \c GradientDirection enum to QML.
namespace GradientDirectionNS {
Q_NAMESPACE
QML_NAMED_ELEMENT(GradientDirection)

/// \brief Determines the axis along which a gradient color is applied.
enum class Direction {
    Horizontal, ///< \brief Gradient varies along the X (data-value) axis.
    Vertical,   ///< \brief Gradient varies along the Y (data-value) axis.
};
Q_ENUM_NS(Direction)
} // namespace GradientDirectionNS

using GradientDirection = GradientDirectionNS::Direction;

/// \brief Namespace exposing the \c GradientFillBaseline enum to QML.
namespace GradientFillBaselineNS {
Q_NAMESPACE
QML_NAMED_ELEMENT(GradientFillBaseline)

/// \brief Determines where the baseline of a GradientFill area starts.
enum class Mode {
    AxisMinimum, ///< \brief Baseline follows the minimum visible value of the axis.
    Value,       ///< \brief Baseline is a fixed data-space value set via \c GradientFill::baselineValue.
};
Q_ENUM_NS(Mode)
} // namespace GradientFillBaselineNS

using GradientFillBaseline = GradientFillBaselineNS::Mode;

/// \brief Namespace exposing the \c GradientValueSource enum to QML.
namespace GradientValueSourceNS {
Q_NAMESPACE
QML_NAMED_ELEMENT(GradientValueSource)

/// \brief Determines how the gradient normalisation range is resolved.
enum class Source {
    DataRange, ///< \brief Range is derived from the curve's current data range.
    Fixed,     ///< \brief Range is set explicitly via \c gradientValueMin / \c gradientValueMax.
};
Q_ENUM_NS(Source)
} // namespace GradientValueSourceNS

using GradientValueSource = GradientValueSourceNS::Source;

/// \brief A single color stop within a gradient definition.
struct GradientStopData {
    float position{0.0f};          ///< \brief Normalized stop position in [0, 1].
    QColor color{Qt::transparent}; ///< \brief Color at this stop.
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

    bool enabled{false};                                        ///< \brief \c true when gradient stroke is active.
    GradientDirection direction{GradientDirection::Horizontal}; ///< \brief Gradient direction.
    std::vector<GradientStopData> stops;                        ///< \brief Color stops in position order.
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

    bool enabled{false};                                        ///< \brief \c true when gradient fill is active.
    GradientDirection direction{GradientDirection::Horizontal}; ///< \brief Gradient direction.
    std::vector<GradientStopData> stops;                        ///< \brief Color stops in position order.
    /// \brief Fixed min for gradient normalisation; \c std::nullopt = derived from data range.
    std::optional<qreal> gradientValueMin;
    /// \brief Fixed max for gradient normalisation; \c std::nullopt = derived from data range.
    std::optional<qreal> gradientValueMax;
    GradientFillBaseline baseline{GradientFillBaseline::AxisMinimum}; ///< \brief Baseline mode.
    qreal baselineValue{0.0};                                         ///< \brief Fixed baseline data value when mode is \c Value.
    float opacity{1.0f};                                              ///< \brief Overall opacity of the fill in [0, 1].
};

} // namespace QAccelPlot
