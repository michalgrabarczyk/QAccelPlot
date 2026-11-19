//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

class QSinglePointEvent;

namespace QAccelPlot {

class Axis;
class PlotView;

/// \brief Carries mouse event data for the mouse signals.
///
/// An instance of this class is passed to each \c PlotView mouse signal and to \c Axis::doubleClicked().
/// Mirrors the Qt \c QEvent API: call \c accept() in a handler to consume the event and suppress the
/// built-in handling (dragging, double-click rescaling, etc.).
///
/// \note The object is owned by \c QAccelPlot and is destroyed after the signal — do not
/// store a reference to it beyond the signal handler.
class PlotMouseEvent : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(PlotMouseEvent)
    QML_UNCREATABLE("PlotMouseEvent can only be created by PlotView or Axis.")

    /// \brief The mouse button that caused the event; \c Qt.NoButton for a move.
    Q_PROPERTY(Qt::MouseButton button READ button CONSTANT)
    /// \brief The mouse buttons held down during the event.
    Q_PROPERTY(Qt::MouseButtons buttons READ buttons CONSTANT)
    /// \brief Item-local pixel X coordinate of the event.
    Q_PROPERTY(qreal x READ x CONSTANT)
    /// \brief Item-local pixel Y coordinate of the event.
    Q_PROPERTY(qreal y READ y CONSTANT)
    /// \brief Active keyboard modifiers at the time of the event.
    Q_PROPERTY(Qt::KeyboardModifiers modifiers READ modifiers CONSTANT)
    /// \brief Whether this event has been accepted by a handler.
    /// Read via \c isAccepted(); set via \c accept() or \c ignore().
    Q_PROPERTY(bool accepted READ isAccepted WRITE setAccepted)

public:
    /// \brief Constructs a PlotMouseEvent with an optional \p parent.
    explicit PlotMouseEvent(QObject* parent = nullptr);

    /// \brief Returns the mouse button that caused the event; \c Qt::NoButton for a move.
    Qt::MouseButton button() const;
    /// \brief Returns the mouse buttons held down during the event.
    Qt::MouseButtons buttons() const;
    /// \brief Returns the item-local pixel X coordinate of the event.
    qreal x() const;
    /// \brief Returns the item-local pixel Y coordinate of the event.
    qreal y() const;
    /// \brief Returns the active keyboard modifiers at the time of the event.
    Qt::KeyboardModifiers modifiers() const;
    /// \brief Returns true if the event has been accepted by a handler.
    bool isAccepted() const;
    /// \brief Accepts the event, suppressing the plot's built-in handling.
    Q_INVOKABLE void accept();
    /// \brief Ignores the event, allowing the plot's built-in handling to proceed.
    Q_INVOKABLE void ignore();
    /// \brief Sets whether the event is accepted. Equivalent to \c accept() if \p accepted is true, or \c ignore() if false.
    void setAccepted(bool accepted);

private:
    friend class Axis;
    friend class PlotView;

    /// \brief Constructs an unaccepted event that copies \p event, for a mouse signal about to be emitted.
    explicit PlotMouseEvent(const QSinglePointEvent& event);

    Qt::MouseButton button_{Qt::NoButton};
    Qt::MouseButtons buttons_{Qt::NoButton};
    qreal x_{0.0};
    qreal y_{0.0};
    Qt::KeyboardModifiers modifiers_{Qt::NoModifier};
    bool accepted_{false};
};

} // namespace QAccelPlot
