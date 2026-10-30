//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/internal/PointerTracker.hpp"

#include <QPointingDevice>

namespace QAccelPlot::Internal {

PointerTracker::PointerTracker(QObject* parent)
    : QObject(parent)
{
}

std::optional<QPointF> PointerTracker::position() const
{
    return position_;
}

void PointerTracker::press(const QMouseEvent& event)
{
    touchPointer_ = event.pointingDevice() && event.pointingDevice()->type() == QInputDevice::DeviceType::TouchScreen;
    touchPressed_ = touchPointer_;
    setPosition(event.position());
}

void PointerTracker::release()
{
    touchPressed_ = false;
}

void PointerTracker::hoverEnter(const QHoverEvent& event)
{
    // A touch in progress also produces hover events at the finger; a later one comes from the mouse.
    if (!touchPressed_) {
        touchPointer_ = false;
        setPosition(event.position());
    }
}

void PointerTracker::hoverMove(const QHoverEvent& event)
{
    touchPointer_ = false;
    setPosition(event.position());
}

void PointerTracker::hoverLeave()
{
    // A touch has no hover: the pointer stays where the finger was until the next tap or mouse movement.
    if (!touchPointer_) {
        position_.reset();
        emit changed();
    }
}

void PointerTracker::mouseMove(const QMouseEvent& event)
{
    // After a touch is released, Qt Quick sends a button-less move to the mouse cursor's old position.
    if (!touchPointer_ || event.buttons() != Qt::NoButton) {
        setPosition(event.position());
    }
}

void PointerTracker::setPosition(const QPointF& position)
{
    position_ = position;
    emit changed();
}

} // namespace QAccelPlot::Internal
