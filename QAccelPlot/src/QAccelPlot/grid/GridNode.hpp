//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>
#include <QRectF>
#include <QSGNode>
#include <QVector>

#include <functional>

namespace QAccelPlot {

class Axis;
class Grid;

/// \brief Internal QSGNode responsible for rendering the plot grid into the scene graph.
///
/// Computes grid line positions from the same tick step as \c AxisTickPainter so that
/// grid lines stay aligned with axis tick marks under all zoom levels, including log scale.
class GridNode : public QSGNode {
public:
    /// \brief Constructs an empty GridNode.
    explicit GridNode() = default;

    /// \brief Recomputes and updates all grid geometry from the supplied configuration objects.
    /// \param grid Grid configuration (line widths, visibility flags, colors).
    /// \param xAxis Horizontal axis used to determine column positions.
    /// \param yAxis Vertical axis used to determine row positions.
    /// \param plotRect Plot area rectangle in item-local pixel coordinates.
    void update(const Grid* grid, const Axis* xAxis, const Axis* yAxis, const QRectF& plotRect);

private:
    // Groups the orientation-specific inputs so the shared loop logic in
    // collectAxisGridLines() can handle both vertical and horizontal grid lines.
    struct GridLineCollectionParams {
        const Axis* axis;
        bool gridLinesVisible;
        bool subGridLinesVisible;
        // makeRect(dataValue, halfLineWidth) -> the QRectF to add for that grid line
        std::function<QRectF(qreal dataValue, qreal halfWidth)> makeRect;
    };

    void collectAxisGridLines(QVector<QRectF>& mainRects, QVector<QRectF>& subRects, const Grid* grid, const GridLineCollectionParams& p) const;

    QRectF plotRect_;
};

} // namespace QAccelPlot
