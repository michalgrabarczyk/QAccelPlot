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

enum class Coverage { None, Partial, Full };

// First index whose X is not smaller than x.
int lowerBound(const InspectionSource& source, const double x)
{
    auto first = 0;
    auto length = source.count;
    while (length > 0) {
        const auto half = length / 2;
        if (source.x(first + half) < x) {
            first += half + 1;
            length -= half + 1;
        } else {
            length = half;
        }
    }
    return first;
}

// First index whose X is greater than x.
int upperBound(const InspectionSource& source, const double x)
{
    auto first = 0;
    auto length = source.count;
    while (length > 0) {
        const auto half = length / 2;
        if (x < source.x(first + half)) {
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

Coverage coverage(const SummaryAccumulator& stats, const InspectionBounds& bounds)
{
    if (stats.count == 0 || stats.maximum < bounds.yMin || stats.minimum > bounds.yMax) {
        return Coverage::None;
    }
    return stats.minimum >= bounds.yMin && stats.maximum <= bounds.yMax ? Coverage::Full : Coverage::Partial;
}

} // namespace

bool SourceInspection::isSortedX(const InspectionSource& source)
{
    auto previous = std::numeric_limits<double>::lowest();
    for (auto i = 0; i < source.count; ++i) {
        const auto x = source.x(i);
        if (!isValidSample(x, false) || x < previous) {
            return false;
        }
        previous = x;
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

InspectionHit SourceInspection::nearestX(const InspectionSource& source, const InspectionMetric& metric, const double pixelX, const double radius) const
{
    auto best = InspectionHit{-1, radius};
    const auto pivot = lowerBound(source, metric.coordX(pixelX));
    const auto left = previousValid(source, pivot - 1);
    if (left >= 0) {
        best.consider(left, std::abs(metric.pixelX(source.x(left)) - pixelX));
    }
    auto right = nextValid(source, pivot);
    if (right >= 0) {
        // Among samples sharing this X, the highest index is the one drawn last.
        const auto x = source.x(right);
        for (auto i = right + 1; i < source.count && source.x(i) == x; ++i) {
            if (source.valid(i)) {
                right = i;
            }
        }
        best.consider(right, std::abs(metric.pixelX(x) - pixelX));
    }
    return best;
}

InspectionHit SourceInspection::nearest(const InspectionSource& source, const InspectionMetric& metric, const QPointF& position, const double radius)
{
    auto best = InspectionHit{-1, radius};
    if (source.count <= 0) {
        return best;
    }
    syncBlocks(source.count);
    // Blocks are visited outwards from the cursor, so the search stops as soon as a block is
    // farther away horizontally than the best match.
    const auto blockCount = static_cast<int>(blocks_.size());
    auto right = std::min(lowerBound(source, metric.coordX(position.x())), source.count - 1) / kBlockSize;
    auto left = right - 1;
    while (left >= 0 || right < blockCount) {
        if (right < blockCount) {
            right = visitBlock(source, metric, position, right, best) ? right + 1 : blockCount;
        }
        if (left >= 0) {
            left = visitBlock(source, metric, position, left, best) ? left - 1 : -1;
        }
    }
    return best;
}

InspectionNeighbors SourceInspection::neighbors(const InspectionSource& source, const double x) const
{
    const auto pivot = upperBound(source, x);
    return {previousValid(source, pivot - 1), nextValid(source, pivot)};
}

SummaryAccumulator SourceInspection::summarize(const InspectionSource& source, const InspectionBounds& bounds)
{
    auto summary = SummaryAccumulator{};
    if (source.count <= 0) {
        return summary;
    }
    syncBlocks(source.count);
    const auto last = upperBound(source, bounds.xMax);
    auto index = lowerBound(source, bounds.xMin);
    while (index < last) {
        const auto blockIndex = index / kBlockSize;
        const auto end = std::min(last, (blockIndex + 1) * kBlockSize);
        const auto wholeBlock = index % kBlockSize == 0 && (end - index == kBlockSize || end == source.count);
        const auto covered = wholeBlock ? coverage(block(source, blockIndex).stats, bounds) : Coverage::Partial;
        if (covered == Coverage::Full) {
            summary.merge(block(source, blockIndex).stats);
        } else if (covered == Coverage::Partial) {
            scanSummary(source, bounds, index, end, summary);
        }
        index = end;
    }
    return summary;
}

void SourceInspection::collect(const InspectionSource& source, const InspectionBounds& bounds, int offset, const int limit, QList<int>& indices)
{
    if (source.count <= 0) {
        return;
    }
    syncBlocks(source.count);
    const auto last = upperBound(source, bounds.xMax);
    auto index = lowerBound(source, bounds.xMin);
    while (index < last && indices.size() < limit) {
        const auto blockIndex = index / kBlockSize;
        const auto end = std::min(last, (blockIndex + 1) * kBlockSize);
        const auto wholeBlock = index % kBlockSize == 0 && (end - index == kBlockSize || end == source.count);
        const auto covered = wholeBlock ? coverage(block(source, blockIndex).stats, bounds) : Coverage::Partial;
        if (covered == Coverage::Full && offset >= block(source, blockIndex).stats.count) {
            // Every valid sample of the block matches, so the whole block can be skipped.
            offset -= block(source, blockIndex).stats.count;
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
        // X is non-decreasing, so the first and last valid samples bound the block.
        if (result.stats.count == 0) {
            result.xMin = source.x(i);
        }
        result.xMax = source.x(i);
        result.stats.add(source.y(i), i);
    }
    result.computed = true;
    return result;
}

bool SourceInspection::visitBlock(const InspectionSource& source, const InspectionMetric& metric, const QPointF& position, const int index, InspectionHit& best)
{
    const auto& candidate = block(source, index);
    if (candidate.stats.count == 0) {
        return true;
    }
    const auto dx = distanceToInterval(position.x(), metric.pixelX(candidate.xMin), metric.pixelX(candidate.xMax));
    if (std::abs(dx) > best.distance) {
        return false;
    }
    const auto dy = distanceToInterval(position.y(), metric.pixelY(candidate.stats.minimum), metric.pixelY(candidate.stats.maximum));
    const auto distance = std::hypot(dx, dy);
    const auto first = index * kBlockSize;
    const auto last = std::min(source.count, first + kBlockSize);
    if (distance < best.distance || (distance == best.distance && last - 1 > best.index)) {
        scanNearest(source, metric, position, first, last, best);
    }
    return true;
}

namespace InspectionScan {

InspectionHit nearestX(const InspectionSource& source, const InspectionMetric& metric, const double pixelX, const double radius)
{
    auto best = InspectionHit{-1, radius};
    for (auto i = 0; i < source.count; ++i) {
        if (source.valid(i)) {
            best.consider(i, std::abs(metric.pixelX(source.x(i)) - pixelX));
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

InspectionNeighbors neighbors(const InspectionSource& source, const double x)
{
    auto result = InspectionNeighbors{};
    for (auto i = 0; i < source.count; ++i) {
        if (!source.valid(i)) {
            continue;
        }
        const auto candidate = source.x(i);
        if (candidate <= x && (result.left < 0 || candidate >= source.x(result.left))) {
            result.left = i;
        } else if (candidate > x && (result.right < 0 || candidate < source.x(result.right))) {
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
