//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/inspection/SelectionTool.hpp"

#include "QAccelPlot/inspection/internal/SelectionRectangle.hpp"

#include <algorithm>
#include <cmath>

namespace QAccelPlot {
namespace {

constexpr auto kInfinity = std::numeric_limits<qreal>::infinity();

bool unbounded(const qreal min, const qreal max)
{
    return std::isinf(min) && std::isinf(max);
}

// Maps one region limit to a plot-local pixel; an infinite limit maps to the given plot-area edge.
qreal limitToPixel(const Axis* axis, const qreal value, const qreal origin, const qreal length, const qreal edge)
{
    return std::isinf(value) || !axis ? edge : origin + axis->coordToPixel(value, length);
}

} // namespace

SelectionTool::SelectionTool(QObject* parent)
    : QObject(parent)
    , model_(new InspectionRowModel(this))
    , rectangle_(new SelectionRectangle(*this))
{
}

::QAccelPlot::QAccelPlot* SelectionTool::plot() const
{
    return plot_;
}

void SelectionTool::setPlot(::QAccelPlot::QAccelPlot* plot)
{
    if (plot_ == plot) {
        return;
    }
    clear();
    plot_ = plot;
    reconnect();
    emit plotChanged();
    emit xAxisChanged();
    emit yAxisChanged();
}

Axis* SelectionTool::xAxis() const
{
    return xAxis_ ? xAxis_.data() : (plot_ ? plot_->xAxis() : nullptr);
}

void SelectionTool::setXAxis(Axis* axis)
{
    if (xAxis_ == axis) {
        return;
    }
    clear();
    xAxis_ = axis;
    reconnect();
    emit xAxisChanged();
}

Axis* SelectionTool::yAxis() const
{
    return yAxis_ ? yAxis_.data() : (plot_ ? plot_->yAxis() : nullptr);
}

void SelectionTool::setYAxis(Axis* axis)
{
    if (yAxis_ == axis) {
        return;
    }
    clear();
    yAxis_ = axis;
    reconnect();
    emit yAxisChanged();
}

bool SelectionTool::enabled() const
{
    return enabled_;
}

void SelectionTool::setEnabled(const bool enabled)
{
    if (enabled_ == enabled) {
        return;
    }
    enabled_ = enabled;
    if (!enabled) {
        cancelGesture();
    }
    emit enabledChanged();
}

SelectionTool::Mode SelectionTool::mode() const
{
    return mode_;
}

void SelectionTool::setMode(const Mode mode)
{
    if (mode_ == mode) {
        return;
    }
    cancelGesture();
    mode_ = mode;
    emit modeChanged();
}

int SelectionTool::button() const
{
    return button_;
}

void SelectionTool::setButton(const int button)
{
    if (button_ == button) {
        return;
    }
    cancelGesture();
    button_ = button;
    emit buttonChanged();
}

int SelectionTool::modifiers() const
{
    return modifiers_;
}

void SelectionTool::setModifiers(const int modifiers)
{
    if (modifiers_ == modifiers) {
        return;
    }
    modifiers_ = modifiers;
    emit modifiersChanged();
}

qreal SelectionTool::minimumSize() const
{
    return minimumSize_;
}

void SelectionTool::setMinimumSize(qreal size)
{
    if (!std::isfinite(size)) {
        return;
    }
    size = std::max(qreal{0}, size);
    if (minimumSize_ == size) {
        return;
    }
    minimumSize_ = size;
    emit minimumSizeChanged();
}

QColor SelectionTool::fillColor() const
{
    return fillColor_;
}

void SelectionTool::setFillColor(const QColor& color)
{
    if (fillColor_ == color) {
        return;
    }
    fillColor_ = color;
    emit fillColorChanged();
}

QColor SelectionTool::borderColor() const
{
    return borderColor_;
}

void SelectionTool::setBorderColor(const QColor& color)
{
    if (borderColor_ == color) {
        return;
    }
    borderColor_ = color;
    emit borderColorChanged();
}

bool SelectionTool::rectangleVisible() const
{
    return rectangleVisible_;
}

void SelectionTool::setRectangleVisible(const bool visible)
{
    if (rectangleVisible_ == visible) {
        return;
    }
    rectangleVisible_ = visible;
    emit rectangleVisibleChanged();
}

bool SelectionTool::selecting() const
{
    return drag_.active();
}

bool SelectionTool::hasSelection() const
{
    return hasSelection_;
}

QRectF SelectionTool::pixelRect() const
{
    if (!plot_) {
        return {};
    }
    if (drag_.active()) {
        return gestureRect();
    }
    return hasSelection_ ? regionRect() : QRectF{};
}

qreal SelectionTool::xMin() const
{
    return region_.xMin;
}

qreal SelectionTool::xMax() const
{
    return region_.xMax;
}

qreal SelectionTool::yMin() const
{
    return region_.yMin;
}

qreal SelectionTool::yMax() const
{
    return region_.yMax;
}

InspectionRowModel* SelectionTool::model() const
{
    return model_;
}

bool SelectionTool::select(const qreal xMin, const qreal xMax, const qreal yMin, const qreal yMax)
{
    if (std::isnan(xMin) || std::isnan(xMax) || std::isnan(yMin) || std::isnan(yMax)) {
        return false;
    }
    cancelGesture();
    setRegion({std::min(xMin, xMax), std::max(xMin, xMax), std::min(yMin, yMax), std::max(yMin, yMax)});
    return true;
}

InspectionPage SelectionTool::indices(PlotSeries* series, const int offset, const int limit)
{
    if (!hasSelection_ || !series || !selects(series)) {
        auto result = InspectionPage{};
        result.status = hasSelection_ ? InspectionStatus::InvalidArgument : InspectionStatus::Unavailable;
        return result;
    }
    return series->inspection()->indices(region_.xMin, region_.xMax, region_.yMin, region_.yMax, offset, limit);
}

void SelectionTool::clear()
{
    cancelGesture();
    if (!hasSelection_) {
        return;
    }
    hasSelection_ = false;
    region_ = {};
    model_->setRows({});
    emit selectionChanged();
    emit pixelRectChanged();
}

void SelectionTool::reconnect()
{
    for (const auto& connection : std::as_const(connections_)) {
        disconnect(connection);
    }
    connections_.clear();
    if (!plot_) {
        clear();
        return;
    }
    using Plot = ::QAccelPlot::QAccelPlot;
    connections_.push_back(connect(plot_, &Plot::mousePressed, this, &SelectionTool::press));
    connections_.push_back(connect(plot_, &Plot::mouseMoved, this, &SelectionTool::move));
    connections_.push_back(connect(plot_, &Plot::mouseReleased, this, &SelectionTool::release));
    connections_.push_back(connect(plot_, &Plot::escapePressed, this, &SelectionTool::clear));
    connections_.push_back(connect(plot_, &Plot::pointerGrabLost, this, &SelectionTool::cancelGesture));
    connections_.push_back(connect(plot_, &Plot::xAxisChanged, this, [this] {
        if (!xAxis_) {
            resetAxes();
        }
    }));
    connections_.push_back(connect(plot_, &Plot::yAxisChanged, this, [this] {
        if (!yAxis_) {
            resetAxes();
        }
    }));
    connections_.push_back(connect(plot_, &QObject::destroyed, this, &SelectionTool::reconnect));
    connections_.push_back(connect(plot_, &Plot::plotRectChanged, this, [this] {
        cancelGesture();
        emit pixelRectChanged();
    }));
    connections_.push_back(connect(plot_, &Plot::seriesChanged, this, [this] {
        model_->retainSeries(plot_ ? plot_->series() : QList<PlotSeries*>{});
        reconnect();
        refreshRows();
    }));
    for (auto* axis : {xAxis(), yAxis()}) {
        if (axis) {
            connections_.push_back(connect(axis, &QObject::destroyed, this, &SelectionTool::resetAxes));
            connections_.push_back(connect(axis, &Axis::rangeChanged, this, &SelectionTool::pixelRectChanged));
            connections_.push_back(connect(axis, &Axis::logScaleChanged, this, &SelectionTool::pixelRectChanged));
        }
    }
    const auto series = plot_->series();
    for (auto* item : series) {
        connectSeries(item);
    }
}

void SelectionTool::connectSeries(PlotSeries* series)
{
    connections_.push_back(connect(series, &PlotSeries::dataRevisionChanged, this, &SelectionTool::refreshRows));
    connections_.push_back(connect(series->inspection(), &SeriesInspection::statusChanged, this, &SelectionTool::refreshRows));
    connections_.push_back(connect(series, &PlotSeries::xAxisChanged, this, &SelectionTool::refreshRows));
    connections_.push_back(connect(series, &PlotSeries::yAxisChanged, this, &SelectionTool::refreshRows));
    connections_.push_back(connect(series, &QQuickItem::visibleChanged, this, &SelectionTool::refreshRows));
}

void SelectionTool::resetAxes()
{
    clear();
    reconnect();
    emit xAxisChanged();
    emit yAxisChanged();
}

void SelectionTool::press(PlotMouseEvent* event)
{
    if (event->isAccepted() || !enabled_ || !plot_ || !xAxis() || !yAxis() || event->button() != button_
        || !PlotDragRect::modifiersMatch(event->modifiers(), modifiers_) || !plot_->isInsidePlotArea(event->x(), event->y())) {
        return;
    }
    clear();
    drag_.begin({event->x(), event->y()});
    // Focus delivers Escape to the plot, which cancels the gesture.
    plot_->setFocus(true);
    event->accept();
    emit selectingChanged();
    emit pixelRectChanged();
}

void SelectionTool::move(PlotMouseEvent* event)
{
    if (!drag_.active()) {
        return;
    }
    drag_.moveTo({event->x(), event->y()}, plot_->plotRect());
    event->accept();
    emit pixelRectChanged();
}

void SelectionTool::release(PlotMouseEvent* event)
{
    if (!drag_.active() || event->button() != button_) {
        return;
    }
    drag_.moveTo({event->x(), event->y()}, plot_->plotRect());
    const auto selects = PlotDragRect::meetsMinimum(drag_.rect(), minimumSize_, mode_ != YRange, mode_ != XRange);
    const auto region = regionFromGesture(drag_.rect());
    drag_.end();
    event->accept();
    emit selectingChanged();
    if (!selects || std::isnan(region.xMin) || std::isnan(region.xMax) || std::isnan(region.yMin) || std::isnan(region.yMax)) {
        emit pixelRectChanged();
        return;
    }
    setRegion(region);
    emit completed();
}

void SelectionTool::cancelGesture()
{
    if (!drag_.active()) {
        return;
    }
    drag_.end();
    emit selectingChanged();
    emit pixelRectChanged();
}

void SelectionTool::setRegion(const Region& region)
{
    region_ = region;
    hasSelection_ = true;
    refreshRows();
    emit selectionChanged();
    emit pixelRectChanged();
}

void SelectionTool::refreshRows()
{
    if (!hasSelection_ || !plot_) {
        return;
    }

    struct Pending {
        InspectionRow row;
        QPointer<Axis> yAxis;
        qreal height;
    };

    const auto generation = ++rowsGeneration_;
    auto pending = QList<Pending>{};
    const auto series = plot_->series();
    for (auto* item : series) {
        if (selects(item)) {
            auto row = InspectionRow{};
            row.series = item;
            row.hasSummary = true;
            row.summary = item->inspection()->summarize(region_.xMin, region_.xMax, region_.yMin, region_.yMax);
            pending.push_back({row, item->yAxis(), item->height()});
        }
    }
    // Formatting runs user code, so every series is queried before the first label is produced.
    auto rows = QList<InspectionRow>{};
    for (auto& entry : pending) {
        if (entry.row.summary.valid() && entry.yAxis) {
            entry.row.minimumText = entry.yAxis->formatValue(entry.row.summary.minimum, entry.height);
            entry.row.maximumText = entry.yAxis ? entry.yAxis->formatValue(entry.row.summary.maximum, entry.height) : QString{};
            entry.row.meanText = entry.yAxis ? entry.yAxis->formatValue(entry.row.summary.mean, entry.height) : QString{};
        }
        rows.push_back(entry.row);
    }
    // A formatter that changed the data has already triggered a newer refresh.
    if (generation == rowsGeneration_) {
        model_->setRows(rows);
    }
}

bool SelectionTool::selects(const PlotSeries* series) const
{
    return series->isVisible() && series->inspection()->supported() && (unbounded(region_.xMin, region_.xMax) || series->xAxis() == xAxis())
        && (unbounded(region_.yMin, region_.yMax) || series->yAxis() == yAxis());
}

QRectF SelectionTool::gestureRect() const
{
    auto rect = drag_.rect();
    const auto area = plot_->plotRect();
    if (mode_ == XRange) {
        rect.setTop(area.top());
        rect.setBottom(area.bottom());
    }
    if (mode_ == YRange) {
        rect.setLeft(area.left());
        rect.setRight(area.right());
    }
    return rect;
}

QRectF SelectionTool::regionRect() const
{
    const auto area = plot_->plotRect();
    const auto first = QPointF{
        limitToPixel(xAxis(), region_.xMin, area.x(), area.width(), area.left()), limitToPixel(yAxis(), region_.yMin, area.y(), area.height(), area.bottom())};
    const auto second = QPointF{
        limitToPixel(xAxis(), region_.xMax, area.x(), area.width(), area.right()), limitToPixel(yAxis(), region_.yMax, area.y(), area.height(), area.top())};
    return QRectF{first, second}.normalized();
}

SelectionTool::Region SelectionTool::regionFromGesture(const QRectF& pixels) const
{
    const auto area = plot_->plotRect();
    auto region = Region{-kInfinity, kInfinity, -kInfinity, kInfinity};
    if (mode_ != YRange) {
        const auto a = xAxis()->pixelToCoord(pixels.left() - area.x(), area.width());
        const auto b = xAxis()->pixelToCoord(pixels.right() - area.x(), area.width());
        region.xMin = std::min(a, b);
        region.xMax = std::max(a, b);
    }
    if (mode_ != XRange) {
        const auto a = yAxis()->pixelToCoord(pixels.top() - area.y(), area.height());
        const auto b = yAxis()->pixelToCoord(pixels.bottom() - area.y(), area.height());
        region.yMin = std::min(a, b);
        region.yMax = std::max(a, b);
    }
    return region;
}

} // namespace QAccelPlot
