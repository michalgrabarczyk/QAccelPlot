//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "series/PlotSeries.hpp"

#include "MathUtils.hpp"

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

PlotSeries::PlotSeries(QQuickItem* parent)
    : QQuickItem(parent)
{
    setClip(true);
}

QString PlotSeries::name() const
{
    return name_;
}

void PlotSeries::setName(const QString& name)
{
    if (name_ == name) {
        return;
    }
    name_ = name;
    emit nameChanged();
}

Axis* PlotSeries::xAxis() const
{
    return xAxis_.data();
}

void PlotSeries::setXAxis(Axis* axis)
{
    if (xAxis_ == axis) {
        return;
    }
    const auto wasLogScale = xAxis_ && xAxis_->logScale();
    if (xAxis_) {
        disconnect(xAxis_, &Axis::rangeChanged, this, &PlotSeries::onAxisRangeChanged);
        disconnect(xAxis_, &Axis::logScaleChanged, this, &PlotSeries::onAxisScaleChanged);
        xAxis_->clearSourceDataRange(this, Axis::Horizontal);
    }
    xAxis_ = axis;
    if (xAxis_) {
        connect(xAxis_, &Axis::rangeChanged, this, &PlotSeries::onAxisRangeChanged);
        connect(xAxis_, &Axis::logScaleChanged, this, &PlotSeries::onAxisScaleChanged);
        reportXDataRangeToAxis();
    }
    if (wasLogScale != (xAxis_ && xAxis_->logScale())) {
        onAxisScaleChanged();
    }
    emit xAxisChanged();
    update();
}

Axis* PlotSeries::yAxis() const
{
    return yAxis_.data();
}

void PlotSeries::setYAxis(Axis* axis)
{
    if (yAxis_ == axis) {
        return;
    }
    const auto wasLogScale = yAxis_ && yAxis_->logScale();
    if (yAxis_) {
        disconnect(yAxis_, &Axis::rangeChanged, this, &PlotSeries::onAxisRangeChanged);
        disconnect(yAxis_, &Axis::logScaleChanged, this, &PlotSeries::onAxisScaleChanged);
        yAxis_->clearSourceDataRange(this, Axis::Vertical);
    }
    yAxis_ = axis;
    if (yAxis_) {
        connect(yAxis_, &Axis::rangeChanged, this, &PlotSeries::onAxisRangeChanged);
        connect(yAxis_, &Axis::logScaleChanged, this, &PlotSeries::onAxisScaleChanged);
        reportYDataRangeToAxis();
    }
    if (wasLogScale != (yAxis_ && yAxis_->logScale())) {
        onAxisScaleChanged();
    }
    emit yAxisChanged();
    update();
}

QRectF PlotSeries::plotRect() const
{
    return plotRect_;
}

void PlotSeries::setPlotRect(const QRectF& rect)
{
    if (plotRect_ == rect) {
        return;
    }
    plotRect_ = rect;
    setPosition(rect.topLeft());
    setSize(rect.size());
    emit plotRectChanged();
    update();
}

PlotSeries::LegendSymbol PlotSeries::legendSymbol() const
{
    return legendSymbol_;
}

void PlotSeries::setLegendSymbol(const LegendSymbol symbol)
{
    if (legendSymbol_ == symbol) {
        return;
    }
    legendSymbol_ = symbol;
    emit legendSymbolChanged();
}

void PlotSeries::setDataRanges(const qreal xMin, const qreal xMax, const qreal yMin, const qreal yMax)
{
    setXDataRange(xMin, xMax);
    setYDataRange(yMin, yMax);
}

void PlotSeries::setXDataRange(const qreal min, const qreal max)
{
    if (!std::isfinite(min) || !std::isfinite(max)) {
        return;
    }
    if (!nearly_equal(min, lastXMin_) || !nearly_equal(max, lastXMax_)) {
        lastXMin_ = min;
        lastXMax_ = max;
        reportXDataRangeToAxis();
        emit xDataRangeChanged(min, max);
    }
}

void PlotSeries::setYDataRange(const qreal min, const qreal max)
{
    if (!std::isfinite(min) || !std::isfinite(max)) {
        return;
    }
    if (!nearly_equal(min, lastYMin_) || !nearly_equal(max, lastYMax_)) {
        lastYMin_ = min;
        lastYMax_ = max;
        reportYDataRangeToAxis();
        emit yDataRangeChanged(min, max);
    }
}

void PlotSeries::extendXDataRange(const qreal x)
{
    if (std::isfinite(x)) {
        setXDataRange(std::min(lastXMin_, x), std::max(lastXMax_, x));
    }
}

void PlotSeries::extendYDataRange(const qreal y)
{
    if (std::isfinite(y)) {
        setYDataRange(std::min(lastYMin_, y), std::max(lastYMax_, y));
    }
}

void PlotSeries::clearDataRanges()
{
    clearXDataRange();
    clearYDataRange();
}

void PlotSeries::clearXDataRange()
{
    if (xAxis_) {
        xAxis_->clearSourceDataRange(this, Axis::Horizontal);
    }
    lastXMin_ = std::numeric_limits<qreal>::max();
    lastXMax_ = std::numeric_limits<qreal>::lowest();
}

void PlotSeries::clearYDataRange()
{
    if (yAxis_) {
        yAxis_->clearSourceDataRange(this, Axis::Vertical);
    }
    lastYMin_ = std::numeric_limits<qreal>::max();
    lastYMax_ = std::numeric_limits<qreal>::lowest();
}

void PlotSeries::onAxisScaleChanged()
{
}

void PlotSeries::onAxisRangeChanged()
{
    update();
}

void PlotSeries::reportXDataRangeToAxis() const
{
    if (xAxis_ && lastXMin_ <= lastXMax_) {
        xAxis_->setSourceDataRange(this, Axis::Horizontal, lastXMin_, lastXMax_);
    }
}

void PlotSeries::reportYDataRangeToAxis() const
{
    if (yAxis_ && lastYMin_ <= lastYMax_) {
        yAxis_->setSourceDataRange(this, Axis::Vertical, lastYMin_, lastYMax_);
    }
}

} // namespace QAccelPlot
