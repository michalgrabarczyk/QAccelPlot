

# File ErrorBarSeries.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**ErrorBarSeries.hpp**](ErrorBarSeries_8hpp.md)

[Go to the documentation of this file](ErrorBarSeries_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/series/PlotSeries.hpp"
#include "QAccelPlot/series/SpatialGrid.hpp"
#include "QAccelPlot/transitions/DataTransition.hpp"

#include <QColor>
#include <QList>
#include <QVariantMap>

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <vector>

class QSGGeometryNode;

namespace QAccelPlot {

class ErrorBarMaterial;
class TransitionRunner;

class ErrorBarSeries : public PlotSeries {
    Q_OBJECT
    QML_NAMED_ELEMENT(ErrorBarSeries)

    
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    Q_PROPERTY(qreal capLength READ capLength WRITE setCapLength NOTIFY capLengthChanged)
    Q_PROPERTY(qreal offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(QColor hoverColor READ hoverColor WRITE setHoverColor NOTIFY hoverColorChanged)
    Q_PROPERTY(qreal hoverRadius READ hoverRadius WRITE setHoverRadius NOTIFY hoverRadiusChanged)
    Q_PROPERTY(DataTransition* transition READ transition WRITE setTransition NOTIFY transitionChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int hoveredIndex READ hoveredIndex NOTIFY hoveredIndexChanged)

public:
    explicit ErrorBarSeries(QQuickItem* parent = nullptr);
    ~ErrorBarSeries() override;

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation orientation);

    QColor color() const;
    void setColor(const QColor& color);

    qreal lineWidth() const;
    void setLineWidth(qreal width);

    qreal capLength() const;
    void setCapLength(qreal length);

    qreal offset() const;
    void setOffset(qreal offset);

    QColor hoverColor() const;
    void setHoverColor(const QColor& color);

    qreal hoverRadius() const;
    void setHoverRadius(qreal radius);

    DataTransition* transition() const;
    void setTransition(DataTransition* transition);

    int count() const;
    int hoveredIndex() const;

    Q_INVOKABLE void appendData(qreal x, qreal y, qreal low, qreal high);
    Q_INVOKABLE void setData(const QList<qreal>& xs, const QList<qreal>& ys, const QList<qreal>& lows, const QList<qreal>& highs);
    void setData(const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& lows, const std::vector<double>& highs);
    // The PlotSeries overloads take interleaved (x, y, low, high) quadruples, as doubles or floats.
    using PlotSeries::setData;
    using PlotSeries::setDataF;

    Q_INVOKABLE void appendErrorData(qreal x, qreal y, qreal error);
    Q_INVOKABLE void appendErrorData(qreal x, qreal y, qreal minus, qreal plus);
    Q_INVOKABLE void setErrorData(const QList<qreal>& xs, const QList<qreal>& ys, const QList<qreal>& errors);
    Q_INVOKABLE void setErrorData(const QList<qreal>& xs, const QList<qreal>& ys, const QList<qreal>& minuses, const QList<qreal>& pluses);
    void setErrorData(const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& errors);
    void setErrorData(const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& minuses, const std::vector<double>& pluses);

    Q_INVOKABLE void clearData() override;

    Q_INVOKABLE QVariantMap errorBarAt(int index) const;
    Q_INVOKABLE int errorBarIndexAt(const QPointF& position) const;
    bool contains(const QPointF& point) const override;

signals:
    void orientationChanged();
    void colorChanged();
    void lineWidthChanged();
    void capLengthChanged();
    void offsetChanged();
    void hoverColorChanged();
    void hoverRadiusChanged();
    void transitionChanged();
    void countChanged();
    void hoveredIndexChanged();

protected:
    bool recordInspectionSupported() const override;
    InspectionRecord inspectionRecord(int index) const override;
    InspectionRecord inspectionRecordAt(const QPointF& position) const override;
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;
    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent* event) override;
    void applyDataPoints(DataPointInput&& input) override;
    void onAxisScaleChanged() override;
    void onDataPointsChanged(DataChange change, int previousCount) override;

private:
    // A drawn error bar in data coordinates: its position across the bounds, shifted by the offset,
    // and its ends along them. A NaN bound is replaced by the value and has no cap.
    struct DataBar {
        double position;
        double start;
        double end;
        bool startCapped;
        bool endCapped;
    };

    bool isHorizontal() const;
    double component(int index, int column) const;
    // Returns error bar \a index of \a dataPoints as drawn, or nullopt when it is not drawn.
    std::optional<DataBar> dataBar(const DataPointStore::View& dataPoints, int index) const;
    // Pixel distance from \a position to the line and caps of drawn error bar \a index, or nullopt when it is not drawn.
    std::optional<qreal> pixelDistance(int index, const QPointF& position) const;
    void applyInterleavedData(std::vector<double>&& data);
    void applyDoubleData(std::vector<double>&& data, int dataPointCount);
    // Starts animating from the \a drawnCount error bars in \a drawn to the stored ones.
    void startTransition(const std::vector<double>& drawn, int drawnCount);
    void advanceTransition();
    // Shows the stored data points at once, when a transition was drawing others.
    void endTransition();
    void discardTransition();
    void setHoveredIndex(int index);
    void onGeometryChanged();
    void ensureSpatialGrid() const;
    void updateVertices(QSGGeometryNode& node, int drawnCount) const;
    void updateMaterial(ErrorBarMaterial& material, int drawnCount) const;

    Qt::Orientation orientation_{Qt::Vertical};
    QColor color_;
    qreal lineWidth_{1.5};
    qreal capLength_{8.0};
    qreal offset_{0.0};
    QColor hoverColor_;
    qreal hoverRadius_{6.0};
    int hoveredIndex_{-1};
    std::unique_ptr<TransitionRunner> transition_;
    // Frame buffer of the animation, recycled from frame to frame.
    std::vector<double> frameBuffer_;
    // Render revision of the data points last uploaded to the GPU.
    std::uint64_t uploadedRevision_{0};
    // Whether the last frame held more data points than the GPU buffers do; warns on each crossing.
    bool overCapacity_{false};
    // Error bars the GPU draws, as of the last frame; those beyond are not hovered either.
    int renderCapacity_{std::numeric_limits<int>::max()};
    // Data-space rectangles of the error bars that bound the hover candidates.
    mutable std::vector<double> hitRects_;
    mutable SpatialGrid spatialGrid_;
    mutable bool spatialGridValid_{false};
};

} // namespace QAccelPlot
```


