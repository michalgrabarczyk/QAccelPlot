//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/inspection/internal/InspectionIndex.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>

namespace QAccelPlot {
namespace {

constexpr auto kLeafSize = int{32};

bool intersects(const InspectionBounds& a, const InspectionBounds& b)
{
    return a.xMin <= b.xMax && a.xMax >= b.xMin && a.yMin <= b.yMax && a.yMax >= b.yMin;
}

bool containsBounds(const InspectionBounds& a, const InspectionBounds& b)
{
    return a.xMin <= b.xMin && a.xMax >= b.xMax && a.yMin <= b.yMin && a.yMax >= b.yMax;
}

} // namespace

bool InspectionIndex::build(std::vector<Point>&& points, const bool logX, const bool logY, const std::atomic_bool& cancelled)
{
    logX_ = logX;
    logY_ = logY;
    points_ = std::move(points);
    nodes_.clear();
    xOrder_.clear();
    if (points_.empty()) {
        return !cancelled.load();
    }
    auto leaves = std::size_t{1};
    while ((points_.size() + leaves - 1) / leaves > static_cast<std::size_t>(kLeafSize)) {
        leaves *= 2;
    }
    nodes_.reserve(leaves * 2 - 1);
    buildNodes(cancelled);
    if (cancelled.load()) {
        return false;
    }
    xOrder_.resize(points_.size());
    std::iota(xOrder_.begin(), xOrder_.end(), 0);
    std::sort(xOrder_.begin(), xOrder_.end(), [this](const int a, const int b) {
        const auto& p = points_[static_cast<std::size_t>(a)];
        const auto& q = points_[static_cast<std::size_t>(b)];
        return p.x < q.x || (p.x == q.x && p.index < q.index);
    });
    return !cancelled.load();
}

std::size_t InspectionIndex::storageBytes() const
{
    return sizeof(*this) + points_.capacity() * sizeof(Point) + xOrder_.capacity() * sizeof(int) + nodes_.capacity() * sizeof(Node);
}

InspectionHit InspectionIndex::nearestX(const InspectionMetric& metric, const double pixelX, const double radius) const
{
    auto best = InspectionHit{-1, radius};
    const auto x = metric.coordX(pixelX);
    const auto it = std::lower_bound(
        xOrder_.begin(), xOrder_.end(), x, [this](const int i, const double value) { return points_[static_cast<std::size_t>(i)].x < value; });
    const auto consider = [this, &metric, &best, pixelX](const int i) {
        const auto& point = points_[static_cast<std::size_t>(i)];
        best.consider(point.index, std::abs(metric.pixelX(point.x) - pixelX));
    };
    if (it != xOrder_.begin()) {
        consider(*std::prev(it));
    }
    if (it != xOrder_.end()) {
        // Among samples sharing this X, the highest index is the one drawn last.
        const auto sharedX = points_[static_cast<std::size_t>(*it)].x;
        const auto end
            = std::upper_bound(it, xOrder_.end(), sharedX, [this](const double value, const int i) { return value < points_[static_cast<std::size_t>(i)].x; });
        consider(*std::prev(end));
    }
    return best;
}

InspectionHit InspectionIndex::nearest(const InspectionMetric& metric, const QPointF& position, const double radius) const
{
    auto best = InspectionHit{-1, radius};
    if (nodes_.empty()) {
        return best;
    }
    auto pending = std::array<int, 32>{};
    auto size = 1;
    pending[0] = 0;
    while (size > 0) {
        const auto& node = nodes_[static_cast<std::size_t>(pending[static_cast<std::size_t>(--size)])];
        const auto distance = distanceToBounds(node.bounds, metric, position);
        if (distance > best.distance || (distance == best.distance && node.maxIndex <= best.index)) {
            continue;
        }
        if (node.left >= 0) {
            const auto& left = nodes_[static_cast<std::size_t>(node.left)];
            const auto& right = nodes_[static_cast<std::size_t>(node.right)];
            const auto a = distanceToBounds(left.bounds, metric, position);
            const auto b = distanceToBounds(right.bounds, metric, position);
            const auto leftFirst = a < b || (a == b && left.maxIndex > right.maxIndex);
            pending[static_cast<std::size_t>(size++)] = leftFirst ? node.right : node.left;
            pending[static_cast<std::size_t>(size++)] = leftFirst ? node.left : node.right;
            continue;
        }
        for (auto i = node.first; i < node.last; ++i) {
            const auto& point = points_[static_cast<std::size_t>(i)];
            best.consider(point.index, std::hypot(metric.pixelX(point.x) - position.x(), metric.pixelY(point.y) - position.y()));
        }
    }
    return best;
}

InspectionNeighbors InspectionIndex::neighbors(const double x) const
{
    // xOrder_ sorts equal X by index, so the entries around the pivot are the highest index at or
    // before x and the lowest index after it.
    const auto it = std::upper_bound(
        xOrder_.begin(), xOrder_.end(), x, [this](const double value, const int i) { return value < points_[static_cast<std::size_t>(i)].x; });
    auto result = InspectionNeighbors{};
    if (it != xOrder_.begin()) {
        result.left = points_[static_cast<std::size_t>(*std::prev(it))].index;
    }
    if (it != xOrder_.end()) {
        result.right = points_[static_cast<std::size_t>(*it)].index;
    }
    return result;
}

SummaryAccumulator InspectionIndex::summarize(const InspectionBounds& bounds) const
{
    auto summary = SummaryAccumulator{};
    if (nodes_.empty()) {
        return summary;
    }
    auto pending = std::array<int, 32>{};
    auto size = 1;
    pending[0] = 0;
    while (size > 0) {
        const auto& node = nodes_[static_cast<std::size_t>(pending[static_cast<std::size_t>(--size)])];
        if (!intersects(bounds, node.bounds)) {
            continue;
        }
        if (containsBounds(bounds, node.bounds)) {
            summary.merge(node.summary);
            continue;
        }
        if (node.left >= 0) {
            pending[static_cast<std::size_t>(size++)] = node.right;
            pending[static_cast<std::size_t>(size++)] = node.left;
            continue;
        }
        for (auto i = node.first; i < node.last; ++i) {
            const auto& point = points_[static_cast<std::size_t>(i)];
            if (bounds.contains(point.x, point.y)) {
                summary.add(point.y, point.index);
            }
        }
    }
    return summary;
}

void InspectionIndex::collect(const InspectionBounds& bounds, int offset, const int limit, QList<int>& indices) const
{
    if (nodes_.empty()) {
        return;
    }
    auto pending = std::array<int, 32>{};
    auto size = 1;
    pending[0] = 0;
    while (size > 0 && indices.size() < limit) {
        const auto& node = nodes_[static_cast<std::size_t>(pending[static_cast<std::size_t>(--size)])];
        if (!intersects(bounds, node.bounds)) {
            continue;
        }
        if (containsBounds(bounds, node.bounds) && offset >= node.last - node.first) {
            offset -= node.last - node.first;
            continue;
        }
        if (node.left >= 0) {
            pending[static_cast<std::size_t>(size++)] = node.right;
            pending[static_cast<std::size_t>(size++)] = node.left;
            continue;
        }
        for (auto i = node.first; i < node.last && indices.size() < limit; ++i) {
            const auto& point = points_[static_cast<std::size_t>(i)];
            if (!bounds.contains(point.x, point.y)) {
                continue;
            }
            if (offset > 0) {
                --offset;
            } else {
                indices.push_back(point.index);
            }
        }
    }
}

void InspectionIndex::buildNodes(const std::atomic_bool& cancelled)
{
    struct Task {
        int first;
        int last;
        int parent;
        bool right;
    };

    // Median splits with leaves of kLeafSize bound the pending stack to 27 for int-sized input.
    auto pending = std::array<Task, 32>{};
    auto size = 1;
    pending[0] = {0, static_cast<int>(points_.size()), -1, false};
    while (size > 0 && !cancelled.load()) {
        const auto task = pending[static_cast<std::size_t>(--size)];
        const auto index = createNode(task.first, task.last, cancelled);
        if (task.parent >= 0) {
            auto& parent = nodes_[static_cast<std::size_t>(task.parent)];
            (task.right ? parent.right : parent.left) = index;
        }
        if (task.last - task.first > kLeafSize && !cancelled.load()) {
            const auto middle = task.first + (task.last - task.first) / 2;
            pending[static_cast<std::size_t>(size++)] = {middle, task.last, index, true};
            pending[static_cast<std::size_t>(size++)] = {task.first, middle, index, false};
        }
    }
}

int InspectionIndex::createNode(const int first, const int last, const std::atomic_bool& cancelled)
{
    const auto& initial = points_[static_cast<std::size_t>(first)];
    auto node = Node{{initial.x, initial.x, initial.y, initial.y}, {}, first, last};
    for (auto i = first; i < last; ++i) {
        if ((i - first) % 4096 == 0 && cancelled.load()) {
            return -1;
        }
        const auto& point = points_[static_cast<std::size_t>(i)];
        node.bounds.xMin = std::min(node.bounds.xMin, point.x);
        node.bounds.xMax = std::max(node.bounds.xMax, point.x);
        node.bounds.yMin = std::min(node.bounds.yMin, point.y);
        node.bounds.yMax = std::max(node.bounds.yMax, point.y);
        node.maxIndex = std::max(node.maxIndex, point.index);
        node.summary.add(point.y, point.index);
    }
    const auto index = static_cast<int>(nodes_.size());
    nodes_.push_back(node);
    if (last - first > kLeafSize) {
        const auto middle = first + (last - first) / 2;
        const auto splitX = mappedX(node.bounds.xMax) - mappedX(node.bounds.xMin) >= mappedY(node.bounds.yMax) - mappedY(node.bounds.yMin);
        std::nth_element(points_.begin() + first, points_.begin() + middle, points_.begin() + last, [splitX](const Point& a, const Point& b) {
            const auto p = splitX ? a.x : a.y;
            const auto q = splitX ? b.x : b.y;
            return p < q || (p == q && a.index < b.index);
        });
    }
    return index;
}

double InspectionIndex::distanceToBounds(const InspectionBounds& bounds, const InspectionMetric& metric, const QPointF& position) const
{
    const auto x = distanceToInterval(position.x(), metric.pixelX(bounds.xMin), metric.pixelX(bounds.xMax));
    const auto y = distanceToInterval(position.y(), metric.pixelY(bounds.yMin), metric.pixelY(bounds.yMax));
    return std::hypot(x, y);
}

double InspectionIndex::mappedX(const double value) const
{
    return logX_ ? std::log10(value) : value;
}

double InspectionIndex::mappedY(const double value) const
{
    return logY_ ? std::log10(value) : value;
}

} // namespace QAccelPlot
