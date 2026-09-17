

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
#include "QAccelPlot/theme/ColorPalette.hpp"
#include "QAccelPlot/transitions/DataTransition.hpp"

#include <QPointF>
#include <QPointer>
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
    Q_PROPERTY(bool markerFilled READ markerFilled WRITE setMarkerFilled NOTIFY markerFilledChanged)
    Q_PROPERTY(qreal markerStrokeWidth READ markerStrokeWidth WRITE setMarkerStrokeWidth NOTIFY markerStrokeWidthChanged)
    Q_PROPERTY(bool antialiasingEnabled READ antialiasingEnabled WRITE setAntialiasingEnabled NOTIFY antialiasingEnabledChanged)
    Q_PROPERTY(qreal antialiasingFeather READ antialiasingFeather WRITE setAntialiasingFeather NOTIFY antialiasingFeatherChanged)
    Q_PROPERTY(QQmlListProperty<LineCurveEffect> effects READ effects)
    Q_PROPERTY(LineCurveGaps* gaps READ gaps CONSTANT)

public:
    enum class PointShape {
        None,          
        Circle,        
        Square,        
        Diamond,       
        TriangleUp,    
        TriangleDown,  
        TriangleLeft,  
        TriangleRight, 
        Cross,         
        XCross,        
        HLine,         
        VLine,         
        Star,          
        Asterisk,      
        Pixel,         
        Hexagon,       
        Pentagon       
    };
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

    bool markerFilled() const;
    void setMarkerFilled(bool filled);

    qreal markerStrokeWidth() const;
    void setMarkerStrokeWidth(qreal width);

    bool antialiasingEnabled() const;
    void setAntialiasingEnabled(bool enabled);

    qreal antialiasingFeather() const;
    void setAntialiasingFeather(qreal feather);

    QQmlListProperty<LineCurveEffect> effects();

    LineCurveGaps* gaps() const;

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
    void onAxisScaleChanged() override;

signals:
    void colorChanged();
    void lineWidthChanged();
    void hoveredChanged();
    void transitionChanged();
    void lineStyleChanged();
    void markerShapeChanged();
    void markerSizeChanged();
    void markerFilledChanged();
    void markerStrokeWidthChanged();
    void antialiasingEnabledChanged();
    void antialiasingFeatherChanged();

private:
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
    DataTransition* transition_{nullptr};
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
```


