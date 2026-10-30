//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QHoverEvent>
#include <QMouseEvent>
#include <QObject>
#include <QPointF>

#include <optional>

namespace QAccelPlot::Internal {

/// \brief Tracks the pointer over a plot from the plot's mouse and hover events.
///
/// A touch places the pointer where the finger is and leaves it there after the release,
/// until the next tap or mouse movement.
class PointerTracker final : public QObject {
    Q_OBJECT

public:
    /// \brief Constructs a tracker owned by \a parent.
    explicit PointerTracker(QObject* parent = nullptr);

    /// \brief Returns the pointer position in plot-local logical pixels, or nothing while no pointer is over the plot.
    [[nodiscard]] std::optional<QPointF> position() const;

    /// \brief Places the pointer at a mouse or touch press.
    void press(const QMouseEvent& event);
    /// \brief Ends a press, on release or when the plot loses its grab.
    void release();
    /// \brief Places the pointer where the hover enters, unless a touch is in progress.
    void hoverEnter(const QHoverEvent& event);
    /// \brief Places the pointer at a hover move, which always comes from the mouse.
    void hoverMove(const QHoverEvent& event);
    /// \brief Removes the pointer when the mouse leaves; a touch pointer stays.
    void hoverLeave();
    /// \brief Places the pointer at a move, except the button-less move Qt Quick sends after a touch release.
    void mouseMove(const QMouseEvent& event);

signals:
    /// \brief Emitted when the pointer moves, appears, or leaves.
    void changed();

private:
    void setPosition(const QPointF& position);

    std::optional<QPointF> position_;
    // True while the pointer was last placed by a touch, which keeps it in place after the release.
    bool touchPointer_{false};
    bool touchPressed_{false};
};

} // namespace QAccelPlot::Internal
