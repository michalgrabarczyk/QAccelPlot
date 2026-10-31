//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/Border.hpp"
#include "QAccelPlot/series/Category.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"
#include "QAccelPlot/series/SpatialGrid.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <QList>
#include <QPointF>
#include <QQmlListProperty>
#include <QSizeF>

#include <memory>
#include <optional>
#include <vector>

namespace QAccelPlot {

namespace Internal {
class CategoryList;
class HoverIndexBudget;
class RectVertexCache;
}
class RectMaterial;

/// \brief A hardware-accelerated QML item that renders a large list of axis-aligned rectangles.
///
/// Rectangles are stored as interleaved (x1, y1, x2, y2) values and uploaded to the GPU as a float
/// data texture, making it suitable for millions of rectangles. The \c setData() overloads keep
/// doubles and upload them relative to an origin near the data, so large coordinates such as epoch
/// timestamps stay precise. The \c setDataF() overloads store floats and upload them without conversion.
/// Hover detection uses an internal \c SpatialGrid; rectangles spanning more than 64 grid cells are tested linearly.
///
/// An infinite edge extends the rectangle to the plot edge, e.g. \c y1 = -Infinity and
/// \c y2 = +Infinity for a full-height span. Infinite edges don't affect the axes' data ranges.
/// Rectangles with a NaN edge are not drawn or hovered.
///
/// Each rectangle can carry a \c category. List the categories in \c categories as \c Category
/// objects, each with a \c value and an optional \c name and \c color:
///
/// \code
/// QAccelPlot.RectangleSeries {
///     categories: [
///         QAccelPlot.Category { value: 0; name: "Running"; color: "seagreen" },
///         QAccelPlot.Category { value: 1; name: "Alarm"; color: "firebrick" }
///     ]
///     Component.onCompleted: setData([{ x1: 0, x2: 40, category: "Running" }])
/// }
/// \endcode
///
/// A rectangle is filled with the color of the category whose \c value equals its \c category.
/// Rectangles without a category, or whose category has no color or is not listed, use \c color.
/// The legend shows one row per category.
///
/// Set \c antialiasing to smooth edges that fall between pixels. It is off by default because
/// rectangles that touch then show faint seams.
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
    /// \brief The rectangles' categories. Default: empty.
    ///
    /// The first of categories that share a value or a name wins, and the others log a warning.
    /// Edits to a listed \c Category apply at once.
    Q_PROPERTY(QQmlListProperty<Category> categories READ categories NOTIFY categoriesChanged)
    /// \brief Outline inside each rectangle's edges, e.g. <tt>border.width: 1</tt>. Default: black, width 0.
    ///
    /// On rectangles too small for two outlines, the outline narrows so at least 1 px of fill stays visible.
    Q_PROPERTY(Border* border READ border CONSTANT)
    /// \brief Fill color of the rectangle under the cursor. Default: an invalid color, no highlight.
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    /// \brief Minimum drawn width in pixels, so narrow rectangles stay visible when zoomed out. Default: 1.
    ///
    /// Narrower rectangles are widened around their center. Hover uses the widened size. Clamped to at least 0.
    Q_PROPERTY(qreal minimumWidth READ minimumWidth WRITE setMinimumWidth NOTIFY minimumWidthChanged)
    /// \brief Minimum drawn height in pixels, like \c minimumWidth. Default: 1.
    Q_PROPERTY(qreal minimumHeight READ minimumHeight WRITE setMinimumHeight NOTIFY minimumHeightChanged)
    /// \brief Read-only: index of the rectangle under the cursor, or -1 when none.
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)

public:
    /// \brief Constructs a RectangleSeries with the given \a parent.
    explicit RectangleSeries(QQuickItem* parent = nullptr);
    /// \brief Destroys the rectangle series.
    ~RectangleSeries() override;

    /// \brief Returns the rectangle fill color.
    QColor color() const;
    /// \brief Sets the fill color to \a color.
    void setColor(const QColor& color);

    /// \brief Returns the categories as a QML list.
    QQmlListProperty<Category> categories();
    /// \brief Replaces the categories with \a categories. The series does not take ownership.
    void setCategories(const QList<Category*>& categories);

    /// \brief Returns the grouped outline settings. The object is owned by the series.
    Border* border() const;

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

    /// \brief Returns the index of the hovered rectangle, or -1 if none.
    int hoveredIndex() const;

    /// \brief Loads rectangles from \a rects, a QML list of objects with \c x1, \c y1, \c x2, \c y2 properties.
    ///
    /// A missing or null \c x1 / \c y1 is -Infinity and a missing \c x2 / \c y2 is +Infinity, so
    /// <tt>{ x1: 8, x2: 12 }</tt> is a full-height span. An optional \c category is a category
    /// \c value or \c name; names resolve against \c categories now. Any other \c category is
    /// ignored with a warning.
    Q_INVOKABLE void setData(const QVariantList& rects);

    // The PlotSeries overloads take interleaved (x1, y1, x2, y2) rectangles, as doubles or floats,
    // and clear the rectangles' categories.
    using PlotSeries::postData;
    using PlotSeries::setData;
    using PlotSeries::setDataF;

    /// \brief Moves \a data and per-rectangle category values \a categories (empty, or exactly \a rectCount) into the series.
    void setData(std::vector<double>&& data, std::vector<int>&& categories, int rectCount);

    /// \brief Like \c setDataF(\a data, \a rectCount) and also moves per-rectangle category values \a categories (empty, or exactly \a rectCount) into the
    /// series.
    void setDataF(std::vector<float>&& data, std::vector<int>&& categories, int rectCount);

    /// \brief Thread-safe: queues \c setData(\a data, \a categories, \a rectCount) to the item's thread.
    void postData(std::vector<double>&& data, std::vector<int>&& categories, int rectCount);

    /// \brief Thread-safe: queues \c setDataF(\a data, \a categories, \a rectCount) to the item's thread.
    void postData(std::vector<float>&& data, std::vector<int>&& categories, int rectCount);

    /// \brief Removes all rectangles.
    Q_INVOKABLE void clearData() override;
    /// \brief Returns the legend entry: the symbol and color of the rectangles.
    std::optional<LegendEntry> legendEntry() const override;
    /// \brief Returns one legend entry per category, or the \c legendEntry() when there are no categories.
    QList<LegendEntry> legendEntries() const override;

    /// \brief Sets one category per rectangle, each a category \c value or \c name.
    ///
    /// An empty list clears them; any other size must equal \c count. Other values leave their
    /// rectangle without a category and log a warning.
    Q_INVOKABLE void setDataPointCategories(const QVariantList& categories);

    /// \brief Returns rectangle \a index as an object with \c x1, \c y1, \c x2, \c y2 properties.
    ///
    /// Includes \c category when the rectangles have categories, and \c categoryName when that
    /// category has a name. Returns an empty object when \a index is out of range.
    Q_INVOKABLE QVariantMap rectangleAt(int index) const;

    /// \brief Returns the index of the topmost rectangle under item position \a position, or -1.
    int rectangleIndexAt(const QPointF& position) const;
    /// \brief Returns \c true when a rectangle lies under item position \a point.
    ///
    /// Hover delivery uses this test, so stacked series underneath still receive hover events
    /// outside this series rectangles.
    bool contains(const QPointF& point) const override;

signals:
    /// \brief Emitted when the color property changes.
    void colorChanged();
    /// \brief Emitted when a category is added, removed, or edited.
    void categoriesChanged();
    /// \brief Emitted when the hoverColor property changes.
    void hoverColorChanged();
    /// \brief Emitted when the minimumWidth property changes.
    void minimumWidthChanged();
    /// \brief Emitted when the minimumHeight property changes.
    void minimumHeightChanged();
    /// \brief Emitted when the hovered rectangle index changes.
    void hoveredIndexChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
    /// \brief Returns true: rectangles are inspected as records.
    bool recordInspectionSupported() const override;
    /// \brief Returns the rectangle at \a index with its edges and category.
    InspectionRecord inspectionRecord(int index) const override;
    /// \brief Returns the rectangle drawn at the series-local \a position.
    InspectionRecord inspectionRecordAt(const QPointF& position) const override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    /// \brief Repaints with the coordinates mapped for the new scale.
    void onAxisScaleChanged() override;
    /// \brief Refreshes the vertices, the hover index, and the count after the rectangles changed.
    void onDataPointsChanged(DataChange change, int previousCount) override;

private:
    void onCategoriesChanged();
    bool validateCategories(std::size_t categoryCount, int rectCount) const;
    void applyData(std::vector<double>&& data, std::vector<int>&& categories, int rectCount);
    void applyFloatData(std::vector<float>&& data, std::vector<int>&& categories, int rectCount);
    // Returns edge \a component (0 = x1, 1 = y1, 2 = x2, 3 = y2) of rectangle \a index.
    double coordinate(int index, int component) const;
    bool hasCategories() const;
    QColor rectangleColor(int index) const;
    // Tests rectangle \a index against item position \a position in pixels, widened like the shader draws it.
    bool containsInPixels(int index, const QPointF& position) const;
    void setHoveredIndex(int index);
    // Sets the uniforms for drawing the first \a drawnCount rectangles.
    void updateMaterial(RectMaterial& material, int drawnCount) const;
    // Returns true when the spatial grid answers the next query, building it once scanning has cost as much.
    bool spatialGridReady() const;

    // Everything a hit test reads besides the rectangles, so equal inputs give an equal answer.
    struct HitTestInputs {
        QPointF position;
        QSizeF itemSize;
        QSizeF minimumSize;
        // The axis mappings, pinned down by each axis' scale and the coordinates at its two ends.
        double xAtLeft;
        double xAtRight;
        double yAtTop;
        double yAtBottom;
        bool logScaleX;
        bool logScaleY;

        bool operator==(const HitTestInputs& other) const;
    };

    HitTestInputs hitTestInputs(const QPointF& position) const;
    int topmostRectangleAt(const QPointF& position) const;
    // Builds the vertices of the first \a rectCount rectangles.
    void buildVertexCache(int rectCount);

    QColor color_;
    std::unique_ptr<Internal::CategoryList> categories_;
    Border* border_{new Border{Qt::black, 0.0, this}};
    QColor hoverColor_;
    qreal minimumWidth_{1.0};
    qreal minimumHeight_{1.0};
    int hoveredIndex_{-1};
    // Render revision of the rectangles last uploaded to the GPU.
    std::uint64_t uploadedRevision_{0};
    // Vertex colors are category colors when categories are set; otherwise \c color is a uniform.
    std::unique_ptr<Internal::RectVertexCache> vertexCache_;
    // Whether the vertex cache holds category colors.
    bool vertexCacheHasCategories_{false};
    // Built once scanning the same data has cost as much as the build, so streaming skips it.
    mutable SpatialGrid spatialGrid_;
    mutable bool spatialGridValid_{false};
    std::unique_ptr<Internal::HoverIndexBudget> spatialGridBudget_;
    // The last hit test and its answer. Qt tests contains() several times per hover event before
    // the handler asks again, each time at the same position.
    mutable std::optional<HitTestInputs> lastHitTest_;
    mutable int lastHitIndex_{-1};
};

} // namespace QAccelPlot
