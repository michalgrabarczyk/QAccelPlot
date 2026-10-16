//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/series/ColormappedSeries.hpp"

#include <QColor>
#include <QList>
#include <QPointF>

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot {

class HeatmapMaterial;

/// \brief A hardware-accelerated QML item that draws a grid of values as colored cells.
///
/// The grid has \c columns cells per row and \c rows rows of equal cells. Values are stored row by
/// row: the value of a cell has index <tt>row * columns + column</tt>, with row 0 at \c yMin and
/// column 0 at \c xMin. \c xMin, \c xMax, \c yMin, and \c yMax are the outer edges of the grid; an
/// unset bound places the grid in index space, so column \a c spans [c, c + 1].
///
/// Each cell is colored by mapping its value through \c colormap. The series starts with a Viridis
/// colormap of its own whose range is resolved from the data; assign a shared \c Colormap to
/// replace it. Nothing is drawn while \c colormap is null.
///
/// \code
/// QAccelPlot.HeatmapSeries {
///     xMin: -10; xMax: 10
///     yMin: -5; yMax: 5
///     // Three columns by two rows.
///     Component.onCompleted: setValues([0, 1, 2, 3, 4, 5], 3)
/// }
/// \endcode
///
/// \par Data
/// \c setValues() replaces the values and the shape together. The \c PlotSeries setters take one
/// value per cell and keep \c columns, so their count must be a multiple of it. Values are stored
/// in single precision. \c postValues() and \c postData() may be called from any thread.
///
/// \par Invalid cells
/// Cells with a non-finite value, and cells with a value at or below zero under a \c Log colormap,
/// are transparent. Non-finite values do not count toward the value range.
///
/// \par Inspection
/// \c inspection.recordAt() and \c inspection.recordAtPosition() return a record with the fields
/// \c x and \c y (the cell center), \c value, \c row, and \c column. A cell with a non-finite value
/// is still a match. \c PlotInspector does not inspect heatmaps.
///
/// \par Limits
/// Each pixel takes the color of the cell under its center, without interpolation. Logarithmic axes
/// are not supported: the series draws nothing on one, reports no data range for it, and logs a
/// warning. A grid of more cells than the GPU data texture holds, about 67 million for an 8192-pixel
/// texture limit, draws nothing and logs a warning.
///
/// \sa ColormappedSeries, Colormap, ColorBar, PlotSeries
class HeatmapSeries : public ColormappedSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(HeatmapSeries)

    /// \brief Number of cells per row. Changing it keeps the values and recomputes \c rows. Default: 0.
    ///
    /// Values of an incomplete last row are kept but not drawn or inspected.
    Q_PROPERTY(int columns READ columns WRITE setColumns NOTIFY columnsChanged)
    /// \brief Read-only: number of complete rows of \c columns values.
    Q_PROPERTY(int rows READ rows NOTIFY rowsChanged)
    /// \brief Left edge of column 0 in data coordinates. Unset (NaN) means 0. Default: unset.
    Q_PROPERTY(qreal xMin READ xMin WRITE setXMin NOTIFY xMinChanged)
    /// \brief Right edge of the last column in data coordinates. Unset (NaN) means \c columns. Default: unset.
    Q_PROPERTY(qreal xMax READ xMax WRITE setXMax NOTIFY xMaxChanged)
    /// \brief Bottom edge of row 0 in data coordinates. Unset (NaN) means 0. Default: unset.
    Q_PROPERTY(qreal yMin READ yMin WRITE setYMin NOTIFY yMinChanged)
    /// \brief Top edge of the last row in data coordinates. Unset (NaN) means \c rows. Default: unset.
    Q_PROPERTY(qreal yMax READ yMax WRITE setYMax NOTIFY yMaxChanged)
    /// \brief Read-only: color at the middle of the colormap's ramp, used by the legend.
    Q_PROPERTY(QColor color READ color NOTIFY colorChanged)

public:
    /// \brief Constructs a HeatmapSeries with the given \a parent.
    explicit HeatmapSeries(QQuickItem* parent = nullptr);

    /// \brief Returns the number of cells per row.
    int columns() const;
    /// \brief Sets the number of cells per row to \a columns. Negative values are clamped to 0.
    void setColumns(int columns);

    /// \brief Returns the number of complete rows.
    int rows() const;

    /// \brief Returns the left grid edge, or NaN when unset.
    qreal xMin() const;
    /// \brief Sets the left grid edge to \a value. NaN unsets it.
    void setXMin(qreal value);

    /// \brief Returns the right grid edge, or NaN when unset.
    qreal xMax() const;
    /// \brief Sets the right grid edge to \a value. NaN unsets it.
    void setXMax(qreal value);

    /// \brief Returns the bottom grid edge, or NaN when unset.
    qreal yMin() const;
    /// \brief Sets the bottom grid edge to \a value. NaN unsets it.
    void setYMin(qreal value);

    /// \brief Returns the top grid edge, or NaN when unset.
    qreal yMax() const;
    /// \brief Sets the top grid edge to \a value. NaN unsets it.
    void setYMax(qreal value);

    /// \brief Returns the color at the middle of the ramp, or transparent without a colormap.
    QColor color() const;

    /// \brief Replaces the grid with \a values, row by row, \a columns per row.
    ///
    /// The number of values must be a multiple of \a columns. An empty list clears the values and
    /// keeps \c columns.
    Q_INVOKABLE void setValues(const QList<qreal>& values, int columns);
    /// \brief Replaces the grid with \a columns by \a rows values moved from \a values, row by row.
    void setValues(std::vector<float>&& values, int columns, int rows);
    /// \brief Thread-safe: queues \c setValues(\a values, \a columns, \a rows) to the item's thread.
    void postValues(std::vector<float>&& values, int columns, int rows);
    /// \brief Returns the value of the cell at \a row and \a column, or NaN when it is outside the grid.
    Q_INVOKABLE qreal valueAt(int row, int column) const;
    /// \brief Removes all values and keeps \c columns.
    Q_INVOKABLE void clearData() override;

signals:
    /// \brief Emitted when the columns property changes.
    void columnsChanged();
    /// \brief Emitted when the rows property changes.
    void rowsChanged();
    /// \brief Emitted when the xMin property changes.
    void xMinChanged();
    /// \brief Emitted when the xMax property changes.
    void xMaxChanged();
    /// \brief Emitted when the yMin property changes.
    void yMinChanged();
    /// \brief Emitted when the yMax property changes.
    void yMaxChanged();
    /// \brief Emitted when the color property changes.
    void colorChanged();

protected:
    /// \cond INTERNAL
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    /// \endcond

    /// \brief Stores the cell values passed to the \c PlotSeries setters, keeping \c columns.
    void applyDataPoints(DataPointInput&& input) override;
    /// \brief Returns 1: the \c PlotSeries setters take one value per cell.
    int dataPointValueCount() const override;
    /// \brief Returns the cell record at \a index.
    InspectionRecord inspectionRecord(int index) const override;
    /// \brief Returns the record of the cell drawn at the series-local \a position.
    InspectionRecord inspectionRecordAt(const QPointF& position) const override;
    /// \brief Returns the grid edges in each linear dimension.
    DataRanges computeDataRanges() const override;
    /// \brief Repaints, because the series draws nothing on a logarithmic axis.
    void onAxisScaleChanged() override;
    /// \brief Returns the span of the finite cell values.
    std::optional<ValueSpan> scanValueRange() const override;
    /// \brief Updates \c color to the middle of the new ramp.
    void onColorStopsChanged() override;

private:
    /// \brief Grid edges with unset bounds resolved to index space.
    struct Extent {
        double xMin;
        double xMax;
        double yMin;
        double yMax;
    };

    Extent resolvedExtent() const;
    int cellCount() const;
    bool validateShape(std::size_t valueCount, int columns, int rows) const;
    /// \brief Replaces the values with \a values of \a columns per row and finishes the change.
    void storeValues(std::vector<float>&& values, int columns);
    /// \brief Sets the shape to \a columns per row and finishes the change of the cells.
    void applyShape(int columns);
    void setBound(qreal& bound, qreal value, void (HeatmapSeries::*changed)());
    /// \brief Returns the cell at series-local \a position as (row, column), or \c std::nullopt outside the grid.
    std::optional<std::pair<int, int>> cellAt(const QPointF& position) const;
    /// \brief Returns whether the grid can be drawn into a window of \a window; warns once per reason it cannot.
    bool canDraw(QQuickWindow* window);
    /// \brief Writes the cell mapping, value range, and textures for the next frame into \a material.
    void updateMaterial(HeatmapMaterial& material, QQuickWindow* window, bool newNode);

    std::vector<float> values_;
    int columns_{0};
    int rows_{0};
    qreal xMin_;
    qreal xMax_;
    qreal yMin_;
    qreal yMax_;
    QColor color_;
    // Incremented by every value change; the GPU texture holds the values of uploadedRevision_.
    std::uint64_t valuesRevision_{1};
    std::uint64_t uploadedRevision_{0};
    bool warnedAboutLogAxis_{false};
    bool overCapacity_{false};
};

} // namespace QAccelPlot
