//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief Carries mouse event data for the mouse signals.
///
/// An instance of this class is passed to each mouse signal. Mirrors the Qt \c QEvent
/// API: call \c accept() in a handler to consume the event and suppress the plot's
/// built-in handling (dragging, double-click rescaling, etc.).
///
/// \note The object is owned by \c QAccelPlot and is reused across events — do not
/// store a reference to it beyond the signal handler.
class PlotMouseEvent : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(PlotMouseEvent)
    QML_UNCREATABLE("PlotMouseEvent can only be created by QAccelPlot.")

    /// \brief The mouse button involved (Qt::MouseButton value).
    Q_PROPERTY(int button READ button CONSTANT)
    /// \brief Item-local pixel X coordinate of the event.
    Q_PROPERTY(qreal x READ x CONSTANT)
    /// \brief Item-local pixel Y coordinate of the event.
    Q_PROPERTY(qreal y READ y CONSTANT)
    /// \brief Active keyboard modifiers at the time of the event (Qt::KeyboardModifiers value).
    Q_PROPERTY(int modifiers READ modifiers CONSTANT)
    /// \brief Whether this event has been accepted by a handler.
    /// Read via \c isAccepted(); set via \c accept() or \c ignore().
    Q_PROPERTY(bool accepted READ isAccepted WRITE setAccepted)

public:
    /// \brief Constructs a PlotMouseEvent with an optional \p parent.
    explicit PlotMouseEvent(QObject* parent = nullptr);

    /// \brief Returns the mouse button involved in the event (Qt::MouseButton value).
    int button() const;
    /// \brief Returns the item-local pixel X coordinate of the event.
    qreal x() const;
    /// \brief Returns the item-local pixel Y coordinate of the event.
    qreal y() const;
    /// \brief Returns the active keyboard modifiers at the time of the event (Qt::KeyboardModifiers value).
    int modifiers() const;
    /// \brief Returns true if the event has been accepted by a handler.
    bool isAccepted() const;
    /// \brief Accepts the event, suppressing the plot's built-in handling.
    Q_INVOKABLE void accept();
    /// \brief Ignores the event, allowing the plot's built-in handling to proceed.
    Q_INVOKABLE void ignore();
    /// @brief Sets whether the event is accepted. Equivalent to \c accept() if \p accepted is true, or \c ignore() if false.
    void setAccepted(bool accepted);
    /// \brief Resets the event data for a new event. Called by QAccelPlot before emitting mouse signals.
    void reset(int button, qreal x, qreal y, int modifiers);

private:
    int button_{0};
    qreal x_{0.0};
    qreal y_{0.0};
    int modifiers_{0};
    bool accepted_{false};
};

} // namespace QAccelPlot
