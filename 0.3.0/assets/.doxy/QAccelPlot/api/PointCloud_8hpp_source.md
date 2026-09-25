

# File PointCloud.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**PointCloud.hpp**](PointCloud_8hpp.md)

[Go to the documentation of this file](PointCloud_8hpp.md)


```C++
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

#include <QColor>
#include <QList>
#include <QMetaObject>
#include <QPointF>
#include <QPointer>

#include <vector>

namespace QAccelPlot {

class PointCloud : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(PointCloud)

    
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(SeriesMarker* marker READ marker CONSTANT)
    Q_PROPERTY(Colormap* colormap READ colormap WRITE setColormap NOTIFY colormapChanged)
    Q_PROPERTY(bool antialiasingEnabled READ antialiasingEnabled WRITE setAntialiasingEnabled NOTIFY antialiasingEnabledChanged)
    Q_PROPERTY(qreal antialiasingFeather READ antialiasingFeather WRITE setAntialiasingFeather NOTIFY antialiasingFeatherChanged)
    Q_PROPERTY(qreal hoverRadius READ hoverRadius WRITE setHoverRadius NOTIFY hoverRadiusChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool hasValues READ hasValues NOTIFY countChanged)
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)
    Q_PROPERTY(qreal dataValueMin READ dataValueMin NOTIFY valueRangeChanged)
    Q_PROPERTY(qreal dataValueMax READ dataValueMax NOTIFY valueRangeChanged)

public:
    explicit PointCloud(QQuickItem* parent = nullptr);

    QColor color() const;
    void setColor(const QColor& color);

    SeriesMarker* marker() const;

    Colormap* colormap() const;
    void setColormap(Colormap* colormap);

    bool antialiasingEnabled() const;
    void setAntialiasingEnabled(bool enabled);

    qreal antialiasingFeather() const;
    void setAntialiasingFeather(qreal feather);

    qreal hoverRadius() const;
    void setHoverRadius(qreal radius);

    int count() const;
    bool hasValues() const;
    int hoveredIndex() const;
    qreal dataValueMin() const;
    qreal dataValueMax() const;

    Q_INVOKABLE void setData(const QList<QPointF>& points);
    Q_INVOKABLE void setValues(const QList<qreal>& values);
    Q_INVOKABLE void clearData();
    Q_INVOKABLE QPointF pointAt(int index) const;
    Q_INVOKABLE qreal valueAt(int index) const;

    void setDataF(const float* xyInterleaved, int pointCount);
    void setDataF(std::vector<float>&& xyInterleaved, int pointCount);
    void setDataF(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    void setDataFNoRange(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    void postData(std::vector<float>&& xyInterleaved, int pointCount);
    void postData(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount);

    void setData(std::vector<double>&& xyInterleaved, int pointCount);
    void setData(std::vector<double>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    void setDataNoRange(std::vector<double>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    void postData(std::vector<double>&& xyInterleaved, int pointCount);
    void postData(std::vector<double>&& xyInterleaved, std::vector<float>&& values, int pointCount);

    int pointIndexAt(const QPointF& position) const;
    bool contains(const QPointF& point) const override;

signals:
    void colorChanged();
    void colormapChanged();
    void antialiasingEnabledChanged();
    void antialiasingFeatherChanged();
    void hoverRadiusChanged();
    void countChanged();
    void hoveredIndexChanged();
    void valueRangeChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;

protected:
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

    QColor color_{ColorPalette::dark().seriesPrimary};
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
```


