//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/inspection/SeriesInspection.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/inspection/internal/InspectionCache.hpp"
#include "QAccelPlot/inspection/internal/SourceInspection.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"

#include <algorithm>
#include <cmath>

namespace QAccelPlot {
namespace {

constexpr auto kMaximumPageSize = int{4096};
// Unordered series up to this size are scanned per query instead of being indexed.
constexpr auto kScanLimit = int{20000};

std::optional<InspectionBounds> normalizedBounds(const qreal xMin, const qreal xMax, const qreal yMin, const qreal yMax)
{
    if (std::isnan(xMin) || std::isnan(xMax) || std::isnan(yMin) || std::isnan(yMax)) {
        return std::nullopt;
    }
    return InspectionBounds{std::min(xMin, xMax), std::max(xMin, xMax), std::min(yMin, yMax), std::max(yMin, yMax)};
}

InspectionSummary toSummary(const SummaryAccumulator& stats, const quint64 revision)
{
    auto result = InspectionSummary{};
    result.dataRevision = revision;
    if (stats.count == 0) {
        return result;
    }
    result.status = InspectionStatus::Ready;
    result.count = stats.count;
    result.minimum = stats.minimum;
    result.maximum = stats.maximum;
    result.minimumIndex = stats.minimumIndex;
    result.maximumIndex = stats.maximumIndex;
    result.mean = stats.mean;
    result.standardDeviation = stats.standardDeviation();
    return result;
}

} // namespace

struct SeriesInspection::Private : InspectionCache::Host {
    enum class Order { Unknown, Sorted, Unordered };
    enum class Backend { Sorted, Scan, Index };

    explicit Private(SeriesInspection& owner)
        : owner(owner)
    {
    }

    InspectionSource indexSource() const override
    {
        return owner.series_.inspectionSource();
    }

    bool indexSourceAvailable() const override
    {
        return owner.series_.inspectionAvailable();
    }

    void indexReady() override
    {
        emit owner.statusChanged();
    }

    Backend backend(const InspectionSource& source)
    {
        if (order == Order::Unknown) {
            order = SourceInspection::isSortedX(source) ? Order::Sorted : Order::Unordered;
        }
        if (order == Order::Sorted) {
            return Backend::Sorted;
        }
        return source.count <= kScanLimit ? Backend::Scan : Backend::Index;
    }

    InspectionHit nearestX(const InspectionSource& source, const InspectionMetric& metric, const double pixelX, const double radius)
    {
        switch (backend(source)) {
        case Backend::Sorted:
            return sorted.nearestX(source, metric, pixelX, radius);
        case Backend::Scan:
            return InspectionScan::nearestX(source, metric, pixelX, radius);
        case Backend::Index:
            break;
        }
        return cache.index()->nearestX(metric, pixelX, radius);
    }

    InspectionHit nearest(const InspectionSource& source, const InspectionMetric& metric, const QPointF& position, const double radius)
    {
        switch (backend(source)) {
        case Backend::Sorted:
            return sorted.nearest(source, metric, position, radius);
        case Backend::Scan:
            return InspectionScan::nearest(source, metric, position, radius);
        case Backend::Index:
            break;
        }
        return cache.index()->nearest(metric, position, radius);
    }

    InspectionNeighbors neighbors(const InspectionSource& source, const double x)
    {
        switch (backend(source)) {
        case Backend::Sorted:
            return sorted.neighbors(source, x);
        case Backend::Scan:
            return InspectionScan::neighbors(source, x);
        case Backend::Index:
            break;
        }
        return cache.index()->neighbors(x);
    }

    SummaryAccumulator summarize(const InspectionSource& source, const InspectionBounds& bounds)
    {
        switch (backend(source)) {
        case Backend::Sorted:
            return sorted.summarize(source, bounds);
        case Backend::Scan:
            return InspectionScan::summarize(source, bounds);
        case Backend::Index:
            break;
        }
        return cache.index()->summarize(bounds);
    }

    void collect(const InspectionSource& source, const InspectionBounds& bounds, const int offset, const int limit, QList<int>& indices)
    {
        switch (backend(source)) {
        case Backend::Sorted:
            sorted.collect(source, bounds, offset, limit, indices);
            return;
        case Backend::Scan:
            InspectionScan::collect(source, bounds, offset, limit, indices);
            return;
        case Backend::Index:
            break;
        }
        cache.index()->collect(bounds, offset, limit, indices);
    }

    SeriesInspection& owner;
    Order order{Order::Unknown};
    SourceInspection sorted;
    InspectionCache cache{*this};
};

SeriesInspection::~SeriesInspection() = default;

InspectionStatus SeriesInspection::status() const
{
    const auto source = series_.inspectionSource();
    if (!source.supported()) {
        return InspectionStatus::Unsupported;
    }
    if (!series_.inspectionAvailable()) {
        return InspectionStatus::Unavailable;
    }
    if (d_->backend(source) != Private::Backend::Index || d_->cache.index()) {
        return InspectionStatus::Ready;
    }
    return d_->cache.requested() ? InspectionStatus::Preparing : InspectionStatus::Idle;
}

bool SeriesInspection::supported() const
{
    return series_.inspectionSource().supported();
}

int SeriesInspection::maximumPageSize() const
{
    return kMaximumPageSize;
}

quint64 SeriesInspection::indexBytes() const
{
    auto bytes = quint64{d_->sorted.storageBytes()};
    if (const auto* index = d_->cache.index()) {
        bytes += index->storageBytes();
    }
    return bytes;
}

void SeriesInspection::prepare()
{
    acquire(series_.inspectionSource());
}

InspectionSample SeriesInspection::sampleAt(const int index) const
{
    auto result = InspectionSample{};
    result.dataRevision = series_.dataRevision();
    const auto source = series_.inspectionSource();
    if (!source.supported()) {
        result.status = InspectionStatus::Unsupported;
    } else if (!series_.inspectionAvailable()) {
        result.status = InspectionStatus::Unavailable;
    } else if (index < 0 || index >= source.count) {
        result.status = InspectionStatus::InvalidArgument;
    } else {
        result = makeSample(source, index, std::numeric_limits<qreal>::quiet_NaN());
        if (!source.valid(index)) {
            result.status = InspectionStatus::NoMatch;
        }
    }
    return result;
}

InspectionSample SeriesInspection::nearest(const QPointF& position, const qreal radius)
{
    auto result = InspectionSample{};
    result.dataRevision = series_.dataRevision();
    const auto source = series_.inspectionSource();
    result.status = acquire(source);
    if (result.status != InspectionStatus::Ready) {
        return result;
    }
    if (!std::isfinite(position.x()) || !std::isfinite(position.y()) || !(radius >= 0)) {
        result.status = InspectionStatus::InvalidArgument;
        return result;
    }
    const auto mapping = metric();
    if (!mapping) {
        result.status = InspectionStatus::Unavailable;
        return result;
    }
    const auto hit = d_->nearest(source, *mapping, position, radius);
    return makeSample(source, hit.index, hit.distance);
}

InspectionSample SeriesInspection::nearestByX(const qreal pixelX, const qreal radius)
{
    auto result = InspectionSample{};
    result.dataRevision = series_.dataRevision();
    const auto source = series_.inspectionSource();
    result.status = acquire(source);
    if (result.status != InspectionStatus::Ready) {
        return result;
    }
    if (!std::isfinite(pixelX) || !(radius >= 0)) {
        result.status = InspectionStatus::InvalidArgument;
        return result;
    }
    const auto mapping = metric();
    if (!mapping) {
        result.status = InspectionStatus::Unavailable;
        return result;
    }
    const auto hit = d_->nearestX(source, *mapping, pixelX, radius);
    return makeSample(source, hit.index, hit.distance);
}

InspectionBracket SeriesInspection::bracketByX(const qreal pixelX)
{
    auto result = InspectionBracket{};
    result.dataRevision = series_.dataRevision();
    const auto source = series_.inspectionSource();
    result.status = acquire(source);
    if (result.status != InspectionStatus::Ready) {
        return result;
    }
    const auto mapping = metric();
    if (!std::isfinite(pixelX) || !mapping) {
        result.status = std::isfinite(pixelX) ? InspectionStatus::Unavailable : InspectionStatus::InvalidArgument;
        return result;
    }
    const auto found = d_->neighbors(source, mapping->coordX(pixelX));
    const auto distanceTo = [&](const int index) { return index < 0 ? 0.0 : std::abs(mapping->pixelX(source.x(index)) - pixelX); };
    result.left = makeSample(source, found.left, distanceTo(found.left));
    result.right = makeSample(source, found.right, distanceTo(found.right));
    result.status = found.left >= 0 || found.right >= 0 ? InspectionStatus::Ready : InspectionStatus::NoMatch;
    result.adjacent = found.left >= 0 && found.right == found.left + 1;
    result.interpolated.dataRevision = result.dataRevision;
    if (result.adjacent) {
        result.interpolated = interpolate(*mapping, result.left, result.right, pixelX);
    }
    return result;
}

InspectionSummary SeriesInspection::summarize(const qreal xMin, const qreal xMax, const qreal yMin, const qreal yMax)
{
    auto result = InspectionSummary{};
    result.dataRevision = series_.dataRevision();
    const auto source = series_.inspectionSource();
    result.status = acquire(source);
    if (result.status != InspectionStatus::Ready) {
        return result;
    }
    const auto bounds = normalizedBounds(xMin, xMax, yMin, yMax);
    if (!bounds) {
        result.status = InspectionStatus::InvalidArgument;
        return result;
    }
    return toSummary(d_->summarize(source, *bounds), result.dataRevision);
}

InspectionSummary SeriesInspection::summarizeRange(const qreal xMin, const qreal xMax)
{
    return summarize(xMin, xMax, -std::numeric_limits<qreal>::infinity(), std::numeric_limits<qreal>::infinity());
}

InspectionPage SeriesInspection::indices(
    const qreal xMin, const qreal xMax, const qreal yMin, const qreal yMax, const int offset, const int limit, const quint64 expectedRevision)
{
    auto result = InspectionPage{};
    result.dataRevision = series_.dataRevision();
    result.offset = offset;
    result.limit = std::clamp(limit, 0, kMaximumPageSize);
    if (expectedRevision != 0 && expectedRevision != result.dataRevision) {
        result.status = InspectionStatus::Stale;
        return result;
    }
    const auto source = series_.inspectionSource();
    result.status = acquire(source);
    if (result.status != InspectionStatus::Ready) {
        return result;
    }
    const auto bounds = normalizedBounds(xMin, xMax, yMin, yMax);
    if (!bounds || offset < 0 || limit <= 0) {
        result.status = InspectionStatus::InvalidArgument;
        return result;
    }
    result.total = d_->summarize(source, *bounds).count;
    result.sourceOrder = d_->backend(source) != Private::Backend::Index;
    d_->collect(source, *bounds, offset, result.limit, result.indices);
    result.hasMore = offset + result.indices.size() < result.total;
    result.status = result.total > 0 ? InspectionStatus::Ready : InspectionStatus::NoMatch;
    return result;
}

InspectionRecord SeriesInspection::recordAt(const int index) const
{
    auto result = series_.inspectionRecord(index);
    result.dataRevision = series_.dataRevision();
    return result;
}

InspectionRecord SeriesInspection::recordAtPosition(const QPointF& position) const
{
    auto result = series_.inspectionRecordAt(position);
    result.dataRevision = series_.dataRevision();
    return result;
}

SeriesInspection::SeriesInspection(PlotSeries& series)
    : QObject(&series)
    , series_(series)
    , d_(std::make_unique<Private>(*this))
{
}

void SeriesInspection::sourceChanged(const bool appended)
{
    d_->cache.invalidate();
    const auto source = series_.inspectionSource();
    if (appended && d_->order == Private::Order::Sorted && source.count > 0) {
        const auto last = source.count - 1;
        const auto x = source.x(last);
        if (isValidSample(x, false) && (last == 0 || x >= source.x(last - 1))) {
            d_->sorted.appended(source.count);
        } else {
            d_->order = Private::Order::Unordered;
            d_->sorted.reset();
        }
    } else if (!appended || d_->order != Private::Order::Unordered) {
        d_->order = Private::Order::Unknown;
        d_->sorted.reset();
    }
    emit statusChanged();
}

void SeriesInspection::sourceInvalidated()
{
    d_->cache.invalidate();
    d_->order = Private::Order::Unknown;
    d_->sorted.reset();
    emit statusChanged();
}

InspectionStatus SeriesInspection::acquire(const InspectionSource& source)
{
    if (!source.supported()) {
        return InspectionStatus::Unsupported;
    }
    if (!series_.inspectionAvailable()) {
        return InspectionStatus::Unavailable;
    }
    if (d_->backend(source) != Private::Backend::Index) {
        return InspectionStatus::Ready;
    }
    const auto wasRequested = d_->cache.requested();
    d_->cache.request();
    if (!wasRequested) {
        emit statusChanged();
    }
    return d_->cache.index() ? InspectionStatus::Ready : InspectionStatus::Preparing;
}

std::optional<InspectionMetric> SeriesInspection::metric() const
{
    const auto* xAxis = series_.xAxis();
    const auto* yAxis = series_.yAxis();
    if (!xAxis || !yAxis || !(series_.width() > 0) || !(series_.height() > 0)) {
        return std::nullopt;
    }
    const auto result = InspectionMetric{xAxis->mapping(), yAxis->mapping(), series_.width(), series_.height()};
    if (!result.x.valid || !result.y.valid) {
        return std::nullopt;
    }
    return result;
}

InspectionSample SeriesInspection::makeSample(const InspectionSource& source, const int index, const qreal distance) const
{
    auto result = InspectionSample{};
    result.dataRevision = series_.dataRevision();
    if (index < 0) {
        return result;
    }
    result.status = InspectionStatus::Ready;
    result.index = index;
    result.position = {source.x(index), source.y(index)};
    result.value = source.value(index);
    result.distance = distance;
    if (const auto mapping = metric(); mapping && source.valid(index)) {
        result.pixelPosition = {mapping->pixelX(result.position.x()), mapping->pixelY(result.position.y())};
    }
    return result;
}

InspectionSample SeriesInspection::interpolate(
    const InspectionMetric& metric, const InspectionSample& left, const InspectionSample& right, const qreal pixelX) const
{
    auto result = InspectionSample{};
    result.dataRevision = left.dataRevision;
    const auto span = right.pixelPosition.x() - left.pixelPosition.x();
    if (!std::isfinite(span) || span == 0.0) {
        return result;
    }
    const auto ratio = std::clamp((pixelX - left.pixelPosition.x()) / span, 0.0, 1.0);
    const auto pixelY = left.pixelPosition.y() + ratio * (right.pixelPosition.y() - left.pixelPosition.y());
    result.status = InspectionStatus::Ready;
    result.index = left.index;
    result.interpolated = true;
    result.distance = 0.0;
    result.pixelPosition = {pixelX, pixelY};
    result.position = {metric.coordX(pixelX), metric.y.toCoord(pixelY, metric.height)};
    return result;
}

} // namespace QAccelPlot
