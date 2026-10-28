//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/PlotMouseEvent.hpp"

#include <QSinglePointEvent>

namespace QAccelPlot {

PlotMouseEvent::PlotMouseEvent(QObject* parent)
    : QObject(parent)
{
}

Qt::MouseButton PlotMouseEvent::button() const
{
    return button_;
}

Qt::MouseButtons PlotMouseEvent::buttons() const
{
    return buttons_;
}

qreal PlotMouseEvent::x() const
{
    return x_;
}

qreal PlotMouseEvent::y() const
{
    return y_;
}

Qt::KeyboardModifiers PlotMouseEvent::modifiers() const
{
    return modifiers_;
}

bool PlotMouseEvent::isAccepted() const
{
    return accepted_;
}

void PlotMouseEvent::accept()
{
    accepted_ = true;
}

void PlotMouseEvent::ignore()
{
    accepted_ = false;
}

void PlotMouseEvent::setAccepted(bool accepted)
{
    accepted_ = accepted;
}

void PlotMouseEvent::reset(const QSinglePointEvent& event)
{
    button_ = event.button();
    buttons_ = event.buttons();
    x_ = event.position().x();
    y_ = event.position().y();
    modifiers_ = event.modifiers();
    accepted_ = false;
}

} // namespace QAccelPlot
