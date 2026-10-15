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
#include "QAccelPlot/theme/ColorPalette.hpp"
#include "QAccelPlot/transitions/DataTransition.hpp"

#include <QColor>
#include <QList>
#include <QMetaObject>
#include <QPointF>
#include <QPointer>

#include <limits>
#include <memory>
#include <optional>
#include <vector>

namespace QAccelPlot {

namespace Internal {
class HoverIndexBudget;
}

class TransitionRunner;

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
/// \par Transitions
/// Assign a \c MorphTransition or \c DrawTransition to \c transition to animate data updates.
/// With \c MorphTransition, points move to their new position, and their values, when the old and
/// the new points both have them, change with them. New points appear, and removed points
/// disappear, at once. \c count, \c pointAt(), \c valueAt(), the data and value ranges, and
/// inspection use the new data from the start of the animation, while hover picks the points as
/// drawn. While a transition is enabled, \c setValues() animates as well and the \c setDataF()
/// overloads store doubles. \c clearData(), and replacing, clearing, or cancelling \c transition,
/// skip to the new data.
///
/// \par Limits
/// Coordinates are single precision. The number of renderable points is bounded by the GPU's maximum
/// texture size: about 22 million points with values, or 33 million without, for an 8192-pixel
/// limit, and twice that for 16384. Points beyond it are not drawn or hovered, and a warning is logged
/// once.
///
/// \sa LineCurve, Axis, PlotSeries, ColorBar, MorphTransition, DrawTransition
class PointCloud : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(PointCloud)

    /// \brief Uniform marker color, also used by the legend. Default: \c Colors.dark.seriesPrimary.
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
    /// \brief Optional data transition animation applied when new data arrives.
    Q_PROPERTY(DataTransition* transition READ transition WRITE setTransition NOTIFY transitionChanged)
    /// \brief Read-only: number of points currently stored, including invalid ones.
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    /// \brief Read-only: \c true when per-point values are stored.
    Q_PROPERTY(bool hasValues READ hasValues NOTIFY countChanged)
    /// \brief Read-only: index of the point under the cursor, or -1 when none.
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)
    /// \brief Read-only: lower bound of the value range, taken from \c Colormap::min when it is set
    /// and from the data otherwise. \c ColorBar labels its ticks from this and \c dataValueMax.
    Q_PROPERTY(qreal dataValueMin READ dataValueMin NOTIFY valueRangeChanged)
    /// \brief Read-only: upper bound of the value range, taken from \c Colormap::max when it is set
    /// and from the data otherwise.
    Q_PROPERTY(qreal dataValueMax READ dataValueMax NOTIFY valueRangeChanged)

public:
    /// \brief Constructs a PointCloud with the given \a parent.
    explicit PointCloud(QQuickItem* parent = nullptr);
    /// \brief Destroys the point cloud, ending its animation on the assigned transition.
    ~PointCloud() override;

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

    /// \brief Returns the active data transition, or \c nullptr if none.
    DataTransition* transition() const;
    /// \brief Sets the data transition to \a transition.
    void setTransition(DataTransition* transition);

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
    /// \brief Returns point \a index, or a NaN point when \a index is out of range.
    Q_INVOKABLE QPointF pointAt(int index) const;
    /// \brief Returns the value of point \a index, or NaN when out of range or no values are stored.
    Q_INVOKABLE qreal valueAt(int index) const;

    /// \brief Sets double-precision positions and per-point \a values (empty, or exactly \a pointCount floats).
    void setData(std::vector<double>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    /// \brief Sets positions and per-point \a values (empty, or exactly \a pointCount floats).
    void setDataF(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    using PlotSeries::postData;
    using PlotSeries::setData;
    using PlotSeries::setDataF;
    /// \brief Thread-safe: queues \c setData(\a xyInterleaved, \a values, \a pointCount) to the item's thread.
    void postData(std::vector<double>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    /// \brief Thread-safe: queues \c setDataF(\a xyInterleaved, \a values, \a pointCount) to the item's thread.
    void postData(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    /// \brief Removes all points and values.
    Q_INVOKABLE void clearData() override;

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
    /// \brief Emitted when the transition property changes.
    void transitionChanged();
    /// \brief Emitted when the number of points or the presence of values changes.
    void countChanged();
    /// \brief Emitted when the hovered point changes.
    void hoveredIndexChanged();
    /// \brief Emitted when \c dataValueMin or \c dataValueMax changes.
    void valueRangeChanged();

protected:
    /// \cond INTERNAL
    InspectionSource inspectionSource() const override;
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    /// \endcond

protected:
    /// \brief Applies new points without values, animating to them when a transition is enabled.
    void applyDataPoints(DataPointInput&& input) override;
    /// \brief Rebuilds the hover index, because log dimensions map points differently.
    void onAxisScaleChanged() override;
    /// \brief Refreshes the hover index, the value range, and the count after the points changed.
    void onDataPointsChanged(DataChange change, int previousCount) override;

private:
    Q_SLOT void onColormapUpdated();

    /// \brief Points of \c stride doubles each: (x, y), or (x, y, value).
    struct Points {
        std::vector<double> values;
        int count{0};
        int stride{2};
    };

    bool validateValues(std::size_t valueCount, int pointCount) const;
    void applyData(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    /// \brief Replaces the points with float data, without a transition.
    void applyFloatData(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    void applyDoubleData(std::vector<double>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    /// \brief Finishes a change of the values alone, which leaves the data ranges as they are.
    void finishValueChange(int previousCount);
    void updateValueRange();
    void reconnectColormapSignals();
    void refreshColorStops();
    void setHoveredIndex(int index);
    /// \brief Returns the number of points that take part in hover: those the data texture holds.
    int hoverPointCount() const;
    void invalidateSpatialIndex();
    /// \brief Returns \c true when the spatial index answers the next query, building it once scanning has cost as much.
    bool spatialIndexReady(PointSpatialIndex::Mapping mapping) const;

    /// \brief A hover query in the spatial index's coordinates. With the data, it decides the answer.
    struct HoverQuery {
        double x;
        double y;
        double radiusX;
        double radiusY;
        PointSpatialIndex::Mapping mapping;

        bool operator==(const HoverQuery& other) const;
    };

    int nearestPoint(const HoverQuery& query) const;
    /// \brief Replaces the values, animating from the drawn points.
    void animateValues(std::vector<float>&& values);
    /// \brief Returns the data as one point of doubles per stride: (x, y), or (x, y, value).
    std::vector<double> interleavedData() const;
    /// \brief Returns the points drawn now: the transition points, or the data.
    Points drawnPoints() const;
    /// \brief Animates from \a drawn to the data. A point without a counterpart starts at its new position.
    void startTransition(Points&& drawn);
    void advanceTransition();
    /// \brief Shows the data instead of the transition points.
    void endTransition();
    /// \brief Ends the run and drops the transition points, without redrawing.
    void discardTransition();

    QColor color_{ColorPalette::dark().seriesPrimary};
    SeriesMarker* marker_{new SeriesMarker{MarkerShape::Circle, 3.0, SeriesMarker::NoneShape::Rejected, this}};
    QPointer<Colormap> colormap_;
    std::vector<GradientStopData> colorStops_;
    bool antialiasingEnabled_{true};
    qreal antialiasingFeather_{1.0};
    qreal hoverRadius_{6.0};
    int hoveredIndex_{-1};
    std::unique_ptr<TransitionRunner> transition_;
    // The previous transition frame, reused for the next one.
    std::vector<double> frameBuffer_;

    // hasValues() as countChanged() last reported it.
    bool reportedHasValues_{false};
    qreal dataValueMin_{0.0};
    qreal dataValueMax_{1.0};
    // Render revision of the data points last uploaded to the GPU.
    std::uint64_t uploadedRevision_{0};
    // Whether the last frame held more records than the GPU buffers do; warns on each crossing.
    bool overCapacity_{false};

    // Points the GPU data texture holds, recorded during the last sync. Points beyond it are
    // neither drawn nor hovered.
    int renderCapacity_{std::numeric_limits<int>::max()};

    mutable PointSpatialIndex spatialIndex_;
    mutable bool spatialIndexValid_{false};
    // Render revision of the data points the spatial index holds.
    mutable std::uint64_t indexedRevision_{0};
    std::unique_ptr<Internal::HoverIndexBudget> spatialIndexBudget_;
    // The last hover query and its answer. Qt tests contains() several times per hover event
    // before the handler asks again, each time at the same position.
    mutable std::optional<HoverQuery> lastHoverQuery_;
    mutable int lastHoverIndex_{-1};

    QList<QMetaObject::Connection> gradientConnections_;
};

} // namespace QAccelPlot
