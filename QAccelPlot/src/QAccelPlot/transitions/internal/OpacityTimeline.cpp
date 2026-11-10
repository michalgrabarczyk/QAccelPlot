//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/transitions/internal/OpacityTimeline.hpp"

#include <algorithm>
#include <utility>

namespace QAccelPlot::Internal {

OpacityTimeline::OpacityTimeline(std::vector<OpacitySegment> segments, const double length)
{
    std::stable_sort(segments.begin(), segments.end(), [](const OpacitySegment& left, const OpacitySegment& right) { return left.start < right.start; });
    segments_.reserve(segments.size());
    auto end = 0.0;
    for (auto& segment : segments) {
        const auto start = std::max(segment.start, 0.0);
        const auto duration = std::max(segment.duration, 0.0);
        const auto from = segment.from ? *segment.from : valueAt(start, segments_.size());
        segments_.push_back(Resolved{start, duration, from, segment.to, std::move(segment.easing)});
        end = std::max(end, start + duration);
    }
    length_ = std::max(length, end);
}

bool OpacityTimeline::empty() const noexcept
{
    return segments_.empty();
}

double OpacityTimeline::length() const noexcept
{
    return length_;
}

double OpacityTimeline::valueAt(const double time) const
{
    return valueAt(time, segments_.size());
}

double OpacityTimeline::valueAt(const double time, const std::size_t segmentCount) const
{
    if (segmentCount == 0) {
        return 1.0;
    }
    const auto begin = segments_.cbegin();
    const auto last = std::upper_bound(
        begin, begin + static_cast<std::ptrdiff_t>(segmentCount), time, [](const double value, const Resolved& segment) { return value < segment.start; });
    if (last == begin) {
        return segments_.front().from;
    }
    const auto& segment = *(last - 1);
    if (segment.duration <= 0.0 || time >= segment.start + segment.duration) {
        return segment.to;
    }
    const auto progress = (time - segment.start) / segment.duration;
    return segment.from + (segment.to - segment.from) * segment.easing.valueForProgress(progress);
}

} // namespace QAccelPlot::Internal
