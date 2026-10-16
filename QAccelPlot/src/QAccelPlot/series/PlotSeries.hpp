//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/inspection/SeriesInspection.hpp"
#include "QAccelPlot/series/DataPointStore.hpp"

#include <QPointer>
#include <QQuickItem>
#include <QRectF>
#include <QString>
#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace QAccelPlot {

/// \brief Common QML item contract for data series hosted by \c PlotView.
///
/// PlotSeries owns the integration shared by every plot type: axis bindings,
/// plot-area layout, data-range reporting, legend metadata, and a common C++ data-setting API
/// backed by the series' data points. Concrete series define their data point layout, rendering,
/// and hit testing.
///
/// \c setData(), \c setDataF(), and \c postData() take interleaved values, one data point after
/// another, in the layout of the series:
///
/// | Series             | Data point        | Values |
/// | ------------------ | ----------------- | ------ |
/// | \c LineCurve       | (x, y)            | 2      |
/// | \c PointCloud      | (x, y)            | 2      |
/// | \c RectangleSeries | (x1, y1, x2, y2)  | 4      |
/// | \c BarSeries       | (position, value) | 2      |
/// | \c BandSeries      | (x, low, high)    | 3      |
/// | \c HeatmapSeries   | cell value        | 1      |
///
/// The buffer holds \a count times that many values. A \c HeatmapSeries takes its cells row by row,
/// \c columns per row. Series-specific setters take other layouts, such as point values,
/// categories, or ranged bars.
///
/// \sa LineCurve, PointCloud, RectangleSeries, BarSeries, BandSeries, HeatmapSeries, QAccelPlot
class PlotSeries : public QQuickItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(PlotSeries)
    QML_UNCREATABLE("PlotSeries is a base class for concrete plot series.")

    /// \brief Identifying name used by the default legend.
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    /// \brief Horizontal axis used for data-to-pixel coordinate mapping.
    Q_PROPERTY(Axis* xAxis READ xAxis WRITE setXAxis NOTIFY xAxisChanged)
    /// \brief Vertical axis used for data-to-pixel coordinate mapping.
    Q_PROPERTY(Axis* yAxis READ yAxis WRITE setYAxis NOTIFY yAxisChanged)
    /// \brief Plot area in parent-item coordinates, assigned by \c PlotView.
    Q_PROPERTY(QRectF plotRect READ plotRect WRITE setPlotRect NOTIFY plotRectChanged)
    /// \brief Symbol style requested from the default legend.
    Q_PROPERTY(LegendSymbol legendSymbol READ legendSymbol WRITE setLegendSymbol NOTIFY legendSymbolChanged)

    /// \brief Revision incremented by every accepted change of the series' records.
    Q_PROPERTY(quint64 dataRevision READ dataRevision NOTIFY dataRevisionChanged)
    /// \brief Read-only constant: data queries for this series.
    Q_PROPERTY(::QAccelPlot::SeriesInspection* inspection READ inspection CONSTANT)

public:
    /// \brief Supported default legend symbols.
    ///
    /// \c Line draws the series line style and marker, \c Fill a filled swatch, and \c Marker only
    /// the series marker shape (used by unconnected series such as \c PointCloud).
    enum class LegendSymbol { Line, Fill, Marker };
    Q_ENUM(LegendSymbol)

    /// \brief Marker shapes shared by every series that draws markers.
    ///
    /// Every shape except \c Pixel fits within a square whose half-width is the marker size. The shaders
    /// select a shape by this value minus one, so append new shapes at the end and never reorder
    /// or insert. Series that always draw markers, such as \c PointCloud, do not accept \c None.
    enum class MarkerShape {
        None,          ///< \brief No markers.
        Circle,        ///< \brief Circle.
        Square,        ///< \brief Square.
        Diamond,       ///< \brief Diamond, narrower than it is tall.
        TriangleUp,    ///< \brief Equilateral triangle pointing up.
        TriangleDown,  ///< \brief Equilateral triangle pointing down.
        TriangleLeft,  ///< \brief Equilateral triangle pointing left.
        TriangleRight, ///< \brief Equilateral triangle pointing right.
        Cross,         ///< \brief Plus sign (+).
        XCross,        ///< \brief Diagonal cross (×).
        HLine,         ///< \brief Short horizontal line.
        VLine,         ///< \brief Short vertical line, e.g. for rug and event plots.
        Star,          ///< \brief Five-pointed star.
        Asterisk,      ///< \brief Eight-armed asterisk: a thin plus and a thin diagonal cross.
        Pixel,         ///< \brief A single pixel; ignores the marker size, fill, and anti-aliasing. Suited to very dense scatter plots.
        Hexagon,       ///< \brief Regular hexagon with a vertex up.
        Pentagon       ///< \brief Regular pentagon with a vertex up.
    };
    Q_ENUM(MarkerShape)

    /// \brief Extent of the valid coordinates in one dimension.
    struct DataExtent {
        qreal min; ///< \brief Smallest valid coordinate.
        qreal max; ///< \brief Largest valid coordinate.
    };

    /// \brief Extents of a data update that the caller already knows.
    ///
    /// Passed with the data, they are reported to the axes in place of a scan over the records.
    /// A dimension whose bounds are not finite or not ordered reports no extent.
    struct DataBounds {
        qreal xMin; ///< \brief Smallest value on the horizontal axis.
        qreal xMax; ///< \brief Largest value on the horizontal axis.
        qreal yMin; ///< \brief Smallest value on the vertical axis.
        qreal yMax; ///< \brief Largest value on the vertical axis.
    };

    explicit PlotSeries(QQuickItem* parent = nullptr);
    ~PlotSeries() override;

    /// \brief Returns the current data revision.
    quint64 dataRevision() const;
    /// \brief Returns the data queries for this series; created on first use and owned by the series.
    SeriesInspection* inspection() const;

    QString name() const;
    void setName(const QString& name);

    Axis* xAxis() const;
    void setXAxis(Axis* axis);

    Axis* yAxis() const;
    void setYAxis(Axis* axis);

    QRectF plotRect() const;
    /// \brief Updates the series geometry to exactly cover \a rect.
    void setPlotRect(const QRectF& rect);

    LegendSymbol legendSymbol() const;
    void setLegendSymbol(LegendSymbol symbol);

    /// \brief Replaces the series data with \a count data points copied from an interleaved double array.
    ///
    /// See the class description for the data point layout of each series.
    void setData(const double* data, int count);
    /// \brief Replaces the series data by moving an interleaved double buffer.
    void setData(std::vector<double>&& data, int count);
    /// \brief Replaces the series data with \a count data points copied from an interleaved float array.
    void setDataF(const float* data, int count);
    /// \brief Replaces the series data by moving an interleaved float buffer.
    void setDataF(std::vector<float>&& data, int count);
    /// \brief Like \c setData(\a data, \a count), with the data extents given as \a bounds instead of scanned for.
    void setData(const double* data, int count, const DataBounds& bounds);
    /// \brief Like \c setData(\a data, \a count), with the data extents given as \a bounds instead of scanned for.
    void setData(std::vector<double>&& data, int count, const DataBounds& bounds);
    /// \brief Like \c setDataF(\a data, \a count), with the data extents given as \a bounds instead of scanned for.
    void setDataF(const float* data, int count, const DataBounds& bounds);
    /// \brief Like \c setDataF(\a data, \a count), with the data extents given as \a bounds instead of scanned for.
    void setDataF(std::vector<float>&& data, int count, const DataBounds& bounds);
    /// \brief Queues a moved double buffer for assignment on the series' thread.
    void postData(std::vector<double>&& data, int count);
    /// \brief Queues a moved float buffer for assignment on the series' thread.
    void postData(std::vector<float>&& data, int count);
    /// \brief Removes all data points from the series.
    virtual void clearData();

    /// \brief Returns the extent this series reports to its horizontal axis, or \c std::nullopt when it has none.
    ///
    /// The first read after a data update scans the records, unless the update came with \c DataBounds.
    std::optional<DataExtent> xDataRange() const;
    /// \brief Returns the extent this series reports to its vertical axis, or \c std::nullopt when it has none.
    std::optional<DataExtent> yDataRange() const;

signals:
    /// \brief Emitted when the dataRevision property changes.
    void dataRevisionChanged();
    void nameChanged();
    void xAxisChanged();
    void yAxisChanged();
    void plotRectChanged();
    void legendSymbolChanged();

protected:
    /// \brief Words used in the warnings about rejected data arguments.
    struct DataPointWording {
        std::string countName;  ///< \brief Name of the count argument, such as "data point" in "data point count".
        std::string plural;     ///< \brief Plural of a data point, such as "points".
        std::string doubleUnit; ///< \brief Plural of one value of double data, such as "doubles".
        std::string floatUnit;  ///< \brief Plural of one value of float data, such as "floats".
    };

    /// \brief Creates a series whose data points have \a layout; \a wording names them in warnings.
    PlotSeries(DataPointStore::Layout layout, DataPointWording wording, QQuickItem* parent);

    /// \brief Data points passed to \c setData() or \c setDataF(), already checked.
    ///
    /// Holds either a moved buffer or an array to copy, of double or float values.
    struct DataPointInput {
        std::vector<double> doubles;        ///< \brief Moved double values, when \c precise is set and \c doubleArray is null.
        std::vector<float> floats;          ///< \brief Moved float values, when \c precise is not set and \c floatArray is null.
        const double* doubleArray{nullptr}; ///< \brief Double values to copy.
        const float* floatArray{nullptr};   ///< \brief Float values to copy.
        int count{0};                       ///< \brief Number of data points.
        int stride{0};                      ///< \brief Values per data point.
        bool precise{true};                 ///< \brief Whether the values are doubles.

        /// \brief Returns the values as doubles, moved, copied, or converted.
        std::vector<double> toDoubles() &&;
    };

    /// \brief Applies data points passed to \c setData() or \c setDataF().
    ///
    /// The default stores them; series that animate data changes override it.
    virtual void applyDataPoints(DataPointInput&& input);
    /// \brief Stores \a input as the data points and finishes the change.
    void storeDataPoints(DataPointInput&& input);
    /// \brief Returns the number of values per data point that \c setData() and \c setDataF() take.
    ///
    /// The default is one per coordinate column.
    virtual int dataPointValueCount() const;

    /// \brief How the records changed in a data update.
    enum class DataChange {
        Replaced, ///< \brief Any record may have changed.
        Appended, ///< \brief One record was added at the end; all others are unchanged.
    };

    /// \brief Returns the data points of the series.
    DataPointStore& dataPoints();
    /// \brief Returns the data points of the series.
    const DataPointStore& dataPoints() const;
    /// \brief Returns whether the horizontal axis is logarithmic; false without an axis.
    bool logScaleX() const;
    /// \brief Returns whether the vertical axis is logarithmic; false without an axis.
    bool logScaleY() const;
    /// \brief Returns whether \a data and \a count are valid arguments for copying data points; warns otherwise.
    bool validateRawDataArguments(const void* data, int count) const;
    /// \brief Returns whether \a valueCount values hold \a count data points; warns otherwise.
    ///
    /// A data point has \a valuesPerDataPoint values, or one per coordinate column when it is 0.
    bool validateDataSize(std::size_t valueCount, int count, bool floatData, int valuesPerDataPoint = 0) const;
    /// \brief Appends one double data point and widens the data ranges by its valid coordinates.
    void appendDataPoint(const double* coordinates);
    /// \brief Finishes a change of the data points: data ranges, \c onDataPointsChanged(), data queries, and a repaint.
    void notifyDataPointsChanged(DataChange change, int previousCount);
    /// \brief Called after the data points changed, before the data queries are refreshed. The default does nothing.
    virtual void onDataPointsChanged(DataChange change, int previousCount);

    /// \brief Advances the data revision and refreshes the data queries. Call after every accepted record change.
    void inspectionDataChanged(DataChange change = DataChange::Replaced);
    /// \brief Refreshes the data queries after record validity changed without a data change, such as an axis scale switch.
    void invalidateInspection();
    /// \brief Returns a view of the XY records that data point queries search. The default has none.
    virtual InspectionSource inspectionSource() const;
    /// \brief Returns false while the records are ambiguous, such as during a data transition. Default: true.
    virtual bool inspectionAvailable() const;
    /// \brief Returns the native record at \a index for series that are not plain XY series. Default: unsupported.
    virtual InspectionRecord inspectionRecord(int index) const;
    /// \brief Returns the native record drawn at the series-local \a position. Default: unsupported.
    virtual InspectionRecord inspectionRecordAt(const QPointF& position) const;

    /// \brief Extents of a series in both dimensions.
    ///
    /// A dimension without a valid coordinate is unset; an extent that is not finite or not ordered counts as unset.
    struct DataRanges {
        std::optional<DataExtent> x; ///< \brief Extent reported to the horizontal axis.
        std::optional<DataExtent> y; ///< \brief Extent reported to the vertical axis.
    };

    /// \brief Scans the records for their extents. The default implementation scans the data points.
    ///
    /// Called when a data range is read after \c invalidateDataRanges(), so a series that nothing asks
    /// for its range never scans.
    virtual DataRanges computeDataRanges() const;
    /// \brief Discards the cached extents and notifies the bound axes.
    ///
    /// Call after every change of the records, or of anything else \c computeDataRanges() depends on.
    void invalidateDataRanges();
    /// \brief Widens the X extent to include \a x and notifies the bound axes.
    ///
    /// Lets an append-style ingestion path keep computed extents current in O(1) instead of
    /// rescanning the whole buffer. A non-finite \a x leaves the extent unchanged.
    void extendXDataRange(qreal x);
    /// \brief Widens the Y extent to include \a y. A non-finite \a y leaves the extent unchanged.
    void extendYDataRange(qreal y);
    /// \brief Called when a bound axis switches between linear and logarithmic scale, or a different axis is bound.
    ///
    /// Log scale changes which data points are valid. The data ranges of a changed scale are already invalidated; series
    /// override this to refresh cached geometry. The default implementation does nothing.
    virtual void onAxisScaleChanged();
    /// \brief Called when the viewport of a bound axis changes. The default implementation schedules a repaint.
    virtual void onAxisRangeChanged();
    /// \brief Returns the plot area to render into: \c plotRect when set, otherwise the item's current size.
    ///
    /// Safe to call from \c updatePaintNode(); it never modifies the item.
    QRectF resolvePlotRect() const;
    /// \brief Withholds hover events from a series beneath another series under the cursor.
    ///
    /// Series ignore hover events so that the plot receives them too. Qt Quick 6.3 and newer stop
    /// at the topmost hovered item anyway; older versions also deliver the event to the series
    /// beneath, which then see a hover leave instead.
    bool event(QEvent* event) override;

private:
    friend class SeriesInspection;

    void handleAxisScaleChanged();
    void handleAxisRangeChanged();
    // Returns whether the scale of a dimension changed.
    bool syncDataPointDimensions();
    void ensureDataRanges() const;
    void reportDataRangesChanged() const;
    void deliverTopmostHover(QHoverEvent* event);
    bool coveredBySeriesAbove(const QPointF& position) const;

    bool hoverDelivered_{false};
    quint64 dataRevision_{0};
    mutable SeriesInspection* inspection_{nullptr};
    QString name_;
    QPointer<Axis> xAxis_;
    QPointer<Axis> yAxis_;
    QMetaObject::Connection xAxisDestroyed_;
    QMetaObject::Connection yAxisDestroyed_;
    QRectF plotRect_;
    LegendSymbol legendSymbol_{LegendSymbol::Line};
    // The extents are a cache of a scan over the records, refreshed by const readers.
    mutable std::optional<DataExtent> xDataExtent_;
    mutable std::optional<DataExtent> yDataExtent_;
    mutable bool dataRangesStale_{false};
    // Bounds given with the data update in progress; they replace its scan.
    std::optional<DataBounds> updateBounds_;
    DataPointStore dataPoints_;
    DataPointWording wording_;
};

} // namespace QAccelPlot
