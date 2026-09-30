

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
    void setData(const double* data, int sampleCount) override;
    void setData(std::vector<double>&& data, int sampleCount) override;
    void setDataNoRange(const double* data, int sampleCount) override;
    void setDataNoRange(std::vector<double>&& data, int sampleCount) override;
    void setDataF(const float* data, int sampleCount) override;
    void setDataF(std::vector<float>&& data, int sampleCount) override;
    void setDataFNoRange(const float* data, int sampleCount) override;
    void setDataFNoRange(std::vector<float>&& data, int sampleCount) override;
    void postData(std::vector<double>&& data, int sampleCount) override;
    void postData(std::vector<float>&& data, int sampleCount) override;
    Q_INVOKABLE void clearData() override;

    Q_INVOKABLE QVariantMap valueAt(qreal x) const;

    bool contains(const QPointF& point) const override;

signals:
    void colorChanged();
    void countChanged();
    void hoveredChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void onAxisScaleChanged() override;
    void onAxisRangeChanged() override;

private:
    // The drawn band at one X: the smaller and larger interpolated value.
    struct Span {
        qreal low;
        qreal high;
    };

    // What the fill and the edge lines draw this frame: viewport uniforms relative to the render
    // origin, and the number of samples within the GPU capacity.
    struct RenderView {
        QVector2D domainMin;
        QVector2D domainMax;
        QVector2D viewportSize;
        int drawnSampleCount;
        // Vertex buffer room, grown in powers of two so that appends rarely rebuild the vertices.
        int reservedSampleCount;
    };

    bool validateRawDataArguments(const void* data, int sampleCount) const;
    bool validateVectorArguments(std::size_t valueCount, int sampleCount) const;
    void copyData(const double* data, int sampleCount, bool reportRanges);
    void copyFloatData(const float* data, int sampleCount, bool reportRanges);
    // Applies (x, low, high) triples built from separate lists, reporting ranges.
    void applyInterleavedData(std::vector<double>&& data);
    void applyData(std::vector<double>&& data, int sampleCount, bool reportRanges);
    void applyFloatData(std::vector<float>&& data, int sampleCount, bool reportRanges);
    void finishDataChange(int sampleCount, bool reportRanges);
    void promoteFloatDataToDouble();
    // True when the double setData() overloads supplied the data; false for the setDataF() overloads.
    bool hasPreciseData() const;
    // Returns value \a component (0 = x, 1 = low, 2 = high) of sample \a index.
    double value(int index, int component) const;
    bool sampleValid(int index) const;
    bool logScaleX() const;
    bool logScaleY() const;
    // Reports the data ranges to the axes and updates xAscending_ in the same pass.
    void updateDataRanges();
    void updateXAscending();
    std::optional<Span> spanAt(qreal x) const;
    bool edgesVisible() const;
    QColor edgeColor() const;
    void setHovered(bool hovered);
    // Re-tests the last hover position after the data or the axes moved under the cursor.
    void refreshHovered();
    // Follows edits of the current edge line style, such as a new dash pattern.
    void onEdgeLineStyleChanged();
    // Rebuilds renderData_ (origin-relative floats) from data_ for the current axes.
    void rebuildRenderData();
    RenderView renderView() const;
    void updateFillNode(QSGGeometryNode* node, const RenderView& view);
    QSGGeometryNode* paintEdge(
        QSGGeometryNode* oldNode, const BandEdgeRenderer& renderer, const RenderView& view, const std::shared_ptr<DataTexture>& dataTexture) const;

    // Which setter family supplied the data: setData() keeps doubles, setDataF() keeps floats.
    enum class DataType { Double, Float };

    QColor color_;
    BandEdges* edges_{new BandEdges{this}};
    bool hovered_{false};
    // Cursor position while hover events arrive, in item coordinates.
    std::optional<QPointF> hoverPosition_;
    DataType dataType_{DataType::Double};
    // Data: 3 doubles per sample (x, low, high), full precision. Empty for DataType::Float.
    std::vector<double> data_;
    // Uploaded to the GPU: an origin-relative float mirror of data_, or the setDataF() data itself.
    std::vector<float> renderData_;
    qreal renderOriginX_{0.0};
    qreal renderOriginY_{0.0};
    // True when renderData_ matches data_, the origin, and the axis scales it was built for.
    bool renderDataValid_{false};
    bool renderLogScaleX_{false};
    bool renderLogScaleY_{false};
    // False while no finite value fixed the origin, so appended samples must rebuild renderData_.
    bool renderOriginSettled_{false};
    int sampleCount_{0};
    bool dataChanged_{true};
    bool xAscending_{true};
    // True when the most recent data update computed ranges; the NoRange APIs leave range
    // management to the caller, so scale changes must not overwrite it.
    bool autoDataRanges_{true};
    QMetaObject::Connection edgeStyleConnection_;
    BandEdgeRenderer lowerEdgeRenderer_{BandEdgeMaterial::Edge::Lower};
    BandEdgeRenderer upperEdgeRenderer_{BandEdgeMaterial::Edge::Upper};
};

} // namespace QAccelPlot
```


