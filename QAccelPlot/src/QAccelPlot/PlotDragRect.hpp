//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QPointF>
#include <QRectF>

namespace QAccelPlot {

/// \brief Rectangle dragged out inside the plot area, shared by rectangle zoom and data selection.
class PlotDragRect {
public:
    /// \brief Starts a drag at \a position.
    void begin(const QPointF& position);
    /// \brief Moves the free corner to \a position, clamped to \a bounds.
    void moveTo(const QPointF& position, const QRectF& bounds);
    /// \brief Ends the drag.
    void end();
    /// \brief Returns true between \c begin() and \c end().
    [[nodiscard]] bool active() const;
    /// \brief Returns the normalized rectangle between the pressed and the free corner.
    [[nodiscard]] QRectF rect() const;

    /// \brief Returns true when the pressed modifiers are exactly the \a required ones.
    [[nodiscard]] static bool modifiersMatch(int pressed, int required);
    /// \brief Returns true when \a rect is at least \a minimumSize in every checked dimension.
    [[nodiscard]] static bool meetsMinimum(const QRectF& rect, qreal minimumSize, bool checkWidth = true, bool checkHeight = true);

private:
    bool active_{false};
    QPointF start_;
    QPointF end_;
};

} // namespace QAccelPlot
