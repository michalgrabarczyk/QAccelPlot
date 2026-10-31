//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/PlotRectangleZoom.hpp"

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

PlotRectangleZoom::PlotRectangleZoom(QObject* parent)
    : QObject(parent)
{
}

bool PlotRectangleZoom::enabled() const
{
    return enabled_;
}

void PlotRectangleZoom::setEnabled(bool value)
{
    if (enabled_ == value) {
        return;
    }
    enabled_ = value;
    emit enabledChanged();
}

Qt::KeyboardModifiers PlotRectangleZoom::modifiers() const
{
    return modifiers_;
}

void PlotRectangleZoom::setModifiers(const Qt::KeyboardModifiers value)
{
    if (modifiers_ == value) {
        return;
    }
    modifiers_ = value;
    emit modifiersChanged();
}

qreal PlotRectangleZoom::minimumSize() const
{
    return minimumSize_;
}

void PlotRectangleZoom::setMinimumSize(qreal value)
{
    if (!std::isfinite(value)) {
        return;
    }
    value = std::max(qreal{0}, value);
    if (minimumSize_ == value) {
        return;
    }
    minimumSize_ = value;
    emit minimumSizeChanged();
}

QColor PlotRectangleZoom::fillColor() const
{
    return fillColor_;
}

void PlotRectangleZoom::setFillColor(const QColor& value)
{
    if (fillColor_ == value) {
        return;
    }
    fillColor_ = value;
    emit fillColorChanged();
}

Border* PlotRectangleZoom::border() const
{
    return border_;
}

bool PlotRectangleZoom::isVisible() const
{
    return visible_;
}

void PlotRectangleZoom::setVisible(const bool value)
{
    if (visible_ == value) {
        return;
    }
    visible_ = value;
    emit visibleChanged();
}

bool PlotRectangleZoom::active() const
{
    return active_;
}

QRectF PlotRectangleZoom::selectionRect() const
{
    return selectionRect_;
}

void PlotRectangleZoom::setSelection(const bool active, const QRectF& rect)
{
    const auto activeChanged = active_ != active;
    const auto rectChanged = selectionRect_ != rect;
    active_ = active;
    selectionRect_ = rect;
    if (rectChanged) {
        emit selectionRectChanged();
    }
    if (activeChanged) {
        emit this->activeChanged();
    }
}

} // namespace QAccelPlot
