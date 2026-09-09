//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "PlotBorder.hpp"
#include "MathUtils.hpp"

#include <algorithm>

namespace QAccelPlot {

PlotBorder::PlotBorder(QObject* parent)
    : QObject(parent)
{
}

QColor PlotBorder::color() const
{
    return color_;
}

void PlotBorder::setColor(const QColor& color)
{
    if (color_ == color) {
        return;
    }
    color_ = color;
    emit colorChanged();
}

qreal PlotBorder::width() const
{
    return width_;
}

void PlotBorder::setWidth(const qreal width)
{
    const auto clampedWidth = std::max(qreal{0.0}, width);
    if (nearly_equal(width_, clampedWidth)) {
        return;
    }
    width_ = clampedWidth;
    emit widthChanged();
}

} // namespace QAccelPlot
