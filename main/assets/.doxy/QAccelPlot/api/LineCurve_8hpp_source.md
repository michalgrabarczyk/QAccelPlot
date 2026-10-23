

# File LineCurve.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**LineCurve.hpp**](LineCurve_8hpp.md)

[Go to the documentation of this file](LineCurve_8hpp.md)


```C++
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
#include "QAccelPlot/series/SeriesMarker.hpp"
#include "QAccelPlot/series/StepMode.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"
#include "QAccelPlot/transitions/DataTransition.hpp"

#include <QPointF>
#include <QPointer>
#include <QQmlListProperty>
#include <QQuickItem>

#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace QAccelPlot {

class TransitionRunner;

class LineCurve : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(LineCurve)

    
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    Q_PROPERTY(bool hovered READ hovered NOTIFY hoveredChanged)
    Q_PROPERTY(qreal hoverRadius READ hoverRadius WRITE setHoverRadius NOTIFY hoverRadiusChanged)
    Q_PROPERTY(DataTransition* transition READ transition WRITE setTransition NOTIFY transitionChanged)
    Q_PROPERTY(LineStyle* lineStyle READ lineStyle WRITE setLineStyle NOTIFY lineStyleChanged)
    Q_PROPERTY(bool antialiasingEnabled READ antialiasingEnabled WRITE setAntialiasingEnabled NOTIFY antialiasingEnabledChanged)
    Q_PROPERTY(qreal antialiasingFeather READ antialiasingFeather WRITE setAntialiasingFeather NOTIFY antialiasingFeatherChanged)
    Q_PROPERTY(QQmlListProperty<LineCurveEffect> effects READ effects)
    Q_PROPERTY(LineCurveGaps* gaps READ gaps CONSTANT)
    Q_PROPERTY(SeriesMarker* marker READ marker CONSTANT)
    Q_PROPERTY(StepMode stepMode READ stepMode WRITE setStepMode NOTIFY stepModeChanged)

public:
    explicit LineCurve(QQuickItem* parent = nullptr);
    ~LineCurve() override;

    QColor color() const;
    void setColor(const QColor& c);

    qreal lineWidth() const;
    void setLineWidth(qreal w);

    bool hovered() const;

    qreal hoverRadius() const;
    void setHoverRadius(qreal radius);

    DataTransition* transition() const;
    void setTransition(DataTransition* transition);

    LineStyle* lineStyle() const;
    void setLineStyle(LineStyle* style);

    bool antialiasingEnabled() const;
    void setAntialiasingEnabled(bool enabled);

    qreal antialiasingFeather() const;
    void setAntialiasingFeather(qreal feather);

    QQmlListProperty<LineCurveEffect> effects();

    LineCurveGaps* gaps() const;

    SeriesMarker* marker() const;

    StepMode stepMode() const;
    void setStepMode(StepMode mode);

    Q_INVOKABLE void appendData(qreal x, qreal y);
    Q_INVOKABLE void setData(const QList<QPointF>& data);
    void setData(const std::vector<double>& xs, const std::vector<double>& ys);
    using PlotSeries::postData;
    using PlotSeries::setData;
    using PlotSeries::setDataF;
    void setDataFWithCache(std::vector<float>&& data, int pointCount, std::vector<char>&& vertexCache);
    void setDataFWithCache(const float* xyInterleaved, int pointCount, std::vector<char>&& vertexCache);
    Q_INVOKABLE void clearData() override;
    std::optional<LegendEntry> legendEntry() const override;

protected:
    InspectionSource inspectionSource() const override;
    bool inspectionAvailable() const override;
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    void itemChange(ItemChange change, const ItemChangeData& value) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    bool contains(const QPointF& point) const override;
    void applyDataPoints(DataPointInput&& input) override;
    void onAxisScaleChanged() override;
    void onDataPointsChanged(DataChange change, int previousCount) override;

signals:
    void colorChanged();
    void lineWidthChanged();
    void hoveredChanged();
    void hoverRadiusChanged();
    void transitionChanged();
    void lineStyleChanged();
    void antialiasingEnabledChanged();
    void antialiasingFeatherChanged();
    void stepModeChanged();

private:
    void onTransitionDestroyed();
    void onLineStyleChanged();
    void onLineStyleDestroyed();
    void onNanGapModeChanged();
    void onMarkerShapeChanged();

    static void appendEffect(QQmlListProperty<LineCurveEffect>* list, LineCurveEffect* effect);
    static qsizetype effectCount(QQmlListProperty<LineCurveEffect>* list);
    static LineCurveEffect* effectAt(QQmlListProperty<LineCurveEffect>* list, qsizetype index);
    static void clearEffects(QQmlListProperty<LineCurveEffect>* list);

    bool hasGradientEffect() const;
    GradientColorPayload resolveGradientColorPayload() const;
    GradientFillPayload resolveGradientFillPayload() const;
    std::pair<qreal, qreal> gradientDataRange(Qt::Orientation direction) const;

    void applyNewData(std::vector<float>&& newData, int newPointCount);
    void applyNewData(std::vector<double>&& newData, int newPointCount);
    // Replaces the points with float data copied from \a xyInterleaved, without a transition.
    void applyFloatArray(const float* xyInterleaved, int pointCount);
    const std::vector<float>& renderData() const;
    CurveDataView sourceDataView() const;
    // Number of data points in renderData() / sourceDataView(); smaller than the point count
    // when NanGapMode::Connect has removed invalid data points.
    int renderPointCount() const;
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
    // Shows the target data of a transition that stopped before its end.
    void finishTransition();
    // Starts a transition from the points drawn now to the stored points, the new data.
    void startTransition(std::vector<double>&& from, int fromCount);
    // Effects refresh on the GUI thread once per frame, from QQuickWindow::afterAnimating.
    void connectAnimationTicks(QQuickWindow* window);
    void refreshEffects();
    void advanceTransition();

    QColor color_{ColorPalette::dark().seriesPrimary};
    qreal lineWidth_{1.0};
    bool hovered_{false};
    qreal hoverRadius_{10.0};
    // While its run is pending, the frames are the store's drawn points; the stored points are its target.
    std::unique_ptr<TransitionRunner> transition_;
    // The previous transition frame, reused for the next one.
    std::vector<double> frameBuffer_;
    // A vertex cache passed with the data, installed once the data points changed.
    std::optional<std::vector<char>> pendingVertexCache_;
    QPointer<QQuickWindow> animationTickWindow_;
    QPointer<LineStyle> lineStyle_{new SolidLine{this}};
    bool antialiasingEnabled_{true};
    qreal antialiasingFeather_{1.0};
    bool styleChanged_{false};
    LineCurveGaps* gaps_{new LineCurveGaps{this}};
    SeriesMarker* marker_{new SeriesMarker{MarkerShape::None, 4.0, SeriesMarker::NoneShape::Accepted, this}};
    StepMode stepMode_{StepMode::None};
    QList<LineCurveEffect*> effects_;
    LineCurveLineRenderer lineRenderer_;
    LineCurvePointRenderer pointRenderer_;

    // True when vertex buffer needs to be rebuilt (style or point-count changed). A change of the
    // render data is told by its revision. Does NOT include axis-range, color, lineWidth, or
    // markerSize changes — those only require a material-uniform update, which happens every frame in paint().
    bool dataChanged_{true};
    // Render revision of the points last uploaded to the GPU.
    std::uint64_t uploadedRevision_{0};
    // Render revision of the points the vertex cache was built from.
    std::uint64_t vertexCacheRevision_{0};

    // Pre-built vertex cache. The render thread consumes it via a single memcpy, which is
    // significantly faster than writing to GPU-mapped memory struct-by-struct.
    LineCurveVertexCache vertexCache_;

    // Data-space bounding boxes per chunk of kChunkSize points, extended to
    // adjacent boundary points to cover bridging segments between chunks.
    // Rebuilt lazily when chunksValid_ is false. Marked mutable for lazy init
    // from the const contains() path.
    mutable std::vector<CurveChunk> chunks_;
    mutable bool chunksValid_{false};
};

} // namespace QAccelPlot
```


