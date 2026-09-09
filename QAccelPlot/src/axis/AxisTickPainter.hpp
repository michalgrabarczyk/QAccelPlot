//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "axis/Axis.hpp"
#include "axis/AxisTicker.hpp"

#include <QColor>
#include <QRectF>

#include <functional>

QT_FORWARD_DECLARE_CLASS(QPainter)

namespace QAccelPlot {

/// \brief Internal helper that paints tick marks and labels onto a QPainter for a single Axis.
///
/// Used by \c Axis::paint(). Also provides \c computeNiceStep(), which is shared with
/// \c GridNode to keep grid lines aligned with tick positions.
class AxisTickPainter {
public:
    /// \brief All inputs required for a single paint call, bundled to reduce parameter count.
    struct Params {
        qreal viewportMin{0.0};                          ///< \brief Axis minimum value.
        qreal viewportMax{1.0};                          ///< \brief Axis maximum value.
        Axis::Orientation orientation{Axis::Horizontal}; ///< \brief Axis orientation.
        Axis::Side side{Axis::Left};                     ///< \brief Axis side.
        bool logScale{false};                            ///< \brief Whether log-scale tick placement is used.
        bool hovered{false};                             ///< \brief Whether the axis is currently hovered.
        const AxisTicker* ticker{nullptr};               ///< \brief Tick style configuration object.
        QColor hoverColor;                               ///< \brief Label/tick color when hovered.
        QColor defaultSubtickColor;                      ///< \brief Sub-tick color when not hovered.
        qreal tickStep{0.0};                             ///< \brief Computed major tick step (filled by \c paintLinearTicks).
        bool clampEdgeLabels{false};                     ///< \brief Whether labels at the extremes are clamped inward.
        qreal labelOverflow{0.0};                        ///< \brief Extra space reserved for edge label overflow.
    };

    /// \brief Function type that maps a data value to a pixel position along the axis.
    using MapToPosition = std::function<qreal(qreal value, qreal length)>;

    /// \brief Groups the QPainter context arguments passed into sub-painting helpers.
    struct PaintContext {
        QPainter* painter; ///< \brief Active painter.
        QRectF rect;       ///< \brief Bounding rectangle of the axis widget.
        qreal axisX;       ///< \brief Pixel X coordinate of the axis line.
        qreal axisY;       ///< \brief Pixel Y coordinate of the axis line.
    };

    /// \brief Paints all tick marks and labels onto \a painter using the supplied \a params.
    static void paintTicks(QPainter* painter, const QRectF& rect, qreal axisX, qreal axisY, const Params& params, const MapToPosition& mapToPosition);

    /// \brief Returns a "nice" major tick step for the given \a viewportMin, \a viewportMax range and target \a tickCount.
    ///
    /// Shared by both AxisTickPainter and GridNode so grid lines remain aligned with ticks.
    static qreal computeNiceStep(qreal viewportMin, qreal viewportMax, int tickCount);

private:
    static void drawTickLabel(QPainter* painter, const QRectF& labelRect, int alignment, const QString& label, qreal rotation);
    static void paintTick(const PaintContext& ctx, qreal value, const Params& params, const MapToPosition& mapToPosition);
    static void paintSubtick(const PaintContext& ctx, qreal value, const Params& params, const MapToPosition& mapToPosition);
    static void paintLogScaleTicks(const PaintContext& ctx, const Params& params, const MapToPosition& mapToPosition);
    static void paintLinearTicks(const PaintContext& ctx, const Params& params, const MapToPosition& mapToPosition);
};

} // namespace QAccelPlot
