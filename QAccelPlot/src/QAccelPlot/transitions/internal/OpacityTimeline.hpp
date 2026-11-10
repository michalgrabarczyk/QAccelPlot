//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QEasingCurve>

#include <optional>
#include <vector>

namespace QAccelPlot::Internal {

/// \brief One animation of item opacity, on the timeline of an item.
struct OpacitySegment {
    double start{0.0};          ///< \brief Start time in milliseconds after the item starts.
    double duration{0.0};       ///< \brief Duration in milliseconds; 0 sets \c to at once.
    std::optional<double> from; ///< \brief Opacity at the start; without it, the opacity reached so far.
    double to{1.0};             ///< \brief Opacity at the end.
    QEasingCurve easing;        ///< \brief Easing from \c from to \c to.
};

/// \brief The opacity of an item over the time since it started, built from opacity animations.
///
/// At a given time, the segment that started last applies; segments that start together apply
/// in their order. Before the first segment, the item shows that segment's start opacity, and
/// an item without segments is opaque.
class OpacityTimeline {
public:
    /// \brief Constructs a timeline without segments.
    OpacityTimeline() = default;
    /// \brief Constructs the timeline of \a segments, in declaration order, lasting \a length milliseconds.
    ///
    /// \a length can exceed the end of the last segment, for example after a pause.
    OpacityTimeline(std::vector<OpacitySegment> segments, double length);

    /// \brief Returns \c true when no segment animates opacity.
    [[nodiscard]] bool empty() const noexcept;
    /// \brief Returns the duration of the timeline in milliseconds.
    [[nodiscard]] double length() const noexcept;
    /// \brief Returns the opacity \a time milliseconds after the item started, not clamped.
    [[nodiscard]] double valueAt(double time) const;

private:
    /// \brief A segment with its start opacity resolved.
    struct Resolved {
        double start;
        double duration;
        double from;
        double to;
        QEasingCurve easing;
    };

    [[nodiscard]] double valueAt(double time, std::size_t segmentCount) const;

    std::vector<Resolved> segments_;
    double length_{0.0};
};

} // namespace QAccelPlot::Internal
