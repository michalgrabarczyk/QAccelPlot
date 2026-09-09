//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "series/PlotSeries.hpp"

#include "MathUtils.hpp"

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
    if (xAxis_) {
        disconnect(xAxis_, &Axis::rangeChanged, this, &PlotSeries::onAxisRangeChanged);
        xAxis_->clearSourceDataRange(this, Axis::Horizontal);
    }
    xAxis_ = axis;
    if (xAxis_) {
        connect(xAxis_, &Axis::rangeChanged, this, &PlotSeries::onAxisRangeChanged);
        reportXDataRangeToAxis();
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
    if (yAxis_) {
        disconnect(yAxis_, &Axis::rangeChanged, this, &PlotSeries::onAxisRangeChanged);
        yAxis_->clearSourceDataRange(this, Axis::Vertical);
    }
    yAxis_ = axis;
    if (yAxis_) {
        connect(yAxis_, &Axis::rangeChanged, this, &PlotSeries::onAxisRangeChanged);
        reportYDataRangeToAxis();
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
    if (!std::isfinite(xMin) || !std::isfinite(xMax) || !std::isfinite(yMin) || !std::isfinite(yMax)) {
        return;
    }
    if (!nearly_equal(xMin, lastXMin_) || !nearly_equal(xMax, lastXMax_)) {
        lastXMin_ = xMin;
        lastXMax_ = xMax;
        reportXDataRangeToAxis();
        emit xDataRangeChanged(xMin, xMax);
    }
    if (!nearly_equal(yMin, lastYMin_) || !nearly_equal(yMax, lastYMax_)) {
        lastYMin_ = yMin;
        lastYMax_ = yMax;
        reportYDataRangeToAxis();
        emit yDataRangeChanged(yMin, yMax);
    }
}

void PlotSeries::clearDataRanges()
{
    if (xAxis_) {
        xAxis_->clearSourceDataRange(this, Axis::Horizontal);
    }
    if (yAxis_) {
        yAxis_->clearSourceDataRange(this, Axis::Vertical);
    }
    lastXMin_ = std::numeric_limits<qreal>::max();
    lastXMax_ = std::numeric_limits<qreal>::lowest();
    lastYMin_ = std::numeric_limits<qreal>::max();
    lastYMax_ = std::numeric_limits<qreal>::lowest();
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
