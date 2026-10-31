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
#include <optional>

namespace QAccelPlot {
namespace {

constexpr auto kMaximumPageSize = int{4096};
// Series up to this size are scanned per query instead of being indexed, when they are not ordered as the query needs.
constexpr auto kScanLimit = int{20000};

std::optional<Internal::InspectionBounds> normalizedBounds(const qreal xMin, const qreal xMax, const qreal yMin, const qreal yMax)
{
    if (std::isnan(xMin) || std::isnan(xMax) || std::isnan(yMin) || std::isnan(yMax)) {
        return std::nullopt;
    }
    return Internal::InspectionBounds{std::min(xMin, xMax), std::max(xMin, xMax), std::min(yMin, yMax), std::max(yMin, yMax)};
}

InspectionSummary toSummary(const Internal::SummaryAccumulator& stats, const quint64 revision)
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

// Where a line drawn with a step mode passes between two adjacent data points.
struct StepPoint {
    bool ownedByRight;    // The point belongs to the right data point: it is on its step or its riser.
    bool acrossFromRight; // The coordinate across the query axis is the right data point's, not the left one's.
    bool acrossBetween;   // The coordinate across the query axis lies halfway between the data points.
};

// Returns where a line drawn with \a mode passes at \a ratio of the way from the left to the right data point
// along the query axis. Along X the line holds a value; along Y it climbs the riser between the two values.
StepPoint stepPointBetween(const StepMode mode, const bool alongX, const double ratio)
{
    if (mode == StepMode::After) {
        return alongX ? StepPoint{false, false, false} : StepPoint{true, true, false};
    }
    if (mode == StepMode::Before) {
        const auto right = alongX && ratio > 0.0;
        return StepPoint{right, right, false};
    }
    const auto right = ratio >= 0.5;
    return StepPoint{right, right, !alongX};
}

// Whether appending the last record of source keeps its coordinates along axis non-decreasing.
bool appendKeepsOrder(const Internal::InspectionSource& source, const Internal::InspectionAxis axis)
{
    const auto last = source.count - 1;
    const auto value = source.coordinate(axis, last);
    return isValidCoordinate(value, false) && (last == 0 || value >= source.coordinate(axis, last - 1));
}

} // namespace

struct SeriesInspection::Private : Internal::InspectionCache::Host {
    enum class Backend { Sorted, Scan, Index };

    // How a query is answered, and for the in-place search, the axis the records are ordered along.
    struct Plan {
        Backend backend;
        Internal::InspectionAxis order;
    };

    explicit Private(SeriesInspection& owner)
        : owner(owner)
    {
    }

    Internal::InspectionSource indexSource() const override
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

    void classify(const Internal::InspectionSource& source)
    {
        if (!classified) {
            sortedX = Internal::SourceInspection::isSorted(source, Internal::InspectionAxis::X);
            sortedY = Internal::SourceInspection::isSorted(source, Internal::InspectionAxis::Y);
            classified = true;
        }
    }

    Plan fallback(const Internal::InspectionSource& source) const
    {
        return {source.count <= kScanLimit ? Backend::Scan : Backend::Index, Internal::InspectionAxis::X};
    }

    // On-screen and region queries can search in place when the records are ordered along either axis.
    Plan regionPlan(const Internal::InspectionSource& source)
    {
        classify(source);
        if (sortedX || sortedY) {
            return {Backend::Sorted, sortedX ? Internal::InspectionAxis::X : Internal::InspectionAxis::Y};
        }
        return fallback(source);
    }

    // Queries along one axis can search in place only when the records are ordered along that axis.
    Plan alongPlan(const Internal::InspectionSource& source, const Internal::InspectionAxis axis)
    {
        classify(source);
        if (axis == Internal::InspectionAxis::X ? sortedX : sortedY) {
            return {Backend::Sorted, axis};
        }
        return fallback(source);
    }

    Plan plan(const Internal::InspectionSource& source, const std::optional<Internal::InspectionAxis> along)
    {
        return along ? alongPlan(source, *along) : regionPlan(source);
    }

    Internal::InspectionHit nearestAlong(const Internal::InspectionSource& source, const Internal::InspectionMetric& metric,
        const Internal::InspectionAxis axis, const double pixel, const double radius)
    {
        switch (alongPlan(source, axis).backend) {
        case Backend::Sorted:
            return sorted.nearestAlong(source, metric, axis, pixel, radius);
        case Backend::Scan:
            return Internal::InspectionScan::nearestAlong(source, metric, axis, pixel, radius);
        case Backend::Index:
            break;
        }
        return cache.index()->nearestAlong(metric, axis, pixel, radius);
    }

    Internal::InspectionHit nearest(
        const Internal::InspectionSource& source, const Internal::InspectionMetric& metric, const QPointF& position, const double radius)
    {
        const auto chosen = regionPlan(source);
        switch (chosen.backend) {
        case Backend::Sorted:
            return sorted.nearest(source, metric, chosen.order, position, radius);
        case Backend::Scan:
            return Internal::InspectionScan::nearest(source, metric, position, radius);
        case Backend::Index:
            break;
        }
        return cache.index()->nearest(metric, position, radius);
    }

    Internal::InspectionNeighbors neighbors(const Internal::InspectionSource& source, const Internal::InspectionAxis axis, const double value)
    {
        switch (alongPlan(source, axis).backend) {
        case Backend::Sorted:
            return sorted.neighbors(source, axis, value);
        case Backend::Scan:
            return Internal::InspectionScan::neighbors(source, axis, value);
        case Backend::Index:
            break;
        }
        return cache.index()->neighbors(axis, value);
    }

    Internal::SummaryAccumulator summarize(const Internal::InspectionSource& source, const Internal::InspectionBounds& bounds)
    {
        const auto chosen = regionPlan(source);
        switch (chosen.backend) {
        case Backend::Sorted:
            return sorted.summarize(source, chosen.order, bounds);
        case Backend::Scan:
            return Internal::InspectionScan::summarize(source, bounds);
        case Backend::Index:
            break;
        }
        return cache.index()->summarize(bounds);
    }

    void collect(const Internal::InspectionSource& source, const Internal::InspectionBounds& bounds, const int offset, const int limit, QList<int>& indices)
    {
        const auto chosen = regionPlan(source);
        switch (chosen.backend) {
        case Backend::Sorted:
            sorted.collect(source, chosen.order, bounds, offset, limit, indices);
            return;
        case Backend::Scan:
            Internal::InspectionScan::collect(source, bounds, offset, limit, indices);
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
    Internal::SourceInspection sorted;
    Internal::InspectionCache cache{*this};
};

SeriesInspection::~SeriesInspection() = default;

InspectionStatus SeriesInspection::status() const
{
    const auto source = series_.inspectionSource();
    if (!source.supported() && !series_.recordInspectionSupported()) {
        return InspectionStatus::Unsupported;
    }
    if (!series_.inspectionAvailable()) {
        return InspectionStatus::Unavailable;
    }
    if (!source.supported() || d_->regionPlan(source).backend != Private::Backend::Index || d_->cache.index()) {
        return InspectionStatus::Ready;
    }
    return d_->cache.requested() ? InspectionStatus::Preparing : InspectionStatus::Idle;
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

InspectionDataPoint SeriesInspection::dataPointAt(const int index) const
{
    auto result = InspectionDataPoint{};
    result.dataRevision = series_.dataRevision();
    const auto source = series_.inspectionSource();
    if (!source.supported()) {
        result.status = InspectionStatus::Unsupported;
    } else if (!series_.inspectionAvailable()) {
        result.status = InspectionStatus::Unavailable;
    } else if (index < 0 || index >= source.count) {
        result.status = InspectionStatus::InvalidArgument;
    } else {
        result = makeDataPoint(source, index, std::numeric_limits<qreal>::quiet_NaN());
        if (!source.valid(index)) {
            result.status = InspectionStatus::NoMatch;
        }
    }
    return result;
}

InspectionDataPoint SeriesInspection::nearest(const QPointF& position, const qreal radius)
{
    auto result = InspectionDataPoint{};
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
    return makeDataPoint(source, hit.index, hit.distance);
}

InspectionDataPoint SeriesInspection::nearestByX(const qreal pixelX, const qreal radius)
{
    return nearestAlong(Internal::InspectionAxis::X, pixelX, radius);
}

InspectionDataPoint SeriesInspection::nearestByY(const qreal pixelY, const qreal radius)
{
    return nearestAlong(Internal::InspectionAxis::Y, pixelY, radius);
}

InspectionBracket SeriesInspection::bracketByX(const qreal pixelX)
{
    return bracketAlong(Internal::InspectionAxis::X, pixelX);
}

InspectionBracket SeriesInspection::bracketByY(const qreal pixelY)
{
    return bracketAlong(Internal::InspectionAxis::Y, pixelY);
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

InspectionDataPoint SeriesInspection::dataPointAtPosition(const QPointF& position) const
{
    auto result = InspectionDataPoint{};
    if (!series_.positionInspectionSupported()) {
        result.status = InspectionStatus::Unsupported;
    } else if (!std::isfinite(position.x()) || !std::isfinite(position.y())) {
        result.status = InspectionStatus::InvalidArgument;
    } else {
        result = series_.inspectionDataPointAt(position);
    }
    result.dataRevision = series_.dataRevision();
    return result;
}

SeriesInspection::SeriesInspection(PlotSeries& series)
    : QObject(&series)
    , series_(series)
    , d_(std::make_unique<Private>(*this))
{
}

Internal::InspectionMatching SeriesInspection::matching() const
{
    if (series_.inspectionSource().supported()) {
        return Internal::InspectionMatching::Nearest;
    }
    return series_.positionInspectionSupported() ? Internal::InspectionMatching::Containing : Internal::InspectionMatching::None;
}

void SeriesInspection::sourceChanged(const bool appended)
{
    d_->cache.invalidate();
    const auto source = series_.inspectionSource();
    if (appended && d_->classified && source.count > 0) {
        d_->sortedX = d_->sortedX && appendKeepsOrder(source, Internal::InspectionAxis::X);
        d_->sortedY = d_->sortedY && appendKeepsOrder(source, Internal::InspectionAxis::Y);
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

InspectionStatus SeriesInspection::acquire(const Internal::InspectionSource& source, const std::optional<Internal::InspectionAxis> along)
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

std::optional<Internal::InspectionMetric> SeriesInspection::metric() const
{
    const auto* xAxis = series_.xAxis();
    const auto* yAxis = series_.yAxis();
    if (!xAxis || !yAxis || !(series_.width() > 0) || !(series_.height() > 0)) {
        return std::nullopt;
    }
    const auto result = Internal::InspectionMetric{xAxis->mapping(), yAxis->mapping(), series_.width(), series_.height()};
    if (!result.x.valid || !result.y.valid) {
        return std::nullopt;
    }
    return result;
}

InspectionDataPoint SeriesInspection::nearestAlong(const Internal::InspectionAxis axis, const qreal pixel, const qreal radius)
{
    auto result = InspectionDataPoint{};
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
    return makeDataPoint(source, hit.index, hit.distance);
}

InspectionBracket SeriesInspection::bracketAlong(const Internal::InspectionAxis axis, const qreal pixel)
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
    result.left = makeDataPoint(source, found.left, distanceTo(found.left));
    result.right = makeDataPoint(source, found.right, distanceTo(found.right));
    result.status = found.left >= 0 || found.right >= 0 ? InspectionStatus::Ready : InspectionStatus::NoMatch;
    result.adjacent = found.left >= 0 && found.right == found.left + 1;
    result.interpolated.dataRevision = result.dataRevision;
    if (result.adjacent) {
        result.interpolated = interpolate(*mapping, axis, result.left, result.right, pixel, source.stepMode);
    }
    return result;
}

InspectionDataPoint SeriesInspection::makeDataPoint(const Internal::InspectionSource& source, const int index, const qreal distance) const
{
    auto result = InspectionDataPoint{};
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

InspectionDataPoint SeriesInspection::interpolate(const Internal::InspectionMetric& metric, const Internal::InspectionAxis axis,
    const InspectionDataPoint& left, const InspectionDataPoint& right, const qreal pixel, const StepMode stepMode) const
{
    auto result = InspectionDataPoint{};
    result.dataRevision = left.dataRevision;
    const auto alongX = axis == Internal::InspectionAxis::X;
    const auto acrossAxis = alongX ? Internal::InspectionAxis::Y : Internal::InspectionAxis::X;
    const auto pixelAlong = [alongX](const QPointF& position) { return alongX ? position.x() : position.y(); };
    const auto pixelAcross = [alongX](const QPointF& position) { return alongX ? position.y() : position.x(); };
    const auto from = pixelAlong(left.pixelPosition);
    const auto span = pixelAlong(right.pixelPosition) - from;
    if (!std::isfinite(span) || span == 0.0) {
        return result;
    }
    const auto ratio = std::clamp((pixel - from) / span, 0.0, 1.0);
    auto across = pixelAcross(left.pixelPosition) + ratio * (pixelAcross(right.pixelPosition) - pixelAcross(left.pixelPosition));
    auto acrossCoordinate = std::optional<double>{};
    result.index = left.index;
    if (stepMode != StepMode::None) {
        const auto step = stepPointBetween(stepMode, alongX, ratio);
        const auto& acrossPoint = step.acrossFromRight ? right : left;
        result.index = step.ownedByRight ? right.index : left.index;
        across = step.acrossBetween ? (pixelAcross(left.pixelPosition) + pixelAcross(right.pixelPosition)) * 0.5 : pixelAcross(acrossPoint.pixelPosition);
        if (!step.acrossBetween) {
            acrossCoordinate = alongX ? acrossPoint.position.y() : acrossPoint.position.x();
        }
    }
    result.status = InspectionStatus::Ready;
    result.interpolated = true;
    result.distance = 0.0;
    result.pixelPosition = alongX ? QPointF{pixel, across} : QPointF{across, pixel};
    const auto alongCoordinate = metric.coord(axis, pixel);
    const auto acrossValue = acrossCoordinate.value_or(metric.coord(acrossAxis, across));
    result.position = alongX ? QPointF{alongCoordinate, acrossValue} : QPointF{acrossValue, alongCoordinate};
    return result;
}

} // namespace QAccelPlot
