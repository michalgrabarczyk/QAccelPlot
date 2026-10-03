//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/inspection/internal/SourceInspection.hpp"

#include "QAccelPlot/MathUtils.hpp"

#include <algorithm>
#include <cmath>

namespace QAccelPlot {
namespace {

constexpr auto kBlockSize = int{512};

// First index whose coordinate along axis is not smaller than value.
int lowerBound(const InspectionSource& source, const InspectionAxis axis, const double value)
{
    auto first = 0;
    auto length = source.count;
    while (length > 0) {
        const auto half = length / 2;
        if (source.coordinate(axis, first + half) < value) {
            first += half + 1;
            length -= half + 1;
        } else {
            length = half;
        }
    }
    return first;
}

// First index whose coordinate along axis is greater than value.
int upperBound(const InspectionSource& source, const InspectionAxis axis, const double value)
{
    auto first = 0;
    auto length = source.count;
    while (length > 0) {
        const auto half = length / 2;
        if (value < source.coordinate(axis, first + half)) {
            length = half;
        } else {
            first += half + 1;
            length -= half + 1;
        }
    }
    return first;
}

int previousValid(const InspectionSource& source, int index)
{
    while (index >= 0 && !source.valid(index)) {
        --index;
    }
    return index;
}

int nextValid(const InspectionSource& source, int index)
{
    while (index < source.count && !source.valid(index)) {
        ++index;
    }
    return index < source.count ? index : -1;
}

void scanNearest(const InspectionSource& source, const InspectionMetric& metric, const QPointF& position, const int first, const int last, InspectionHit& best)
{
    for (auto i = first; i < last; ++i) {
        if (source.valid(i)) {
            best.consider(i, std::hypot(metric.pixelX(source.x(i)) - position.x(), metric.pixelY(source.y(i)) - position.y()));
        }
    }
}

void scanSummary(const InspectionSource& source, const InspectionBounds& bounds, const int first, const int last, SummaryAccumulator& summary)
{
    for (auto i = first; i < last; ++i) {
        if (source.valid(i) && bounds.contains(source.x(i), source.y(i))) {
            summary.add(source.y(i), i);
        }
    }
}

void scanCollect(
    const InspectionSource& source, const InspectionBounds& bounds, const int first, const int last, int& offset, const int limit, QList<int>& indices)
{
    for (auto i = first; i < last && indices.size() < limit; ++i) {
        if (!source.valid(i) || !bounds.contains(source.x(i), source.y(i))) {
            continue;
        }
        if (offset > 0) {
            --offset;
        } else {
            indices.push_back(i);
        }
    }
}

// Limits of the bounds along the axis the records are ordered by.
double lowerLimit(const InspectionBounds& bounds, const InspectionAxis order)
{
    return order == InspectionAxis::X ? bounds.xMin : bounds.yMin;
}

double upperLimit(const InspectionBounds& bounds, const InspectionAxis order)
{
    return order == InspectionAxis::X ? bounds.xMax : bounds.yMax;
}

} // namespace

bool SourceInspection::isSorted(const InspectionSource& source, const InspectionAxis axis)
{
    auto previous = std::numeric_limits<double>::lowest();
    for (auto i = 0; i < source.count; ++i) {
        const auto value = source.coordinate(axis, i);
        if (!isValidSample(value, false) || value < previous) {
            return false;
        }
        previous = value;
    }
    return true;
}

void SourceInspection::reset()
{
    blocks_.clear();
}

void SourceInspection::appended(const int count)
{
    if (blocks_.empty() || count <= 0) {
        return;
    }
    syncBlocks(count);
    blocks_[static_cast<std::size_t>((count - 1) / kBlockSize)].computed = false;
}

std::size_t SourceInspection::storageBytes() const
{
    return blocks_.capacity() * sizeof(Block);
}

InspectionHit SourceInspection::nearestAlong(
    const InspectionSource& source, const InspectionMetric& metric, const InspectionAxis order, const double pixel, const double radius) const
{
    auto best = InspectionHit{-1, radius};
    const auto distanceTo = [&](const int index) { return std::abs(metric.pixel(order, source.coordinate(order, index)) - pixel); };
    const auto pivot = lowerBound(source, order, metric.coord(order, pixel));
    const auto before = previousValid(source, pivot - 1);
    if (before >= 0) {
        best.consider(before, distanceTo(before));
    }
    auto after = nextValid(source, pivot);
    if (after >= 0) {
        // Among samples sharing this coordinate, the highest index is the one drawn last.
        const auto shared = source.coordinate(order, after);
        for (auto i = after + 1; i < source.count && source.coordinate(order, i) == shared; ++i) {
            if (source.valid(i)) {
                after = i;
            }
        }
        best.consider(after, distanceTo(after));
    }
    return best;
}

InspectionHit SourceInspection::nearest(
    const InspectionSource& source, const InspectionMetric& metric, const InspectionAxis order, const QPointF& position, const double radius)
{
    auto best = InspectionHit{-1, radius};
    if (source.count <= 0) {
        return best;
    }
    syncBlocks(source.count);
    // Blocks are visited outwards from the cursor, so the search stops as soon as a block is
    // farther away along the ordered axis than the best match.
    const auto blockCount = static_cast<int>(blocks_.size());
    const auto cursor = metric.coord(order, order == InspectionAxis::X ? position.x() : position.y());
    auto after = std::min(lowerBound(source, order, cursor), source.count - 1) / kBlockSize;
    auto before = after - 1;
    while (before >= 0 || after < blockCount) {
        if (after < blockCount) {
            after = visitBlock(source, metric, order, position, after, best) ? after + 1 : blockCount;
        }
        if (before >= 0) {
            before = visitBlock(source, metric, order, position, before, best) ? before - 1 : -1;
        }
    }
    return best;
}

InspectionNeighbors SourceInspection::neighbors(const InspectionSource& source, const InspectionAxis order, const double value) const
{
    const auto pivot = upperBound(source, order, value);
    return {previousValid(source, pivot - 1), nextValid(source, pivot)};
}

SummaryAccumulator SourceInspection::summarize(const InspectionSource& source, const InspectionAxis order, const InspectionBounds& bounds)
{
    auto summary = SummaryAccumulator{};
    if (source.count <= 0) {
        return summary;
    }
    syncBlocks(source.count);
    const auto last = upperBound(source, order, upperLimit(bounds, order));
    auto index = lowerBound(source, order, lowerLimit(bounds, order));
    while (index < last) {
        const auto end = std::min(last, (index / kBlockSize + 1) * kBlockSize);
        const auto covered = coverage(source, order, bounds, index, end);
        if (covered == Coverage::Full) {
            summary.merge(block(source, index / kBlockSize).stats);
        } else if (covered == Coverage::Partial) {
            scanSummary(source, bounds, index, end, summary);
        }
        index = end;
    }
    return summary;
}

void SourceInspection::collect(
    const InspectionSource& source, const InspectionAxis order, const InspectionBounds& bounds, int offset, const int limit, QList<int>& indices)
{
    if (source.count <= 0) {
        return;
    }
    syncBlocks(source.count);
    const auto last = upperBound(source, order, upperLimit(bounds, order));
    auto index = lowerBound(source, order, lowerLimit(bounds, order));
    while (index < last && indices.size() < limit) {
        const auto end = std::min(last, (index / kBlockSize + 1) * kBlockSize);
        const auto covered = coverage(source, order, bounds, index, end);
        const auto matches = covered == Coverage::Full ? block(source, index / kBlockSize).stats.count : 0;
        if (covered == Coverage::Full && offset >= matches) {
            // Every valid sample of the block matches, so the whole block can be skipped.
            offset -= matches;
        } else if (covered != Coverage::None) {
            scanCollect(source, bounds, index, end, offset, limit, indices);
        }
        index = end;
    }
}

void SourceInspection::syncBlocks(const int count)
{
    const auto needed = static_cast<std::size_t>((count + kBlockSize - 1) / kBlockSize);
    if (blocks_.size() != needed) {
        blocks_.resize(needed);
    }
}

const SourceInspection::Block& SourceInspection::block(const InspectionSource& source, const int index)
{
    auto& result = blocks_[static_cast<std::size_t>(index)];
    if (result.computed) {
        return result;
    }
    result = Block{};
    const auto first = index * kBlockSize;
    const auto last = std::min(source.count, first + kBlockSize);
    for (auto i = first; i < last; ++i) {
        if (!source.valid(i)) {
            continue;
        }
        const auto x = source.x(i);
        result.xMin = result.stats.count == 0 ? x : std::min(result.xMin, x);
        result.xMax = result.stats.count == 0 ? x : std::max(result.xMax, x);
        result.stats.add(source.y(i), i);
    }
    result.computed = true;
    return result;
}

SourceInspection::Coverage SourceInspection::coverage(
    const InspectionSource& source, const InspectionAxis order, const InspectionBounds& bounds, const int first, const int end)
{
    const auto wholeBlock = first % kBlockSize == 0 && (end - first == kBlockSize || end == source.count);
    if (!wholeBlock) {
        return Coverage::Partial;
    }
    const auto& candidate = block(source, first / kBlockSize);
    if (candidate.stats.count == 0) {
        return Coverage::None;
    }
    // The ordered axis is covered by the index range, so only the other axis is compared.
    const auto ordered = order == InspectionAxis::X;
    const auto low = ordered ? candidate.stats.minimum : candidate.xMin;
    const auto high = ordered ? candidate.stats.maximum : candidate.xMax;
    const auto lowLimit = ordered ? bounds.yMin : bounds.xMin;
    const auto highLimit = ordered ? bounds.yMax : bounds.xMax;
    if (high < lowLimit || low > highLimit) {
        return Coverage::None;
    }
    return low >= lowLimit && high <= highLimit ? Coverage::Full : Coverage::Partial;
}

bool SourceInspection::visitBlock(
    const InspectionSource& source, const InspectionMetric& metric, const InspectionAxis order, const QPointF& position, const int index, InspectionHit& best)
{
    const auto& candidate = block(source, index);
    if (candidate.stats.count == 0) {
        return true;
    }
    const auto dx = distanceToInterval(position.x(), metric.pixelX(candidate.xMin), metric.pixelX(candidate.xMax));
    const auto dy = distanceToInterval(position.y(), metric.pixelY(candidate.stats.minimum), metric.pixelY(candidate.stats.maximum));
    if (std::abs(order == InspectionAxis::X ? dx : dy) > best.distance) {
        return false;
    }
    const auto distance = std::hypot(dx, dy);
    const auto first = index * kBlockSize;
    const auto last = std::min(source.count, first + kBlockSize);
    if (distance < best.distance || (distance == best.distance && last - 1 > best.index)) {
        scanNearest(source, metric, position, first, last, best);
    }
    return true;
}

namespace InspectionScan {

InspectionHit nearestAlong(const InspectionSource& source, const InspectionMetric& metric, const InspectionAxis axis, const double pixel, const double radius)
{
    auto best = InspectionHit{-1, radius};
    for (auto i = 0; i < source.count; ++i) {
        if (source.valid(i)) {
            best.consider(i, std::abs(metric.pixel(axis, source.coordinate(axis, i)) - pixel));
        }
    }
    return best;
}

InspectionHit nearest(const InspectionSource& source, const InspectionMetric& metric, const QPointF& position, const double radius)
{
    auto best = InspectionHit{-1, radius};
    scanNearest(source, metric, position, 0, source.count, best);
    return best;
}

InspectionNeighbors neighbors(const InspectionSource& source, const InspectionAxis axis, const double value)
{
    auto result = InspectionNeighbors{};
    for (auto i = 0; i < source.count; ++i) {
        if (!source.valid(i)) {
            continue;
        }
        const auto candidate = source.coordinate(axis, i);
        if (candidate <= value && (result.left < 0 || candidate >= source.coordinate(axis, result.left))) {
            result.left = i;
        } else if (candidate > value && (result.right < 0 || candidate < source.coordinate(axis, result.right))) {
            result.right = i;
        }
    }
    return result;
}

SummaryAccumulator summarize(const InspectionSource& source, const InspectionBounds& bounds)
{
    auto summary = SummaryAccumulator{};
    scanSummary(source, bounds, 0, source.count, summary);
    return summary;
}

void collect(const InspectionSource& source, const InspectionBounds& bounds, int offset, const int limit, QList<int>& indices)
{
    scanCollect(source, bounds, 0, source.count, offset, limit, indices);
}

} // namespace InspectionScan

} // namespace QAccelPlot
