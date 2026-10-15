//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/PlotSeries.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/inspection/internal/InspectionTypes.hpp"

#include <QHoverEvent>

#include <algorithm>
#include <cmath>
#include <string_view>
#include <utility>

namespace QAccelPlot {

namespace {

// Qt Quick older than 6.3 also delivers an ignored hover event to the items beneath the hovered one.
constexpr auto kHoverReachesSeriesBeneath = QT_VERSION < QT_VERSION_CHECK(6, 3, 0);

std::optional<PlotSeries::DataExtent> toDataExtent(const std::optional<DataPointStore::Extent>& extent)
{
    if (!extent) {
        return std::nullopt;
    }
    return PlotSeries::DataExtent{extent->min, extent->max};
}

// The name of the library class a series is, without the namespace, as the data argument warnings
// name the series. QML types and C++ subclasses are named after the library class they derive from.
const char* seriesName(const QObject& series)
{
    constexpr auto kNamespace = std::string_view{"QAccelPlot::"};
    for (const auto* type = series.metaObject(); type; type = type->superClass()) {
        const auto* name = type->className();
        if (std::string_view{name}.substr(0, kNamespace.size()) == kNamespace) {
            return name + kNamespace.size();
        }
    }
    return series.metaObject()->className();
}

DataPointStore::DimensionState dimensionState(const Axis* axis)
{
    if (!axis) {
        return {};
    }
    return DataPointStore::DimensionState{axis->logScale(), DataPointStore::Viewport{axis->viewportMin(), axis->viewportMax()}};
}

std::optional<PlotSeries::DataExtent> validExtent(const std::optional<PlotSeries::DataExtent>& extent)
{
    if (extent && std::isfinite(extent->min) && std::isfinite(extent->max) && extent->min <= extent->max) {
        return extent;
    }
    return std::nullopt;
}

} // namespace

PlotSeries::PlotSeries(QQuickItem* parent)
    : PlotSeries(DataPointStore::Layout{}, DataPointWording{"data point", "points", "doubles", "floats"}, parent)
{
}

PlotSeries::PlotSeries(DataPointStore::Layout layout, DataPointWording wording, QQuickItem* parent)
    : QQuickItem(parent)
    , dataPoints_(std::move(layout))
    , wording_(std::move(wording))
{
    setClip(true);
}

PlotSeries::~PlotSeries()
{
    // Leave the plot while this is still a PlotSeries. ~QQuickItem would do it later, when the
    // observers of QAccelPlot::seriesChanged() can no longer use their pointers to this series.
    setParentItem(nullptr);

    if (xAxis_) {
        xAxis_->removeDataRangeSource(this, Qt::Horizontal);
    }
    if (yAxis_) {
        yAxis_->removeDataRangeSource(this, Qt::Vertical);
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
        disconnect(xAxis_, &Axis::rangeChanged, this, &PlotSeries::handleAxisRangeChanged);
        disconnect(xAxis_, &Axis::logScaleChanged, this, &PlotSeries::handleAxisScaleChanged);
        xAxis_->removeDataRangeSource(this, Qt::Horizontal);
    }
    disconnect(xAxisDestroyed_);
    xAxis_ = axis;
    if (xAxis_) {
        connect(xAxis_, &Axis::rangeChanged, this, &PlotSeries::handleAxisRangeChanged);
        connect(xAxis_, &Axis::logScaleChanged, this, &PlotSeries::handleAxisScaleChanged);
        xAxisDestroyed_ = connect(xAxis_, &QObject::destroyed, this, [this] {
            handleAxisScaleChanged();
            emit xAxisChanged();
            update();
        });
    }
    // Not skipped when the scales match: a destroyed previous axis can no longer report its scale.
    handleAxisScaleChanged();
    if (xAxis_) {
        xAxis_->addDataRangeSource(this, Qt::Horizontal);
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
        disconnect(yAxis_, &Axis::rangeChanged, this, &PlotSeries::handleAxisRangeChanged);
        disconnect(yAxis_, &Axis::logScaleChanged, this, &PlotSeries::handleAxisScaleChanged);
        yAxis_->removeDataRangeSource(this, Qt::Vertical);
    }
    disconnect(yAxisDestroyed_);
    yAxis_ = axis;
    if (yAxis_) {
        connect(yAxis_, &Axis::rangeChanged, this, &PlotSeries::handleAxisRangeChanged);
        connect(yAxis_, &Axis::logScaleChanged, this, &PlotSeries::handleAxisScaleChanged);
        yAxisDestroyed_ = connect(yAxis_, &QObject::destroyed, this, [this] {
            handleAxisScaleChanged();
            emit yAxisChanged();
            update();
        });
    }
    // Not skipped when the scales match: a destroyed previous axis can no longer report its scale.
    handleAxisScaleChanged();
    if (yAxis_) {
        yAxis_->addDataRangeSource(this, Qt::Vertical);
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

void PlotSeries::setData(const double* data, const int count)
{
    if (!validateRawDataArguments(data, count)) {
        return;
    }
    auto input = DataPointInput{};
    input.doubleArray = data;
    input.count = count;
    input.stride = dataPointValueCount();
    applyDataPoints(std::move(input));
}

void PlotSeries::setData(std::vector<double>&& data, const int count)
{
    if (!validateDataSize(data.size(), count, false, dataPointValueCount())) {
        return;
    }
    auto input = DataPointInput{};
    input.doubles = std::move(data);
    input.count = count;
    input.stride = dataPointValueCount();
    applyDataPoints(std::move(input));
}

void PlotSeries::setDataF(const float* data, const int count)
{
    if (!validateRawDataArguments(data, count)) {
        return;
    }
    auto input = DataPointInput{};
    input.floatArray = data;
    input.count = count;
    input.stride = dataPointValueCount();
    input.precise = false;
    applyDataPoints(std::move(input));
}

void PlotSeries::setDataF(std::vector<float>&& data, const int count)
{
    if (!validateDataSize(data.size(), count, true, dataPointValueCount())) {
        return;
    }
    auto input = DataPointInput{};
    input.floats = std::move(data);
    input.count = count;
    input.stride = dataPointValueCount();
    input.precise = false;
    applyDataPoints(std::move(input));
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

void PlotSeries::postData(std::vector<double>&& data, const int count)
{
    // The series is the context object, so Qt drops the call if the series is destroyed first.
    QMetaObject::invokeMethod(this, [this, values = std::move(data), count]() mutable { setData(std::move(values), count); }, Qt::QueuedConnection);
}

void PlotSeries::postData(std::vector<float>&& data, const int count)
{
    QMetaObject::invokeMethod(this, [this, values = std::move(data), count]() mutable { setDataF(std::move(values), count); }, Qt::QueuedConnection);
}

void PlotSeries::clearData()
{
    const auto previousCount = dataPoints_.count();
    dataPoints_.clear();
    notifyDataPointsChanged(DataChange::Replaced, previousCount);
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

std::vector<double> PlotSeries::DataPointInput::toDoubles() &&
{
    const auto size = static_cast<std::size_t>(std::max(count, 0)) * static_cast<std::size_t>(stride);
    if (doubleArray) {
        return std::vector<double>(doubleArray, doubleArray + size);
    }
    if (floatArray) {
        return std::vector<double>(floatArray, floatArray + size);
    }
    if (precise) {
        return std::move(doubles);
    }
    return std::vector<double>(floats.cbegin(), floats.cend());
}

void PlotSeries::applyDataPoints(DataPointInput&& input)
{
    storeDataPoints(std::move(input));
}

void PlotSeries::storeDataPoints(DataPointInput&& input)
{
    const auto previousCount = dataPoints_.count();
    if (input.doubleArray) {
        dataPoints_.assignPrecise(input.doubleArray, input.count);
    } else if (input.floatArray) {
        // Reuses the allocation of the stored data points.
        dataPoints_.assignFloat(input.floatArray, input.count);
    } else if (input.precise) {
        dataPoints_.setPrecise(std::move(input.doubles), input.count);
    } else {
        dataPoints_.setFloat(std::move(input.floats), input.count);
    }
    notifyDataPointsChanged(DataChange::Replaced, previousCount);
}

int PlotSeries::dataPointValueCount() const
{
    return dataPoints_.columnCount();
}

DataPointStore& PlotSeries::dataPoints()
{
    return dataPoints_;
}

const DataPointStore& PlotSeries::dataPoints() const
{
    return dataPoints_;
}

bool PlotSeries::logScaleX() const
{
    return xAxis_ && xAxis_->logScale();
}

bool PlotSeries::logScaleY() const
{
    return yAxis_ && yAxis_->logScale();
}

bool PlotSeries::validateRawDataArguments(const void* data, const int count) const
{
    const auto* series = seriesName(*this);
    if (count < 0) {
        qCWarning(lcQAccelPlot).nospace() << series << " " << wording_.countName.c_str() << " count cannot be negative: " << count;
        return false;
    }
    if (count > 0 && !data) {
        qCWarning(lcQAccelPlot).nospace() << series << " received a null data pointer for " << count << " " << wording_.plural.c_str();
        return false;
    }
    return true;
}

bool PlotSeries::validateDataSize(const std::size_t valueCount, const int count, const bool floatData, const int valuesPerDataPoint) const
{
    const auto* series = seriesName(*this);
    if (count < 0) {
        qCWarning(lcQAccelPlot).nospace() << series << " " << wording_.countName.c_str() << " count cannot be negative: " << count;
        return false;
    }
    const auto stride = valuesPerDataPoint > 0 ? valuesPerDataPoint : dataPoints_.columnCount();
    const auto expectedValueCount = static_cast<std::size_t>(count) * static_cast<std::size_t>(stride);
    if (valueCount != expectedValueCount) {
        qCWarning(lcQAccelPlot).nospace() << series << " received " << valueCount << " " << (floatData ? wording_.floatUnit : wording_.doubleUnit).c_str()
                                          << " for " << count << " " << wording_.plural.c_str() << "; expected " << expectedValueCount;
        return false;
    }
    return true;
}

void PlotSeries::appendDataPoint(const double* coordinates)
{
    const auto previousCount = dataPoints_.count();
    dataPoints_.append(coordinates);
    // The new data point widens the current extents in O(1) instead of a rescan.
    const auto extents = dataPoints_.extents(logScaleX(), logScaleY(), previousCount);
    if (extents.x) {
        extendXDataRange(extents.x->min);
        extendXDataRange(extents.x->max);
    }
    if (extents.y) {
        extendYDataRange(extents.y->min);
        extendYDataRange(extents.y->max);
    }
    notifyDataPointsChanged(DataChange::Appended, previousCount);
}

void PlotSeries::notifyDataPointsChanged(const DataChange change, const int previousCount)
{
    if (change == DataChange::Replaced) {
        invalidateDataRanges();
    }
    onDataPointsChanged(change, previousCount);
    inspectionDataChanged(change);
    update();
}

void PlotSeries::onDataPointsChanged(const DataChange /*change*/, const int /*previousCount*/)
{
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
    const auto extents = dataPoints_.extents(logScaleX(), logScaleY());
    return {toDataExtent(extents.x), toDataExtent(extents.y)};
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

void PlotSeries::handleAxisScaleChanged()
{
    // Log scale changes which coordinates are valid.
    if (syncDataPointDimensions()) {
        invalidateDataRanges();
    }
    invalidateInspection();
    onAxisScaleChanged();
}

void PlotSeries::handleAxisRangeChanged()
{
    // Only the viewports: an axis switching scale moves its viewport before it reports the new scale.
    using Dimension = DataPointStore::Dimension;
    for (const auto dimension : {Dimension::X, Dimension::Y}) {
        auto state = dimensionState(dimension == Dimension::X ? xAxis_ : yAxis_);
        state.logScale = dataPoints_.dimensionState(dimension).logScale;
        dataPoints_.setDimensionState(dimension, state);
    }
    onAxisRangeChanged();
}

bool PlotSeries::syncDataPointDimensions()
{
    const auto x = dimensionState(xAxis_);
    const auto y = dimensionState(yAxis_);
    using Dimension = DataPointStore::Dimension;
    const auto scaleChanged
        = x.logScale != dataPoints_.dimensionState(Dimension::X).logScale || y.logScale != dataPoints_.dimensionState(Dimension::Y).logScale;
    dataPoints_.setDimensionState(Dimension::X, x);
    dataPoints_.setDimensionState(Dimension::Y, y);
    return scaleChanged;
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
