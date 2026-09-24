//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/PlotMouseEvent.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/grid/Grid.hpp"
#include "QAccelPlot/grid/GridNode.hpp"

#include <QCoreApplication>
#include <QHoverEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QQuickWindow>
#include <QSGRectangleNode>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

namespace {

// Zoom scale factor applied when no axis is configured (a sensible 10% zoom per step).
constexpr auto kDefaultZoomScaleFactor = qreal{0.9};
// Clamped lower bound for the effective zoom factor: prevents a zero or negative viewport range.
constexpr auto kMinEffectiveZoomFactor = qreal{0.01};
// Clamped upper bound for the effective zoom factor: prevents the factor from reaching 1.0 (no-op zoom).
constexpr auto kMaxEffectiveZoomFactor = qreal{0.99};

qreal effectiveZoomScaleFactor(const Axis* axis)
{
    if (!axis) {
        return kDefaultZoomScaleFactor;
    }

    const auto factor = axis->zoomScaleFactor();
    if (factor <= 0.0) {
        return kMinEffectiveZoomFactor;
    }
    if (factor >= 1.0) {
        return kMaxEffectiveZoomFactor;
    }
    return static_cast<qreal>(factor);
}

qreal horizontalRatio(const QPointF& pos, const QRectF& rect)
{
    return qBound(0.0, (pos.x() - rect.x()) / rect.width(), 1.0);
}

qreal verticalRatio(const QPointF& pos, const QRectF& rect)
{
    return qBound(0.0, 1.0 - (pos.y() - rect.y()) / rect.height(), 1.0);
}

}

QAccelPlot::QAccelPlot(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    setFlag(ItemAcceptsInputMethod, false);
    setFocus(true);
    grid_ = new Grid(this);
    border_ = new PlotBorder(this);
    connect(grid_, &Grid::lineWidthChanged, this, &QAccelPlot::update);
    connect(grid_, &Grid::subGridLineWidthChanged, this, &QAccelPlot::update);
    connect(grid_, &Grid::gridColorChanged, this, &QAccelPlot::update);
    connect(grid_, &Grid::subGridColorChanged, this, &QAccelPlot::update);
    connect(grid_, &Grid::gridVisibleChanged, this, &QAccelPlot::update);
    connect(grid_, &Grid::subGridVisibleChanged, this, &QAccelPlot::update);
    connect(grid_, &Grid::gridHorizontalLinesVisibleChanged, this, &QAccelPlot::update);
    connect(grid_, &Grid::gridVerticalLinesVisibleChanged, this, &QAccelPlot::update);
    connect(grid_, &Grid::subGridHorizontalLinesVisibleChanged, this, &QAccelPlot::update);
    connect(grid_, &Grid::subGridVerticalLinesVisibleChanged, this, &QAccelPlot::update);
    connect(border_, &PlotBorder::colorChanged, this, &QAccelPlot::update);
    connect(border_, &PlotBorder::widthChanged, this, &QAccelPlot::update);
}

QAccelPlot::~QAccelPlot()
{
    for (auto* axis : {xAxis_, yAxis_, x2Axis_, y2Axis_}) {
        if (axis) {
            disconnectAxisSignals(axis);
        }
    }

    for (auto* axis : std::as_const(extraAxes_)) {
        disconnectAxisSignals(axis);
    }
}

qreal QAccelPlot::dataToPixelX(qreal dataValue) const
{
    if (!xAxis_) {
        return 0.0;
    }
    return plotRect_.x() + xAxis_->coordToPixel(dataValue, plotRect_.width());
}

qreal QAccelPlot::dataToPixelY(qreal dataValue) const
{
    if (!yAxis_) {
        return 0.0;
    }
    return plotRect_.y() + yAxis_->coordToPixel(dataValue, plotRect_.height());
}

qreal QAccelPlot::pixelToDataX(qreal pixelX) const
{
    if (!xAxis_) {
        return 0.0;
    }
    return xAxis_->pixelToCoord(pixelX - plotRect_.x(), plotRect_.width());
}

qreal QAccelPlot::pixelToDataY(qreal pixelY) const
{
    if (!yAxis_) {
        return 0.0;
    }
    return yAxis_->pixelToCoord(pixelY - plotRect_.y(), plotRect_.height());
}

bool QAccelPlot::isInsidePlotArea(qreal x, qreal y) const
{
    return plotRect_.contains(x, y);
}

Axis* QAccelPlot::xAxis() const
{
    return xAxis_;
}

void QAccelPlot::setXAxis(Axis* axis)
{
    if (xAxis_ == axis) {
        return;
    }
    if (xAxis_) {
        disconnectAxis(xAxis_);
    }
    xAxis_ = axis;
    if (xAxis_) {
        connectAxis(xAxis_, Axis::Bottom);
    }
    emit xAxisChanged();
    layoutAxes();
}

Axis* QAccelPlot::yAxis() const
{
    return yAxis_;
}

void QAccelPlot::setYAxis(Axis* axis)
{
    if (yAxis_ == axis) {
        return;
    }
    if (yAxis_) {
        disconnectAxis(yAxis_);
    }
    yAxis_ = axis;
    if (yAxis_) {
        connectAxis(yAxis_, Axis::Left);
    }
    emit yAxisChanged();
    layoutAxes();
}

Axis* QAccelPlot::x2Axis() const
{
    return x2Axis_;
}

void QAccelPlot::setX2Axis(Axis* axis)
{
    if (x2Axis_ == axis) {
        return;
    }
    if (x2Axis_) {
        disconnectAxis(x2Axis_);
    }
    x2Axis_ = axis;
    if (x2Axis_) {
        connectAxis(x2Axis_, Axis::Top);
    }
    emit x2AxisChanged();
    layoutAxes();
}

Axis* QAccelPlot::y2Axis() const
{
    return y2Axis_;
}

void QAccelPlot::setY2Axis(Axis* axis)
{
    if (y2Axis_ == axis) {
        return;
    }
    if (y2Axis_) {
        disconnectAxis(y2Axis_);
    }
    y2Axis_ = axis;
    if (y2Axis_) {
        connectAxis(y2Axis_, Axis::Right);
    }
    emit y2AxisChanged();
    layoutAxes();
}

QQmlListProperty<Axis> QAccelPlot::extraAxes()
{
    return QQmlListProperty<Axis>(
        this, &extraAxes_, &QAccelPlot::appendExtraAxis, &QAccelPlot::extraAxisCount, &QAccelPlot::extraAxis, &QAccelPlot::clearExtraAxes);
}

qreal QAccelPlot::padding() const
{
    return padding_;
}

void QAccelPlot::setPadding(const qreal p)
{
    if (nearly_equal(padding_, p)) {
        return;
    }
    padding_ = p;
    emit paddingChanged();
    layoutAxes();
}

QRectF QAccelPlot::plotRect() const
{
    return plotRect_;
}

QColor QAccelPlot::plotAreaColor() const
{
    return plotAreaColor_;
}

void QAccelPlot::setPlotAreaColor(const QColor& c)
{
    if (plotAreaColor_ == c) {
        return;
    }
    plotAreaColor_ = c;
    emit plotAreaColorChanged();
    update();
}

QColor QAccelPlot::axesAreaColor() const
{
    return axesAreaColor_;
}

Grid* QAccelPlot::grid() const
{
    return grid_;
}

QList<PlotSeries*> QAccelPlot::series() const
{
    return series_;
}

void QAccelPlot::setAxesAreaColor(const QColor& c)
{
    if (axesAreaColor_ == c) {
        return;
    }
    axesAreaColor_ = c;
    emit axesAreaColorChanged();
    update();
}

PlotBorder* QAccelPlot::border() const
{
    return border_;
}

void QAccelPlot::wheelEvent(QWheelEvent* event)
{
    if (event->angleDelta().y() == 0) {
        QQuickItem::wheelEvent(event);
        return;
    }

    const auto pos = event->position();
    const auto zoomingIn = event->angleDelta().y() > 0;

    // First try zooming any axis under the cursor, prioritizing primary axes over extra axes
    for (const auto axis : {xAxis_, x2Axis_, yAxis_, y2Axis_}) {
        if (tryZoomAxisAtPosition(axis, pos, zoomingIn)) {
            event->accept();
            return;
        }
    }

    for (const auto axis : extraAxes_) {
        if (tryZoomAxisAtPosition(axis, pos, zoomingIn)) {
            event->accept();
            return;
        }
    }

    // Zoom all axes if cursor is inside the plot area
    if (!plotRect_.isEmpty() && plotRect_.contains(pos)) {
        const auto xRatio = horizontalRatio(pos, plotRect_);
        const auto yRatio = verticalRatio(pos, plotRect_);

        zoomAxisAtRatio(xAxis_, xRatio, zoomingIn);
        zoomAxisAtRatio(x2Axis_, xRatio, zoomingIn);
        zoomAxisAtRatio(yAxis_, yRatio, zoomingIn);
        zoomAxisAtRatio(y2Axis_, yRatio, zoomingIn);
        for (const auto axis : extraAxes_) {
            const auto ratio = (axis->orientation() == Axis::Horizontal) ? xRatio : yRatio;
            zoomAxisAtRatio(axis, ratio, zoomingIn);
        }
        event->accept();
        return;
    }

    QQuickItem::wheelEvent(event);
}

void QAccelPlot::mousePressEvent(QMouseEvent* event)
{
    pointerPos_ = event->position();
    mousePressEvent_.reset(static_cast<int>(event->button()), event->position().x(), event->position().y(), static_cast<int>(event->modifiers()));
    emit mousePressed(&mousePressEvent_);
    if (mousePressEvent_.isAccepted()) {
        return;
    }
    if (event->button() == Qt::LeftButton) {
        setFocus(true);
        isDragging_ = true;
        lastMousePos_ = event->position();
        event->accept();
    } else {
        QQuickItem::mousePressEvent(event);
    }
}

void QAccelPlot::hoverEnterEvent(QHoverEvent* event)
{
    pointerPos_ = event->position();
    QQuickItem::hoverEnterEvent(event);
}

void QAccelPlot::hoverMoveEvent(QHoverEvent* event)
{
    pointerPos_ = event->position();
    if (isDragging_) {
        QQuickItem::hoverMoveEvent(event);
        return;
    }

    mouseMoveEvent_.reset(static_cast<int>(Qt::NoButton), event->position().x(), event->position().y(), static_cast<int>(event->modifiers()));
    emit mouseMoved(&mouseMoveEvent_);
    if (mouseMoveEvent_.isAccepted()) {
        event->accept();
        return;
    }

    QQuickItem::hoverMoveEvent(event);
}

void QAccelPlot::hoverLeaveEvent(QHoverEvent* event)
{
    pointerPos_.reset();
    QQuickItem::hoverLeaveEvent(event);
}

void QAccelPlot::mouseMoveEvent(QMouseEvent* event)
{
    pointerPos_ = event->position();
    mouseMoveEvent_.reset(static_cast<int>(event->buttons()), event->position().x(), event->position().y(), static_cast<int>(event->modifiers()));
    emit mouseMoved(&mouseMoveEvent_);
    if (mouseMoveEvent_.isAccepted()) {
        return;
    }
    if (isDragging_) {
        const auto delta = event->position() - lastMousePos_;
        lastMousePos_ = event->position();

        // Use plot area dimensions for accurate panning speed
        const auto pw = plotRect_.width() > 0 ? plotRect_.width() : width();
        const auto ph = plotRect_.height() > 0 ? plotRect_.height() : height();

        panAxis(xAxis_, -delta.x(), pw);
        panAxis(x2Axis_, -delta.x(), pw);
        panAxis(yAxis_, delta.y(), ph);
        panAxis(y2Axis_, delta.y(), ph);

        for (const auto axis : extraAxes_) {
            if (axis->orientation() == Axis::Horizontal) {
                panAxis(axis, -delta.x(), pw);
            } else {
                panAxis(axis, delta.y(), ph);
            }
        }
        event->accept();
    } else {
        QQuickItem::mouseMoveEvent(event);
    }
}

void QAccelPlot::mouseReleaseEvent(QMouseEvent* event)
{
    const auto endedDrag = event->button() == Qt::LeftButton && isDragging_;
    if (event->button() == Qt::LeftButton) {
        isDragging_ = false;
    }

    mouseReleaseEvent_.reset(static_cast<int>(event->button()), event->position().x(), event->position().y(), static_cast<int>(event->modifiers()));
    emit mouseReleased(&mouseReleaseEvent_);
    if (mouseReleaseEvent_.isAccepted()) {
        return;
    }
    if (endedDrag) {
        event->accept();
    } else {
        QQuickItem::mouseReleaseEvent(event);
    }
}

void QAccelPlot::mouseDoubleClickEvent(QMouseEvent* event)
{
    mouseDoubleClickEvent_.reset(static_cast<int>(event->button()), event->position().x(), event->position().y(), static_cast<int>(event->modifiers()));
    emit mouseDoubleClicked(&mouseDoubleClickEvent_);
    if (mouseDoubleClickEvent_.isAccepted()) {
        return;
    }
    if (event->button() == Qt::LeftButton) {
        rescaleAllAxes();
        event->accept();
    } else {
        QQuickItem::mouseDoubleClickEvent(event);
    }
}

void QAccelPlot::keyPressEvent(QKeyEvent* event)
{
    if (!pointerPos_) {
        QQuickItem::keyPressEvent(event);
        return;
    }

    for (const auto axis : {xAxis_, yAxis_, x2Axis_, y2Axis_}) {
        if (tryForwardKeyEventToAxis(axis, *pointerPos_, event) && event->isAccepted()) {
            return;
        }
    }

    for (Axis* axis : extraAxes_) {
        if (tryForwardKeyEventToAxis(axis, *pointerPos_, event) && event->isAccepted()) {
            return;
        }
    }

    QQuickItem::keyPressEvent(event);
}

void QAccelPlot::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    layoutAxes();
}

void QAccelPlot::itemChange(ItemChange change, const ItemChangeData& value)
{
    QQuickItem::itemChange(change, value);
    if (change == ItemChildAddedChange) {
        if (auto* series = qobject_cast<PlotSeries*>(value.item)) {
            if (!series_.contains(series)) {
                series->setPlotRect(plotRect_);
                series_.append(series);
                emit seriesChanged();
            }
        }
    } else if (change == ItemChildRemovedChange) {
        // During QQuickItem destruction the child no longer casts to PlotSeries.
        const auto removed = series_.removeIf([item = value.item](const PlotSeries* series) { return series == item; });
        if (removed > 0) {
            emit seriesChanged();
        }
    }
}

QSGNode* QAccelPlot::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    auto* root = oldNode ? static_cast<QSGNode*>(oldNode) : new QSGNode;

    // First call: build persistent child node structure
    if (root->childCount() == 0) {
        auto* axesBg = window()->createRectangleNode();
        auto* plotBg = window()->createRectangleNode();
        gridNode_ = new GridNode;
        auto* borderLeft = window()->createRectangleNode();
        auto* borderTop = window()->createRectangleNode();
        auto* borderRight = window()->createRectangleNode();
        auto* borderBottom = window()->createRectangleNode();
        root->appendChildNode(axesBg);
        root->appendChildNode(plotBg);
        root->appendChildNode(gridNode_);
        root->appendChildNode(borderLeft);
        root->appendChildNode(borderTop);
        root->appendChildNode(borderRight);
        root->appendChildNode(borderBottom);
    }

    auto* axesBg = static_cast<QSGRectangleNode*>(root->firstChild());
    auto* plotBg = static_cast<QSGRectangleNode*>(axesBg->nextSibling());
    auto* borderLeft = static_cast<QSGRectangleNode*>(gridNode_->nextSibling());
    auto* borderTop = static_cast<QSGRectangleNode*>(borderLeft->nextSibling());
    auto* borderRight = static_cast<QSGRectangleNode*>(borderTop->nextSibling());
    auto* borderBottom = static_cast<QSGRectangleNode*>(borderRight->nextSibling());

    axesBg->setRect(QRectF(0, 0, width(), height()));
    axesBg->setColor(axesAreaColor_);

    plotBg->setRect(plotRect_);
    plotBg->setColor(plotAreaColor_);

    gridNode_->update(grid_, xAxis_, yAxis_, plotRect_);

    const auto borderWidth = std::min({border_->width(), plotRect_.width() / 2.0, plotRect_.height() / 2.0});
    const auto borderColor = borderWidth > 0.0 ? border_->color() : QColor(Qt::transparent);
    borderLeft->setRect(QRectF(plotRect_.left(), plotRect_.top(), borderWidth, plotRect_.height()));
    borderTop->setRect(QRectF(plotRect_.left(), plotRect_.top(), plotRect_.width(), borderWidth));
    borderRight->setRect(QRectF(plotRect_.right() - borderWidth, plotRect_.top(), borderWidth, plotRect_.height()));
    borderBottom->setRect(QRectF(plotRect_.left(), plotRect_.bottom() - borderWidth, plotRect_.width(), borderWidth));
    borderLeft->setColor(borderColor);
    borderTop->setColor(borderColor);
    borderRight->setColor(borderColor);
    borderBottom->setColor(borderColor);

    return root;
}

void QAccelPlot::rescaleAllAxes()
{
    for (const auto axis : {xAxis_, yAxis_, x2Axis_, y2Axis_}) {
        if (axis) {
            axis->rescaleToData();
        }
    }
    for (const auto axis : extraAxes_) {
        axis->rescaleToData();
    }
}

void QAccelPlot::appendExtraAxis(QQmlListProperty<Axis>* list, Axis* axis)
{
    auto* plot = list ? qobject_cast<QAccelPlot*>(list->object) : nullptr;
    if (!plot || !axis) {
        return;
    }
    // An axis listed twice would otherwise be laid out twice, reserving its space twice.
    if (plot->extraAxes_.contains(axis)) {
        qCDebug(lcQAccelPlot) << "extra axis already registered, ignoring duplicate append";
        return;
    }

    axis->setParentItem(plot);
    plot->connectAxisSignals(axis);
    plot->extraAxes_.append(axis);
    plot->layoutAxes();
}

qsizetype QAccelPlot::extraAxisCount(QQmlListProperty<Axis>* list)
{
    const auto* plot = list ? qobject_cast<QAccelPlot*>(list->object) : nullptr;
    return plot ? plot->extraAxes_.size() : 0;
}

Axis* QAccelPlot::extraAxis(QQmlListProperty<Axis>* list, qsizetype index)
{
    const auto* plot = list ? qobject_cast<QAccelPlot*>(list->object) : nullptr;
    if (!plot || index < 0 || index >= plot->extraAxes_.size()) {
        qCDebug(lcQAccelPlot) << "invalid list or out-of-bounds extra axis index" << index << ", returning nullptr";
        return nullptr;
    }

    return plot->extraAxes_.at(index);
}

void QAccelPlot::clearExtraAxes(QQmlListProperty<Axis>* list)
{
    auto* plot = list ? qobject_cast<QAccelPlot*>(list->object) : nullptr;
    if (!plot) {
        return;
    }

    for (const auto axis : plot->extraAxes_) {
        plot->disconnectAxisSignals(axis);
        axis->setParentItem(nullptr);
    }
    plot->extraAxes_.clear();
    plot->layoutAxes();
}

bool QAccelPlot::tryForwardKeyEventToAxis(Axis* axis, const QPointF& mousePos, QKeyEvent* event)
{
    if (!axis || !axis->isVisible()) {
        return false;
    }
    const QRectF rect(axis->position(), axis->size());
    if (rect.contains(mousePos)) {
        QCoreApplication::sendEvent(axis, event);
        return true;
    }
    return false;
}

bool QAccelPlot::tryZoomAxisAtPosition(Axis* axis, const QPointF& pos, const bool zoomingIn)
{
    if (!axis || !axis->isVisible()) {
        return false;
    }
    const auto rect = QRectF(axis->position(), axis->size());
    if (!rect.contains(pos)) {
        return false;
    }
    const auto ratio = (axis->orientation() == Axis::Horizontal) ? horizontalRatio(pos, rect) : verticalRatio(pos, rect);
    zoomAxisAtRatio(axis, ratio, zoomingIn);
    return true;
}

void QAccelPlot::zoomAxisAtRatio(Axis* axis, const qreal ratio, const bool zoomingIn)
{
    if (!axis) {
        return;
    }
    const auto zoomInFactor = effectiveZoomScaleFactor(axis);
    const auto factor = zoomingIn ? zoomInFactor : (1.0 / zoomInFactor);
    zoomAxis(axis, factor, ratio);
}

void QAccelPlot::connectAxisSignals(Axis* axis)
{
    connect(axis, &Axis::doubleClicked, axis, &Axis::rescaleToData, Qt::UniqueConnection);
    connect(axis, &Axis::viewportMinChanged, this, &QAccelPlot::update, Qt::UniqueConnection);
    connect(axis, &Axis::viewportMaxChanged, this, &QAccelPlot::update, Qt::UniqueConnection);
    connect(axis, &Axis::dataMinChanged, this, &QAccelPlot::update, Qt::UniqueConnection);
    connect(axis, &Axis::dataMaxChanged, this, &QAccelPlot::update, Qt::UniqueConnection);
    connect(axis, &Axis::rangeChanged, this, &QAccelPlot::update, Qt::UniqueConnection);
    connect(axis, &QQuickItem::visibleChanged, this, &QAccelPlot::layoutAxes, Qt::UniqueConnection);
    connect(axis, &Axis::layoutSizeChanged, this, &QAccelPlot::layoutAxes, Qt::UniqueConnection);
    connect(axis, &QObject::destroyed, this, &QAccelPlot::axisDestroyed, Qt::UniqueConnection);
    connect(axis->ticker(), &AxisTicker::tickCountChanged, this, &QAccelPlot::update, Qt::UniqueConnection);
    connect(axis->ticker(), &AxisTicker::subtickCountChanged, this, &QAccelPlot::update, Qt::UniqueConnection);
    connect(axis, &Axis::axisLinePaddingChanged, this, &QAccelPlot::layoutAxes, Qt::UniqueConnection);
    connect(axis, &Axis::orientationChanged, this, &QAccelPlot::layoutAxes, Qt::UniqueConnection);
    connect(axis->ticker(), &AxisTicker::tickLengthInChanged, this, &QAccelPlot::layoutAxes, Qt::UniqueConnection);
    connect(axis->ticker(), &AxisTicker::subtickLengthInChanged, this, &QAccelPlot::layoutAxes, Qt::UniqueConnection);
}

void QAccelPlot::disconnectAxisSignals(Axis* axis)
{
    disconnect(axis, &Axis::doubleClicked, axis, &Axis::rescaleToData);
    disconnect(axis, &Axis::viewportMinChanged, this, &QAccelPlot::update);
    disconnect(axis, &Axis::viewportMaxChanged, this, &QAccelPlot::update);
    disconnect(axis, &Axis::dataMinChanged, this, &QAccelPlot::update);
    disconnect(axis, &Axis::dataMaxChanged, this, &QAccelPlot::update);
    disconnect(axis, &Axis::rangeChanged, this, &QAccelPlot::update);
    disconnect(axis, &QQuickItem::visibleChanged, this, &QAccelPlot::layoutAxes);
    disconnect(axis, &Axis::layoutSizeChanged, this, &QAccelPlot::layoutAxes);
    disconnect(axis, &QObject::destroyed, this, &QAccelPlot::axisDestroyed);
    disconnect(axis->ticker(), &AxisTicker::tickCountChanged, this, &QAccelPlot::update);
    disconnect(axis->ticker(), &AxisTicker::subtickCountChanged, this, &QAccelPlot::update);
    disconnect(axis, &Axis::axisLinePaddingChanged, this, &QAccelPlot::layoutAxes);
    disconnect(axis, &Axis::orientationChanged, this, &QAccelPlot::layoutAxes);
    disconnect(axis->ticker(), &AxisTicker::tickLengthInChanged, this, &QAccelPlot::layoutAxes);
    disconnect(axis->ticker(), &AxisTicker::subtickLengthInChanged, this, &QAccelPlot::layoutAxes);
}

void QAccelPlot::axisDestroyed(QObject* object)
{
    auto layoutChanged = false;
    if (xAxis_ == object) {
        xAxis_ = nullptr;
        layoutChanged = true;
        emit xAxisChanged();
    }
    if (yAxis_ == object) {
        yAxis_ = nullptr;
        layoutChanged = true;
        emit yAxisChanged();
    }
    if (x2Axis_ == object) {
        x2Axis_ = nullptr;
        layoutChanged = true;
        emit x2AxisChanged();
    }
    if (y2Axis_ == object) {
        y2Axis_ = nullptr;
        layoutChanged = true;
        emit y2AxisChanged();
    }

    const auto removedExtraAxis = extraAxes_.removeIf([object](Axis* axis) { return axis == object; });
    if (layoutChanged || removedExtraAxis > 0) {
        layoutAxes();
    }
}

void QAccelPlot::connectAxis(Axis* axis, Axis::Side side)
{
    axis->setParentItem(this);
    axis->setSide(side);
    connectAxisSignals(axis);
}

void QAccelPlot::disconnectAxis(Axis* axis)
{
    disconnectAxisSignals(axis);
    axis->setParentItem(nullptr);
}

void QAccelPlot::zoomAxis(Axis* axis, const qreal factor, const qreal centerRatio)
{
    if (!axis) {
        return;
    }
    if (axis->logScale() && axis->viewportMin() > 0.0 && axis->viewportMax() > 0.0) {
        const auto logMin = std::log10(axis->viewportMin());
        const auto logMax = std::log10(axis->viewportMax());
        const auto logRange = logMax - logMin;
        const auto logCenter = logMin + logRange * centerRatio;
        const auto newLogRange = logRange * factor;
        axis->setViewportMin(std::pow(10.0, logCenter - newLogRange * centerRatio));
        axis->setViewportMax(std::pow(10.0, logCenter + newLogRange * (1.0 - centerRatio)));
    } else {
        const auto range = axis->viewportMax() - axis->viewportMin();
        const auto center = axis->viewportMin() + range * centerRatio;
        const auto newRange = range * factor;
        axis->setViewportMin(center - newRange * centerRatio);
        axis->setViewportMax(center + newRange * (1.0 - centerRatio));
    }
}

void QAccelPlot::panAxis(Axis* axis, const qreal delta, const qreal length)
{
    if (!axis || length == 0) {
        return;
    }
    if (axis->logScale() && axis->viewportMin() > 0.0 && axis->viewportMax() > 0.0) {
        const auto logMin = std::log10(axis->viewportMin());
        const auto logMax = std::log10(axis->viewportMax());
        const auto logShift = (delta / length) * (logMax - logMin);
        axis->setViewportMin(std::pow(10.0, logMin + logShift));
        axis->setViewportMax(std::pow(10.0, logMax + logShift));
    } else {
        const auto range = axis->viewportMax() - axis->viewportMin();
        const auto shift = (delta / length) * range;
        axis->setViewportMin(axis->viewportMin() + shift);
        axis->setViewportMax(axis->viewportMax() + shift);
    }
}

void QAccelPlot::layoutAxes()
{
    const auto w = width();
    const auto h = height();

    // Extra axes reserve layout space on their own declared side, stacked outward
    // (away from the plot) beyond the primary axis on that side.
    auto extraTopHeight = qreal{0};
    auto extraBottomHeight = qreal{0};
    auto extraLeftWidth = qreal{0};
    auto extraRightWidth = qreal{0};
    for (const auto* axis : extraAxes_) {
        if (!axis->isVisible()) {
            continue;
        }
        if (axis->orientation() == Axis::Horizontal) {
            if (axis->side() == Axis::Top) {
                extraTopHeight += axis->layoutSize() + axis->inwardTickOverlap();
            } else {
                extraBottomHeight += axis->layoutSize() + axis->inwardTickOverlap();
            }
        } else {
            if (axis->side() == Axis::Left) {
                extraLeftWidth += axis->layoutSize() + axis->inwardTickOverlap();
            } else {
                extraRightWidth += axis->layoutSize() + axis->inwardTickOverlap();
            }
        }
    }

    const auto visibleAxisSize = [](const Axis* axis) { return axis && axis->isVisible() ? axis->layoutSize() : qreal{0}; };
    const auto leftW = visibleAxisSize(yAxis_) + extraLeftWidth;
    const auto rightW = visibleAxisSize(y2Axis_) + extraRightWidth;
    const auto topH = visibleAxisSize(x2Axis_) + extraTopHeight;
    const auto botH = visibleAxisSize(xAxis_) + extraBottomHeight;

    const auto plotX = padding_ + leftW;
    const auto plotY = padding_ + topH;
    const auto plotW = std::max(1.0, w - 2.0 * padding_ - leftW - rightW);
    const auto plotH = std::max(1.0, h - 2.0 * padding_ - topH - botH);

    const auto newRect = QRectF(plotX, plotY, plotW, plotH);
    if (newRect != plotRect_) {
        plotRect_ = newRect;
        for (auto* series : std::as_const(series_)) {
            series->setPlotRect(plotRect_);
        }
        emit plotRectChanged();
    }

    if (yAxis_ && yAxis_->isVisible()) {
        const auto ov = yAxis_->inwardTickOverlap();
        const auto lOv = yAxis_->labelOverflow();
        yAxis_->setPosition(QPointF(plotX - yAxis_->layoutSize(), plotY - lOv));
        yAxis_->setSize(QSizeF(yAxis_->layoutSize() + ov, plotH + 2.0 * lOv));
    } else if (yAxis_) {
        yAxis_->setSize(QSizeF{});
    }
    if (y2Axis_ && y2Axis_->isVisible()) {
        const auto ov = y2Axis_->inwardTickOverlap();
        const auto lOv = y2Axis_->labelOverflow();
        y2Axis_->setPosition(QPointF(plotX + plotW - ov, plotY - lOv));
        y2Axis_->setSize(QSizeF(y2Axis_->layoutSize() + ov, plotH + 2.0 * lOv));
    } else if (y2Axis_) {
        y2Axis_->setSize(QSizeF{});
    }
    if (xAxis_ && xAxis_->isVisible()) {
        const auto ov = xAxis_->inwardTickOverlap();
        const auto lOv = xAxis_->labelOverflow();
        xAxis_->setPosition(QPointF(plotX - lOv, plotY + plotH - ov));
        xAxis_->setSize(QSizeF(plotW + 2.0 * lOv, xAxis_->layoutSize() + ov));
    } else if (xAxis_) {
        xAxis_->setSize(QSizeF{});
    }
    if (x2Axis_ && x2Axis_->isVisible()) {
        const auto ov = x2Axis_->inwardTickOverlap();
        const auto lOv = x2Axis_->labelOverflow();
        x2Axis_->setPosition(QPointF(plotX - lOv, plotY - x2Axis_->layoutSize()));
        x2Axis_->setSize(QSizeF(plotW + 2.0 * lOv, x2Axis_->layoutSize() + ov));
    } else if (x2Axis_) {
        x2Axis_->setSize(QSizeF{});
    }

    // Stack extra axes outward from the plot on their own side: Bottom/Right extras grow
    // away from the plot past their primary axis; Top/Left extras grow away from the plot
    // starting at the widget edge, ending where their primary axis begins.
    auto currentBottom = plotY + plotH + visibleAxisSize(xAxis_);
    auto currentTop = padding_;
    auto currentLeft = padding_;
    auto currentRight = w - padding_;
    for (const auto axis : extraAxes_) {
        if (!axis->isVisible()) {
            axis->setSize(QSizeF{});
            continue;
        }
        const auto ov = axis->inwardTickOverlap();
        const auto lOv = axis->labelOverflow();
        if (axis->orientation() == Axis::Horizontal) {
            axis->setSize(QSizeF(plotW + 2.0 * lOv, axis->layoutSize() + ov));
            if (axis->side() == Axis::Top) {
                axis->setPosition(QPointF(plotX - lOv, currentTop));
                currentTop += axis->layoutSize() + ov;
            } else {
                axis->setPosition(QPointF(plotX - lOv, currentBottom));
                currentBottom += axis->layoutSize() + ov;
            }
        } else {
            axis->setSize(QSizeF(axis->layoutSize() + ov, plotH + 2.0 * lOv));
            if (axis->side() == Axis::Left) {
                axis->setPosition(QPointF(currentLeft, plotY - lOv));
                currentLeft += axis->layoutSize() + ov;
            } else {
                currentRight -= axis->layoutSize() + ov;
                axis->setPosition(QPointF(currentRight, plotY - lOv));
            }
        }
    }
}

} // namespace QAccelPlot
