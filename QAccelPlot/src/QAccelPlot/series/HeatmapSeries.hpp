//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/series/ColormappedSeries.hpp"

#include <QList>
#include <QPointF>

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)

namespace QAccelPlot {

namespace Internal {
struct HeatmapGridDimension;
}
class HeatmapMaterial;

/// \brief A hardware-accelerated QML item that draws a grid of values as colored cells.
///
/// The grid has \c columns cells per row and \c rows rows of cells. Values are stored row by
/// row: the value of a cell has index <tt>row * columns + column</tt>, with row 0 at \c yMin and
/// column 0 at \c xMin. \c xMin, \c xMax, \c yMin, and \c yMax are the outer edges of the grid; an
/// unset bound places the grid in index space, so column \a c spans [c, c + 1].
///
/// Each cell is colored by mapping its value through \c colormap. The series starts with a Viridis
/// colormap of its own whose range is resolved from the data; assign a shared \c Colormap to
/// replace it. Nothing is drawn while \c colormap is null. The series has no legend entry: a
/// \c ColorBar is its key.
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
/// value per cell and keep \c columns, so their count must be a multiple of it. \c DataBounds
/// passed to them replace the grid edges as the data range until the values, the shape, or an edge
/// next changes. Values are stored in single precision. \c postValues() and \c postData() may be
/// called from any thread.
///
/// \par Invalid cells
/// Cells with a non-finite value, and cells with a value at or below zero under a \c Log colormap,
/// are transparent. Non-finite values do not count toward the value range.
///
/// \par Logarithmic axes and spacing
/// \c xSpacing and \c ySpacing space the cells evenly in data or in log10, independently of the
/// axis scale. On a log axis, \c Log cells have equal widths, \c Linear cells narrow toward larger
/// values, and the parts of cells at or below zero are not drawn; the data range starts at the
/// first cell edge above zero. Under \c Log spacing, both edges of that dimension must be positive,
/// so set them: an unset edge resolves to index space, which starts at 0. Otherwise the series
/// draws nothing, reports no data range in that dimension, and logs a warning.
///
/// \par Inspection
/// \c inspection.recordAt() and \c inspection.recordAtPosition() return a record with the fields
/// \c x and \c y (the cell center, geometric under \c Log spacing), \c value, \c row, and
/// \c column. \c inspection.dataPointAtPosition() returns the cell as a data point at its center,
/// with \c index <tt>row * columns + column</tt>, which \c PlotInspector reports in every mode. A
/// cell with a non-finite value is still a match. While a grid edge is not finite, or not positive
/// under \c Log spacing, \c recordAt() returns \c Unavailable and no position matches a cell.
///
/// \par Limits
/// Each pixel takes the color of the cell under its center, without interpolation. A grid with an
/// infinite edge draws nothing. A grid of more than 16,777,216 (2^24)
/// columns or rows, or of more cells than the GPU data texture holds, about 67 million for an
/// 8192-pixel texture limit, draws nothing and logs a warning.
///
/// \sa ColormappedSeries, Colormap, ColorBar, PlotSeries
class HeatmapSeries : public ColormappedSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(HeatmapSeries)

    /// \brief Number of cells per row. Changing it keeps the values and recomputes \c rows. Default: 0.
    ///
    /// Values of an incomplete last row are kept but not drawn or inspected. \c setValues() sets it,
    /// so do not bind \c columns while using \c setValues(): the binding reshapes the grid when it
    /// is re-evaluated.
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
    /// \brief How the columns are spaced between \c xMin and \c xMax. Default: \c Linear.
    Q_PROPERTY(Spacing xSpacing READ xSpacing WRITE setXSpacing NOTIFY xSpacingChanged)
    /// \brief How the rows are spaced between \c yMin and \c yMax. Default: \c Linear.
    Q_PROPERTY(Spacing ySpacing READ ySpacing WRITE setYSpacing NOTIFY ySpacingChanged)

public:
    /// \brief How the cells of one dimension are spaced between its grid edges.
    enum class Spacing {
        Linear, ///< \brief Cells of equal size in data coordinates.
        Log     ///< \brief Cells of equal size in log10, each spanning the same ratio. Both edges must be positive.
    };
    Q_ENUM(Spacing)

    /// \brief Constructs a HeatmapSeries with the given \a parent.
    explicit HeatmapSeries(QQuickItem* parent = nullptr);

    /// \brief Returns the number of cells per row.
    int columns() const;
    /// \brief Sets the number of cells per row to \a columns. Negative values are clamped to 0.
    void setColumns(int columns);

    /// \brief Returns the number of complete rows.
    int rows() const;
    /// \brief Returns the number of cells, \c columns × \c rows.
    int count() const override;

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

    /// \brief Returns how the columns are spaced.
    Spacing xSpacing() const;
    /// \brief Sets how the columns are spaced to \a spacing.
    void setXSpacing(Spacing spacing);

    /// \brief Returns how the rows are spaced.
    Spacing ySpacing() const;
    /// \brief Sets how the rows are spaced to \a spacing.
    void setYSpacing(Spacing spacing);

    /// \brief Replaces the grid with \a values, row by row, \a columns per row.
    ///
    /// The number of values must be a multiple of \a columns. An empty list clears the values and
    /// keeps \c columns. Sets \c columns without removing a binding on it, so do not bind
    /// \c columns while using this function.
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
    /// \brief Emitted when the xSpacing property changes.
    void xSpacingChanged();
    /// \brief Emitted when the ySpacing property changes.
    void ySpacingChanged();

protected:
    /// \cond INTERNAL
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    /// \endcond

    /// \brief Stores the cell values passed to the \c PlotSeries setters, keeping \c columns.
    void applyDataPoints(DataPointInput&& input) override;
    /// \brief Returns 1: the \c PlotSeries setters take one value per cell.
    int dataPointValueCount() const override;
    /// \brief Returns true: cells are inspected as records.
    bool recordInspectionSupported() const override;
    /// \brief Returns the cell record at \a index.
    InspectionRecord inspectionRecord(int index) const override;
    /// \brief Returns the record of the cell drawn at the series-local \a position.
    InspectionRecord inspectionRecordAt(const QPointF& position) const override;
    /// \brief Returns true: cells are matched by position.
    bool positionInspectionSupported() const override;
    /// \brief Returns the cell drawn at the series-local \a position as a data point at its center.
    InspectionDataPoint inspectionDataPointAt(const QPointF& position) const override;
    /// \brief Returns the grid edges in each dimension; on a log axis from the first cell edge above zero.
    DataRanges computeDataRanges() const override;
    /// \brief Repaints, because the mapping from pixels to cells depends on the axis scale.
    void onAxisScaleChanged() override;
    /// \brief Returns the span of the finite cell values.
    std::optional<ValueSpan> scanValueRange() const override;

private:
    /// \brief Grid edges. In \c edges_ an unset edge is NaN; \c resolvedExtent() resolves it to index space.
    struct Extent {
        qreal xMin;
        qreal xMax;
        qreal yMin;
        qreal yMax;

        /// \brief Returns whether all four edges are finite.
        bool isFinite() const;
    };

    Extent resolvedExtent() const;
    /// \brief Returns the columns of the grid.
    Internal::HeatmapGridDimension xDimension() const;
    /// \brief Returns the rows of the grid.
    Internal::HeatmapGridDimension yDimension() const;
    int cellCount() const;
    bool validateShape(std::size_t valueCount, int columns, int rows) const;
    /// \brief Replaces the values with \a values of \a columns per row and finishes the change.
    void storeValues(std::vector<float>&& values, int columns);
    /// \brief Sets the shape to \a columns per row and finishes the change of the cells.
    void applyShape(int columns);
    void setBound(qreal& bound, qreal value, void (HeatmapSeries::*changed)());
    void setSpacing(Spacing& current, Spacing spacing, void (HeatmapSeries::*changed)());
    /// \brief Returns the cell at series-local \a position as (row, column), or \c std::nullopt outside the grid.
    std::optional<std::pair<int, int>> cellAt(const QPointF& position) const;
    /// \brief Returns the center of the cell at \a row and \a column in data coordinates; geometric under \c Log spacing.
    QPointF cellCenter(int row, int column) const;
    /// \brief Returns whether the grid can be drawn into a window of \a window; warns once per reason it cannot.
    bool canDraw(QQuickWindow* window);
    /// \brief Writes the cell mapping, value range, and textures for the next frame into \a material.
    void updateMaterial(HeatmapMaterial& material, QQuickWindow* window, bool newNode);

    std::vector<float> values_;
    int columns_{0};
    int rows_{0};
    Extent edges_;
    Spacing xSpacing_{Spacing::Linear};
    Spacing ySpacing_{Spacing::Linear};
    // Incremented by every value change; the GPU texture holds the values of uploadedRevision_.
    std::uint64_t valuesRevision_{1};
    std::uint64_t uploadedRevision_{0};
    bool invalidLogSpacing_{false};
    bool overPrecision_{false};
};

} // namespace QAccelPlot
