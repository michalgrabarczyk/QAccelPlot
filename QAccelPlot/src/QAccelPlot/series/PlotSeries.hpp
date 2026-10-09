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
#include <vector>

namespace QAccelPlot {

/// \brief Common QML item contract for data series hosted by \c PlotView.
///
/// PlotSeries owns the integration shared by every plot type: axis bindings,
/// plot-area layout, data-range reporting, legend metadata, and a common C++ data-setting API.
/// Concrete series define their record layout, rendering, and hit testing.
///
/// \sa LineCurve, PointCloud, RectangleSeries, QAccelPlot
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

    /// \brief Replaces the series data with \a count records copied from an interleaved double array.
    /// Each concrete series defines its record layout (XY pairs or rectangle edges).
    virtual void setData(const double* data, int count) = 0;
    /// \brief Replaces the series data by moving an interleaved double buffer.
    virtual void setData(std::vector<double>&& data, int count) = 0;
    /// \brief Replaces the series data with \a count records copied from an interleaved float array.
    virtual void setDataF(const float* data, int count) = 0;
    /// \brief Replaces the series data by moving an interleaved float buffer.
    virtual void setDataF(std::vector<float>&& data, int count) = 0;
    /// \brief Like \c setData(\a data, \a count), with the data extents given as \a bounds instead of scanned for.
    void setData(const double* data, int count, const DataBounds& bounds);
    /// \brief Like \c setData(\a data, \a count), with the data extents given as \a bounds instead of scanned for.
    void setData(std::vector<double>&& data, int count, const DataBounds& bounds);
    /// \brief Like \c setDataF(\a data, \a count), with the data extents given as \a bounds instead of scanned for.
    void setDataF(const float* data, int count, const DataBounds& bounds);
    /// \brief Like \c setDataF(\a data, \a count), with the data extents given as \a bounds instead of scanned for.
    void setDataF(std::vector<float>&& data, int count, const DataBounds& bounds);
    /// \brief Queues a moved double buffer for assignment on the series' thread.
    virtual void postData(std::vector<double>&& data, int count) = 0;
    /// \brief Queues a moved float buffer for assignment on the series' thread.
    virtual void postData(std::vector<float>&& data, int count) = 0;
    /// \brief Removes all records from the series.
    virtual void clearData() = 0;

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
    /// \brief How the records changed in a data update.
    enum class DataChange {
        Replaced, ///< \brief Any record may have changed.
        Appended, ///< \brief One record was added at the end; all others are unchanged.
    };

    /// \brief Advances the data revision and refreshes the data queries. Call after every accepted record change.
    void inspectionDataChanged(DataChange change = DataChange::Replaced);
    /// \brief Refreshes the data queries after record validity changed without a data change, such as an axis scale switch.
    void invalidateInspection();
    /// \brief Returns a view of the XY records that sample queries search. The default has none.
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

    /// \brief Scans the records for their extents. The default implementation has none.
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
    /// Log scale changes which samples are valid, so series that apply the invalid-sample contract
    /// override this to refresh ranges and cached geometry. The default implementation does nothing.
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
};

} // namespace QAccelPlot
