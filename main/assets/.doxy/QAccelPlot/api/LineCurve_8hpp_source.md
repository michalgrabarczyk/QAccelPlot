

# File LineCurve.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**series**](dir_d1bb17d10be635dda10fdf13c9e6bbc5.md) **>** [**LineCurve.hpp**](LineCurve_8hpp.md)

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

class LineCurve : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(LineCurve)

    
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    Q_PROPERTY(bool hovered READ hovered NOTIFY hoveredChanged)
    Q_PROPERTY(DataTransition* transition READ transition WRITE setTransition NOTIFY transitionChanged)
    Q_PROPERTY(LineStyle* lineStyle READ lineStyle WRITE setLineStyle NOTIFY lineStyleChanged)
    Q_PROPERTY(PointShape markerShape READ markerShape WRITE setMarkerShape NOTIFY markerShapeChanged)
    Q_PROPERTY(qreal markerSize READ markerSize WRITE setMarkerSize NOTIFY markerSizeChanged)
    Q_PROPERTY(bool antialiasingEnabled READ antialiasingEnabled WRITE setAntialiasingEnabled NOTIFY antialiasingEnabledChanged)
    Q_PROPERTY(qreal antialiasingFeather READ antialiasingFeather WRITE setAntialiasingFeather NOTIFY antialiasingFeatherChanged)
    Q_PROPERTY(QQmlListProperty<LineCurveEffect> effects READ effects)

public:
    enum class PointShape { None, Circle, Square, Diamond, TriangleUp, TriangleDown, Cross };
    Q_ENUM(PointShape)

    
    explicit LineCurve(QQuickItem* parent = nullptr);

    QColor color() const;
    void setColor(const QColor& c);

    qreal lineWidth() const;
    void setLineWidth(qreal w);

    bool hovered() const;

    DataTransition* transition() const;
    void setTransition(DataTransition* transition);

    LineStyle* lineStyle() const;
    void setLineStyle(LineStyle* style);

    PointShape markerShape() const;
    void setMarkerShape(PointShape shape);

    qreal markerSize() const;
    void setMarkerSize(qreal r);

    bool antialiasingEnabled() const;
    void setAntialiasingEnabled(bool enabled);

    qreal antialiasingFeather() const;
    void setAntialiasingFeather(qreal feather);

    QQmlListProperty<LineCurveEffect> effects();

    Q_INVOKABLE void appendData(qreal x, qreal y);
    Q_INVOKABLE void clearData();
    Q_INVOKABLE void setData(const QList<QPointF>& data);
    void setData(const std::vector<double>& xs, const std::vector<double>& ys);
    void setDataF(const float* xyInterleaved, int pointCount);
    void setDataF(std::vector<float>&& data, int pointCount);
    void setDataFNoRange(std::vector<float>&& data, int pointCount);
    void setDataFNoRange(const float* xyInterleaved, int pointCount);
    void setDataFNoRangeWithCache(std::vector<float>&& data, int pointCount, std::vector<char>&& vertexCache);
    void setDataFNoRangeWithCache(const float* xyInterleaved, int pointCount, std::vector<char>&& vertexCache);
    void postData(std::vector<float>&& xyInterleaved, int pointCount);

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    bool contains(const QPointF& point) const override;

signals:
    void colorChanged();
    void lineWidthChanged();
    void hoveredChanged();
    void transitionChanged();
    void lineStyleChanged();
    void markerShapeChanged();
    void markerSizeChanged();
    void antialiasingEnabledChanged();
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
```


