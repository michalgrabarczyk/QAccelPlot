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

/// \brief The main plot canvas QML item — hosts axes, curves, and a grid.
///
/// PlotView manages the layout of up to four axes (X, Y, X2, Y2) plus any number
/// of additional axes, and renders \c LineCurve children via the Qt Scene Graph.
/// Data-to-pixel and pixel-to-data conversion helpers are exposed as invokable methods
/// so overlay items can align themselves to the plot coordinate system.
///
/// \par Usage
/// \code
/// PlotView {
///     Axis { id: xAxis; side: Axis.Bottom }
///     Axis { id: yAxis; side: Axis.Left }
///     xAxis: xAxis
///     yAxis: yAxis
///     LineCurve { xAxis: xAxis; yAxis: yAxis; color: "steelblue" }
/// }
/// \endcode
///
/// \sa Axis, LineCurve, Grid
class QAccelPlot : public QQuickItem {
    Q_OBJECT
    QML_NAMED_ELEMENT(PlotView)

    /// \brief Primary horizontal (bottom/top) axis.
    Q_PROPERTY(Axis* xAxis READ xAxis WRITE setXAxis NOTIFY xAxisChanged)
    /// \brief Primary vertical (left/right) axis.
    Q_PROPERTY(Axis* yAxis READ yAxis WRITE setYAxis NOTIFY yAxisChanged)
    /// \brief Optional secondary horizontal axis (opposite side to xAxis).
    Q_PROPERTY(Axis* x2Axis READ x2Axis WRITE setX2Axis NOTIFY x2AxisChanged)
    /// \brief Optional secondary vertical axis (opposite side to yAxis).
    Q_PROPERTY(Axis* y2Axis READ y2Axis WRITE setY2Axis NOTIFY y2AxisChanged)
    /// \brief Additional axes beyond the primary four (linked via curves' axis properties).
    Q_PROPERTY(QQmlListProperty<Axis> extraAxes READ extraAxes)
    /// \brief Uniform padding in pixels between the plot area and the canvas edge. Default: 24.
    Q_PROPERTY(qreal padding READ padding WRITE setPadding NOTIFY paddingChanged)
    /// \brief Read-only: plot area rectangle in item-local pixel coordinates.
    Q_PROPERTY(QRectF plotRect READ plotRect NOTIFY plotRectChanged)
    /// \brief Background color of the plot data area. Default: d3d3d3.
    Q_PROPERTY(QColor plotAreaColor READ plotAreaColor WRITE setPlotAreaColor NOTIFY plotAreaColorChanged)
    /// \brief Background color of the axes surround area. Default: c8c8c8.
    Q_PROPERTY(QColor axesAreaColor READ axesAreaColor WRITE setAxesAreaColor NOTIFY axesAreaColorChanged)
    /// \brief Decorative frame configuration for the plot area.
    Q_PROPERTY(PlotBorder* border READ border CONSTANT)
    /// \brief Read-only constant: grid configuration object.
    Q_PROPERTY(Grid* grid READ grid CONSTANT)
    /// \brief Read-only: all registered plot series.
    Q_PROPERTY(QList<PlotSeries*> series READ series NOTIFY seriesChanged)

public:
    /// \brief Constructs a PlotView with the given \a parent.
    explicit QAccelPlot(QQuickItem* parent = nullptr);
    /// \brief Destroys the plot after disconnecting attached axis signals.
    ~QAccelPlot() override;

    /// \brief Converts a horizontal data-space value to an item-local pixel X coordinate.
    Q_INVOKABLE qreal dataToPixelX(qreal dataValue) const;
    /// \brief Converts a vertical data-space value to an item-local pixel Y coordinate.
    Q_INVOKABLE qreal dataToPixelY(qreal dataValue) const;
    /// \brief Converts an item-local pixel X coordinate to a horizontal data-space value.
    Q_INVOKABLE qreal pixelToDataX(qreal pixelX) const;
    /// \brief Converts an item-local pixel Y coordinate to a vertical data-space value.
    Q_INVOKABLE qreal pixelToDataY(qreal pixelY) const;
    /// \brief Returns \c true if the item-local point (\a x, \a y) lies inside the plot area.
    Q_INVOKABLE bool isInsidePlotArea(qreal x, qreal y) const;

    /// \brief Returns the primary horizontal axis.
    Axis* xAxis() const;
    /// \brief Sets the primary horizontal axis to \a axis.
    void setXAxis(Axis* axis);

    /// \brief Returns the primary vertical axis.
    Axis* yAxis() const;
    /// \brief Sets the primary vertical axis to \a axis.
    void setYAxis(Axis* axis);

    /// \brief Returns the secondary horizontal axis, or \c nullptr if not set.
    Axis* x2Axis() const;
    /// \brief Sets the secondary horizontal axis to \a axis.
    void setX2Axis(Axis* axis);

    /// \brief Returns the secondary vertical axis, or \c nullptr if not set.
    Axis* y2Axis() const;
    /// \brief Sets the secondary vertical axis to \a axis.
    void setY2Axis(Axis* axis);

    /// \brief Returns the QML list property for extra axes.
    QQmlListProperty<Axis> extraAxes();

    /// \brief Returns the uniform canvas padding in pixels.
    qreal padding() const;
    /// \brief Sets the canvas padding to \a p pixels.
    void setPadding(qreal p);

    /// \brief Returns the current plot area rectangle in item-local coordinates.
    QRectF plotRect() const;

    /// \brief Returns the plot area background color.
    QColor plotAreaColor() const;
    /// \brief Sets the plot area background color to \a c.
    void setPlotAreaColor(const QColor& c);

    /// \brief Returns the axes surround background color.
    QColor axesAreaColor() const;
    /// \brief Sets the axes surround background color to \a c.
    void setAxesAreaColor(const QColor& c);

    /// \brief Returns the decorative plot-frame configuration object.
    PlotBorder* border() const;

    /// \brief Returns the grid configuration object.
    Grid* grid() const;

    /// \brief Returns all plot-series children currently registered with this canvas.
    QList<PlotSeries*> series() const;

signals:
    /// \brief Emitted when the xAxis property changes.
    void xAxisChanged();
    /// \brief Emitted when the yAxis property changes.
    void yAxisChanged();
    /// \brief Emitted when the x2Axis property changes.
    void x2AxisChanged();
    /// \brief Emitted when the y2Axis property changes.
    void y2AxisChanged();
    /// \brief Emitted when the padding property changes.
    void paddingChanged();
    /// \brief Emitted when the plotRect changes (axis layout recalculated).
    void plotRectChanged();
    /// \brief Emitted when the plotAreaColor property changes.
    void plotAreaColorChanged();
    /// \brief Emitted when the axesAreaColor property changes.
    void axesAreaColorChanged();
    /// \brief Emitted when the set of registered plot series changes.
    void seriesChanged();
    /// \brief Emitted when a mouse button is pressed over the plot.
    /// Call \c event->accept() to consume the event and suppress built-in handling (drag start).
    void mousePressed(::QAccelPlot::PlotMouseEvent* event);
    /// \brief Emitted when a mouse button is released over the plot.
    /// Call \c event->accept() to consume further built-in handling. A left-button release always ends an active drag.
    void mouseReleased(::QAccelPlot::PlotMouseEvent* event);
    /// \brief Emitted when a mouse button is double-clicked over the plot.
    /// Call \c event->accept() to consume the event and suppress built-in handling (rescale all axes).
    void mouseDoubleClicked(::QAccelPlot::PlotMouseEvent* event);
    /// \brief Emitted when the mouse is moved over the plot.
    /// Call \c event->accept() to consume the event and suppress built-in handling (panning).
    void mouseMoved(::QAccelPlot::PlotMouseEvent* event);

protected:
    /// \cond INTERNAL
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override; ///< \brief Qt event override — internal.
    void itemChange(ItemChange change, const ItemChangeData& value) override;
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
    /// \endcond

    /// \brief Calls \c Axis::rescaleToData() on all attached axes.
    Q_INVOKABLE void rescaleAllAxes();

private:
    static void appendExtraAxis(QQmlListProperty<Axis>* list, Axis* axis);
    static qsizetype extraAxisCount(QQmlListProperty<Axis>* list);
    static Axis* extraAxis(QQmlListProperty<Axis>* list, qsizetype index);
    static void clearExtraAxes(QQmlListProperty<Axis>* list);

    void connectAxisSignals(Axis* axis);
    void disconnectAxisSignals(Axis* axis);
    void axisDestroyed(QObject* object);
    void connectAxis(Axis* axis, Axis::Orientation orientation);
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
