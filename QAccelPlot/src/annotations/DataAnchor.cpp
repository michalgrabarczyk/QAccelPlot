//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "annotations/DataAnchor.hpp"

#include "MathUtils.hpp"

#include <algorithm>

namespace QAccelPlot {

DataAnchor::DataAnchor(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, false);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::NoButton);
}

Axis* DataAnchor::xAxis() const
{
    return xAxis_;
}

void DataAnchor::setXAxis(Axis* axis)
{
    if (xAxis_ == axis) {
        return;
    }
    if (xAxis_) {
        disconnect(xAxis_, &Axis::rangeChanged, this, &DataAnchor::updateGeometry);
    }
    xAxis_ = axis;
    if (xAxis_) {
        connect(xAxis_, &Axis::rangeChanged, this, &DataAnchor::updateGeometry);
    }
    emit xAxisChanged();
    updateGeometry();
}

Axis* DataAnchor::yAxis() const
{
    return yAxis_;
}

void DataAnchor::setYAxis(Axis* axis)
{
    if (yAxis_ == axis) {
        return;
    }
    if (yAxis_) {
        disconnect(yAxis_, &Axis::rangeChanged, this, &DataAnchor::updateGeometry);
    }
    yAxis_ = axis;
    if (yAxis_) {
        connect(yAxis_, &Axis::rangeChanged, this, &DataAnchor::updateGeometry);
    }
    emit yAxisChanged();
    updateGeometry();
}

QRectF DataAnchor::plotRect() const
{
    return plotRect_;
}

void DataAnchor::setPlotRect(const QRectF& rect)
{
    if (plotRect_ == rect) {
        return;
    }
    plotRect_ = rect;
    emit plotRectChanged();
    updateGeometry();
}

qreal DataAnchor::dataX1() const
{
    return dataX1_;
}

void DataAnchor::setDataX1(const qreal value)
{
    if (nearly_equal(dataX1_, value)) {
        return;
    }
    dataX1_ = value;
    emit dataX1Changed();
    updateGeometry();
}

qreal DataAnchor::dataY1() const
{
    return dataY1_;
}

void DataAnchor::setDataY1(const qreal value)
{
    if (nearly_equal(dataY1_, value)) {
        return;
    }
    dataY1_ = value;
    emit dataY1Changed();
    updateGeometry();
}

qreal DataAnchor::dataX2() const
{
    return dataX2_;
}

void DataAnchor::setDataX2(const qreal value)
{
    if (nearly_equal(dataX2_, value)) {
        return;
    }
    dataX2_ = value;
    emit dataX2Changed();
    updateGeometry();
}

qreal DataAnchor::dataY2() const
{
    return dataY2_;
}

void DataAnchor::setDataY2(const qreal value)
{
    if (nearly_equal(dataY2_, value)) {
        return;
    }
    dataY2_ = value;
    emit dataY2Changed();
    updateGeometry();
}

qreal DataAnchor::hoverThreshold() const
{
    return hoverThreshold_;
}

void DataAnchor::setHoverThreshold(const qreal threshold)
{
    if (nearly_equal(hoverThreshold_, threshold)) {
        return;
    }
    hoverThreshold_ = threshold;
    emit hoverThresholdChanged();
}

bool DataAnchor::contains(const QPointF& point) const
{
    const auto expanded = QRectF(0, 0, width(), height()).adjusted(-hoverThreshold_, -hoverThreshold_, hoverThreshold_, hoverThreshold_);
    return expanded.contains(point);
}

void DataAnchor::updateGeometry()
{
    if (!xAxis_ || !yAxis_ || plotRect_.isEmpty()) {
        return;
    }

    const auto px1 = plotRect_.x() + xAxis_->coordToPixel(dataX1_, plotRect_.width());
    const auto py1 = plotRect_.y() + yAxis_->coordToPixel(dataY1_, plotRect_.height());
    const auto px2 = plotRect_.x() + xAxis_->coordToPixel(dataX2_, plotRect_.width());
    const auto py2 = plotRect_.y() + yAxis_->coordToPixel(dataY2_, plotRect_.height());

    setX(std::min(px1, px2));
    setY(std::min(py1, py2));
    setWidth(std::abs(px2 - px1));
    setHeight(std::abs(py2 - py1));
}

} // namespace QAccelPlot
