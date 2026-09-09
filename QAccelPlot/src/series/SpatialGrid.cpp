//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "series/SpatialGrid.hpp"

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

void SpatialGrid::build(const float* data, const int itemCount, const int floatsPerItem)
{
    cells_.clear();
    itemBounds_.clear();
    cols_ = 0;
    rows_ = 0;

    if (itemCount <= 0 || !data) {
        return;
    }

    computeDataBounds(data, itemCount, floatsPerItem);
    computeGridDimensions(itemCount);
    fillSpatialGrid(itemCount);
}

int SpatialGrid::query(const float x, const float y) const
{
    if (cols_ <= 0 || rows_ <= 0) {
        return -1;
    }
    if (x < minX_ || x > maxX_ || y < minY_ || y > maxY_) {
        return -1;
    }

    const auto c = std::max(0, std::min(cols_ - 1, static_cast<int>((x - minX_) / cellW_)));
    const auto r = std::max(0, std::min(rows_ - 1, static_cast<int>((y - minY_) / cellH_)));
    const auto& cell = cells_[static_cast<size_t>(r) * static_cast<size_t>(cols_) + static_cast<size_t>(c)];

    // Return the last (topmost) item that actually contains the point.
    for (auto it = cell.rbegin(); it != cell.rend(); ++it) {
        const auto& bounds = itemBounds_[static_cast<size_t>(*it)];
        if (bounds.contains(x, y)) {
            return *it;
        }
    }

    return -1;
}

bool SpatialGrid::ItemBounds::contains(const float x, const float y) const
{
    return x >= minX && x <= maxX && y >= minY && y <= maxY;
}

void SpatialGrid::computeDataBounds(const float* data, const int itemCount, const int floatsPerItem)
{
    itemBounds_.reserve(static_cast<size_t>(itemCount));
    for (auto i = 0; i < itemCount; ++i) {
        const auto base = i * floatsPerItem;
        const auto bounds = ItemBounds{
            std::min(data[base], data[base + 2]),
            std::min(data[base + 1], data[base + 3]),
            std::max(data[base], data[base + 2]),
            std::max(data[base + 1], data[base + 3]),
        };
        itemBounds_.push_back(bounds);

        if (i == 0) {
            minX_ = bounds.minX;
            minY_ = bounds.minY;
            maxX_ = bounds.maxX;
            maxY_ = bounds.maxY;
        } else {
            minX_ = std::min(minX_, bounds.minX);
            minY_ = std::min(minY_, bounds.minY);
            maxX_ = std::max(maxX_, bounds.maxX);
            maxY_ = std::max(maxY_, bounds.maxY);
        }
    }

    // Add small padding to avoid zero-size ranges
    if (maxX_ <= minX_) {
        maxX_ = minX_ + 1.0f;
    }
    if (maxY_ <= minY_) {
        maxY_ = minY_ + 1.0f;
    }
}

void SpatialGrid::computeGridDimensions(const int itemCount)
{
    // Grid dimensions: sqrt(N), capped at kMaxGridDimension
    constexpr auto kMaxGridDimension = int{1024}; // Upper cap on the number of rows/columns to bound memory and iteration cost.
    const auto dim = std::max(1, std::min(kMaxGridDimension, static_cast<int>(std::sqrt(static_cast<float>(itemCount)))));
    cols_ = dim;
    rows_ = dim;
    cellW_ = (maxX_ - minX_) / static_cast<float>(cols_);
    cellH_ = (maxY_ - minY_) / static_cast<float>(rows_);

    cells_.resize(static_cast<size_t>(cols_) * static_cast<size_t>(rows_));
}

void SpatialGrid::fillSpatialGrid(const int itemCount)
{
    // Insert each rect into all overlapping cells
    for (auto i = 0; i < itemCount; ++i) {
        const auto& bounds = itemBounds_[static_cast<size_t>(i)];

        const auto c0 = std::max(0, std::min(cols_ - 1, static_cast<int>((bounds.minX - minX_) / cellW_)));
        const auto r0 = std::max(0, std::min(rows_ - 1, static_cast<int>((bounds.minY - minY_) / cellH_)));
        const auto c1 = std::max(0, std::min(cols_ - 1, static_cast<int>((bounds.maxX - minX_) / cellW_)));
        const auto r1 = std::max(0, std::min(rows_ - 1, static_cast<int>((bounds.maxY - minY_) / cellH_)));

        for (auto r = r0; r <= r1; ++r) {
            for (auto c = c0; c <= c1; ++c) {
                cells_[static_cast<size_t>(r) * static_cast<size_t>(cols_) + static_cast<size_t>(c)].push_back(i);
            }
        }
    }
}

} // namespace QAccelPlot
