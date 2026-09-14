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
#include "axis/AxisTicks.hpp"

#include <QColor>
#include <QRectF>

#include <functional>

QT_FORWARD_DECLARE_CLASS(QPainter)

namespace QAccelPlot {

/// \brief Internal helper that computes and paints tick marks and labels for a single Axis.
///
/// Tick computation and painting are split: \c computeTicks() formats labels and must run on
/// the formatter's (GUI) thread, while \c paintTicks() only draws precomputed ticks and is safe to
/// call from \c Axis::paint() on the render thread. Also provides \c computeNiceStep(), which is shared
/// with \c GridNode to keep grid lines aligned with tick positions.
class AxisTickPainter {
public:
    /// \brief All style inputs required for a single paint call, bundled to reduce parameter count.
    struct Params {
        Axis::Orientation orientation{Axis::Horizontal}; ///< \brief Axis orientation.
        Axis::Side side{Axis::Left};                     ///< \brief Axis side.
        bool hovered{false};                             ///< \brief Whether the axis is currently hovered.
        const AxisTicker* ticker{nullptr};               ///< \brief Tick style configuration object.
        QColor hoverColor;                               ///< \brief Label/tick color when hovered.
        QColor defaultSubtickColor;                      ///< \brief Sub-tick color when not hovered.
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

    /// \brief Returns the visible ticks, subticks and formatted labels for the \a viewportMin to \a viewportMax range.
    ///
    /// Uses log-scale placement when \a logScale is set and both bounds are positive. Labels are produced by
    /// \a ticker's formatter, which may invoke a QML/JS callback, so this must be called on the formatter's thread.
    static AxisTicks computeTicks(qreal viewportMin, qreal viewportMax, bool logScale, const AxisTicker* ticker);

    /// \brief Paints \a ticks and their labels onto \a painter, placing them according to \a params and \a mapToPosition.
    static void paintTicks(
        QPainter* painter, const QRectF& rect, qreal axisX, qreal axisY, const Params& params, const AxisTicks& ticks, const MapToPosition& mapToPosition);

    /// \brief Returns a "nice" major tick step for the given \a viewportMin, \a viewportMax range and target \a tickCount.
    ///
    /// Shared by both AxisTickPainter and GridNode so grid lines remain aligned with ticks.
    static qreal computeNiceStep(qreal viewportMin, qreal viewportMax, int tickCount);

private:
    static AxisTicks computeLogScaleTicks(qreal viewportMin, qreal viewportMax, const AxisTicker& ticker);
    static AxisTicks computeLinearTicks(qreal viewportMin, qreal viewportMax, const AxisTicker& ticker);
    static void paintTick(const PaintContext& ctx, const AxisTick& tick, const Params& params, const MapToPosition& mapToPosition);
    static void paintSubtick(const PaintContext& ctx, qreal value, const Params& params, const MapToPosition& mapToPosition);
    static void drawTickLabel(QPainter* painter, const QRectF& labelRect, int alignment, const QString& label, qreal rotation);
};

} // namespace QAccelPlot
