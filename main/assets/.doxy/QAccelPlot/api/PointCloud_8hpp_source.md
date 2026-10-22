

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

#include "QAccelPlot/series/ColormappedSeries.hpp"
#include "QAccelPlot/series/PointSpatialIndex.hpp"
#include "QAccelPlot/series/SeriesMarker.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"
#include "QAccelPlot/transitions/DataTransition.hpp"

#include <QColor>
#include <QList>
#include <QPointF>

#include <memory>
#include <optional>
#include <vector>

namespace QAccelPlot {

namespace Internal {
class HoverIndexBudget;
}

class TransitionRunner;

class PointCloud : public ColormappedSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(PointCloud)

    
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(SeriesMarker* marker READ marker CONSTANT)
    Q_PROPERTY(bool antialiasingEnabled READ antialiasingEnabled WRITE setAntialiasingEnabled NOTIFY antialiasingEnabledChanged)
    Q_PROPERTY(qreal antialiasingFeather READ antialiasingFeather WRITE setAntialiasingFeather NOTIFY antialiasingFeatherChanged)
    Q_PROPERTY(qreal hoverRadius READ hoverRadius WRITE setHoverRadius NOTIFY hoverRadiusChanged)
    Q_PROPERTY(DataTransition* transition READ transition WRITE setTransition NOTIFY transitionChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool hasValues READ hasValues NOTIFY countChanged)
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)

public:
    explicit PointCloud(QQuickItem* parent = nullptr);
    ~PointCloud() override;

    QColor color() const;
    void setColor(const QColor& color);

    SeriesMarker* marker() const;

    bool antialiasingEnabled() const;
    void setAntialiasingEnabled(bool enabled);

    qreal antialiasingFeather() const;
    void setAntialiasingFeather(qreal feather);

    qreal hoverRadius() const;
    void setHoverRadius(qreal radius);

    DataTransition* transition() const;
    void setTransition(DataTransition* transition);

    int count() const;
    bool hasValues() const;
    int hoveredIndex() const;

    Q_INVOKABLE void setData(const QList<QPointF>& points);
    Q_INVOKABLE void setValues(const QList<qreal>& values);
    Q_INVOKABLE QPointF pointAt(int index) const;
    Q_INVOKABLE qreal valueAt(int index) const;

    void setData(std::vector<double>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    void setDataF(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    using PlotSeries::postData;
    using PlotSeries::setData;
    using PlotSeries::setDataF;
    void postData(std::vector<double>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    void postData(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    Q_INVOKABLE void clearData() override;
    std::optional<LegendEntry> legendEntry() const override;

    int pointIndexAt(const QPointF& position) const;
    bool contains(const QPointF& point) const override;

signals:
    void colorChanged();
    void antialiasingEnabledChanged();
    void antialiasingFeatherChanged();
    void hoverRadiusChanged();
    void transitionChanged();
    void countChanged();
    void hoveredIndexChanged();

protected:
    InspectionSource inspectionSource() const override;
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;

protected:
    void applyDataPoints(DataPointInput&& input) override;
    void onAxisScaleChanged() override;
    void onDataPointsChanged(DataChange change, int previousCount) override;
    std::optional<ValueSpan> scanValueRange() const override;

private:
    struct Points {
        std::vector<double> values;
        int count{0};
        int stride{2};
    };

    bool validateValues(std::size_t valueCount, int pointCount) const;
    void applyData(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    void applyFloatData(std::vector<float>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    void applyDoubleData(std::vector<double>&& xyInterleaved, std::vector<float>&& values, int pointCount);
    void finishValueChange(int previousCount);
    void setHoveredIndex(int index);
    int hoverPointCount() const;
    void invalidateSpatialIndex();
    bool spatialIndexReady(PointSpatialIndex::Mapping mapping) const;

    struct HoverQuery {
        double x;
        double y;
        double radiusX;
        double radiusY;
        PointSpatialIndex::Mapping mapping;

        bool operator==(const HoverQuery& other) const;
    };

    int nearestPoint(const HoverQuery& query) const;
    void animateValues(std::vector<float>&& values);
    std::vector<double> interleavedData() const;
    Points drawnPoints() const;
    void startTransition(Points&& drawn);
    void advanceTransition();
    void endTransition();
    void discardTransition();

    QColor color_{ColorPalette::dark().seriesPrimary};
    SeriesMarker* marker_{new SeriesMarker{MarkerShape::Circle, 3.0, SeriesMarker::NoneShape::Rejected, this}};
    bool antialiasingEnabled_{true};
    qreal antialiasingFeather_{1.0};
    qreal hoverRadius_{6.0};
    int hoveredIndex_{-1};
    std::unique_ptr<TransitionRunner> transition_;
    // The previous transition frame, reused for the next one.
    std::vector<double> frameBuffer_;

    // hasValues() as countChanged() last reported it.
    bool reportedHasValues_{false};
    // Render revision of the data points last uploaded to the GPU.
    std::uint64_t uploadedRevision_{0};

    mutable PointSpatialIndex spatialIndex_;
    mutable bool spatialIndexValid_{false};
    // Render revision of the data points the spatial index holds.
    mutable std::uint64_t indexedRevision_{0};
    std::unique_ptr<Internal::HoverIndexBudget> spatialIndexBudget_;
    // The last hover query and its answer. Qt tests contains() several times per hover event
    // before the handler asks again, each time at the same position.
    mutable std::optional<HoverQuery> lastHoverQuery_;
    mutable int lastHoverIndex_{-1};
};

} // namespace QAccelPlot
```


