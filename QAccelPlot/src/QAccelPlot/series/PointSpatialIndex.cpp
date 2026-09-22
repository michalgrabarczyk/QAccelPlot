//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/PointSpatialIndex.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace QAccelPlot {

namespace {
// Average number of points per occupied cell the grid aims for.
constexpr auto kTargetPointsPerCell = 4.0;
// Upper bound for grid columns and rows; keeps offset arrays small for degenerate inputs.
constexpr auto kMaxGridDimension = 4096;
// Extent used when all points share one coordinate, so the grid never has zero size.
constexpr auto kMinimumExtent = 1e-6;
} // namespace

void PointSpatialIndex::build(const float* data, const int pointCount, const int stride)
{
    build(data, pointCount, stride, Mapping{});
}

void PointSpatialIndex::build(const float* data, const int pointCount, const int stride, const Mapping mapping)
{
    clear();
    mapping_ = mapping;
    if (!data || pointCount <= 0 || stride < 2) {
        return;
    }

    collectValidPoints(data, pointCount, stride);
    if (points_.empty()) {
        return;
    }
    computeGridDimensions();
    fillCells();
}

void PointSpatialIndex::clear()
{
    points_.clear();
    cellStart_.clear();
    cellEntries_.clear();
    columns_ = 0;
    rows_ = 0;
}

bool PointSpatialIndex::isEmpty() const
{
    return points_.empty();
}

int PointSpatialIndex::validPointCount() const
{
    return static_cast<int>(points_.size());
}

PointSpatialIndex::Mapping PointSpatialIndex::mapping() const
{
    return mapping_;
}

int PointSpatialIndex::nearest(const double x, const double y, const double radiusX, const double radiusY) const
{
    if (points_.empty() || !(radiusX > 0.0) || !(radiusY > 0.0) || !std::isfinite(x) || !std::isfinite(y)) {
        return -1;
    }

    const auto firstColumn = cellColumn(x - radiusX);
    const auto lastColumn = cellColumn(x + radiusX);
    const auto firstRow = cellRow(y - radiusY);
    const auto lastRow = cellRow(y + radiusY);

    auto bestIndex = -1;
    auto bestDistance = std::numeric_limits<double>::max();
    for (auto row = firstRow; row <= lastRow; ++row) {
        for (auto column = firstColumn; column <= lastColumn; ++column) {
            const auto cell = static_cast<std::size_t>(row) * static_cast<std::size_t>(columns_) + static_cast<std::size_t>(column);
            for (auto entry = cellStart_[cell]; entry < cellStart_[cell + 1]; ++entry) {
                const auto& point = points_[static_cast<std::size_t>(cellEntries_[static_cast<std::size_t>(entry)])];
                const auto dx = (static_cast<double>(point.x) - x) / radiusX;
                const auto dy = (static_cast<double>(point.y) - y) / radiusY;
                const auto distance = dx * dx + dy * dy;
                if (distance > 1.0) {
                    continue;
                }
                if (distance < bestDistance || (distance == bestDistance && point.sourceIndex > bestIndex)) {
                    bestDistance = distance;
                    bestIndex = point.sourceIndex;
                }
            }
        }
    }
    return bestIndex;
}

bool PointSpatialIndex::mapCoordinate(const double value, const bool logarithmic, double& mapped)
{
    if (!std::isfinite(value)) {
        return false;
    }
    if (!logarithmic) {
        mapped = value;
        return true;
    }
    if (value <= 0.0) {
        return false;
    }
    mapped = std::log10(value);
    return true;
}

void PointSpatialIndex::collectValidPoints(const float* data, const int pointCount, const int stride)
{
    points_.reserve(static_cast<std::size_t>(pointCount));
    for (auto index = 0; index < pointCount; ++index) {
        const auto base = static_cast<std::size_t>(index) * static_cast<std::size_t>(stride);
        auto mappedX = 0.0;
        auto mappedY = 0.0;
        if (!mapCoordinate(data[base], mapping_.logX, mappedX) || !mapCoordinate(data[base + 1], mapping_.logY, mappedY)) {
            continue;
        }
        points_.push_back({static_cast<float>(mappedX), static_cast<float>(mappedY), index});
    }
}

void PointSpatialIndex::computeGridDimensions()
{
    auto maxX = std::numeric_limits<double>::lowest();
    auto maxY = std::numeric_limits<double>::lowest();
    minX_ = std::numeric_limits<double>::max();
    minY_ = std::numeric_limits<double>::max();
    for (const auto& point : points_) {
        minX_ = std::min(minX_, static_cast<double>(point.x));
        minY_ = std::min(minY_, static_cast<double>(point.y));
        maxX = std::max(maxX, static_cast<double>(point.x));
        maxY = std::max(maxY, static_cast<double>(point.y));
    }

    const auto extentX = std::max(maxX - minX_, kMinimumExtent);
    const auto extentY = std::max(maxY - minY_, kMinimumExtent);
    const auto cellCount = std::max(1.0, static_cast<double>(points_.size()) / kTargetPointsPerCell);
    // Square-ish cells in mapped space: columns / rows follows the aspect ratio of the extent.
    const auto aspect = extentX / extentY;
    columns_ = std::clamp(static_cast<int>(std::ceil(std::sqrt(cellCount * aspect))), 1, kMaxGridDimension);
    rows_ = std::clamp(static_cast<int>(std::ceil(cellCount / columns_)), 1, kMaxGridDimension);
    cellWidth_ = extentX / columns_;
    cellHeight_ = extentY / rows_;
}

void PointSpatialIndex::fillCells()
{
    const auto cellCount = static_cast<std::size_t>(columns_) * static_cast<std::size_t>(rows_);
    auto pointCells = std::vector<int>(points_.size());
    cellStart_.assign(cellCount + 1, 0);

    // Counting sort: count per cell, prefix-sum into offsets, then scatter.
    for (auto position = std::size_t{0}; position < points_.size(); ++position) {
        const auto cell = cellRow(points_[position].y) * columns_ + cellColumn(points_[position].x);
        pointCells[position] = cell;
        ++cellStart_[static_cast<std::size_t>(cell) + 1];
    }
    for (auto cell = std::size_t{0}; cell < cellCount; ++cell) {
        cellStart_[cell + 1] += cellStart_[cell];
    }

    cellEntries_.resize(points_.size());
    auto cursor = std::vector<int>(cellStart_.begin(), cellStart_.end() - 1);
    for (auto position = std::size_t{0}; position < points_.size(); ++position) {
        auto& slot = cursor[static_cast<std::size_t>(pointCells[position])];
        cellEntries_[static_cast<std::size_t>(slot)] = static_cast<int>(position);
        ++slot;
    }
}

int PointSpatialIndex::cellColumn(const double x) const
{
    const auto column = std::floor((x - minX_) / cellWidth_);
    return static_cast<int>(std::clamp(column, 0.0, static_cast<double>(columns_ - 1)));
}

int PointSpatialIndex::cellRow(const double y) const
{
    const auto row = std::floor((y - minY_) / cellHeight_);
    return static_cast<int>(std::clamp(row, 0.0, static_cast<double>(rows_ - 1)));
}

} // namespace QAccelPlot
