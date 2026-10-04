//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/PlotSeries.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/inspection/internal/InspectionTypes.hpp"

#include <QHoverEvent>

#include <algorithm>
#include <cmath>
#include <utility>

namespace QAccelPlot {

namespace {

// Qt Quick older than 6.3 also delivers an ignored hover event to the items beneath the hovered one.
constexpr auto kHoverReachesSeriesBeneath = QT_VERSION < QT_VERSION_CHECK(6, 3, 0);

} // namespace

PlotSeries::PlotSeries(QQuickItem* parent)
    : QQuickItem(parent)
{
    setClip(true);
}

quint64 PlotSeries::dataRevision() const
{
    return dataRevision_;
}

SeriesInspection* PlotSeries::inspection() const
{
    if (!inspection_) {
        // Parented to the series, so it is destroyed with it.
        inspection_ = new SeriesInspection(const_cast<PlotSeries&>(*this));
    }
    return inspection_;
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
        disconnect(xAxis_, &Axis::logScaleChanged, this, &PlotSeries::invalidateInspection);
        disconnect(xAxis_, &Axis::logScaleChanged, this, &PlotSeries::onAxisScaleChanged);
        xAxis_->clearSourceDataRange(this, Axis::Horizontal);
    }
    disconnect(xAxisDestroyed_);
    xAxis_ = axis;
    if (xAxis_) {
        connect(xAxis_, &Axis::rangeChanged, this, &PlotSeries::onAxisRangeChanged);
        connect(xAxis_, &Axis::logScaleChanged, this, &PlotSeries::invalidateInspection);
        connect(xAxis_, &Axis::logScaleChanged, this, &PlotSeries::onAxisScaleChanged);
        xAxisDestroyed_ = connect(xAxis_, &QObject::destroyed, this, [this] {
            invalidateInspection();
            emit xAxisChanged();
        });
        reportXDataRangeToAxis();
    }
    // Not skipped when the scales match: a destroyed previous axis can no longer report its scale.
    invalidateInspection();
    onAxisScaleChanged();
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
        disconnect(yAxis_, &Axis::logScaleChanged, this, &PlotSeries::invalidateInspection);
        disconnect(yAxis_, &Axis::logScaleChanged, this, &PlotSeries::onAxisScaleChanged);
        yAxis_->clearSourceDataRange(this, Axis::Vertical);
    }
    disconnect(yAxisDestroyed_);
    yAxis_ = axis;
    if (yAxis_) {
        connect(yAxis_, &Axis::rangeChanged, this, &PlotSeries::onAxisRangeChanged);
        connect(yAxis_, &Axis::logScaleChanged, this, &PlotSeries::invalidateInspection);
        connect(yAxis_, &Axis::logScaleChanged, this, &PlotSeries::onAxisScaleChanged);
        yAxisDestroyed_ = connect(yAxis_, &QObject::destroyed, this, [this] {
            invalidateInspection();
            emit yAxisChanged();
        });
        reportYDataRangeToAxis();
    }
    // Not skipped when the scales match: a destroyed previous axis can no longer report its scale.
    invalidateInspection();
    onAxisScaleChanged();
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

void PlotSeries::inspectionDataChanged(const DataChange change)
{
    ++dataRevision_;
    if (inspection_) {
        inspection_->sourceChanged(change == DataChange::Appended);
    }
    emit dataRevisionChanged();
}

void PlotSeries::invalidateInspection()
{
    if (inspection_) {
        inspection_->sourceInvalidated();
    }
}

InspectionSource PlotSeries::inspectionSource() const
{
    return {};
}

bool PlotSeries::inspectionAvailable() const
{
    return true;
}

InspectionRecord PlotSeries::inspectionRecord(const int /*index*/) const
{
    auto result = InspectionRecord{};
    result.status = InspectionStatus::Unsupported;
    return result;
}

InspectionRecord PlotSeries::inspectionRecordAt(const QPointF& /*position*/) const
{
    auto result = InspectionRecord{};
    result.status = InspectionStatus::Unsupported;
    return result;
}

std::optional<PlotSeries::DataExtent> PlotSeries::xDataRange() const
{
    if (lastXMin_ > lastXMax_) {
        return std::nullopt;
    }
    return DataExtent{lastXMin_, lastXMax_};
}

std::optional<PlotSeries::DataExtent> PlotSeries::yDataRange() const
{
    if (lastYMin_ > lastYMax_) {
        return std::nullopt;
    }
    return DataExtent{lastYMin_, lastYMax_};
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

QRectF PlotSeries::resolvePlotRect() const
{
    if (!plotRect_.isEmpty()) {
        // setPlotRect() already aligned the item geometry with it.
        return plotRect_;
    }
    return QRectF(0, 0, width(), height());
}

bool PlotSeries::event(QEvent* event)
{
    if constexpr (kHoverReachesSeriesBeneath) {
        switch (event->type()) {
        case QEvent::HoverEnter:
        case QEvent::HoverMove:
        case QEvent::HoverLeave:
            deliverTopmostHover(static_cast<QHoverEvent*>(event));
            return true;
        default:
            break;
        }
    }
    return QQuickItem::event(event);
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

void PlotSeries::deliverTopmostHover(QHoverEvent* event)
{
    const auto hover = event->type() != QEvent::HoverLeave && !coveredBySeriesAbove(event->position());
    const auto wasDelivered = std::exchange(hoverDelivered_, hover);
    if (hover && wasDelivered) {
        hoverMoveEvent(event);
    } else if (hover) {
        hoverEnterEvent(event);
    } else if (wasDelivered) {
        hoverLeaveEvent(event);
    } else {
        // Ignored like a delivered event, so the series beneath and the plot still receive it.
        event->ignore();
    }
}

bool PlotSeries::coveredBySeriesAbove(const QPointF& position) const
{
    const auto* parent = parentItem();
    if (!parent) {
        return false;
    }
    // Siblings are stacked by z and, for equal z, in child order.
    auto afterThis = false;
    const auto siblings = parent->childItems();
    for (const auto* sibling : siblings) {
        if (sibling == this) {
            afterThis = true;
            continue;
        }
        const auto above = sibling->z() > z() || (afterThis && sibling->z() >= z());
        if (above && qobject_cast<const PlotSeries*>(sibling) && sibling->isVisible() && sibling->acceptHoverEvents()
            && sibling->contains(mapToItem(sibling, position))) {
            return true;
        }
    }
    return false;
}

} // namespace QAccelPlot
