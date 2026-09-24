//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/SeriesMarker.hpp"
#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"

#include <algorithm>

namespace QAccelPlot {

SeriesMarker::SeriesMarker(const PlotSeries::MarkerShape shape, const qreal size, const NoneShape noneShape, QObject* parent)
    : QObject(parent)
    , shape_(shape)
    , size_(size)
    , noneShape_(noneShape)
{
}

PlotSeries::MarkerShape SeriesMarker::shape() const
{
    return shape_;
}

void SeriesMarker::setShape(const PlotSeries::MarkerShape shape)
{
    if (shape == PlotSeries::MarkerShape::None && noneShape_ == NoneShape::Rejected) {
        const auto* owner = parent() ? parent()->metaObject()->className() : "Series";
        qCWarning(lcQAccelPlot).nospace() << owner << " marker.shape does not accept None; keeping " << static_cast<int>(shape_);
        return;
    }
    if (shape_ == shape) {
        return;
    }
    shape_ = shape;
    emit shapeChanged();
}

qreal SeriesMarker::size() const
{
    return size_;
}

void SeriesMarker::setSize(const qreal size)
{
    const auto clamped = std::max(size, qreal{0.0});
    if (nearly_equal(size_, clamped)) {
        return;
    }
    size_ = clamped;
    emit sizeChanged();
}

bool SeriesMarker::filled() const
{
    return filled_;
}

void SeriesMarker::setFilled(const bool filled)
{
    if (filled_ == filled) {
        return;
    }
    filled_ = filled;
    emit filledChanged();
}

qreal SeriesMarker::strokeWidth() const
{
    return strokeWidth_;
}

void SeriesMarker::setStrokeWidth(const qreal width)
{
    const auto clamped = std::max(width, qreal{0.0});
    if (nearly_equal(strokeWidth_, clamped)) {
        return;
    }
    strokeWidth_ = clamped;
    emit strokeWidthChanged();
}

} // namespace QAccelPlot
