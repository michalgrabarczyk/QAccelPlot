

# File BandSeries.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**BandSeries.hpp**](BandSeries_8hpp.md)

[Go to the documentation of this file](BandSeries_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/renderers/BandEdgeRenderer.hpp"
#include "QAccelPlot/series/BandEdges.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"

#include <QList>
#include <QVariantMap>
#include <QVector2D>

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <memory>
#include <optional>
#include <vector>

namespace QAccelPlot {

class BandSeries : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(BandSeries)

    
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(BandEdges* edges READ edges CONSTANT)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool hovered READ hovered NOTIFY hoveredChanged)

public:
    explicit BandSeries(QQuickItem* parent = nullptr);

    QColor color() const;
    void setColor(const QColor& color);

    BandEdges* edges() const;

    int count() const;

    bool hovered() const;

    Q_INVOKABLE void appendData(qreal x, qreal low, qreal high);
    Q_INVOKABLE void setData(const QList<qreal>& xs, const QList<qreal>& lows, const QList<qreal>& highs);
    void setData(const std::vector<double>& xs, const std::vector<double>& lows, const std::vector<double>& highs);
    // The PlotSeries overloads take interleaved (x, low, high) triples, as doubles or floats.
    using PlotSeries::setData;
    using PlotSeries::setDataF;
    Q_INVOKABLE void clearData() override;
    std::optional<LegendEntry> legendEntry() const override;

    Q_INVOKABLE QVariantMap valueAt(qreal x) const;

    bool contains(const QPointF& point) const override;

signals:
    void colorChanged();
    void countChanged();
    void hoveredChanged();

protected:
    bool recordInspectionSupported() const override;
    InspectionRecord inspectionRecord(int index) const override;
    InspectionRecord inspectionRecordAt(const QPointF& position) const override;
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void onAxisScaleChanged() override;
    void onAxisRangeChanged() override;
    void onDataPointsChanged(DataChange change, int previousCount) override;

private:
    // The drawn band at one X: the smaller and larger interpolated value.
    struct Span {
        qreal low;
        qreal high;
    };

    // What the fill and the edge lines draw this frame: viewport uniforms relative to the render
    // origin, and the number of data points within the GPU capacity.
    struct RenderView {
        QVector2D domainMin;
        QVector2D domainMax;
        QVector2D viewportSize;
        int drawnCount;
        // Vertex buffer room, grown in powers of two so that appends rarely rebuild the vertices.
        int reservedCount;
    };

    // Applies (x, low, high) triples built from separate lists.
    void applyInterleavedData(std::vector<double>&& data);
    // Returns value \a component (0 = x, 1 = low, 2 = high) of data point \a index.
    double value(int index, int component) const;
    bool dataPointValid(int index) const;
    void updateXAscending();
    std::optional<Span> spanAt(qreal x) const;
    bool edgesVisible() const;
    QColor edgeColor() const;
    void setHovered(bool hovered);
    // Re-tests the last hover position after the data or the axes moved under the cursor.
    void refreshHovered();
    // Follows edits of the current edge line style, such as a new dash pattern.
    void onEdgeLineStyleChanged();
    RenderView renderView();
    void updateFillNode(QSGGeometryNode* node, const RenderView& view, bool dataChanged);
    QSGGeometryNode* paintEdge(QSGGeometryNode* oldNode, const BandEdgeRenderer& renderer, const RenderView& view,
        const std::shared_ptr<DataTexture>& dataTexture, bool dataChanged) const;

    QColor color_;
    BandEdges* edges_{new BandEdges{this}};
    bool hovered_{false};
    // Cursor position while hover events arrive, in item coordinates.
    std::optional<QPointF> hoverPosition_;
    // Render revision of the data points last uploaded to the GPU.
    std::uint64_t uploadedRevision_{0};
    // Whether the last frame held more records than the GPU buffers do; warns on each crossing.
    bool overCapacity_{false};
    bool xAscending_{true};
    QMetaObject::Connection edgeStyleConnection_;
    BandEdgeRenderer lowerEdgeRenderer_{BandEdgeMaterial::Edge::Lower};
    BandEdgeRenderer upperEdgeRenderer_{BandEdgeMaterial::Edge::Upper};
};

} // namespace QAccelPlot
```


