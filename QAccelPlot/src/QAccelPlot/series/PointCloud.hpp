//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/GradientColorTypes.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"
#include "QAccelPlot/series/PointSpatialIndex.hpp"

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
/// Points are colored uniformly with \c color, or by a per-point scalar value mapped through
/// \c colorGradient when \c colorMode is \c ValueColor.
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
    /// \brief Marker shape. Default: \c Circle.
    Q_PROPERTY(MarkerShape markerShape READ markerShape WRITE setMarkerShape NOTIFY markerShapeChanged)
    /// \brief Marker radius in pixels. Default: 3.
    Q_PROPERTY(qreal markerSize READ markerSize WRITE setMarkerSize NOTIFY markerSizeChanged)
    /// \brief Whether closed marker shapes are filled. When \c false they are drawn as outlines of \c markerStrokeWidth
    /// inside the shape's edge. Line-like shapes (\c Cross, \c XCross, \c Asterisk, \c HLine, \c VLine) and \c Pixel
    /// are unaffected. Default: \c true.
    Q_PROPERTY(bool markerFilled READ markerFilled WRITE setMarkerFilled NOTIFY markerFilledChanged)
    /// \brief Outline width in pixels of hollow markers. Has effect only when \c markerFilled is \c false. Default: 1.
    Q_PROPERTY(qreal markerStrokeWidth READ markerStrokeWidth WRITE setMarkerStrokeWidth NOTIFY markerStrokeWidthChanged)
    /// \brief Coloring mode. Default: \c UniformColor.
    Q_PROPERTY(ColorMode colorMode READ colorMode WRITE setColorMode NOTIFY colorModeChanged)
    /// \brief QML \c Gradient used as the colormap in \c ValueColor mode. Its orientation is ignored.
    Q_PROPERTY(QObject* colorGradient READ colorGradient WRITE setColorGradient NOTIFY colorGradientChanged)
    /// \brief Whether \c valueMin is taken from the data or from the \c valueMin property. Default: \c DataRange.
    Q_PROPERTY(GradientValueSource valueMinSource READ valueMinSource WRITE setValueMinSource NOTIFY valueMinSourceChanged)
    /// \brief Value mapped to the start of the colormap when \c valueMinSource is \c Fixed. Default: 0.
    Q_PROPERTY(qreal valueMin READ valueMin WRITE setValueMin NOTIFY valueMinChanged)
    /// \brief Whether \c valueMax is taken from the data or from the \c valueMax property. Default: \c DataRange.
    Q_PROPERTY(GradientValueSource valueMaxSource READ valueMaxSource WRITE setValueMaxSource NOTIFY valueMaxSourceChanged)
    /// \brief Value mapped to the end of the colormap when \c valueMaxSource is \c Fixed. Default: 1.
    Q_PROPERTY(qreal valueMax READ valueMax WRITE setValueMax NOTIFY valueMaxChanged)
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
    /// \brief Read-only: resolved value mapped to the start of the colormap.
    Q_PROPERTY(qreal dataValueMin READ dataValueMin NOTIFY valueRangeChanged)
    /// \brief Read-only: resolved value mapped to the end of the colormap.
    Q_PROPERTY(qreal dataValueMax READ dataValueMax NOTIFY valueRangeChanged)

public:
    /// \brief Marker shapes. Values match \c LineCurve::PointShape.
    enum class MarkerShape {
        Circle = 1,    ///< \brief Circle.
        Square,        ///< \brief Square.
        Diamond,       ///< \brief Diamond, narrower than it is tall.
        TriangleUp,    ///< \brief Equilateral triangle pointing up.
        TriangleDown,  ///< \brief Equilateral triangle pointing down.
        TriangleLeft,  ///< \brief Equilateral triangle pointing left.
        TriangleRight, ///< \brief Equilateral triangle pointing right.
        Cross,         ///< \brief Plus sign (+).
        XCross,        ///< \brief Diagonal cross.
        HLine,         ///< \brief Short horizontal line.
        VLine,         ///< \brief Short vertical line, e.g. for rug and event plots.
        Star,          ///< \brief Five-pointed star.
        Asterisk,      ///< \brief Eight-armed asterisk: a thin plus and a thin diagonal cross.
        Pixel,         ///< \brief A single pixel; ignores \c markerSize, \c markerFilled, and anti-aliasing. Suited to very dense clouds.
        Hexagon,       ///< \brief Regular hexagon with a vertex up.
        Pentagon       ///< \brief Regular pentagon with a vertex up.
    };
    Q_ENUM(MarkerShape)

    /// \brief Point coloring modes.
    enum class ColorMode {
        UniformColor, ///< \brief Every point uses \c color.
        ValueColor,   ///< \brief Points with a finite value are colored through \c colorGradient.
    };
    Q_ENUM(ColorMode)

    /// \brief Constructs a PointCloud with the given \a parent.
    explicit PointCloud(QQuickItem* parent = nullptr);

    /// \brief Returns the uniform marker color.
    QColor color() const;
    /// \brief Sets the uniform marker color to \a color.
    void setColor(const QColor& color);

    /// \brief Returns the marker shape.
    MarkerShape markerShape() const;
    /// \brief Sets the marker shape to \a shape.
    void setMarkerShape(MarkerShape shape);

    /// \brief Returns the marker radius in pixels.
    qreal markerSize() const;
    /// \brief Sets the marker radius to \a size pixels. Negative values are clamped to 0.
    void setMarkerSize(qreal size);

    /// \brief Returns \c true if closed marker shapes are filled.
    bool markerFilled() const;
    /// \brief Sets whether closed marker shapes are filled (\a filled) or drawn as outlines.
    void setMarkerFilled(bool filled);

    /// \brief Returns the outline width of hollow markers in pixels.
    qreal markerStrokeWidth() const;
    /// \brief Sets the outline width of hollow markers to \a width pixels. Negative values are clamped to 0.
    void setMarkerStrokeWidth(qreal width);

    /// \brief Returns the coloring mode.
    ColorMode colorMode() const;
    /// \brief Sets the coloring mode to \a mode.
    void setColorMode(ColorMode mode);

    /// \brief Returns the colormap gradient object.
    QObject* colorGradient() const;
    /// \brief Sets the colormap gradient to \a gradient (a QML \c Gradient).
    void setColorGradient(QObject* gradient);

    /// \brief Returns the source of the lower colormap bound.
    GradientValueSource valueMinSource() const;
    /// \brief Sets the source of the lower colormap bound to \a source.
    void setValueMinSource(GradientValueSource source);
    /// \brief Returns the fixed lower colormap bound.
    qreal valueMin() const;
    /// \brief Sets the fixed lower colormap bound to \a value.
    void setValueMin(qreal value);

    /// \brief Returns the source of the upper colormap bound.
    GradientValueSource valueMaxSource() const;
    /// \brief Sets the source of the upper colormap bound to \a source.
    void setValueMaxSource(GradientValueSource source);
    /// \brief Returns the fixed upper colormap bound.
    qreal valueMax() const;
    /// \brief Sets the fixed upper colormap bound to \a value.
    void setValueMax(qreal value);

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
    /// \brief Returns the resolved lower colormap bound.
    qreal dataValueMin() const;
    /// \brief Returns the resolved upper colormap bound.
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
    /// \brief Emitted when the markerShape property changes.
    void markerShapeChanged();
    /// \brief Emitted when the markerSize property changes.
    void markerSizeChanged();
    /// \brief Emitted when the markerFilled property changes.
    void markerFilledChanged();
    /// \brief Emitted when the markerStrokeWidth property changes.
    void markerStrokeWidthChanged();
    /// \brief Emitted when the colorMode property changes.
    void colorModeChanged();
    /// \brief Emitted when the colorGradient property changes.
    void colorGradientChanged();
    /// \brief Emitted when the valueMinSource property changes.
    void valueMinSourceChanged();
    /// \brief Emitted when the valueMin property changes.
    void valueMinChanged();
    /// \brief Emitted when the valueMaxSource property changes.
    void valueMaxSourceChanged();
    /// \brief Emitted when the valueMax property changes.
    void valueMaxChanged();
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

private:
    Q_SLOT void onColorGradientUpdated();

    bool validateDataArguments(std::size_t xyFloatCount, std::size_t valueCount, int pointCount) const;
    void applyData(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount, bool reportRanges);
    void storeInterleaved(std::vector<float>&& xyInterleaved, const std::vector<float>& values, int pointCount);
    void finishDataChange(int previousCount, bool hadValues, bool reportRanges);
    void updateDataRanges();
    void updateValueRange();
    void reconnectAxisSignals();
    void reconnectGradientSignals();
    void refreshColorStops();
    void setHoveredIndex(int index);
    int stride() const;
    void ensureSpatialIndex() const;

    QColor color_{Qt::blue};
    MarkerShape markerShape_{MarkerShape::Circle};
    qreal markerSize_{3.0};
    bool markerFilled_{true};
    qreal markerStrokeWidth_{1.0};
    ColorMode colorMode_{ColorMode::UniformColor};
    QPointer<QObject> colorGradient_;
    std::vector<GradientStopData> colorStops_;
    GradientValueSource valueMinSource_{GradientValueSource::DataRange};
    qreal valueMin_{0.0};
    GradientValueSource valueMaxSource_{GradientValueSource::DataRange};
    qreal valueMax_{1.0};
    bool antialiasingEnabled_{true};
    qreal antialiasingFeather_{1.0};
    qreal hoverRadius_{6.0};
    int hoveredIndex_{-1};

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
