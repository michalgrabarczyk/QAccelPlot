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
// Series up to this size are scanned per query instead of being indexed, when they are not ordered as the query needs.
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

// Whether appending the last record of source keeps its coordinates along axis non-decreasing.
bool appendKeepsOrder(const InspectionSource& source, const InspectionAxis axis)
{
    const auto last = source.count - 1;
    const auto value = source.coordinate(axis, last);
    return isValidSample(value, false) && (last == 0 || value >= source.coordinate(axis, last - 1));
}

} // namespace

struct SeriesInspection::Private : InspectionCache::Host {
    enum class Backend { Sorted, Scan, Index };

    // How a query is answered, and for the in-place search, the axis the records are ordered along.
    struct Plan {
        Backend backend;
        InspectionAxis order;
    };

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

    void classify(const InspectionSource& source)
    {
        if (!classified) {
            sortedX = SourceInspection::isSorted(source, InspectionAxis::X);
            sortedY = SourceInspection::isSorted(source, InspectionAxis::Y);
            classified = true;
        }
    }

    Plan fallback(const InspectionSource& source) const
    {
        return {source.count <= kScanLimit ? Backend::Scan : Backend::Index, InspectionAxis::X};
    }

    // On-screen and region queries can search in place when the records are ordered along either axis.
    Plan regionPlan(const InspectionSource& source)
    {
        classify(source);
        if (sortedX || sortedY) {
            return {Backend::Sorted, sortedX ? InspectionAxis::X : InspectionAxis::Y};
        }
        return fallback(source);
    }

    // Queries along one axis can search in place only when the records are ordered along that axis.
    Plan alongPlan(const InspectionSource& source, const InspectionAxis axis)
    {
        classify(source);
        if (axis == InspectionAxis::X ? sortedX : sortedY) {
            return {Backend::Sorted, axis};
        }
        return fallback(source);
    }

    Plan plan(const InspectionSource& source, const std::optional<InspectionAxis> along)
    {
        return along ? alongPlan(source, *along) : regionPlan(source);
    }

    InspectionHit nearestAlong(
        const InspectionSource& source, const InspectionMetric& metric, const InspectionAxis axis, const double pixel, const double radius)
    {
        switch (alongPlan(source, axis).backend) {
        case Backend::Sorted:
            return sorted.nearestAlong(source, metric, axis, pixel, radius);
        case Backend::Scan:
            return InspectionScan::nearestAlong(source, metric, axis, pixel, radius);
        case Backend::Index:
            break;
        }
        return cache.index()->nearestAlong(metric, axis, pixel, radius);
    }

    InspectionHit nearest(const InspectionSource& source, const InspectionMetric& metric, const QPointF& position, const double radius)
    {
        const auto chosen = regionPlan(source);
        switch (chosen.backend) {
        case Backend::Sorted:
            return sorted.nearest(source, metric, chosen.order, position, radius);
        case Backend::Scan:
            return InspectionScan::nearest(source, metric, position, radius);
        case Backend::Index:
            break;
        }
        return cache.index()->nearest(metric, position, radius);
    }

    InspectionNeighbors neighbors(const InspectionSource& source, const InspectionAxis axis, const double value)
    {
        switch (alongPlan(source, axis).backend) {
        case Backend::Sorted:
            return sorted.neighbors(source, axis, value);
        case Backend::Scan:
            return InspectionScan::neighbors(source, axis, value);
        case Backend::Index:
            break;
        }
        return cache.index()->neighbors(axis, value);
    }

    SummaryAccumulator summarize(const InspectionSource& source, const InspectionBounds& bounds)
    {
        const auto chosen = regionPlan(source);
        switch (chosen.backend) {
        case Backend::Sorted:
            return sorted.summarize(source, chosen.order, bounds);
        case Backend::Scan:
            return InspectionScan::summarize(source, bounds);
        case Backend::Index:
            break;
        }
        return cache.index()->summarize(bounds);
    }

    void collect(const InspectionSource& source, const InspectionBounds& bounds, const int offset, const int limit, QList<int>& indices)
    {
        const auto chosen = regionPlan(source);
        switch (chosen.backend) {
        case Backend::Sorted:
            sorted.collect(source, chosen.order, bounds, offset, limit, indices);
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
    bool classified{false};
    bool sortedX{false};
    bool sortedY{false};
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
    if (d_->regionPlan(source).backend != Private::Backend::Index || d_->cache.index()) {
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
    acquire(series_.inspectionSource(), std::nullopt);
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
    result.status = acquire(source, std::nullopt);
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
    return nearestAlong(InspectionAxis::X, pixelX, radius);
}

InspectionSample SeriesInspection::nearestByY(const qreal pixelY, const qreal radius)
{
    return nearestAlong(InspectionAxis::Y, pixelY, radius);
}

InspectionBracket SeriesInspection::bracketByX(const qreal pixelX)
{
    return bracketAlong(InspectionAxis::X, pixelX);
}

InspectionBracket SeriesInspection::bracketByY(const qreal pixelY)
{
    return bracketAlong(InspectionAxis::Y, pixelY);
}

InspectionSummary SeriesInspection::summarize(const qreal xMin, const qreal xMax, const qreal yMin, const qreal yMax)
{
    auto result = InspectionSummary{};
    result.dataRevision = series_.dataRevision();
    const auto source = series_.inspectionSource();
    result.status = acquire(source, std::nullopt);
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
    result.status = acquire(source, std::nullopt);
    if (result.status != InspectionStatus::Ready) {
        return result;
    }
    const auto bounds = normalizedBounds(xMin, xMax, yMin, yMax);
    if (!bounds || offset < 0 || limit <= 0) {
        result.status = InspectionStatus::InvalidArgument;
        return result;
    }
    result.total = d_->summarize(source, *bounds).count;
    result.sourceOrder = d_->regionPlan(source).backend != Private::Backend::Index;
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
    if (appended && d_->classified && source.count > 0) {
        d_->sortedX = d_->sortedX && appendKeepsOrder(source, InspectionAxis::X);
        d_->sortedY = d_->sortedY && appendKeepsOrder(source, InspectionAxis::Y);
        d_->sorted.appended(source.count);
    } else {
        d_->classified = false;
        d_->sorted.reset();
    }
    emit statusChanged();
}

void SeriesInspection::sourceInvalidated()
{
    d_->cache.invalidate();
    d_->classified = false;
    d_->sorted.reset();
    emit statusChanged();
}

InspectionStatus SeriesInspection::acquire(const InspectionSource& source, const std::optional<InspectionAxis> along)
{
    if (!source.supported()) {
        return InspectionStatus::Unsupported;
    }
    if (!series_.inspectionAvailable()) {
        return InspectionStatus::Unavailable;
    }
    if (d_->plan(source, along).backend != Private::Backend::Index) {
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

InspectionSample SeriesInspection::nearestAlong(const InspectionAxis axis, const qreal pixel, const qreal radius)
{
    auto result = InspectionSample{};
    result.dataRevision = series_.dataRevision();
    const auto source = series_.inspectionSource();
    result.status = acquire(source, axis);
    if (result.status != InspectionStatus::Ready) {
        return result;
    }
    if (!std::isfinite(pixel) || !(radius >= 0)) {
        result.status = InspectionStatus::InvalidArgument;
        return result;
    }
    const auto mapping = metric();
    if (!mapping) {
        result.status = InspectionStatus::Unavailable;
        return result;
    }
    const auto hit = d_->nearestAlong(source, *mapping, axis, pixel, radius);
    return makeSample(source, hit.index, hit.distance);
}

InspectionBracket SeriesInspection::bracketAlong(const InspectionAxis axis, const qreal pixel)
{
    auto result = InspectionBracket{};
    result.dataRevision = series_.dataRevision();
    const auto source = series_.inspectionSource();
    result.status = acquire(source, axis);
    if (result.status != InspectionStatus::Ready) {
        return result;
    }
    const auto mapping = metric();
    if (!std::isfinite(pixel) || !mapping) {
        result.status = std::isfinite(pixel) ? InspectionStatus::Unavailable : InspectionStatus::InvalidArgument;
        return result;
    }
    const auto found = d_->neighbors(source, axis, mapping->coord(axis, pixel));
    const auto distanceTo = [&](const int index) { return index < 0 ? 0.0 : std::abs(mapping->pixel(axis, source.coordinate(axis, index)) - pixel); };
    result.left = makeSample(source, found.left, distanceTo(found.left));
    result.right = makeSample(source, found.right, distanceTo(found.right));
    result.status = found.left >= 0 || found.right >= 0 ? InspectionStatus::Ready : InspectionStatus::NoMatch;
    result.adjacent = found.left >= 0 && found.right == found.left + 1;
    result.interpolated.dataRevision = result.dataRevision;
    if (result.adjacent) {
        result.interpolated = interpolate(*mapping, axis, result.left, result.right, pixel);
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
    const InspectionMetric& metric, const InspectionAxis axis, const InspectionSample& left, const InspectionSample& right, const qreal pixel) const
{
    auto result = InspectionSample{};
    result.dataRevision = left.dataRevision;
    const auto alongX = axis == InspectionAxis::X;
    const auto from = alongX ? left.pixelPosition.x() : left.pixelPosition.y();
    const auto span = (alongX ? right.pixelPosition.x() : right.pixelPosition.y()) - from;
    if (!std::isfinite(span) || span == 0.0) {
        return result;
    }
    const auto ratio = std::clamp((pixel - from) / span, 0.0, 1.0);
    const auto across = alongX ? left.pixelPosition.y() + ratio * (right.pixelPosition.y() - left.pixelPosition.y())
                               : left.pixelPosition.x() + ratio * (right.pixelPosition.x() - left.pixelPosition.x());
    result.status = InspectionStatus::Ready;
    result.index = left.index;
    result.interpolated = true;
    result.distance = 0.0;
    result.pixelPosition = alongX ? QPointF{pixel, across} : QPointF{across, pixel};
    result.position = {metric.coord(InspectionAxis::X, result.pixelPosition.x()), metric.coord(InspectionAxis::Y, result.pixelPosition.y())};
    return result;
}

} // namespace QAccelPlot
