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
#include <utility>
#include <vector>

namespace QAccelPlot {
class Axis;
}

namespace QAccelPlot::Internal {

/// \brief Finds the drawn record nearest to a pixel position, for series that draw one record per data point.
///
/// A spatial grid of the records' data spans along their position axis narrows the candidates, and a pixel
/// distance test picks among them. The spans are rebuilt lazily after \c invalidate().
class RecordHitIndex {
public:
    /// \brief A record lookup around a pixel position.
    struct Query {
        QPointF position;    ///< \brief Item position to look around.
        qreal radius{0.0};   ///< \brief Largest pixel distance of the found record.
        qreal overhang{0.0}; ///< \brief Pixels a drawn record reaches past its span, such as half its minimum width.
        int drawnCount{0};   ///< \brief Records drawn; those at or above it are not found.
    };

    /// \brief Returns the pixel distance from the query position to record \a index, or nullopt when it is not drawn.
    using Distance = std::function<std::optional<qreal>(int index)>;

    /// \brief Marks the spans out of date.
    void invalidate();
    /// \brief Returns \c true while the spans are up to date.
    bool valid() const;

    /// \brief Rebuilds the spans of \a count records along \a positionAxis from \a spanAt, which returns record \a i's (min, max) or nullopt.
    ///
    /// Only the position axis is indexed. Records such as candles often reach across much of the other
    /// axis, which would push them past the grid's per-rectangle cell limit into its linear scan; the
    /// pixel distance test covers that axis instead.
    template <typename SpanAt> void build(const int count, const Qt::Orientation positionAxis, SpanAt&& spanAt)
    {
        constexpr auto kInf = std::numeric_limits<double>::infinity();
        constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();
        rects_.resize(static_cast<std::size_t>(count) * 4);
        for (auto i = 0; i < count; ++i) {
            const auto span = std::optional<std::pair<double, double>>{spanAt(i)};
            auto rect = std::array<double, 4>{kNaN, kNaN, kNaN, kNaN};
            if (span && positionAxis == Qt::Horizontal) {
                rect = {span->first, -kInf, span->second, kInf};
            } else if (span) {
                rect = {-kInf, span->first, kInf, span->second};
            }
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
