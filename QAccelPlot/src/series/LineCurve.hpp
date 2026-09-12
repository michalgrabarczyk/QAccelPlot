//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "effects/GradientColorTypes.hpp"
#include "effects/LineCurveEffect.hpp"
#include "linestyles/SolidLine.hpp"
#include "renderers/LineCurveLineRenderer.hpp"
#include "renderers/LineCurvePointRenderer.hpp"
#include "series/LineCurveVertexCache.hpp"
#include "series/PlotSeries.hpp"
#include "transitions/DataTransition.hpp"

#include <QPointF>
#include <QQmlListProperty>
#include <QQuickItem>

#include <vector>

namespace QAccelPlot {

/// \brief A hardware-accelerated QML item that renders a 2D line curve with optional markers, dashing, and gradient effects.
///
/// The \c setData() overloads retain double-precision coordinates, including large timestamp values.
/// Methods whose names end in \c F store interleaved float XY pairs <tt>[x0, y0, x1, y1, …]</tt>.
/// For maximum throughput prefer \c setDataF(std::vector<float>&&, int) or \c postData(), which move an
/// already-interleaved float buffer with zero allocation and no type conversion.
/// The curve is rendered on the Qt Scene Graph render thread using GPU-side data textures, making it suitable
/// for real-time plots with hundreds of thousands of points.
///
/// \par Effects
/// Visual effects (gradient stroke, gradient fill) are attached via the \c effects list property.
///
/// \par Transitions
/// Animated data updates are enabled by assigning a \c DrawTransition or \c MorphTransition to \c transition.
///
/// \sa Axis, GradientFill, GradientStroke, DrawTransition, MorphTransition
class LineCurve : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(LineCurve)

    /// \brief Base line color. Default: \c Qt::blue.
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    /// \brief Line stroke width in pixels. Default: 1.
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    /// \brief Read-only: \c true while the mouse cursor is over the curve.
    Q_PROPERTY(bool hovered READ hovered NOTIFY hoveredChanged)
    /// \brief Optional data transition animation applied when new data arrives.
    Q_PROPERTY(DataTransition* transition READ transition WRITE setTransition NOTIFY transitionChanged)
    /// \brief Line style (SolidLine, DashLine, or NoLine). Default: SolidLine.
    Q_PROPERTY(LineStyle* lineStyle READ lineStyle WRITE setLineStyle NOTIFY lineStyleChanged)
    /// \brief Shape drawn at each data point. Default: \c PointShape.None (no markers).
    Q_PROPERTY(PointShape markerShape READ markerShape WRITE setMarkerShape NOTIFY markerShapeChanged)
    /// \brief Radius of each marker in pixels. Default: 4.
    Q_PROPERTY(qreal markerSize READ markerSize WRITE setMarkerSize NOTIFY markerSizeChanged)
    /// \brief Whether GPU-side anti-aliasing is applied to lines and markers. Default: \c true.
    Q_PROPERTY(bool antialiasingEnabled READ antialiasingEnabled WRITE setAntialiasingEnabled NOTIFY antialiasingEnabledChanged)
    /// \brief Anti-aliasing feather width in pixels. Has effect only when \c antialiasingEnabled is \c true. Default: 1.
    Q_PROPERTY(qreal antialiasingFeather READ antialiasingFeather WRITE setAntialiasingFeather NOTIFY antialiasingFeatherChanged)
    /// \brief List of visual effects (e.g. GradientFill, GradientStroke) applied to this curve.
    Q_PROPERTY(QQmlListProperty<LineCurveEffect> effects READ effects)

public:
    /// \brief Marker shape options for data points.
    enum class PointShape { None, Circle, Square, Diamond, TriangleUp, TriangleDown, Cross };
    Q_ENUM(PointShape)

    /// \brief Constructs a LineCurve with the given \a parent.
    explicit LineCurve(QQuickItem* parent = nullptr);

    /// \brief Returns the base line color.
    QColor color() const;
    /// \brief Sets the line color to \a c.
    void setColor(const QColor& c);

    /// \brief Returns the line stroke width.
    qreal lineWidth() const;
    /// \brief Sets the line stroke width to \a w pixels.
    void setLineWidth(qreal w);

    /// \brief Returns \c true if the cursor is currently over the curve.
    bool hovered() const;

    /// \brief Returns the active data transition, or \c nullptr if none.
    DataTransition* transition() const;
    /// \brief Sets the data transition to \a transition.
    void setTransition(DataTransition* transition);

    /// \brief Returns the active line style.
    LineStyle* lineStyle() const;
    /// \brief Sets the line style to \a style.
    void setLineStyle(LineStyle* style);

    /// \brief Returns the marker shape.
    PointShape markerShape() const;
    /// \brief Sets the marker shape to \a shape.
    void setMarkerShape(PointShape shape);

    /// \brief Returns the marker size in pixels.
    qreal markerSize() const;
    /// \brief Sets the marker size to \a r pixels.
    void setMarkerSize(qreal r);

    /// \brief Returns \c true when GPU anti-aliasing is enabled.
    bool antialiasingEnabled() const;
    /// \brief Sets anti-aliasing to \a enabled.
    void setAntialiasingEnabled(bool enabled);

    /// \brief Returns the anti-aliasing feather width.
    qreal antialiasingFeather() const;
    /// \brief Sets the anti-aliasing feather width to \a feather pixels. Has effect only when \c antialiasingEnabled is \c true.
    void setAntialiasingFeather(qreal feather);

    /// \brief Returns the QML list property for attached visual effects.
    QQmlListProperty<LineCurveEffect> effects();

    /// \brief Appends a single data point (\a x, \a y) to the curve. Triggers a redraw.
    Q_INVOKABLE void appendData(qreal x, qreal y);
    /// \brief Removes all data points from the curve.
    Q_INVOKABLE void clearData();
    /// \brief Replaces the curve data with \a data (a list of QPointF values).
    Q_INVOKABLE void setData(const QList<QPointF>& data);
    /// \brief Sets data from separate X and Y vectors. If sizes don't match, the shorter length is used.
    void setData(const std::vector<double>& xs, const std::vector<double>& ys);
    /// \brief High-performance C++ overload: sets data from a raw interleaved float array of \a pointCount XY pairs.
    void setDataF(const float* xyInterleaved, int pointCount);
    /// \brief High-performance C++ overload: sets data by moving a pre-filled float vector of \a pointCount XY pairs.
    void setDataF(std::vector<float>&& data, int pointCount);
    /// \brief Like \c setDataF(vector) but skips emitting \c xDataRangeChanged / \c yDataRangeChanged.
    void setDataFNoRange(std::vector<float>&& data, int pointCount);
    /// \brief Like \c setDataFNoRange(vector) but copies from a raw interleaved float array.
    void setDataFNoRange(const float* xyInterleaved, int pointCount);
    /// \brief Like \c setDataFNoRange but also accepts a pre-built \a vertexCache, bypassing main-thread rebuild.
    void setDataFNoRangeWithCache(std::vector<float>&& data, int pointCount, std::vector<char>&& vertexCache);
    /// \brief Like \c setDataFNoRangeWithCache but copies from a raw interleaved float array.
    void setDataFNoRangeWithCache(const float* xyInterleaved, int pointCount, std::vector<char>&& vertexCache);
    /// \brief Posts data to the curve from any thread. Equivalent to calling \c setDataF() on the UI thread.
    /// The data vector is moved into the queued call; no copy is made. This call is thread-safe.
    void postData(std::vector<float>&& xyInterleaved, int pointCount);

protected:
    /// \cond INTERNAL
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    /// \endcond
    /// \brief Returns \c true if \a point lies within the curve's hit-test region.
    bool contains(const QPointF& point) const override;

signals:
    /// \brief Emitted when the color property changes.
    void colorChanged();
    /// \brief Emitted when the lineWidth property changes.
    void lineWidthChanged();
    /// \brief Emitted when the hovered property changes.
    void hoveredChanged();
    /// \brief Emitted when the transition property changes.
    void transitionChanged();
    /// \brief Emitted when the lineStyle property changes.
    void lineStyleChanged();
    /// \brief Emitted when the markerShape property changes.
    void markerShapeChanged();
    /// \brief Emitted when the markerSize property changes.
    void markerSizeChanged();
    /// \brief Emitted when the antialiasingEnabled property changes.
    void antialiasingEnabledChanged();
    /// \brief Emitted when the antialiasingFeather property changes.
    void antialiasingFeatherChanged();

private:
    void onLineStyleChanged();

    static void appendEffect(QQmlListProperty<LineCurveEffect>* list, LineCurveEffect* effect);
    static qsizetype effectCount(QQmlListProperty<LineCurveEffect>* list);
    static LineCurveEffect* effectAt(QQmlListProperty<LineCurveEffect>* list, qsizetype index);
    static void clearEffects(QQmlListProperty<LineCurveEffect>* list);

    GradientColorPayload resolveGradientColorPayload() const;
    GradientFillPayload resolveGradientFillPayload() const;

    enum class DataType { Float, Double };

    void updateDataRanges(const std::vector<float>& buf, int count);
    void updateDataRanges(const std::vector<double>& buf, int count);
    void applyNewData(std::vector<float>&& newData, int newPointCount);
    void applyNewData(std::vector<double>&& newData, int newPointCount);
    bool validateRawDataArguments(const float* xyInterleaved, int pointCount) const;
    bool validateVectorDataArguments(const std::vector<float>& data, int pointCount) const;
    void copyRawData(const float* xyInterleaved, int pointCount);
    void promoteFloatDataToDouble();
    void rebuildDoubleRenderData(bool logScaleX, bool logScaleY);
    const std::vector<float>& renderData() const;
    CurveDataView sourceDataView() const;
    void refreshVertexCacheForDataChange();
    void installVertexCache(std::vector<char>&& vertexCache);
    std::size_t expectedVertexCacheSize() const;
    // Rebuilds vertexCache_ from the current GPU-ready data and color on the calling thread.
    // Used after color/hover changes when no new data is arriving.
    void rebuildVertexCache();
    void rebuildChunks() const;
    void invalidateVertices();
    void invalidateData();

    QColor color_{Qt::blue};
    qreal lineWidth_{1.0};
    bool hovered_{false};
    DataType dataType_{DataType::Double};
    std::vector<double> data_;      // precise interleaved x,y pairs used by setData()
    std::vector<float> dataF_;      // interleaved x,y pairs used by the *F APIs
    std::vector<float> renderData_; // origin-relative GPU data derived from data_
    qreal renderOriginX_{0.0};
    qreal renderOriginY_{0.0};
    bool renderLogScaleX_{false};
    bool renderLogScaleY_{false};
    int pointCount_{0};
    DataTransition* transition_{nullptr};
    LineStyle* lineStyle_{new SolidLine{}};
    PointShape markerShape_{PointShape::None};
    qreal markerSize_{4.0};
    bool antialiasingEnabled_{true};
    qreal antialiasingFeather_{1.0};
    bool styleChanged_{false};
    QList<LineCurveEffect*> effects_;
    LineCurveLineRenderer lineRenderer_;
    LineCurvePointRenderer pointRenderer_;

    // True when vertex buffer needs to be rebuilt (data, style or point-count changed).
    // Does NOT include axis-range, color, lineWidth, or markerSize changes — those
    // only require a material-uniform update, which happens every frame in paint().
    bool dataChanged_{true};

    // Pre-built vertex cache. The render thread consumes it via a single memcpy, which is
    // significantly faster than writing to GPU-mapped memory struct-by-struct.
    LineCurveVertexCache vertexCache_;

    // Data-space bounding boxes per chunk of kChunkSize points, extended to
    // adjacent boundary points to cover bridging segments between chunks.
    // Rebuilt lazily when chunksValid_ is false. Marked mutable for lazy init
    // from the const contains() path.
    mutable std::vector<CurveChunk> chunks_;
    mutable bool chunksValid_{false};

    // Cached ranges — suppress duplicate xDataRangeChanged/yDataRangeChanged signals.
};

} // namespace QAccelPlot
