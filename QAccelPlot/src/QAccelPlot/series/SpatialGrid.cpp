//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/SpatialGrid.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

namespace QAccelPlot {

void SpatialGrid::build(const double* data, const int itemCount, const int valuesPerItem)
{
    buildFrom(data, itemCount, valuesPerItem);
}

void SpatialGrid::buildF(const float* data, const int itemCount, const int valuesPerItem)
{
    buildFrom(data, itemCount, valuesPerItem);
}

int SpatialGrid::query(const double x, const double y) const
{
    if (cols_ <= 0 || rows_ <= 0 || std::isnan(x) || std::isnan(y)) {
        return -1;
    }

    // Points outside the grid can still hit unbounded rectangles, which reach the edge cells.
    const auto c = cellIndex(x, minX_, cellW_, cols_);
    const auto r = cellIndex(y, minY_, cellH_, rows_);
    const auto& cell = cells_[static_cast<size_t>(r) * static_cast<size_t>(cols_) + static_cast<size_t>(c)];

    auto candidate = -1;
    for (auto it = cell.rbegin(); it != cell.rend(); ++it) {
        const auto& bounds = itemBounds_[static_cast<size_t>(*it)];
        if (bounds.contains(x, y)) {
            candidate = *it;
            break;
        }
    }
    for (auto it = largeItems_.rbegin(); it != largeItems_.rend() && *it > candidate; ++it) {
        if (itemBounds_[static_cast<size_t>(*it)].contains(x, y)) {
            return *it;
        }
    }
    return candidate;
}

int SpatialGrid::queryTopmost(const double minX, const double minY, const double maxX, const double maxY, const std::function<bool(int)>& accept) const
{
    if (cols_ <= 0 || rows_ <= 0 || std::isnan(minX) || std::isnan(minY) || std::isnan(maxX) || std::isnan(maxY)) {
        return -1;
    }

    const auto isCandidate = [&](const int index) { return itemBounds_[static_cast<size_t>(index)].overlaps(minX, minY, maxX, maxY) && accept(index); };
    auto best = -1;
    const auto c1 = cellIndex(maxX, minX_, cellW_, cols_);
    const auto r1 = cellIndex(maxY, minY_, cellH_, rows_);
    for (auto r = cellIndex(minY, minY_, cellH_, rows_); r <= r1; ++r) {
        for (auto c = cellIndex(minX, minX_, cellW_, cols_); c <= c1; ++c) {
            // Cells list their items in ascending index order.
            const auto& cell = cells_[static_cast<size_t>(r) * static_cast<size_t>(cols_) + static_cast<size_t>(c)];
            for (auto it = cell.rbegin(); it != cell.rend() && *it > best; ++it) {
                if (isCandidate(*it)) {
                    best = *it;
                    break;
                }
            }
        }
    }
    for (auto it = largeItems_.rbegin(); it != largeItems_.rend() && *it > best; ++it) {
        if (isCandidate(*it)) {
            return *it;
        }
    }
    return best;
}

int SpatialGrid::scanTopmost(const double* data, const int itemCount, const double minX, const double minY, const double maxX, const double maxY,
    const std::function<bool(int)>& accept, const int valuesPerItem)
{
    return scanTopmostFrom(data, itemCount, valuesPerItem, ItemBounds{minX, minY, maxX, maxY}, accept);
}

int SpatialGrid::scanTopmostF(const float* data, const int itemCount, const double minX, const double minY, const double maxX, const double maxY,
    const std::function<bool(int)>& accept, const int valuesPerItem)
{
    return scanTopmostFrom(data, itemCount, valuesPerItem, ItemBounds{minX, minY, maxX, maxY}, accept);
}

bool SpatialGrid::ItemBounds::contains(const double x, const double y) const
{
    return x >= minX && x <= maxX && y >= minY && y <= maxY;
}

bool SpatialGrid::ItemBounds::overlaps(const double boxMinX, const double boxMinY, const double boxMaxX, const double boxMaxY) const
{
    return minX <= boxMaxX && maxX >= boxMinX && minY <= boxMaxY && maxY >= boxMinY;
}

bool SpatialGrid::ItemBounds::isValid() const
{
    return !std::isnan(minX) && !std::isnan(minY) && !std::isnan(maxX) && !std::isnan(maxY);
}

int SpatialGrid::cellIndex(const double value, const double min, const double cellSize, const int count)
{
    return static_cast<int>(std::clamp((value - min) / cellSize, 0.0, static_cast<double>(count - 1)));
}

template <typename T>
int SpatialGrid::scanTopmostFrom(const T* data, const int itemCount, const int valuesPerItem, const ItemBounds& box, const std::function<bool(int)>& accept)
{
    if (!data || itemCount <= 0 || !box.isValid()) {
        return -1;
    }
    for (auto i = itemCount - 1; i >= 0; --i) {
        const auto* item = data + static_cast<ptrdiff_t>(i) * valuesPerItem;
        // Combined without short-circuiting: each comparison alone fails for about half the
        // rectangles and would mispredict.
        const auto overlaps = (std::min(item[0], item[2]) <= box.maxX) & (std::max(item[0], item[2]) >= box.minX) & (std::min(item[1], item[3]) <= box.maxY)
            & (std::max(item[1], item[3]) >= box.minY);
        if (!overlaps) {
            continue;
        }
        // std::min/max can drop a NaN operand, so invalid items are rejected explicitly.
        if (std::isnan(item[0]) || std::isnan(item[1]) || std::isnan(item[2]) || std::isnan(item[3])) {
            continue;
        }
        if (accept(i)) {
            return i;
        }
    }
    return -1;
}

template <typename T> void SpatialGrid::buildFrom(const T* data, const int itemCount, const int valuesPerItem)
{
    cells_.clear();
    largeItems_.clear();
    itemBounds_.clear();
    cols_ = 0;
    rows_ = 0;
    if (itemCount <= 0 || !data) {
        return;
    }
    computeDataBounds(data, itemCount, valuesPerItem);
    computeGridDimensions(itemCount);
    fillSpatialGrid(itemCount);
}

template <typename T> void SpatialGrid::computeDataBounds(const T* data, const int itemCount, const int valuesPerItem)
{
    constexpr auto kInf = std::numeric_limits<double>::infinity();
    constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();
    auto minX = kInf;
    auto minY = kInf;
    auto maxX = -kInf;
    auto maxY = -kInf;
    itemBounds_.reserve(static_cast<size_t>(itemCount));
    for (auto i = 0; i < itemCount; ++i) {
        const auto* item = data + static_cast<ptrdiff_t>(i) * valuesPerItem;
        // std::min/max can drop a NaN operand, so invalid items are marked explicitly.
        if (std::isnan(item[0]) || std::isnan(item[1]) || std::isnan(item[2]) || std::isnan(item[3])) {
            itemBounds_.push_back({kNaN, kNaN, kNaN, kNaN});
            continue;
        }
        const auto bounds = ItemBounds{
            std::min(item[0], item[2]),
            std::min(item[1], item[3]),
            std::max(item[0], item[2]),
            std::max(item[1], item[3]),
        };
        itemBounds_.push_back(bounds);

        for (const auto x : {bounds.minX, bounds.maxX}) {
            if (std::isfinite(x)) {
                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
            }
        }
        for (const auto y : {bounds.minY, bounds.maxY}) {
            if (std::isfinite(y)) {
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
            }
        }
    }

    boundedX_ = minX <= maxX;
    boundedY_ = minY <= maxY;
    minX_ = boundedX_ ? minX : 0.0;
    minY_ = boundedY_ ? minY : 0.0;
    maxX_ = boundedX_ ? maxX : 0.0;
    maxY_ = boundedY_ ? maxY : 0.0;

    // Add small padding to avoid zero-size ranges
    if (maxX_ <= minX_) {
        maxX_ = minX_ + 1.0;
    }
    if (maxY_ <= minY_) {
        maxY_ = minY_ + 1.0;
    }
}

void SpatialGrid::computeGridDimensions(const int itemCount)
{
    // Grid dimensions: sqrt(N), capped at kMaxGridDimension
    constexpr auto kMaxGridDimension = int{1024}; // Upper cap on the number of rows/columns to bound memory and iteration cost.
    const auto dim = std::max(1, std::min(kMaxGridDimension, static_cast<int>(std::sqrt(static_cast<float>(itemCount)))));
    // An axis without finite edges has nothing to subdivide, so the other axis gets the whole cell budget.
    // Without this, full-height spans would each cover a whole column and fall back to the linear large-item scan.
    cols_ = boundedX_ ? (boundedY_ ? dim : dim * dim) : 1;
    rows_ = boundedY_ ? (boundedX_ ? dim : dim * dim) : 1;
    cellW_ = (maxX_ - minX_) / static_cast<double>(cols_);
    cellH_ = (maxY_ - minY_) / static_cast<double>(rows_);

    cells_.resize(static_cast<size_t>(cols_) * static_cast<size_t>(rows_));
}

void SpatialGrid::fillSpatialGrid(const int itemCount)
{
    // Insert each rect into all overlapping cells
    for (auto i = 0; i < itemCount; ++i) {
        const auto& bounds = itemBounds_[static_cast<size_t>(i)];
        if (!bounds.isValid()) {
            continue;
        }

        const auto c0 = cellIndex(bounds.minX, minX_, cellW_, cols_);
        const auto r0 = cellIndex(bounds.minY, minY_, cellH_, rows_);
        const auto c1 = cellIndex(bounds.maxX, minX_, cellW_, cols_);
        const auto r1 = cellIndex(bounds.maxY, minY_, cellH_, rows_);

        constexpr auto kMaxCellsPerItem = 64;
        if ((c1 - c0 + 1) * (r1 - r0 + 1) > kMaxCellsPerItem) {
            largeItems_.push_back(i);
            continue;
        }

        for (auto r = r0; r <= r1; ++r) {
            for (auto c = c0; c <= c1; ++c) {
                cells_[static_cast<size_t>(r) * static_cast<size_t>(cols_) + static_cast<size_t>(c)].push_back(i);
            }
        }
    }
}

} // namespace QAccelPlot
