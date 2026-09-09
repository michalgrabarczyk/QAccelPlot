//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot.hpp"
#include "MathUtils.hpp"
#include "PlotMouseEvent.hpp"
#include "axis/Axis.hpp"
#include "grid/Grid.hpp"
#include "grid/GridNode.hpp"

#include <QCoreApplication>
#include <QCursor>
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
        connectAxis(xAxis_, Axis::Horizontal);
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
        connectAxis(yAxis_, Axis::Vertical);
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
        connectAxis(x2Axis_, Axis::Horizontal);
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
        connectAxis(y2Axis_, Axis::Vertical);
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

void QAccelPlot::mouseMoveEvent(QMouseEvent* event)
{
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
    const auto mousePos = mapFromGlobal(QPointF(QCursor::pos()));

    for (const auto axis : {xAxis_, yAxis_, x2Axis_, y2Axis_}) {
        if (tryForwardKeyEventToAxis(axis, mousePos, event) && event->isAccepted()) {
            return;
        }
    }

    for (Axis* axis : extraAxes_) {
        if (tryForwardKeyEventToAxis(axis, mousePos, event) && event->isAccepted()) {
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
        if (auto* series = qobject_cast<PlotSeries*>(value.item)) {
            if (series_.removeOne(series)) {
                emit seriesChanged();
            }
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
    QAccelPlot* plot = qobject_cast<QAccelPlot*>(list->object);
    if (axis) {
        axis->setParentItem(plot);
        connect(axis, &Axis::doubleClicked, axis, &Axis::rescaleToData);
        // Keep plot updated when extra axis ranges change so annotations follow data values
        connect(axis, &Axis::viewportMinChanged, plot, &QAccelPlot::update);
        connect(axis, &Axis::viewportMaxChanged, plot, &QAccelPlot::update);
        connect(axis, &Axis::dataMinChanged, plot, &QAccelPlot::update);
        connect(axis, &Axis::dataMaxChanged, plot, &QAccelPlot::update);
        connect(axis, &Axis::rangeChanged, plot, [plot] { plot->update(); });
        connect(axis, &QQuickItem::visibleChanged, plot, &QAccelPlot::layoutAxes);
        plot->extraAxes_.append(axis);
        plot->layoutAxes();
    }
}

qsizetype QAccelPlot::extraAxisCount(QQmlListProperty<Axis>* list)
{
    return qobject_cast<QAccelPlot*>(list->object)->extraAxes_.size();
}

Axis* QAccelPlot::extraAxis(QQmlListProperty<Axis>* list, qsizetype index)
{
    return qobject_cast<QAccelPlot*>(list->object)->extraAxes_.at(index);
}

void QAccelPlot::clearExtraAxes(QQmlListProperty<Axis>* list)
{
    QAccelPlot* plot = qobject_cast<QAccelPlot*>(list->object);
    for (const auto axis : plot->extraAxes_) {
        axis->setParentItem(nullptr);
        disconnect(axis, &Axis::doubleClicked, axis, &Axis::rescaleToData);
        disconnect(axis, &Axis::viewportMinChanged, plot, nullptr);
        disconnect(axis, &Axis::viewportMaxChanged, plot, nullptr);
        disconnect(axis, &Axis::dataMinChanged, plot, nullptr);
        disconnect(axis, &Axis::dataMaxChanged, plot, nullptr);
        disconnect(axis, &Axis::rangeChanged, plot, nullptr);
        disconnect(axis, &QQuickItem::visibleChanged, plot, nullptr);
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

void QAccelPlot::connectAxis(Axis* axis, Axis::Orientation orientation)
{
    axis->setParentItem(this);
    axis->setOrientation(orientation);
    connect(axis, &Axis::doubleClicked, axis, &Axis::rescaleToData);
    auto syncAndUpdate = [this] { update(); };
    connect(axis, &Axis::viewportMinChanged, this, syncAndUpdate);
    connect(axis, &Axis::viewportMaxChanged, this, syncAndUpdate);
    connect(axis, &Axis::dataMinChanged, this, syncAndUpdate);
    connect(axis, &Axis::dataMaxChanged, this, syncAndUpdate);
    connect(axis, &Axis::rangeChanged, this, syncAndUpdate);
    connect(axis, &QQuickItem::visibleChanged, this, &QAccelPlot::layoutAxes);
}

void QAccelPlot::disconnectAxis(Axis* axis)
{
    axis->setParentItem(nullptr);
    disconnect(axis, &Axis::doubleClicked, axis, &Axis::rescaleToData);
    disconnect(axis, &Axis::viewportMinChanged, this, nullptr);
    disconnect(axis, &Axis::viewportMaxChanged, this, nullptr);
    disconnect(axis, &Axis::dataMinChanged, this, nullptr);
    disconnect(axis, &Axis::dataMaxChanged, this, nullptr);
    disconnect(axis, &Axis::rangeChanged, this, nullptr);
    disconnect(axis, &QQuickItem::visibleChanged, this, nullptr);
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
    const auto axisSize = qreal{50};

    auto extraBottomHeight = qreal{0};
    for (const auto* axis : extraAxes_) {
        if (axis->isVisible() && axis->orientation() == Axis::Horizontal) {
            extraBottomHeight += axisSize + axis->inwardTickOverlap();
        }
    }

    const auto visibleAxisSize = [axisSize](const Axis* axis) { return axis && axis->isVisible() ? axisSize : qreal{0}; };
    const auto leftW = visibleAxisSize(yAxis_);
    const auto rightW = visibleAxisSize(y2Axis_);
    const auto topH = visibleAxisSize(x2Axis_);
    const auto botH = visibleAxisSize(xAxis_) + extraBottomHeight;

    const auto plotX = padding_ + leftW;
    const auto plotY = padding_ + topH;
    const auto plotW = w - 2.0 * padding_ - leftW - rightW;
    const auto plotH = h - 2.0 * padding_ - topH - botH;

    const auto newRect = QRectF(plotX, plotY, std::max(1.0, plotW), std::max(1.0, plotH));
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
        yAxis_->setPosition(QPointF(padding_, plotY - lOv));
        yAxis_->setSize(QSizeF(axisSize + ov, plotH + 2.0 * lOv));
    } else if (yAxis_) {
        yAxis_->setSize(QSizeF{});
    }
    if (y2Axis_ && y2Axis_->isVisible()) {
        const auto ov = y2Axis_->inwardTickOverlap();
        const auto lOv = y2Axis_->labelOverflow();
        y2Axis_->setPosition(QPointF(w - padding_ - axisSize - ov, plotY - lOv));
        y2Axis_->setSize(QSizeF(axisSize + ov, plotH + 2.0 * lOv));
    } else if (y2Axis_) {
        y2Axis_->setSize(QSizeF{});
    }
    if (xAxis_ && xAxis_->isVisible()) {
        const auto ov = xAxis_->inwardTickOverlap();
        const auto lOv = xAxis_->labelOverflow();
        xAxis_->setPosition(QPointF(plotX - lOv, h - padding_ - botH - ov));
        xAxis_->setSize(QSizeF(plotW + 2.0 * lOv, axisSize + ov));
    } else if (xAxis_) {
        xAxis_->setSize(QSizeF{});
    }
    if (x2Axis_ && x2Axis_->isVisible()) {
        const auto ov = x2Axis_->inwardTickOverlap();
        const auto lOv = x2Axis_->labelOverflow();
        x2Axis_->setPosition(QPointF(plotX - lOv, padding_));
        x2Axis_->setSize(QSizeF(plotW + 2.0 * lOv, axisSize + ov));
    } else if (x2Axis_) {
        x2Axis_->setSize(QSizeF{});
    }

    // Stack extra axes
    // Extra horizontal axes are stacked below the primary x axis, all within
    // the space reserved beneath the plot rectangle.
    auto currentBot = h - padding_ - botH + visibleAxisSize(xAxis_);
    auto currentLeft = padding_ + leftW;
    for (const auto axis : extraAxes_) {
        if (!axis->isVisible()) {
            axis->setSize(QSizeF{});
            continue;
        }
        const auto ov = axis->inwardTickOverlap();
        const auto lOv = axis->labelOverflow();
        if (axis->orientation() == Axis::Horizontal) {
            axis->setSize(QSizeF(plotW + 2.0 * lOv, axisSize + ov));
            axis->setPosition(QPointF(plotX - lOv, currentBot));
            currentBot += axisSize + ov;
        } else {
            axis->setSize(QSizeF(axisSize, plotH + 2.0 * lOv));
            axis->setPosition(QPointF(currentLeft, plotY - lOv));
            currentLeft += axisSize;
        }
    }
}

} // namespace QAccelPlot
