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
/// Rectangles are stored as interleaved (x1, y1, x2, y2) values and uploaded to the GPU as a float
/// data texture, making it suitable for millions of rectangles. The \c setData() overloads keep
/// doubles and upload them relative to an origin near the data, so large coordinates such as epoch
/// timestamps stay precise. The \c setDataF() overloads store floats and upload them without conversion.
/// Hover detection uses an internal \c SpatialGrid for O(1) hit tests.
///
/// An infinite edge extends the rectangle to the plot edge, e.g. \c y1 = -Infinity and
/// \c y2 = +Infinity for a full-height span. Infinite edges don't affect the axes' data ranges.
/// Rectangles with a NaN edge are not drawn or hovered.
///
/// Each rectangle can carry a \c category, an index into \c categoryColors. Rectangles without a
/// category, or with one outside \c categoryColors, use \c color.
///
/// \par Limits
/// Up to 16,777,216 (2^24) rectangles are drawn correctly. The shader indexes rectangles in single
/// precision, and at that count the data texture reaches 8192 rows, the size every GPU supports.
///
/// \sa LineCurve, Axis
class RectangleSeries : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(RectangleSeries)

    /// \brief Fill color of rectangles without a category color. Default: \c Colors.dark.seriesPrimary with alpha 50.
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    /// \brief Fill colors indexed by each rectangle's \c category. Default: empty.
    Q_PROPERTY(QList<QColor> categoryColors READ categoryColors WRITE setCategoryColors NOTIFY categoryColorsChanged)
    /// \brief Grouped outline settings, e.g. <tt>border.width</tt> and <tt>border.color</tt>. No outline by default.
    Q_PROPERTY(RectangleBorder* border READ border CONSTANT)
    /// \brief Fill color of the rectangle under the cursor. Default: an invalid color, no highlight.
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
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
    /// \brief Constructs a RectangleSeries with the given \a parent.
    explicit RectangleSeries(QQuickItem* parent = nullptr);

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

    /// \brief Returns the fill color of the hovered rectangle.
    QColor hoverColor() const;
    /// \brief Sets the fill color of the hovered rectangle to \a color. An invalid color disables the highlight.
    void setHoverColor(const QColor& color);

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

    /// \brief Loads rectangles from a C++ raw double array, preserving full precision for large
    /// coordinates (e.g. modern Unix-epoch timestamps). \a data must have \a rectCount × 4 doubles.
    void setData(const double* data, int rectCount) override;

    /// \brief Moves \a data (\a rectCount × 4 doubles: x1, y1, x2, y2) into the list and clears categories. No copy is made.
    void setData(std::vector<double>&& data, int rectCount) override;

    /// \brief Moves \a data and per-rectangle \a categories (empty, or exactly \a rectCount) into the list.
    void setData(std::vector<double>&& data, std::vector<int>&& categories, int rectCount);

    /// \brief Like \c setDataNoRange(vector) but copies from a raw interleaved double array.
    void setDataNoRange(const double* data, int rectCount) override;
    /// \brief Like \c setData() but does not report X/Y data ranges to the axes.
    ///
    /// Use it for streaming when the axes' \c dataMin / \c dataMax are managed by the application.
    void setDataNoRange(std::vector<double>&& data, int rectCount) override;

    /// \brief Like \c setDataNoRange(\a data, \a rectCount) and also moves per-rectangle \a categories into the list.
    void setDataNoRange(std::vector<double>&& data, std::vector<int>&& categories, int rectCount);

    /// \brief High-performance C++ overload: copies \a rectCount × 4 floats (x1, y1, x2, y2) from \a data and clears categories.
    void setDataF(const float* data, int rectCount) override;

    /// \brief High-performance C++ overload: moves \a data (\a rectCount × 4 floats) into the list and clears categories.
    void setDataF(std::vector<float>&& data, int rectCount) override;

    /// \brief Like \c setDataF(\a data, \a rectCount) and also moves per-rectangle \a categories (empty, or exactly \a rectCount) into the list.
    void setDataF(std::vector<float>&& data, std::vector<int>&& categories, int rectCount);

    /// \brief Like \c setDataFNoRange(vector) but copies from a raw float array into the list's reusable buffer.
    void setDataFNoRange(const float* data, int rectCount) override;
    /// \brief Like \c setDataF() but does not report X/Y data ranges to the axes.
    ///
    /// Use it for streaming when the axes' \c dataMin / \c dataMax are managed by the application.
    void setDataFNoRange(std::vector<float>&& data, int rectCount) override;

    /// \brief Like \c setDataFNoRange(\a data, \a rectCount) and also moves per-rectangle \a categories into the list.
    void setDataFNoRange(std::vector<float>&& data, std::vector<int>&& categories, int rectCount);

    /// \brief Thread-safe: queues \c setData(\a data, \a rectCount) to the item's thread.
    void postData(std::vector<double>&& data, int rectCount) override;

    /// \brief Thread-safe: queues \c setData(\a data, \a categories, \a rectCount) to the item's thread.
    void postData(std::vector<double>&& data, std::vector<int>&& categories, int rectCount);

    /// \brief Thread-safe: queues \c setDataF(\a data, \a rectCount) to the item's thread.
    void postData(std::vector<float>&& data, int rectCount) override;

    /// \brief Thread-safe: queues \c setDataF(\a data, \a categories, \a rectCount) to the item's thread.
    void postData(std::vector<float>&& data, std::vector<int>&& categories, int rectCount);

    /// \brief Removes all rectangles.
    Q_INVOKABLE void clearData() override;

    /// \brief Sets one category per rectangle. An empty list clears categories; any other size must equal \c count.
    Q_INVOKABLE void setCategories(const QList<int>& categories);

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
    /// \brief Emitted when the hoverColor property changes.
    void hoverColorChanged();
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
    void applyData(std::vector<double>&& data, std::vector<int>&& categories, int rectCount, bool reportRanges);
    void applyFloatData(std::vector<float>&& data, std::vector<int>&& categories, int rectCount, bool reportRanges);
    void setDataFFromArray(const float* data, int rectCount, bool reportRanges);
    void finishDataChange(std::vector<int>&& categories, int rectCount, bool reportRanges);
    // True when the double setData() overloads supplied the data; false for the setDataF() overloads.
    bool hasPreciseData() const;
    // Returns edge \a component (0 = x1, 1 = y1, 2 = x2, 3 = y2) of rectangle \a index.
    double coordinate(int index, int component) const;
    bool hasCategories() const;
    QColor rectangleColor(int index) const;
    // Tests rectangle \a index against item position \a position in pixels, widened like the shader draws it.
    bool containsInPixels(int index, const QPointF& position) const;
    void setHoveredIndex(int index);
    void updateMaterial(RectMaterial& material) const;
    void ensureSpatialGrid() const;
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
    QColor hoverColor_;
    qreal minimumWidth_{1.0};
    qreal minimumHeight_{1.0};
    int hoveredIndex_{-1};
    // Data: 4 doubles per rect (x1, y1, x2, y2), full precision. Empty when setDataF() supplied the data.
    std::vector<double> data_;
    // One category per rect, or empty when no rectangle has one.
    std::vector<int> categories_;
    // Uploaded to the GPU: an origin-relative float mirror of data_, or the setDataF() data itself.
    std::vector<float> renderData_;
    qreal renderOriginX_{0.0};
    qreal renderOriginY_{0.0};
    int rectCount_{0};
    bool dataChanged_{false};
    std::vector<RectVertex> vertexCache_;
    bool vertexCacheValid_{false};
    // Built on the first hit test after a data change, so streaming without hover skips it.
    mutable SpatialGrid spatialGrid_;
    mutable bool spatialGridValid_{false};
};

} // namespace QAccelPlot
