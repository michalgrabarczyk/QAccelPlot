//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "PlotMouseEvent.hpp"

namespace QAccelPlot {

PlotMouseEvent::PlotMouseEvent(QObject* parent)
    : QObject(parent)
{
}

void PlotMouseEvent::reset(const int button, const qreal x, const qreal y, const int modifiers)
{
    button_ = button;
    x_ = x;
    y_ = y;
    modifiers_ = modifiers;
    accepted_ = false;
}

int PlotMouseEvent::button() const
{
    return button_;
}

qreal PlotMouseEvent::x() const
{
    return x_;
}

qreal PlotMouseEvent::y() const
{
    return y_;
}

int PlotMouseEvent::modifiers() const
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

} // namespace QAccelPlot
