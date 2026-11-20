//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/series/ErrorBarSeries.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/Glow.hpp"
#include "QAccelPlot/materials/ErrorBarMaterial.hpp"
#include "QAccelPlot/series/internal/InterleavedData.hpp"
#include "QAccelPlot/series/internal/RecordHitIndex.hpp"
#include "QAccelPlot/series/internal/RecordNode.hpp"
#include "QAccelPlot/series/internal/RectGeometry.hpp"
#include "QAccelPlot/series/internal/SeriesEffectList.hpp"
#include "QAccelPlot/series/internal/SeriesSupport.hpp"
#include "QAccelPlot/series/internal/StoreTransition.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"
#include "QAccelPlot/transitions/TransitionRunner.hpp"

#include <QHoverEvent>
#include <QQuickWindow>

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <utility>

namespace QAccelPlot {

namespace {

// Values per data point: x, y, low, high.
constexpr auto kStride = 4;
constexpr auto kLowColumn = 2;
constexpr auto kHighColumn = 3;
constexpr auto kVertexBytesPerErrorBar = ErrorBarMaterial::kVerticesPerErrorBar * 4;

// The bounds lie on the Y axis of vertical error bars and on the X axis of horizontal ones.
std::vector<DataPointStore::Dimension> errorBarColumns(const Qt::Orientation orientation)
{
    using Dimension = DataPointStore::Dimension;
    const auto bounds = orientation == Qt::Horizontal ? Dimension::X : Dimension::Y;
    return {Dimension::X, Dimension::Y, bounds, bounds};
}

// Returns the bounds of \a value with \a minus below and \a plus above it, ignoring their signs.
std::array<double, 2> boundsAround(const double value, const double minus, const double plus)
{
    return {value - std::abs(minus), value + std::abs(plus)};
}

// Interleaves the first min(sizes) data points into (x, y, low, high) quadruples, with bounds around
// y for vertical and around x for horizontal error bars.
template <typename List> std::vector<double> interleaveErrors(const List& xs, const List& ys, const List& minuses, const List& pluses, const bool horizontal)
{
    const auto count = Internal::shortestLength(xs, ys, minuses, pluses);
    auto data = std::vector<double>(count * kStride);
    for (auto i = std::size_t{0}; i < count; ++i) {
        const auto [low, high] = boundsAround(horizontal ? xs[i] : ys[i], minuses[i], pluses[i]);
        const auto dataPoint = std::array<double, kStride>{xs[i], ys[i], low, high};
        std::copy(dataPoint.cbegin(), dataPoint.cend(), data.begin() + static_cast<std::ptrdiff_t>(i * kStride));
    }
    return data;
}

} // namespace

ErrorBarSeries::ErrorBarSeries(QQuickItem* parent)
    : PlotSeries(DataPointStore::Layout{errorBarColumns(Qt::Vertical), DataPointStore::RangeRule::EachCoordinate, 0},
          DataPointWording{"data point", "data points", "values", "values"}, parent)
    , color_(ColorPalette::dark().seriesPrimary)
    , hoverColor_(ColorPalette::dark().hover)
    , transition_(std::make_unique<TransitionRunner>(this))
    , storeTransition_(std::make_unique<Internal::StoreTransition>(*transition_, dataPoints()))
    , hitIndex_(std::make_unique<Internal::RecordHitIndex>())
{
    setFlag(ItemHasContents, true);
    setAntialiasing(true);
    supportHover();
    setAcceptedMouseButtons(Qt::NoButton);
    connect(transition_.get(), &TransitionRunner::frameDue, this, &ErrorBarSeries::advanceTransition);
    connect(transition_.get(), &TransitionRunner::interrupted, this, &ErrorBarSeries::endTransition);
    transition_->notifyTransitionChanges(this);
    connect(transition_.get(), &TransitionRunner::activeChanged, this, [this] { setAnimating(transition_->active()); });
}

ErrorBarSeries::~ErrorBarSeries()
{
    // runningChanged handlers can still read the series until QObject destruction.
    transition_->cancel();
}

Qt::Orientation ErrorBarSeries::orientation() const
{
    return orientation_;
}

void ErrorBarSeries::setOrientation(const Qt::Orientation orientation)
{
    if (orientation_ == orientation) {
        return;
    }
    orientation_ = orientation;
    dataPoints().remapColumns(errorBarColumns(orientation_));
    onGeometryChanged();
    emit orientationChanged();
}

QColor ErrorBarSeries::color() const
{
    return color_;
}

void ErrorBarSeries::setColor(const QColor& color)
{
    if (color_ == color) {
        return;
    }
    color_ = color;
    emit colorChanged();
    update();
}

qreal ErrorBarSeries::lineWidth() const
{
    return lineWidth_;
}

void ErrorBarSeries::setLineWidth(const qreal width)
{
    const auto clamped = std::max(width, qreal{0.0});
    if (nearlyEqual(lineWidth_, clamped)) {
        return;
    }
    lineWidth_ = clamped;
    emit lineWidthChanged();
    update();
}

qreal ErrorBarSeries::capLength() const
{
    return capLength_;
}

void ErrorBarSeries::setCapLength(const qreal length)
{
    const auto clamped = std::max(length, qreal{0.0});
    if (nearlyEqual(capLength_, clamped)) {
        return;
    }
    capLength_ = clamped;
    emit capLengthChanged();
    update();
}

qreal ErrorBarSeries::offset() const
{
    return offset_;
}

void ErrorBarSeries::setOffset(const qreal offset)
{
    if (!std::isfinite(offset) || nearlyEqual(offset_, offset)) {
        return;
    }
    offset_ = offset;
    onGeometryChanged();
    emit offsetChanged();
}

QColor ErrorBarSeries::hoverColor() const
{
    return hoverColor_;
}

void ErrorBarSeries::setHoverColor(const QColor& color)
{
    if (hoverColor_ == color) {
        return;
    }
    hoverColor_ = color;
    emit hoverColorChanged();
    update();
}

qreal ErrorBarSeries::hoverRadius() const
{
    return hoverRadius_;
}

void ErrorBarSeries::setHoverRadius(const qreal radius)
{
    if (!Internal::assignNonNegative(hoverRadius_, radius)) {
        return;
    }
    emit hoverRadiusChanged();
    // Marks the item dirty so the window re-delivers hover at the cursor and updates hoveredIndex.
    update();
}

EnterExitTransition* ErrorBarSeries::enterTransition() const
{
    return qobject_cast<EnterExitTransition*>(transition_->transition(TransitionRunner::Slot::Enter));
}

void ErrorBarSeries::setEnterTransition(EnterExitTransition* transition)
{
    transition_->setTransition(TransitionRunner::Slot::Enter, transition);
}

DataTransition* ErrorBarSeries::updateTransition() const
{
    return transition_->transition(TransitionRunner::Slot::Update);
}

void ErrorBarSeries::setUpdateTransition(DataTransition* transition)
{
    transition_->setTransition(TransitionRunner::Slot::Update, transition);
}

EnterExitTransition* ErrorBarSeries::exitTransition() const
{
    return qobject_cast<EnterExitTransition*>(transition_->transition(TransitionRunner::Slot::Exit));
}

void ErrorBarSeries::setExitTransition(EnterExitTransition* transition)
{
    transition_->setTransition(TransitionRunner::Slot::Exit, transition);
}

int ErrorBarSeries::hoveredIndex() const
{
    return hoveredIndex_;
}

void ErrorBarSeries::appendData(const qreal x, const qreal y, const qreal low, const qreal high)
{
    endTransition();
    const auto dataPoint = std::array<double, kStride>{x, y, low, high};
    appendDataPoint(dataPoint.data());
}

void ErrorBarSeries::replaceData(const int index, const qreal x, const qreal y, const qreal low, const qreal high)
{
    if (!validateDataPointIndex(index)) {
        return;
    }
    endTransition();
    const auto dataPoint = std::array<double, kStride>{x, y, low, high};
    replaceDataPoint(index, dataPoint.data());
}

void ErrorBarSeries::setData(const QList<qreal>& xs, const QList<qreal>& ys, const QList<qreal>& lows, const QList<qreal>& highs)
{
    applyInterleavedData(Internal::interleave({xs, ys, lows, highs}));
}

void ErrorBarSeries::setData(const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& lows, const std::vector<double>& highs)
{
    applyInterleavedData(Internal::interleave({xs, ys, lows, highs}));
}

void ErrorBarSeries::appendErrorData(const qreal x, const qreal y, const qreal error)
{
    appendErrorData(x, y, error, error);
}

void ErrorBarSeries::appendErrorData(const qreal x, const qreal y, const qreal minus, const qreal plus)
{
    const auto [low, high] = boundsAround(isHorizontal() ? x : y, minus, plus);
    appendData(x, y, low, high);
}

void ErrorBarSeries::setErrorData(const QList<qreal>& xs, const QList<qreal>& ys, const QList<qreal>& errors)
{
    applyInterleavedData(interleaveErrors(xs, ys, errors, errors, isHorizontal()));
}

void ErrorBarSeries::setErrorData(const QList<qreal>& xs, const QList<qreal>& ys, const QList<qreal>& minuses, const QList<qreal>& pluses)
{
    applyInterleavedData(interleaveErrors(xs, ys, minuses, pluses, isHorizontal()));
}

void ErrorBarSeries::setErrorData(const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& errors)
{
    applyInterleavedData(interleaveErrors(xs, ys, errors, errors, isHorizontal()));
}

void ErrorBarSeries::setErrorData(
    const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& minuses, const std::vector<double>& pluses)
{
    applyInterleavedData(interleaveErrors(xs, ys, minuses, pluses, isHorizontal()));
}

void ErrorBarSeries::clearData()
{
    if (transition_->animates(count(), 0)) {
        // Every error bar is removed, so the exit transition animates them.
        applyDoubleData({}, 0);
        return;
    }
    discardTransition();
    PlotSeries::clearData();
}

QVariantMap ErrorBarSeries::errorBarAt(const int index) const
{
    if (index < 0 || index >= count()) {
        return {};
    }
    return {{QStringLiteral("x"), component(index, 0)}, {QStringLiteral("y"), component(index, 1)}, {QStringLiteral("low"), component(index, kLowColumn)},
        {QStringLiteral("high"), component(index, kHighColumn)}};
}

int ErrorBarSeries::errorBarIndexAt(const QPointF& position) const
{
    if (count() <= 0 || !xAxis() || !yAxis() || width() <= 0.0 || height() <= 0.0) {
        return -1;
    }
    ensureSpatialGrid();
    // An error bar's line and caps reach past its position by half their width.
    const auto overhang = 0.5 * std::max(lineWidth_, capLength_);
    const auto drawnCount = std::min(dataPoints().drawnView().count, renderCapacity_);
    return hitIndex_->nearest({position, hoverRadius_, overhang, drawnCount}, *xAxis(), *yAxis(), size(),
        [this, &position](const int index) { return pixelDistance(index, position); });
}

bool ErrorBarSeries::contains(const QPointF& point) const
{
    return boundingRect().contains(point) && errorBarIndexAt(point) >= 0;
}

bool ErrorBarSeries::recordInspectionSupported() const
{
    return true;
}

InspectionRecord ErrorBarSeries::inspectionRecord(const int index) const
{
    auto result = InspectionRecord{};
    if (index < 0 || index >= count()) {
        result.status = InspectionStatus::InvalidArgument;
        return result;
    }
    result.index = index;
    result.fields = errorBarAt(index);
    result.status = dataBar(dataPoints().view(), index) ? InspectionStatus::Ready : InspectionStatus::NoMatch;
    return result;
}

InspectionRecord ErrorBarSeries::inspectionRecordAt(const QPointF& position) const
{
    const auto index = errorBarIndexAt(position);
    return index < 0 ? InspectionRecord{} : inspectionRecord(index);
}

QSGNode* ErrorBarSeries::updatePaintNode(QSGNode* oldNode, [[maybe_unused]] UpdatePaintNodeData* updatePaintNodeData)
{
    auto* window = this->window();
    updateRenderCapacity(Internal::recordCapacity(window, kStride, kVertexBytesPerErrorBar), count());
    const auto drawnCount = std::min(dataPoints().renderCount(), renderCapacity_);
    const auto drawable = drawnCount > 0 && xAxis() && yAxis() && !resolvePlotRect().isEmpty() && window;
    const auto frame = Internal::RecordFrame{window, drawnCount, drawable, kStride, ErrorBarMaterial::kVerticesPerErrorBar, "ErrorBarSeries"};
    return Internal::updateRecordNode<ErrorBarMaterial>(
        oldNode, frame, dataPoints(), uploadedRevision_, [this, drawnCount](ErrorBarMaterial& material) { updateMaterial(material, drawnCount); });
}

void ErrorBarSeries::hoverEnterEvent(QHoverEvent* event)
{
    setHoveredIndex(errorBarIndexAt(event->position()));
    QQuickItem::hoverEnterEvent(event);
}

void ErrorBarSeries::hoverMoveEvent(QHoverEvent* event)
{
    setHoveredIndex(errorBarIndexAt(event->position()));
    QQuickItem::hoverMoveEvent(event);
}

void ErrorBarSeries::hoverLeaveEvent(QHoverEvent* event)
{
    setHoveredIndex(-1);
    QQuickItem::hoverLeaveEvent(event);
}

void ErrorBarSeries::applyDataPoints(DataPointInput&& input)
{
    if (transition_->animates(count(), input.count)) {
        // Transitions animate doubles.
        const auto dataPointCount = input.count;
        applyDoubleData(std::move(input).toDoubles(), dataPointCount);
        return;
    }
    discardTransition();
    storeDataPoints(std::move(input));
}

bool ErrorBarSeries::supportsEffect(const SeriesEffect& effect) const
{
    return qobject_cast<const Glow*>(&effect) != nullptr;
}

void ErrorBarSeries::onAxisScaleChanged()
{
    // Log scale changes which data points are drawn.
    hitIndex_->invalidate();
    update();
}

void ErrorBarSeries::onDataPointsChanged([[maybe_unused]] const DataUpdate& dataUpdate)
{
    hitIndex_->invalidate();
    if (hoveredIndex_ >= count()) {
        setHoveredIndex(-1);
    }
}

bool ErrorBarSeries::isHorizontal() const
{
    return orientation_ == Qt::Horizontal;
}

double ErrorBarSeries::component(const int index, const int column) const
{
    return dataPoints().value(index, column);
}

std::optional<ErrorBarSeries::DataBar> ErrorBarSeries::dataBar(const DataPointStore::View& dataPoints, const int index) const
{
    const auto horizontal = isHorizontal();
    const auto position = dataPoints.value(index, horizontal ? 1 : 0) + offset_;
    const auto value = dataPoints.value(index, horizontal ? 0 : 1);
    if (!isValidCoordinate(position, horizontal ? logScaleY() : logScaleX()) || std::isnan(value)) {
        return std::nullopt;
    }
    const auto valueLogScale = horizontal ? logScaleX() : logScaleY();
    const auto low = dataPoints.value(index, kLowColumn);
    const auto high = dataPoints.value(index, kHighColumn);
    return DataBar{position, std::isnan(low) ? value : low, std::isnan(high) ? value : high, isValidCoordinate(low, valueLogScale),
        isValidCoordinate(high, valueLogScale)};
}

std::optional<qreal> ErrorBarSeries::pixelDistance(const int index, const QPointF& position) const
{
    const auto bar = dataBar(dataPoints().drawnView(), index);
    if (!bar || lineWidth_ <= 0.0) {
        return std::nullopt;
    }
    // u runs across the bounds and v along them.
    const auto horizontal = isHorizontal();
    const auto& positionAxis = horizontal ? *yAxis() : *xAxis();
    const auto& valueAxis = horizontal ? *xAxis() : *yAxis();
    const auto alongLength = horizontal ? width() : height();
    const auto across = positionAxis.coordToPixel(bar->position, horizontal ? height() : width());
    const auto start = Internal::edgePixel(bar->start, valueAxis, alongLength);
    const auto end = Internal::edgePixel(bar->end, valueAxis, alongLength);
    const auto u = horizontal ? position.y() : position.x();
    const auto v = horizontal ? position.x() : position.y();

    const auto halfWidth = 0.5 * lineWidth_;
    auto distance = Internal::distanceToBox(u, v, {across - halfWidth, across + halfWidth}, {std::min(start, end), std::max(start, end)});
    if (capLength_ > 0.0) {
        const auto halfLength = 0.5 * std::max(capLength_, lineWidth_);
        for (const auto& [bound, capped] : {std::pair{start, bar->startCapped}, std::pair{end, bar->endCapped}}) {
            if (capped) {
                distance
                    = std::min(distance, Internal::distanceToBox(u, v, {across - halfLength, across + halfLength}, {bound - halfWidth, bound + halfWidth}));
            }
        }
    }
    return distance;
}

void ErrorBarSeries::applyInterleavedData(std::vector<double>&& data)
{
    const auto dataPointCount = static_cast<int>(data.size() / kStride);
    setData(std::move(data), dataPointCount);
}

void ErrorBarSeries::applyDoubleData(std::vector<double>&& data, const int dataPointCount)
{
    const auto previousCount = count();
    if (storeTransition_->start(std::move(data), dataPointCount, transitionLayout())) {
        advanceTransition();
    } else {
        discardTransition();
    }
    notifyDataPointsReplaced(previousCount);
}

TransitionRunner::Layout ErrorBarSeries::transitionLayout() const
{
    // The value and both bounds grow; the position stays.
    const auto vertical = orientation_ == Qt::Vertical;
    const auto valueColumns = vertical ? std::vector<int>{1, 2, 3} : std::vector<int>{0, 2, 3};
    const auto baseline = Internal::growBaseline(0.0, vertical ? yAxis() : xAxis());
    return TransitionRunner::Layout{kStride, valueColumns, baseline, false};
}

void ErrorBarSeries::advanceTransition()
{
    if (storeTransition_->advance()) {
        hitIndex_->invalidate();
        update();
    }
}

void ErrorBarSeries::endTransition()
{
    if (storeTransition_->end()) {
        hitIndex_->invalidate();
        update();
    }
}

void ErrorBarSeries::discardTransition()
{
    storeTransition_->discard();
    hitIndex_->invalidate();
}

void ErrorBarSeries::setHoveredIndex(const int index)
{
    if (hoveredIndex_ == index) {
        return;
    }
    hoveredIndex_ = index;
    setHovered(index >= 0);
    emit hoveredIndexChanged();
    update();
}

void ErrorBarSeries::onGeometryChanged()
{
    hitIndex_->invalidate();
    invalidateDataRanges();
    update();
}

void ErrorBarSeries::ensureSpatialGrid() const
{
    if (hitIndex_->valid()) {
        return;
    }
    const auto horizontal = isHorizontal();
    const auto drawn = dataPoints().drawnView();
    hitIndex_->build(drawn.count, horizontal ? Qt::Vertical : Qt::Horizontal, [&](const int index) -> std::optional<std::pair<double, double>> {
        const auto bar = dataBar(drawn, index);
        if (!bar) {
            return std::nullopt;
        }
        return std::pair{bar->position, bar->position};
    });
}

void ErrorBarSeries::updateMaterial(ErrorBarMaterial& material, const int drawnCount) const
{
    const auto originX = dataPoints().renderOrigin(DataPointStore::Dimension::X);
    const auto originY = dataPoints().renderOrigin(DataPointStore::Dimension::Y);
    const auto rect = resolvePlotRect();
    material.color = color_;
    material.domainMin = QVector2D(static_cast<float>(xAxis()->viewportMin() - originX), static_cast<float>(yAxis()->viewportMin() - originY));
    material.domainMax = QVector2D(static_cast<float>(xAxis()->viewportMax() - originX), static_cast<float>(yAxis()->viewportMax() - originY));
    material.viewportSize = QVector2D(static_cast<float>(rect.width()), static_cast<float>(rect.height()));
    material.logScaleX = logScaleX() ? 1.0f : 0.0f;
    material.logScaleY = logScaleY() ? 1.0f : 0.0f;
    material.hoverColor = hoverColor_.isValid() ? hoverColor_ : color_;
    material.hoveredIndex = static_cast<float>(hoveredIndex_);
    material.lineWidth = static_cast<float>(lineWidth_);
    material.capLength = static_cast<float>(capLength_);
    material.offset = static_cast<float>(offset_);
    material.horizontal = isHorizontal() ? 1.0f : 0.0f;
    material.errorBarCount = static_cast<float>(drawnCount);
    material.itemFade = transition_->fade();
    material.glow = effectList().glowStyle(color_);
    material.antialiasing = antialiasing() ? 1.0f : 0.0f;
}

} // namespace QAccelPlot
