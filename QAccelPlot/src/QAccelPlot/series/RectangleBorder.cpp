//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/RectangleBorder.hpp"
#include "QAccelPlot/MathUtils.hpp"

#include <algorithm>

namespace QAccelPlot {

RectangleBorder::RectangleBorder(QObject* parent)
    : QObject(parent)
{
}

qreal RectangleBorder::width() const
{
    return width_;
}

void RectangleBorder::setWidth(const qreal width)
{
    const auto clamped = std::max(width, qreal{0.0});
    if (nearly_equal(width_, clamped)) {
        return;
    }
    width_ = clamped;
    emit widthChanged();
}

QColor RectangleBorder::color() const
{
    return color_;
}

void RectangleBorder::setColor(const QColor& color)
{
    if (color_ == color) {
        return;
    }
    color_ = color;
    emit colorChanged();
}

} // namespace QAccelPlot
