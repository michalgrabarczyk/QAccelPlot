

# File QAccelPlot.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot.hpp**](QAccelPlot_8hpp.md)

[Go to the documentation of this file](QAccelPlot_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "PlotBorder.hpp"
#include "PlotMouseEvent.hpp"
#include "axis/Axis.hpp"
#include "grid/Grid.hpp"
#include "series/PlotSeries.hpp"

#include <QColor>
#include <QKeyEvent>
#include <QPointF>
#include <QQmlListProperty>
#include <QQuickItem>
#include <QRectF>

namespace QAccelPlot {

class GridNode;

class QAccelPlot : public QQuickItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(PlotView)

    
    Q_PROPERTY(Axis* xAxis READ xAxis WRITE setXAxis NOTIFY xAxisChanged)
    Q_PROPERTY(Axis* yAxis READ yAxis WRITE setYAxis NOTIFY yAxisChanged)
    Q_PROPERTY(Axis* x2Axis READ x2Axis WRITE setX2Axis NOTIFY x2AxisChanged)
    Q_PROPERTY(Axis* y2Axis READ y2Axis WRITE setY2Axis NOTIFY y2AxisChanged)
    Q_PROPERTY(QQmlListProperty<Axis> extraAxes READ extraAxes)
    Q_PROPERTY(qreal padding READ padding WRITE setPadding NOTIFY paddingChanged)
    Q_PROPERTY(QRectF plotRect READ plotRect NOTIFY plotRectChanged)
    Q_PROPERTY(QColor plotAreaColor READ plotAreaColor WRITE setPlotAreaColor NOTIFY plotAreaColorChanged)
    Q_PROPERTY(QColor axesAreaColor READ axesAreaColor WRITE setAxesAreaColor NOTIFY axesAreaColorChanged)
    Q_PROPERTY(PlotBorder* border READ border CONSTANT)
    Q_PROPERTY(Grid* grid READ grid CONSTANT)
    Q_PROPERTY(QList<PlotSeries*> series READ series NOTIFY seriesChanged)

public:
    explicit QAccelPlot(QQuickItem* parent = nullptr);
    ~QAccelPlot() override;

    Q_INVOKABLE qreal dataToPixelX(qreal dataValue) const;
    Q_INVOKABLE qreal dataToPixelY(qreal dataValue) const;
    Q_INVOKABLE qreal pixelToDataX(qreal pixelX) const;
    Q_INVOKABLE qreal pixelToDataY(qreal pixelY) const;
    Q_INVOKABLE bool isInsidePlotArea(qreal x, qreal y) const;

    Axis* xAxis() const;
    void setXAxis(Axis* axis);

    Axis* yAxis() const;
    void setYAxis(Axis* axis);

    Axis* x2Axis() const;
    void setX2Axis(Axis* axis);

    Axis* y2Axis() const;
    void setY2Axis(Axis* axis);

    QQmlListProperty<Axis> extraAxes();

    qreal padding() const;
    void setPadding(qreal p);

    QRectF plotRect() const;

    QColor plotAreaColor() const;
    void setPlotAreaColor(const QColor& c);

    QColor axesAreaColor() const;
    void setAxesAreaColor(const QColor& c);

    PlotBorder* border() const;

    Grid* grid() const;

    QList<PlotSeries*> series() const;

signals:
    void xAxisChanged();
    void yAxisChanged();
    void x2AxisChanged();
    void y2AxisChanged();
    void paddingChanged();
    void plotRectChanged();
    void plotAreaColorChanged();
    void axesAreaColorChanged();
    void seriesChanged();
    void mousePressed(::QAccelPlot::PlotMouseEvent* event);
    void mouseReleased(::QAccelPlot::PlotMouseEvent* event);
    void mouseDoubleClicked(::QAccelPlot::PlotMouseEvent* event);
    void mouseMoved(::QAccelPlot::PlotMouseEvent* event);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override; 
    void itemChange(ItemChange change, const ItemChangeData& value) override;
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

    Q_INVOKABLE void rescaleAllAxes();

private:
    static void appendExtraAxis(QQmlListProperty<Axis>* list, Axis* axis);
    static qsizetype extraAxisCount(QQmlListProperty<Axis>* list);
    static Axis* extraAxis(QQmlListProperty<Axis>* list, qsizetype index);
    static void clearExtraAxes(QQmlListProperty<Axis>* list);

    void connectAxisSignals(Axis* axis);
    void disconnectAxisSignals(Axis* axis);
    void axisDestroyed(QObject* object);
    void connectAxis(Axis* axis, Axis::Side side);
    void disconnectAxis(Axis* axis);
    bool tryZoomAxisAtPosition(Axis* axis, const QPointF& pos, bool zoomingIn);
    bool tryForwardKeyEventToAxis(Axis* axis, const QPointF& mousePos, QKeyEvent* event);
    void zoomAxisAtRatio(Axis* axis, qreal ratio, bool zoomingIn);
    void zoomAxis(Axis* axis, qreal factor, qreal centerRatio);
    void panAxis(Axis* axis, qreal delta, qreal length);
    void layoutAxes();

    Axis* xAxis_{nullptr};
    Axis* yAxis_{nullptr};
    Axis* x2Axis_{nullptr};
    Axis* y2Axis_{nullptr};
    QList<Axis*> extraAxes_;

    bool isDragging_{false};
    QPointF lastMousePos_;
    PlotMouseEvent mousePressEvent_{this};
    PlotMouseEvent mouseReleaseEvent_{this};
    PlotMouseEvent mouseDoubleClickEvent_{this};
    PlotMouseEvent mouseMoveEvent_{this};
    qreal padding_{24.0};
    QRectF plotRect_;
    QColor plotAreaColor_{QColor("#d3d3d3")}; // lightgrey
    QColor axesAreaColor_{QColor("#c8c8c8")}; // slightly darker
    PlotBorder* border_{nullptr};
    Grid* grid_{nullptr};
    GridNode* gridNode_{nullptr};
    QList<PlotSeries*> series_;
};

} // namespace QAccelPlot
```


