//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/series/SpatialGrid.hpp"

#include <QPointF>
#include <QSizeF>

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <limits>
#include <optional>
#include <vector>

namespace QAccelPlot {
class Axis;
}

namespace QAccelPlot::Internal {

/// \brief Finds the drawn record nearest to a pixel position, for series that draw one record per data point.
///
/// A spatial grid of data-space rectangles narrows the candidates, and a pixel distance test picks among them.
/// The rectangles are rebuilt lazily after \c invalidate().
class RecordHitIndex {
public:
    /// \brief A record lookup around a pixel position.
    struct Query {
        QPointF position;  ///< \brief Item position to look around.
        qreal reach{0.0};  ///< \brief Pixels around \c position that a candidate's rectangle must reach.
        qreal radius{0.0}; ///< \brief Largest pixel distance of the found record.
        int drawnCount{0}; ///< \brief Records drawn; those at or above it are not found.
    };

    /// \brief Returns the pixel distance from the query position to record \a index, or nullopt when it is not drawn.
    using Distance = std::function<std::optional<qreal>(int index)>;

    /// \brief Marks the rectangles out of date.
    void invalidate();
    /// \brief Returns \c true while the rectangles are up to date.
    bool valid() const;

    /// \brief Rebuilds the rectangles of \a count records from \a rectAt, which returns record \a i's (minX, minY, maxX, maxY) or nullopt.
    template <typename RectAt> void build(const int count, RectAt&& rectAt)
    {
        constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();
        rects_.resize(static_cast<std::size_t>(count) * 4);
        for (auto i = 0; i < count; ++i) {
            const auto rect = rectAt(i).value_or(std::array<double, 4>{kNaN, kNaN, kNaN, kNaN});
            std::copy(rect.cbegin(), rect.cend(), rects_.begin() + static_cast<std::ptrdiff_t>(i) * 4);
        }
        grid_.build(rects_.data(), count);
        valid_ = true;
    }

    /// \brief Returns the record of \a query nearest to its position, or -1; on a tie, the one with the highest index.
    ///
    /// \a xAxis and \a yAxis map the position of an item of \a size to data coordinates.
    int nearest(const Query& query, const Axis& xAxis, const Axis& yAxis, const QSizeF& size, const Distance& distance) const;

private:
    std::vector<double> rects_;
    SpatialGrid grid_;
    bool valid_{false};
};

} // namespace QAccelPlot::Internal
