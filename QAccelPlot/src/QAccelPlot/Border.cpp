//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/Border.hpp"
#include "QAccelPlot/MathUtils.hpp"

#include <algorithm>

namespace QAccelPlot {

Border::Border(const QColor& color, const qreal width, QObject* parent)
    : QObject(parent)
    , color_(color)
    , width_(std::max(qreal{0.0}, width))
{
}

QColor Border::color() const
{
    return color_;
}

void Border::setColor(const QColor& color)
{
    if (color_ == color) {
        return;
    }
    color_ = color;
    emit colorChanged();
}

qreal Border::width() const
{
    return width_;
}

void Border::setWidth(const qreal width)
{
    const auto clampedWidth = std::max(qreal{0.0}, width);
    if (nearlyEqual(width_, clampedWidth)) {
        return;
    }
    width_ = clampedWidth;
    emit widthChanged();
}

} // namespace QAccelPlot
