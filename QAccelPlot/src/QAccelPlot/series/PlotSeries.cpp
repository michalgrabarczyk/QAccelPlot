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

std::optional<PlotSeries::DataExtent> validExtent(const std::optional<PlotSeries::DataExtent>& extent)
{
    if (extent && std::isfinite(extent->min) && std::isfinite(extent->max) && extent->min <= extent->max) {
        return extent;
    }
    return std::nullopt;
}

} // namespace

PlotSeries::PlotSeries(QQuickItem* parent)
    : QQuickItem(parent)
{
    setClip(true);
}

PlotSeries::~PlotSeries()
{
    if (xAxis_) {
        xAxis_->removeDataRangeSource(this, Axis::Horizontal);
    }
    if (yAxis_) {
        yAxis_->removeDataRangeSource(this, Axis::Vertical);
    }
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
        xAxis_->removeDataRangeSource(this, Axis::Horizontal);
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
    }
    // Not skipped when the scales match: a destroyed previous axis can no longer report its scale.
    invalidateInspection();
    onAxisScaleChanged();
    if (xAxis_) {
        xAxis_->addDataRangeSource(this, Axis::Horizontal);
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
        disconnect(yAxis_, &Axis::logScaleChanged, this, &PlotSeries::invalidateInspection);
        disconnect(yAxis_, &Axis::logScaleChanged, this, &PlotSeries::onAxisScaleChanged);
        yAxis_->removeDataRangeSource(this, Axis::Vertical);
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
    }
    // Not skipped when the scales match: a destroyed previous axis can no longer report its scale.
    invalidateInspection();
    onAxisScaleChanged();
    if (yAxis_) {
        yAxis_->addDataRangeSource(this, Axis::Vertical);
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

void PlotSeries::setData(const double* data, const int count, const DataBounds& bounds)
{
    updateBounds_ = bounds;
    setData(data, count);
    updateBounds_.reset();
}

void PlotSeries::setData(std::vector<double>&& data, const int count, const DataBounds& bounds)
{
    updateBounds_ = bounds;
    setData(std::move(data), count);
    updateBounds_.reset();
}

void PlotSeries::setDataF(const float* data, const int count, const DataBounds& bounds)
{
    updateBounds_ = bounds;
    setDataF(data, count);
    updateBounds_.reset();
}

void PlotSeries::setDataF(std::vector<float>&& data, const int count, const DataBounds& bounds)
{
    updateBounds_ = bounds;
    setDataF(std::move(data), count);
    updateBounds_.reset();
}

std::optional<PlotSeries::DataExtent> PlotSeries::xDataRange() const
{
    ensureDataRanges();
    return xDataExtent_;
}

std::optional<PlotSeries::DataExtent> PlotSeries::yDataRange() const
{
    ensureDataRanges();
    return yDataExtent_;
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

PlotSeries::DataRanges PlotSeries::computeDataRanges() const
{
    return {};
}

void PlotSeries::invalidateDataRanges()
{
    if (updateBounds_) {
        xDataExtent_ = validExtent(DataExtent{updateBounds_->xMin, updateBounds_->xMax});
        yDataExtent_ = validExtent(DataExtent{updateBounds_->yMin, updateBounds_->yMax});
        dataRangesStale_ = false;
    } else {
        dataRangesStale_ = true;
    }
    reportDataRangesChanged();
}

void PlotSeries::extendXDataRange(const qreal x)
{
    if (!std::isfinite(x)) {
        return;
    }
    // Stale extents stay stale: the next reader scans the records, including this one.
    if (!dataRangesStale_) {
        if (xDataExtent_ && x >= xDataExtent_->min && x <= xDataExtent_->max) {
            return;
        }
        xDataExtent_ = xDataExtent_ ? DataExtent{std::min(xDataExtent_->min, x), std::max(xDataExtent_->max, x)} : DataExtent{x, x};
    }
    if (xAxis_) {
        xAxis_->invalidateDataRange();
    }
}

void PlotSeries::extendYDataRange(const qreal y)
{
    if (!std::isfinite(y)) {
        return;
    }
    if (!dataRangesStale_) {
        if (yDataExtent_ && y >= yDataExtent_->min && y <= yDataExtent_->max) {
            return;
        }
        yDataExtent_ = yDataExtent_ ? DataExtent{std::min(yDataExtent_->min, y), std::max(yDataExtent_->max, y)} : DataExtent{y, y};
    }
    if (yAxis_) {
        yAxis_->invalidateDataRange();
    }
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

void PlotSeries::ensureDataRanges() const
{
    if (!std::exchange(dataRangesStale_, false)) {
        return;
    }
    const auto extents = computeDataRanges();
    xDataExtent_ = validExtent(extents.x);
    yDataExtent_ = validExtent(extents.y);
}

void PlotSeries::reportDataRangesChanged() const
{
    if (xAxis_) {
        xAxis_->invalidateDataRange();
    }
    if (yAxis_) {
        yAxis_->invalidateDataRange();
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
