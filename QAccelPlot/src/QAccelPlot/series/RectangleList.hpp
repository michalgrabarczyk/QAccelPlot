//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/series/PlotSeries.hpp"
#include "QAccelPlot/series/SpatialGrid.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <vector>

namespace QAccelPlot {

/// \brief A hardware-accelerated QML item that renders a large list of axis-aligned rectangles.
///
/// All rectangles are stored as interleaved floats (x1, y1, x2, y2) and uploaded to the GPU
/// as a data texture, making it suitable for tens of thousands of rectangles.
/// Hover detection uses an internal \c SpatialGrid for O(1) hit tests.
///
/// An infinite edge extends the rectangle to the plot edge, e.g. \c y1 = -Infinity and
/// \c y2 = +Infinity for a full-height span. Infinite edges don't affect the axes' data ranges.
/// Rectangles with a NaN edge are not drawn or hovered.
///
/// \sa LineCurve, Axis
class RectangleList : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(RectangleList)

    /// \brief Fill color applied to all rectangles. Default: \c Colors.dark.seriesPrimary with alpha 50.
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    /// \brief Read-only: number of rectangles currently loaded.
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    /// \brief Read-only: index of the rectangle under the cursor, or -1 when none.
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)

public:
    /// \brief Constructs a RectangleList with the given \a parent.
    explicit RectangleList(QQuickItem* parent = nullptr);

    /// \brief Returns the rectangle fill color.
    QColor color() const;
    /// \brief Sets the fill color to \a color.
    void setColor(const QColor& color);

    /// \brief Returns the number of rectangles currently loaded.
    int count() const;
    /// \brief Returns the index of the hovered rectangle, or -1 if none.
    int hoveredIndex() const;

    /// \brief Loads rectangles from \a rects, a QML list of objects with \c x1, \c y1, \c x2, \c y2 properties.
    ///
    /// A missing or null \c x1 / \c y1 is -Infinity and a missing \c x2 / \c y2 is +Infinity, so
    /// <tt>{ x1: 8, x2: 12 }</tt> is a full-height span.
    Q_INVOKABLE void setData(const QVariantList& rects);

    /// \brief Loads rectangles from a C++ raw float array (\a data must have \a rectCount × 4 floats: x1, y1, x2, y2).
    void setData(const float* data, int rectCount);

    /// \brief Loads rectangles from a C++ raw double array, preserving full precision for large
    /// coordinates (e.g. modern Unix-epoch timestamps). \a data must have \a rectCount × 4 doubles.
    void setData(const double* data, int rectCount);

    /// \brief Moves \a data (\a rectCount × 4 doubles: x1, y1, x2, y2) into the list. No copy is made.
    void setData(std::vector<double>&& data, int rectCount);

    /// \brief Thread-safe: queues \c setData(\a data, \a rectCount) to the item's thread.
    void postData(std::vector<double>&& data, int rectCount);

    /// \brief Removes all rectangles.
    Q_INVOKABLE void clearData();

    /// \brief Returns rectangle \a index as an object with \c x1, \c y1, \c x2, \c y2 properties.
    ///
    /// Returns an empty object when \a index is out of range.
    Q_INVOKABLE QVariantMap rectangleAt(int index) const;

    /// \brief Returns the index of the topmost rectangle under item position \a position, or -1.
    int rectangleIndexAt(const QPointF& position) const;
    /// \brief Returns \c true when a rectangle lies under item position \a point.
    ///
    /// Hover delivery uses this test, so stacked series underneath still receive hover events
    /// outside this list's rectangles.
    bool contains(const QPointF& point) const override;

signals:
    /// \brief Emitted when the color property changes.
    void colorChanged();
    /// \brief Emitted when the rectangle count changes.
    void countChanged();
    /// \brief Emitted when the hovered rectangle index changes.
    void hoveredIndexChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    /// \brief Invalidates uploaded coordinates when an axis changes scale.
    void onAxisScaleChanged() override;

private:
    bool validateRawDataArguments(const void* data, int rectCount) const;
    bool validateDataArguments(std::size_t valueCount, int rectCount) const;
    void applyData(std::vector<double>&& data, int rectCount);
    void setHoveredIndex(int index);
    void buildSpatialGrid();
    void buildVertexCache();
    void updateDataRanges();
    // Rebuilds renderData_ (origin-relative float coordinates) from the double-precision
    // data_, so GPU upload and hover hit-testing stay accurate for large coordinates
    // (e.g. modern Unix-epoch timestamps) without needing double-precision textures.
    void rebuildRenderData(bool logScaleX, bool logScaleY);

    // Vertex cache: 6 vertices per rect, 12 bytes each.
    // Rebuilt only when rectCount_ changes — vertex data is deterministic from count alone.
    struct RectVertex {
        float id;
        float corner;
        unsigned char r, g, b, a;
    };

    QColor color_;
    int hoveredIndex_{-1};
    // Data: 4 doubles per rect (x1, y1, x2, y2), full precision.
    std::vector<double> data_;
    // Origin-relative float mirror of data_, uploaded to the GPU and used for hover hit-testing.
    std::vector<float> renderData_;
    qreal renderOriginX_{0.0};
    qreal renderOriginY_{0.0};
    int rectCount_{0};
    bool dataChanged_{false};
    std::vector<RectVertex> vertexCache_;
    bool vertexCacheValid_{false};
    SpatialGrid spatialGrid_;
};

} // namespace QAccelPlot
