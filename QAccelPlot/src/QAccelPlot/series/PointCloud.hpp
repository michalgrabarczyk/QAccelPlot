//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/effects/GradientColorTypes.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"
#include "QAccelPlot/series/PointSpatialIndex.hpp"
#include "QAccelPlot/series/SeriesMarker.hpp"

#include <QColor>
#include <QList>
#include <QMetaObject>
#include <QPointF>
#include <QPointer>

#include <vector>

namespace QAccelPlot {

/// \brief A hardware-accelerated QML item that renders large sets of unconnected 2D points as markers.
///
/// Every point is drawn as a GPU billboard with one of the marker shapes shared with \c LineCurve.
/// Points are colored uniformly with \c color, or by a per-point scalar value mapped through a
/// \c Colormap.
///
/// Positions and values are uploaded as a single data texture; the vertex buffer depends only on the
/// point count, so updating a cloud of constant size is one texture upload per frame. Methods whose
/// names end in \c F take interleaved float XY pairs <tt>[x0, y0, x1, y1, …]</tt> and an optional
/// value vector with one float per point. \c postData() may be called from any thread.
///
/// \par Invalid points
/// Non-finite coordinates, and non-positive coordinates on a logarithmic axis, are kept in storage
/// so indices stay stable, but they are not drawn, not hit-tested, and do not contribute to data ranges.
/// A point with a non-finite value is drawn with the uniform \c color.
///
/// \par Limits
/// Coordinates are single precision. The number of renderable points is bounded by the GPU's maximum
/// texture size: about 5.5 million points with values, or 8.3 million without, for an 8192-pixel
/// limit, and twice that for 16384. Points beyond it are not drawn and a warning is logged once.
///
/// \sa LineCurve, Axis, PlotSeries
class PointCloud : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(PointCloud)

    /// \brief Uniform marker color, also used by the legend. Default: \c Qt::blue.
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    /// \brief Grouped marker settings, e.g. <tt>marker.shape</tt> and <tt>marker.size</tt>. <tt>marker.shape</tt>
    /// defaults to \c Circle and does not accept \c None, because a cloud always draws markers; <tt>marker.size</tt>
    /// defaults to 3.
    Q_PROPERTY(SeriesMarker* marker READ marker CONSTANT)
    /// \brief Maps per-point values to colors. Points are colored uniformly with \c color when this
    /// is null or no values are stored. Default: null.
    Q_PROPERTY(Colormap* colormap READ colormap WRITE setColormap NOTIFY colormapChanged)
    /// \brief Whether GPU-side anti-aliasing is applied to markers. Default: \c true.
    Q_PROPERTY(bool antialiasingEnabled READ antialiasingEnabled WRITE setAntialiasingEnabled NOTIFY antialiasingEnabledChanged)
    /// \brief Anti-aliasing feather width in pixels, clamped to [0, 10]. Default: 1.
    Q_PROPERTY(qreal antialiasingFeather READ antialiasingFeather WRITE setAntialiasingFeather NOTIFY antialiasingFeatherChanged)
    /// \brief Pick radius in pixels used for hover detection. Default: 6.
    Q_PROPERTY(qreal hoverRadius READ hoverRadius WRITE setHoverRadius NOTIFY hoverRadiusChanged)
    /// \brief Read-only: number of points currently stored, including invalid ones.
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    /// \brief Read-only: \c true when per-point values are stored.
    Q_PROPERTY(bool hasValues READ hasValues NOTIFY countChanged)
    /// \brief Read-only: index of the point under the cursor, or -1 when none.
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)
    /// \brief Read-only: lower bound of the value range, taken from \c Colormap::min when it is set
    /// and from the data otherwise. Bind a color bar's scale to this and \c dataValueMax.
    Q_PROPERTY(qreal dataValueMin READ dataValueMin NOTIFY valueRangeChanged)
    /// \brief Read-only: upper bound of the value range, taken from \c Colormap::max when it is set
    /// and from the data otherwise.
    Q_PROPERTY(qreal dataValueMax READ dataValueMax NOTIFY valueRangeChanged)

public:
    /// \brief Constructs a PointCloud with the given \a parent.
    explicit PointCloud(QQuickItem* parent = nullptr);

    /// \brief Returns the uniform marker color.
    QColor color() const;
    /// \brief Sets the uniform marker color to \a color.
    void setColor(const QColor& color);

    /// \brief Returns the grouped marker settings. The object is owned by the cloud.
    SeriesMarker* marker() const;

    /// \brief Returns the colormap, or \c nullptr when points are colored uniformly.
    Colormap* colormap() const;
    /// \brief Sets the colormap to \a colormap. Pass \c nullptr to color every point with \c color.
    void setColormap(Colormap* colormap);

    /// \brief Returns \c true when GPU anti-aliasing is enabled.
    bool antialiasingEnabled() const;
    /// \brief Sets anti-aliasing to \a enabled.
    void setAntialiasingEnabled(bool enabled);

    /// \brief Returns the anti-aliasing feather width.
    qreal antialiasingFeather() const;
    /// \brief Sets the anti-aliasing feather width to \a feather pixels.
    void setAntialiasingFeather(qreal feather);

    /// \brief Returns the hover pick radius in pixels.
    qreal hoverRadius() const;
    /// \brief Sets the hover pick radius to \a radius pixels. Negative values are clamped to 0.
    void setHoverRadius(qreal radius);

    /// \brief Returns the number of stored points.
    int count() const;
    /// \brief Returns \c true when per-point values are stored.
    bool hasValues() const;
    /// \brief Returns the index of the hovered point, or -1.
    int hoveredIndex() const;
    /// \brief Returns the resolved lower bound of the value range.
    qreal dataValueMin() const;
    /// \brief Returns the resolved upper bound of the value range.
    qreal dataValueMax() const;

    /// \brief Replaces all points with \a points and clears per-point values.
    Q_INVOKABLE void setData(const QList<QPointF>& points);
    /// \brief Sets one value per point. An empty list clears values; any other size must equal \c count.
    Q_INVOKABLE void setValues(const QList<qreal>& values);
    /// \brief Removes all points and values.
    Q_INVOKABLE void clearData();
    /// \brief Returns point \a index, or a NaN point when \a index is out of range.
    Q_INVOKABLE QPointF pointAt(int index) const;
    /// \brief Returns the value of point \a index, or NaN when out of range or no values are stored.
    Q_INVOKABLE qreal valueAt(int index) const;

    /// \brief Copies \a pointCount interleaved XY pairs from \a xyInterleaved and clears values.
    void setDataF(const float* xyInterleaved, int pointCount);
    /// \brief Moves \a xyInterleaved (\a pointCount XY pairs) into the cloud and clears values. No copy is made.
    void setDataF(std::vector<float>&& xyInterleaved, int pointCount);
    /// \brief Sets positions and per-point \a values (empty, or exactly \a pointCount floats).
    void setDataF(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    /// \brief Like \c setDataF() but does not report X/Y data ranges to the axes.
    ///
    /// Use it for streaming when the axes' \c dataMin / \c dataMax are managed by the application.
    void setDataFNoRange(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    /// \brief Thread-safe: queues \c setDataF(\a xyInterleaved, \a pointCount) to the item's thread.
    void postData(std::vector<float>&& xyInterleaved, int pointCount);
    /// \brief Thread-safe: queues \c setDataF(\a xyInterleaved, \a values, \a pointCount) to the item's thread.
    void postData(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount);

    /// \brief Moves \a pointCount interleaved XY pairs of doubles into the cloud and clears values.
    ///
    /// The GPU renders in single precision, so positions are uploaded relative to an origin
    /// taken from the first finite point. Coordinates far from zero, such as epoch timestamps,
    /// therefore keep their resolution. Logarithmic dimensions are never origin-shifted.
    void setData(std::vector<double>&& xyInterleaved, int pointCount);
    /// \brief Sets double-precision positions and per-point \a values (empty, or exactly \a pointCount floats).
    void setData(std::vector<double>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    /// \brief Like the double \c setData() but does not report X/Y data ranges to the axes.
    void setDataNoRange(std::vector<double>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    /// \brief Thread-safe: queues \c setData(\a xyInterleaved, \a pointCount) to the item's thread.
    void postData(std::vector<double>&& xyInterleaved, int pointCount);
    /// \brief Thread-safe: queues \c setData(\a xyInterleaved, \a values, \a pointCount) to the item's thread.
    void postData(std::vector<double>&& xyInterleaved, std::vector<float>&& values, int pointCount);

    /// \brief Returns the index of the valid point within \c hoverRadius of item position \a position, or -1.
    int pointIndexAt(const QPointF& position) const;
    /// \brief Returns \c true when a valid point lies within \c hoverRadius of item position \a point.
    ///
    /// Hover delivery uses this test, so stacked series underneath still receive hover events
    /// away from this cloud's points.
    bool contains(const QPointF& point) const override;

signals:
    /// \brief Emitted when the color property changes.
    void colorChanged();
    /// \brief Emitted when the colormap property changes.
    void colormapChanged();
    /// \brief Emitted when the antialiasingEnabled property changes.
    void antialiasingEnabledChanged();
    /// \brief Emitted when the antialiasingFeather property changes.
    void antialiasingFeatherChanged();
    /// \brief Emitted when the hoverRadius property changes.
    void hoverRadiusChanged();
    /// \brief Emitted when the number of points or the presence of values changes.
    void countChanged();
    /// \brief Emitted when the hovered point changes.
    void hoveredIndexChanged();
    /// \brief Emitted when \c dataValueMin or \c dataValueMax changes.
    void valueRangeChanged();

protected:
    /// \cond INTERNAL
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    /// \endcond

protected:
    /// \brief Rebuilds the origin-relative upload buffer, because log dimensions are not shifted.
    void onAxisScaleChanged() override;

private:
    Q_SLOT void onColormapUpdated();

    bool validateDataArguments(std::size_t xyFloatCount, std::size_t valueCount, int pointCount) const;
    void applyData(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount, bool reportRanges);
    void storeInterleaved(std::vector<float>&& xyInterleaved, const std::vector<float>& values, int pointCount);
    void finishDataChange(int previousCount, bool hadValues, bool reportRanges);
    void updateDataRanges();
    void updateValueRange();
    void reconnectAxisSignals();
    void reconnectColormapSignals();
    void refreshColorStops();
    void setHoveredIndex(int index);
    int stride() const;
    void ensureSpatialIndex() const;
    void applyDoubleData(std::vector<double>&& xyInterleaved, std::vector<float>&& values, int pointCount, bool reportRanges);
    void rebuildRenderData();
    bool hasPreciseData() const;

    QColor color_{Qt::blue};
    SeriesMarker* marker_{new SeriesMarker{MarkerShape::Circle, 3.0, SeriesMarker::NoneShape::Rejected, this}};
    QPointer<Colormap> colormap_;
    std::vector<GradientStopData> colorStops_;
    bool antialiasingEnabled_{true};
    qreal antialiasingFeather_{1.0};
    qreal hoverRadius_{6.0};
    int hoveredIndex_{-1};

    // Precise interleaved (x, y) pairs, populated only by the double setData() overloads.
    // Empty when the float *F APIs supplied the data, which is already single precision.
    std::vector<double> dataD_;
    // Per-point values kept alongside dataD_, so the upload buffer can be rebuilt when an
    // axis switches scale without the caller resupplying them.
    std::vector<float> valuesF_;
    // Origin subtracted from dataD_ when building data_, so the float upload keeps its
    // resolution near the data. Zero for a dimension that is unshifted or logarithmic.
    qreal renderOriginX_{0.0};
    qreal renderOriginY_{0.0};
    bool renderLogScaleX_{false};
    bool renderLogScaleY_{false};

    // Interleaved storage: (x, y) per point, or (x, y, value) when hasValues_ is true.
    std::vector<float> data_;
    int pointCount_{0};
    bool hasValues_{false};
    qreal dataValueMin_{0.0};
    qreal dataValueMax_{1.0};

    // True when the data texture must be re-uploaded on the next paint.
    bool dataChanged_{true};

    mutable PointSpatialIndex spatialIndex_;
    mutable bool spatialIndexValid_{false};

    QList<QMetaObject::Connection> axisConnections_;
    QList<QMetaObject::Connection> gradientConnections_;
};

} // namespace QAccelPlot
