//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/series/PlotSeries.hpp"
#include "QAccelPlot/series/RectVertexCache.hpp"
#include "QAccelPlot/series/RectangleBorder.hpp"
#include "QAccelPlot/series/SpatialGrid.hpp"

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <array>
#include <vector>

namespace QAccelPlot {

class BarMaterial;

/// \brief A hardware-accelerated QML item that renders a bar chart.
///
/// Each bar is a (position, value) pair. A vertical bar is centered on \c position along the X axis
/// and spans from \c baselineValue to \c value along the Y axis; \c orientation \c Qt.Horizontal swaps
/// the axes. Bars are uploaded to the GPU as a float data texture of two floats per bar, and
/// \c barWidth, \c barOffset, and \c baselineValue are applied in the shader, so changing them does not
/// re-upload the data. The \c setData() overloads keep doubles and upload them relative to an
/// origin near the data, so large positions such as epoch timestamps stay precise.
///
/// Bars with a NaN or infinite position, or a NaN value, are not drawn or hovered. An infinite
/// value extends the bar to the plot edge. Values below \c baselineValue extend the bar the other way.
///
/// A ranged bar is a (from, to, value) triple that spans from \c from to \c to along the position
/// axis, e.g. a histogram bin. \c barWidth and \c barOffset do not apply to ranged bars. A ranged bar
/// with a NaN edge is not drawn, and an infinite edge extends it to the plot edge. A series holds
/// either ranged bars or (position, value) bars.
///
/// Each bar can carry a \c category, an index into \c categoryColors. Bars without a category, or
/// with one outside \c categoryColors, use \c color. For grouped bars, use one series per group
/// with a narrower \c barWidth and a different \c barOffset.
///
/// \par Limits
/// Up to 16,777,216 (2^24) bars are drawn correctly, as the shader indexes bars in single precision.
///
/// \sa RectangleSeries, Axis
class BarSeries : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(BarSeries)

    /// \brief Direction the bars grow in. Default: \c Qt.Vertical, positions on the X axis and values on the Y axis.
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    /// \brief Bar width in position-axis data units. Default: 0.8. Clamped to at least 0.
    Q_PROPERTY(qreal barWidth READ barWidth WRITE setBarWidth NOTIFY barWidthChanged)
    /// \brief Shift of every bar along the position axis in data units, e.g. for grouped bars. Default: 0.
    Q_PROPERTY(qreal barOffset READ barOffset WRITE setBarOffset NOTIFY barOffsetChanged)
    /// \brief Value the bars start from. Default: 0.
    ///
    /// \c -Infinity, or a non-positive baseline on a logarithmic value axis, starts the bars at the plot edge.
    Q_PROPERTY(qreal baselineValue READ baselineValue WRITE setBaselineValue NOTIFY baselineValueChanged)
    /// \brief Minimum drawn bar width in pixels, so bars stay visible when zoomed out. Default: 1. Clamped to at least 0.
    Q_PROPERTY(qreal minimumWidth READ minimumWidth WRITE setMinimumWidth NOTIFY minimumWidthChanged)
    /// \brief Fill color of bars without a category color. Default: \c Colors.dark.seriesPrimary.
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    /// \brief Fill colors indexed by each bar's \c category. Default: empty.
    Q_PROPERTY(QList<QColor> categoryColors READ categoryColors WRITE setCategoryColors NOTIFY categoryColorsChanged)
    /// \brief Grouped outline settings, e.g. <tt>border.width</tt> and <tt>border.color</tt>. No outline by default.
    Q_PROPERTY(RectangleBorder* border READ border CONSTANT)
    /// \brief Fill color of the bar under the cursor. Default: an invalid color, no highlight.
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    /// \brief Read-only: number of bars currently loaded.
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    /// \brief Read-only: index of the bar under the cursor, or -1 when none.
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)

public:
    /// \brief Constructs a BarSeries with the given \a parent.
    explicit BarSeries(QQuickItem* parent = nullptr);

    /// \brief Returns the direction the bars grow in.
    Qt::Orientation orientation() const;
    /// \brief Sets the direction the bars grow in to \a orientation.
    void setOrientation(Qt::Orientation orientation);

    /// \brief Returns the bar width in position-axis data units.
    qreal barWidth() const;
    /// \brief Sets the bar width to \a width data units. Negative values are clamped to 0.
    void setBarWidth(qreal width);

    /// \brief Returns the shift of every bar along the position axis in data units.
    qreal barOffset() const;
    /// \brief Sets the shift of every bar along the position axis to \a offset data units.
    void setBarOffset(qreal offset);

    /// \brief Returns the value the bars start from.
    qreal baselineValue() const;
    /// \brief Sets the value the bars start from to \a baselineValue. NaN is ignored.
    void setBaselineValue(qreal baselineValue);

    /// \brief Returns the minimum drawn bar width in pixels.
    qreal minimumWidth() const;
    /// \brief Sets the minimum drawn bar width to \a width pixels. Negative values are clamped to 0.
    void setMinimumWidth(qreal width);

    /// \brief Returns the bar fill color.
    QColor color() const;
    /// \brief Sets the fill color to \a color.
    void setColor(const QColor& color);

    /// \brief Returns the fill colors indexed by category.
    QList<QColor> categoryColors() const;
    /// \brief Sets the fill colors indexed by category to \a colors.
    void setCategoryColors(const QList<QColor>& colors);

    /// \brief Returns the grouped outline settings. The object is owned by the series.
    RectangleBorder* border() const;

    /// \brief Returns the fill color of the hovered bar.
    QColor hoverColor() const;
    /// \brief Sets the fill color of the hovered bar to \a color. An invalid color disables the highlight.
    void setHoverColor(const QColor& color);

    /// \brief Returns the number of bars currently loaded.
    int count() const;
    /// \brief Returns the index of the hovered bar, or -1 if none.
    int hoveredIndex() const;

    /// \brief Loads bars from \a bars, a QML list of numbers or objects.
    ///
    /// A number is the value of a bar at position = its list index. An object has \c position,
    /// \c value, and an optional integer \c category that selects the fill color from
    /// \c categoryColors. A missing \c position is the list index; a missing \c value is NaN.
    ///
    /// When any object has \c from or \c to, all bars are ranged: each object has \c from, \c to,
    /// \c value, and an optional \c category, and a missing \c from or \c to is NaN.
    Q_INVOKABLE void setData(const QVariantList& bars);

    /// \brief Loads bars from a C++ raw double array of \a barCount interleaved (position, value) pairs.
    void setData(const double* data, int barCount) override;

    /// \brief Moves \a data (\a barCount × 2 doubles: position, value) into the series and clears categories. No copy is made.
    void setData(std::vector<double>&& data, int barCount) override;

    /// \brief Moves \a data and per-bar \a categories (empty, or exactly \a barCount) into the series.
    void setData(std::vector<double>&& data, std::vector<int>&& categories, int barCount);

    /// \brief Like \c setDataNoRange(vector) but copies from a raw interleaved double array.
    void setDataNoRange(const double* data, int barCount) override;
    /// \brief Like \c setData() but does not report X/Y data ranges to the axes.
    ///
    /// Use it for streaming when the axes' \c dataMin / \c dataMax are managed by the application.
    void setDataNoRange(std::vector<double>&& data, int barCount) override;

    /// \brief Like \c setDataNoRange(\a data, \a barCount) and also moves per-bar \a categories into the series.
    void setDataNoRange(std::vector<double>&& data, std::vector<int>&& categories, int barCount);

    /// \brief High-performance C++ overload: copies \a barCount × 2 floats (position, value) from \a data and clears categories.
    void setDataF(const float* data, int barCount) override;

    /// \brief High-performance C++ overload: moves \a data (\a barCount × 2 floats) into the series and clears categories.
    void setDataF(std::vector<float>&& data, int barCount) override;

    /// \brief Like \c setDataF(\a data, \a barCount) and also moves per-bar \a categories (empty, or exactly \a barCount) into the series.
    void setDataF(std::vector<float>&& data, std::vector<int>&& categories, int barCount);

    /// \brief Like \c setDataFNoRange(vector) but copies from a raw float array into the series' reusable buffer.
    void setDataFNoRange(const float* data, int barCount) override;
    /// \brief Like \c setDataF() but does not report X/Y data ranges to the axes.
    ///
    /// Use it for streaming when the axes' \c dataMin / \c dataMax are managed by the application.
    void setDataFNoRange(std::vector<float>&& data, int barCount) override;

    /// \brief Like \c setDataFNoRange(\a data, \a barCount) and also moves per-bar \a categories into the series.
    void setDataFNoRange(std::vector<float>&& data, std::vector<int>&& categories, int barCount);

    /// \brief Thread-safe: queues \c setData(\a data, \a barCount) to the item's thread.
    void postData(std::vector<double>&& data, int barCount) override;

    /// \brief Thread-safe: queues \c setData(\a data, \a categories, \a barCount) to the item's thread.
    void postData(std::vector<double>&& data, std::vector<int>&& categories, int barCount);

    /// \brief Thread-safe: queues \c setDataF(\a data, \a barCount) to the item's thread.
    void postData(std::vector<float>&& data, int barCount) override;

    /// \brief Thread-safe: queues \c setDataF(\a data, \a categories, \a barCount) to the item's thread.
    void postData(std::vector<float>&& data, std::vector<int>&& categories, int barCount);

    /// \brief Moves ranged bars into the series: \a data holds \a barCount × 3 doubles (from, to, value). Clears categories.
    void setRangedData(std::vector<double>&& data, int barCount);
    /// \brief Like \c setRangedData(\a data, \a barCount) and also moves per-bar \a categories (empty, or exactly \a barCount) into the series.
    void setRangedData(std::vector<double>&& data, std::vector<int>&& categories, int barCount);
    /// \brief Thread-safe: queues \c setRangedData(\a data, \a barCount) to the item's thread.
    void postRangedData(std::vector<double>&& data, int barCount);

    /// \brief Removes all bars.
    Q_INVOKABLE void clearData() override;

    /// \brief Sets one category per bar. An empty list clears categories; any other size must equal \c count.
    Q_INVOKABLE void setCategories(const QList<int>& categories);

    /// \brief Returns bar \a index as an object with \c position and \c value, or \c from, \c to, and \c value for a ranged bar.
    ///
    /// Includes \c category when categories are set. Returns an empty object when \a index is out of range.
    Q_INVOKABLE QVariantMap barAt(int index) const;

    /// \brief Returns the index of the topmost bar under item position \a position, or -1.
    int barIndexAt(const QPointF& position) const;
    /// \brief Returns \c true when a bar lies under item position \a point.
    ///
    /// Hover delivery uses this test, so stacked series underneath still receive hover events outside the bars.
    bool contains(const QPointF& point) const override;

signals:
    /// \brief Emitted when the orientation property changes.
    void orientationChanged();
    /// \brief Emitted when the barWidth property changes.
    void barWidthChanged();
    /// \brief Emitted when the barOffset property changes.
    void barOffsetChanged();
    /// \brief Emitted when the baselineValue property changes.
    void baselineValueChanged();
    /// \brief Emitted when the minimumWidth property changes.
    void minimumWidthChanged();
    /// \brief Emitted when the color property changes.
    void colorChanged();
    /// \brief Emitted when the categoryColors property changes.
    void categoryColorsChanged();
    /// \brief Emitted when the hoverColor property changes.
    void hoverColorChanged();
    /// \brief Emitted when the bar count changes.
    void countChanged();
    /// \brief Emitted when the hovered bar index changes.
    void hoveredIndexChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
    /// \brief Returns the bar at \a index with its position, value, and category.
    InspectionRecord inspectionRecord(int index) const override;
    /// \brief Returns the bar drawn at the series-local \a position.
    InspectionRecord inspectionRecordAt(const QPointF& position) const override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    /// \brief Refreshes data ranges and uploaded coordinates when an axis changes scale.
    void onAxisScaleChanged() override;

private:
    bool validateRawDataArguments(const void* data, int barCount) const;
    bool validateDataArguments(std::size_t valueCount, std::size_t categoryCount, int barCount, int barValueCount = 2) const;
    void applyData(std::vector<double>&& data, std::vector<int>&& categories, int barCount, bool reportRanges);
    void applyRangedData(std::vector<double>&& data, std::vector<int>&& categories, int barCount);
    void applyFloatData(std::vector<float>&& data, std::vector<int>&& categories, int barCount, bool reportRanges);
    void setDataFFromArray(const float* data, int barCount, bool reportRanges);
    void finishDataChange(std::vector<int>&& categories, int barCount, bool reportRanges);
    // Invalidates what depends on the bar geometry after a barWidth, barOffset, baselineValue, or orientation change.
    void onGeometryChanged();
    bool isHorizontal() const;
    // True when the double setData() overloads supplied the data; false for the setDataF() overloads.
    bool hasPreciseData() const;
    // Number of values per bar: (position, value), or (from, to, value) for ranged bars.
    int valuesPerBar() const;
    // Returns value \a offset of bar \a index, counted within the bar.
    double component(int index, int offset) const;
    double value(int index) const;
    // Returns the extent of bar \a index along the position axis, both NaN when the bar has none.
    std::array<double, 2> positionSpan(int index) const;
    // Returns bar \a index as data-space edges (x1, y1, x2, y2), all NaN when the bar is not drawn.
    std::array<double, 4> barRect(int index) const;
    bool hasCategories() const;
    QColor barColor(int index) const;
    // Tests bar \a index against item position \a position in pixels, widened like the shader draws it.
    bool containsInPixels(int index, const QPointF& position) const;
    void setHoveredIndex(int index);
    QSGNode* releaseNode(QSGNode* oldNode) const;
    void updateMaterial(BarMaterial& material) const;
    void ensureSpatialGrid() const;
    void buildVertexCache();
    void updateDataRanges();
    void reportDataRanges(qreal positionMin, qreal positionMax, qreal valueMin, qreal valueMax, bool hasValues);
    // Rebuilds renderData_ (origin-relative float coordinates) from the double-precision data_,
    // so the GPU upload stays accurate for large positions without double-precision textures.
    void rebuildRenderData(bool logScalePosition, bool logScaleValue);

    Qt::Orientation orientation_{Qt::Vertical};
    qreal barWidth_{0.8};
    qreal barOffset_{0.0};
    qreal baselineValue_{0.0};
    qreal minimumWidth_{1.0};
    QColor color_;
    QList<QColor> categoryColors_;
    RectangleBorder* border_{new RectangleBorder{this}};
    QColor hoverColor_;
    int hoveredIndex_{-1};
    // Data: valuesPerBar() doubles per bar, full precision. Empty when setDataF() supplied the data.
    std::vector<double> data_;
    // True when each bar is (from, to, value) instead of (position, value).
    bool ranged_{false};
    // One category per bar, or empty when no bar has one.
    std::vector<int> categories_;
    // Uploaded to the GPU: an origin-relative float mirror of data_, or the setDataF() data itself.
    std::vector<float> renderData_;
    qreal renderOriginPosition_{0.0};
    qreal renderOriginValue_{0.0};
    int barCount_{0};
    bool dataChanged_{false};
    // False after the NoRange overloads, so property changes don't overwrite application-managed ranges.
    bool reportRanges_{true};
    // Vertex colors are category colors when categories are set; otherwise \c color is a uniform.
    RectVertexCache vertexCache_;
    // Built on the first hit test after a data or geometry change, so streaming without hover skips it.
    mutable std::vector<double> hitRects_;
    mutable SpatialGrid spatialGrid_;
    mutable bool spatialGridValid_{false};
};

} // namespace QAccelPlot
