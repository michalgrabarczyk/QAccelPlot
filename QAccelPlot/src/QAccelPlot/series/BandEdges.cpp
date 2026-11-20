//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/series/BandEdges.hpp"
#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/linestyles/SolidLine.hpp"

#include <algorithm>

namespace QAccelPlot {

BandEdges::BandEdges(QObject* parent)
    : QObject(parent)
{
    setLineStyle(new SolidLine{this});
}

qreal BandEdges::width() const
{
    return width_;
}

void BandEdges::setWidth(const qreal width)
{
    const auto clamped = std::max(width, qreal{0.0});
    if (nearlyEqual(width_, clamped)) {
        return;
    }
    width_ = clamped;
    emit widthChanged();
}

QColor BandEdges::color() const
{
    return color_.value();
}

void BandEdges::setColor(const QColor& color)
{
    if (!color.isValid()) {
        resetColor();
        return;
    }
    if (color_.set(color)) {
        emit colorChanged();
    }
}

void BandEdges::resetColor()
{
    if (color_.reset()) {
        emit colorChanged();
    }
}

void BandEdges::setDefaultColor(const QColor& color)
{
    if (color_.setFallback(color)) {
        emit colorChanged();
    }
}

LineStyle* BandEdges::lineStyle() const
{
    return lineStyle_;
}

void BandEdges::setLineStyle(LineStyle* style)
{
    if (lineStyle_ == style) {
        return;
    }
    if (lineStyle_) {
        disconnect(lineStyle_, nullptr, this, nullptr);
    }
    lineStyle_ = style;
    if (lineStyle_) {
        connect(lineStyle_, &QObject::destroyed, this, &BandEdges::onLineStyleDestroyed);
    }
    emit lineStyleChanged();
}

void BandEdges::onLineStyleDestroyed()
{
    // lineStyle_ is a QPointer and has already been cleared.
    emit lineStyleChanged();
}

} // namespace QAccelPlot
