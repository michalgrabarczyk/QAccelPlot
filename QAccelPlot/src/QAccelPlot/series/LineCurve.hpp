//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/effects/GradientColorTypes.hpp"
#include "QAccelPlot/effects/LineCurveEffect.hpp"
#include "QAccelPlot/linestyles/SolidLine.hpp"
#include "QAccelPlot/renderers/LineCurveLineRenderer.hpp"
#include "QAccelPlot/renderers/LineCurvePointRenderer.hpp"
#include "QAccelPlot/series/LineCurveGaps.hpp"
#include "QAccelPlot/series/LineCurveVertexCache.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"
#include "QAccelPlot/transitions/DataTransition.hpp"

#include <QPointF>
#include <QPointer>
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
/// for real-time plots with millions of points.
///
/// \par Effects
/// Visual effects (gradient stroke, gradient fill) are attached via the \c effects list property.
///
/// \par Transitions
/// Animated data updates are enabled by assigning a \c DrawTransition or \c MorphTransition to \c transition.
///
/// \par Invalid samples and gaps
/// A sample is invalid when its X or Y coordinate is NaN or ±Inf, or is not strictly positive on a
/// log-scale axis. Invalid samples are never drawn as markers, never hit-tested, and are excluded from
/// auto-ranging coordinate by coordinate (a finite X with an invalid Y still extends the X range). How the line and
/// gradient fill treat them is controlled by the \c gaps grouped property: <tt>gaps.nanMode</tt> \c Break (default)
/// leaves a gap, \c Connect joins the neighboring valid samples. Insert \c NaN to mark missing telemetry explicitly.
///
/// \sa Axis, GradientFill, GradientStroke, DrawTransition, MorphTransition
class LineCurve : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(LineCurve)

    /// \brief Base line color. Default: \c Colors.dark.seriesPrimary.
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
    /// \brief Whether closed marker shapes are filled. When \c false they are drawn as outlines of \c markerStrokeWidth
    /// inside the shape's edge. Line-like shapes (\c Cross, \c XCross, \c Asterisk, \c HLine, \c VLine) and \c Pixel
    /// are unaffected. Default: \c true.
    Q_PROPERTY(bool markerFilled READ markerFilled WRITE setMarkerFilled NOTIFY markerFilledChanged)
    /// \brief Outline width in pixels of hollow markers. Has effect only when \c markerFilled is \c false. Default: 1.
    Q_PROPERTY(qreal markerStrokeWidth READ markerStrokeWidth WRITE setMarkerStrokeWidth NOTIFY markerStrokeWidthChanged)
    /// \brief Whether GPU-side anti-aliasing is applied to lines and markers. Default: \c true.
    Q_PROPERTY(bool antialiasingEnabled READ antialiasingEnabled WRITE setAntialiasingEnabled NOTIFY antialiasingEnabledChanged)
    /// \brief Anti-aliasing feather width in pixels. Has effect only when \c antialiasingEnabled is \c true. Default: 1.
    Q_PROPERTY(qreal antialiasingFeather READ antialiasingFeather WRITE setAntialiasingFeather NOTIFY antialiasingFeatherChanged)
    /// \brief List of visual effects (e.g. GradientFill, GradientStroke) applied to this curve.
    Q_PROPERTY(QQmlListProperty<LineCurveEffect> effects READ effects)
    /// \brief Grouped gap-rendering settings, e.g. <tt>gaps.nanMode</tt>.
    Q_PROPERTY(LineCurveGaps* gaps READ gaps CONSTANT)

public:
    /// \brief Marker shape options for data points.
    ///
    /// Every shape except \c Pixel fits within a square of half-width \c markerSize.
    enum class PointShape {
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
        Pixel,         ///< \brief A single pixel; ignores \c markerSize, \c markerFilled, and anti-aliasing. Suited to very dense scatter plots.
        Hexagon,       ///< \brief Regular hexagon with a vertex up.
        Pentagon       ///< \brief Regular pentagon with a vertex up.
    };
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

    /// \brief Returns \c true if closed marker shapes are filled.
    bool markerFilled() const;
    /// \brief Sets whether closed marker shapes are filled (\a filled) or drawn as outlines.
    void setMarkerFilled(bool filled);

    /// \brief Returns the outline width of hollow markers in pixels.
    qreal markerStrokeWidth() const;
    /// \brief Sets the outline width of hollow markers to \a width pixels.
    void setMarkerStrokeWidth(qreal width);

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

    /// \brief Returns the grouped gap-rendering settings. The object is owned by the curve.
    LineCurveGaps* gaps() const;

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
    /// \brief Refreshes ranges and cached geometry when a bound axis changes between linear and log scale.
    void onAxisScaleChanged() override;

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
    /// \brief Emitted when the markerFilled property changes.
    void markerFilledChanged();
    /// \brief Emitted when the markerStrokeWidth property changes.
    void markerStrokeWidthChanged();
    /// \brief Emitted when the antialiasingEnabled property changes.
    void antialiasingEnabledChanged();
    /// \brief Emitted when the antialiasingFeather property changes.
    void antialiasingFeatherChanged();

private:
    void onTransitionDestroyed();
    void onLineStyleChanged();
    void onLineStyleDestroyed();
    void onNanGapModeChanged();

    static void appendEffect(QQmlListProperty<LineCurveEffect>* list, LineCurveEffect* effect);
    static qsizetype effectCount(QQmlListProperty<LineCurveEffect>* list);
    static LineCurveEffect* effectAt(QQmlListProperty<LineCurveEffect>* list, qsizetype index);
    static void clearEffects(QQmlListProperty<LineCurveEffect>* list);

    GradientColorPayload resolveGradientColorPayload() const;
    GradientFillPayload resolveGradientFillPayload() const;

    enum class DataType { Float, Double };

    void updateDataRanges(const std::vector<float>& buf, int count);
    void updateDataRanges(const std::vector<double>& buf, int count);
    void applyDataExtents(qreal xMin, qreal xMax, qreal yMin, qreal yMax);
    void recomputeDataRanges();
    bool logScaleX() const;
    bool logScaleY() const;
    void applyNewData(std::vector<float>&& newData, int newPointCount);
    void applyNewData(std::vector<double>&& newData, int newPointCount);
    bool validateRawDataArguments(const float* xyInterleaved, int pointCount) const;
    bool validateVectorDataArguments(const std::vector<float>& data, int pointCount) const;
    void copyRawData(const float* xyInterleaved, int pointCount);
    void promoteFloatDataToDouble();
    void rebuildDoubleRenderData(bool logScaleX, bool logScaleY);
    const std::vector<float>& renderData() const;
    CurveDataView sourceDataView() const;
    // Number of samples in renderData() / sourceDataView(); smaller than pointCount_
    // when NanGapMode::Connect has removed invalid samples.
    int renderPointCount() const;
    void rebuildGapConnectData();
    void releaseGapConnectData();
    void refreshVertexCacheForDataChange();
    void installVertexCache(std::vector<char>&& vertexCache);
    std::size_t expectedVertexCacheSize() const;
    // Rebuilds vertexCache_ from the current GPU-ready data and color on the calling thread.
    // Used after color/hover changes when no new data is arriving.
    void rebuildVertexCache();
    void rebuildChunks() const;
    void invalidateVertices();
    void invalidateData();
    void cancelRunningTransition();

    QColor color_{ColorPalette::dark().seriesPrimary};
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
    bool renderOriginXSettled_{false};
    bool renderOriginYSettled_{false};
    int pointCount_{0};
    QPointer<DataTransition> transition_;
    QPointer<LineStyle> lineStyle_{new SolidLine{this}};
    PointShape markerShape_{PointShape::None};
    qreal markerSize_{4.0};
    bool markerFilled_{true};
    qreal markerStrokeWidth_{1.0};
    bool antialiasingEnabled_{true};
    qreal antialiasingFeather_{1.0};
    bool styleChanged_{false};
    LineCurveGaps* gaps_{new LineCurveGaps{this}};
    // True when the most recent data update computed ranges; the NoRange APIs leave
    // range management to the caller, so log-scale changes must not overwrite it.
    bool autoDataRanges_{true};
    // NanGapMode::Connect copies with invalid samples removed. Only populated when
    // Connect mode is on and the data contains invalid samples.
    bool gapConnectCompacted_{false};
    int gapConnectPointCount_{0};
    std::vector<double> gapConnectData_;      // valid double samples (double data only)
    std::vector<float> gapConnectRenderData_; // valid GPU samples; also the source view for float data
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
