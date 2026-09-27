//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/series/PlotSeries.hpp"
#include "QAccelPlot/series/RectangleBorder.hpp"
#include "QAccelPlot/series/SpatialGrid.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <vector>

namespace QAccelPlot {

class RectMaterial;

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
/// Each rectangle can carry a \c category, an index into \c categoryColors. Rectangles without a
/// category, or with one outside \c categoryColors, use \c color.
///
/// \sa LineCurve, Axis
class RectangleList : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(RectangleList)

    /// \brief Fill color of rectangles without a category color. Default: \c Colors.dark.seriesPrimary with alpha 50.
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    /// \brief Fill colors indexed by each rectangle's \c category. Default: empty.
    Q_PROPERTY(QList<QColor> categoryColors READ categoryColors WRITE setCategoryColors NOTIFY categoryColorsChanged)
    /// \brief Grouped outline settings, e.g. <tt>border.width</tt> and <tt>border.color</tt>. No outline by default.
    Q_PROPERTY(RectangleBorder* border READ border CONSTANT)
    /// \brief Minimum drawn width in pixels, so narrow rectangles stay visible when zoomed out. Default: 1.
    ///
    /// Narrower rectangles are widened around their center. Hover uses the widened size. Clamped to at least 0.
    Q_PROPERTY(qreal minimumWidth READ minimumWidth WRITE setMinimumWidth NOTIFY minimumWidthChanged)
    /// \brief Minimum drawn height in pixels, like \c minimumWidth. Default: 1.
    Q_PROPERTY(qreal minimumHeight READ minimumHeight WRITE setMinimumHeight NOTIFY minimumHeightChanged)
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

    /// \brief Returns the fill colors indexed by category.
    QList<QColor> categoryColors() const;
    /// \brief Sets the fill colors indexed by category to \a colors.
    void setCategoryColors(const QList<QColor>& colors);

    /// \brief Returns the grouped outline settings. The object is owned by the list.
    RectangleBorder* border() const;

    /// \brief Returns the minimum drawn width in pixels.
    qreal minimumWidth() const;
    /// \brief Sets the minimum drawn width to \a width pixels. Negative values are clamped to 0.
    void setMinimumWidth(qreal width);

    /// \brief Returns the minimum drawn height in pixels.
    qreal minimumHeight() const;
    /// \brief Sets the minimum drawn height to \a height pixels. Negative values are clamped to 0.
    void setMinimumHeight(qreal height);

    /// \brief Returns the number of rectangles currently loaded.
    int count() const;
    /// \brief Returns the index of the hovered rectangle, or -1 if none.
    int hoveredIndex() const;

    /// \brief Loads rectangles from \a rects, a QML list of objects with \c x1, \c y1, \c x2, \c y2 properties.
    ///
    /// A missing or null \c x1 / \c y1 is -Infinity and a missing \c x2 / \c y2 is +Infinity, so
    /// <tt>{ x1: 8, x2: 12 }</tt> is a full-height span. An optional integer \c category selects
    /// the fill color from \c categoryColors.
    Q_INVOKABLE void setData(const QVariantList& rects);

    /// \brief Loads rectangles from a C++ raw float array (\a data must have \a rectCount × 4 floats: x1, y1, x2, y2).
    void setData(const float* data, int rectCount);

    /// \brief Loads rectangles from a C++ raw double array, preserving full precision for large
    /// coordinates (e.g. modern Unix-epoch timestamps). \a data must have \a rectCount × 4 doubles.
    void setData(const double* data, int rectCount);

    /// \brief Moves \a data (\a rectCount × 4 doubles: x1, y1, x2, y2) into the list and clears categories. No copy is made.
    void setData(std::vector<double>&& data, int rectCount);

    /// \brief Moves \a data and per-rectangle \a categories (empty, or exactly \a rectCount) into the list.
    void setData(std::vector<double>&& data, std::vector<int>&& categories, int rectCount);

    /// \brief Thread-safe: queues \c setData(\a data, \a rectCount) to the item's thread.
    void postData(std::vector<double>&& data, int rectCount);

    /// \brief Thread-safe: queues \c setData(\a data, \a categories, \a rectCount) to the item's thread.
    void postData(std::vector<double>&& data, std::vector<int>&& categories, int rectCount);

    /// \brief Sets one category per rectangle. An empty list clears categories; any other size must equal \c count.
    Q_INVOKABLE void setCategories(const QList<int>& categories);

    /// \brief Removes all rectangles.
    Q_INVOKABLE void clearData();

    /// \brief Returns rectangle \a index as an object with \c x1, \c y1, \c x2, \c y2 properties.
    ///
    /// Includes \c category when categories are set. Returns an empty object when \a index is out of range.
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
    /// \brief Emitted when the categoryColors property changes.
    void categoryColorsChanged();
    /// \brief Emitted when the minimumWidth property changes.
    void minimumWidthChanged();
    /// \brief Emitted when the minimumHeight property changes.
    void minimumHeightChanged();
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
    bool validateDataArguments(std::size_t valueCount, std::size_t categoryCount, int rectCount) const;
    void applyData(std::vector<double>&& data, std::vector<int>&& categories, int rectCount);
    bool hasCategories() const;
    QColor rectangleColor(int index) const;
    // Tests rectangle \a index against item position \a position in pixels, widened like the shader draws it.
    bool containsInPixels(int index, const QPointF& position) const;
    void setHoveredIndex(int index);
    void updateMaterial(RectMaterial& material) const;
    void buildSpatialGrid();
    void buildVertexCache();
    void updateDataRanges();
    // Rebuilds renderData_ (origin-relative float coordinates) from the double-precision
    // data_, so the GPU upload stays accurate for large coordinates
    // (e.g. modern Unix-epoch timestamps) without needing double-precision textures.
    void rebuildRenderData(bool logScaleX, bool logScaleY);

    // Vertex cache: 6 vertices per rect, 12 bytes each. The color is the rectangle's category
    // color when categories are set; otherwise it is unused and \c color is a uniform.
    struct RectVertex {
        float id;
        float corner;
        unsigned char r, g, b, a;
    };

    QColor color_;
    QList<QColor> categoryColors_;
    RectangleBorder* border_{new RectangleBorder{this}};
    qreal minimumWidth_{1.0};
    qreal minimumHeight_{1.0};
    int hoveredIndex_{-1};
    // Data: 4 doubles per rect (x1, y1, x2, y2), full precision.
    std::vector<double> data_;
    // One category per rect, or empty when no rectangle has one.
    std::vector<int> categories_;
    // Origin-relative float mirror of data_, uploaded to the GPU.
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
