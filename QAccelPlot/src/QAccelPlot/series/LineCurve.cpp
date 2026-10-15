//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/GradientCoordinateUtils.hpp"
#include "QAccelPlot/effects/GradientFill.hpp"
#include "QAccelPlot/effects/GradientStroke.hpp"
#include "QAccelPlot/inspection/internal/InspectionTypes.hpp"
#include "QAccelPlot/series/LineCurveGapFilter.hpp"
#include "QAccelPlot/series/internal/SeriesSupport.hpp"
#include "QAccelPlot/transitions/DataTransition.hpp"
#include "QAccelPlot/transitions/TransitionRunner.hpp"

#include <QHoverEvent>
#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QtMath>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <type_traits>

namespace QAccelPlot {

namespace {

// Fallback data-range bounds used when no axis is attached to resolve gradient normalization.
constexpr auto kFallbackDataMin = qreal{0.0};
constexpr auto kFallbackDataMax = qreal{1.0};

// Per-dimension extents of the valid coordinates in a curve buffer. A dimension
// without any valid coordinate keeps min > max.
struct DataExtents {
    qreal xMin{std::numeric_limits<qreal>::max()};
    qreal xMax{std::numeric_limits<qreal>::lowest()};
    qreal yMin{std::numeric_limits<qreal>::max()};
    qreal yMax{std::numeric_limits<qreal>::lowest()};
};

// Returns true when a dimension is empty (min > max) or both its bounds are valid.
bool isValidExtent(const qreal min, const qreal max, const bool logScale)
{
    return min > max || (isValidSample(min, logScale) && isValidSample(max, logScale));
}

// Extents of the valid coordinates in samples [begin, end) of interleaved data.
// Each coordinate is judged on its own: an invalid Y does not remove its X from
// the X extent, and vice versa.
//
// The first pass is a branch-free min/max scan in the buffer's own type, using
// local accumulators and pointer iteration so the compiler keeps it as cheap as
// the pre-gap implementation. It is exact whenever the data is valid, and NaN
// never wins a `value < min ? value : min` comparison, so NaN is ignored as
// required. +/-Inf and non-positive log-scale values show up in the resulting
// bounds, which triggers the validity-checked second pass.
template <typename T> DataExtents computeValidExtents(const T* data, const std::size_t begin, const std::size_t end, const bool logScaleX, const bool logScaleY)
{
    auto xMin = std::numeric_limits<T>::max();
    auto xMax = std::numeric_limits<T>::lowest();
    auto yMin = std::numeric_limits<T>::max();
    auto yMax = std::numeric_limits<T>::lowest();
    const auto* last = data + end * 2;
    for (auto* sample = data + begin * 2; sample != last; sample += 2) {
        const auto x = sample[0];
        const auto y = sample[1];
        xMin = x < xMin ? x : xMin;
        xMax = x > xMax ? x : xMax;
        yMin = y < yMin ? y : yMin;
        yMax = y > yMax ? y : yMax;
    }
    auto extents = DataExtents{static_cast<qreal>(xMin), static_cast<qreal>(xMax), static_cast<qreal>(yMin), static_cast<qreal>(yMax)};
    if (begin == end) {
        extents = DataExtents{};
    }
    if (isValidExtent(extents.xMin, extents.xMax, logScaleX) && isValidExtent(extents.yMin, extents.yMax, logScaleY)) {
        return extents;
    }

    extents = DataExtents{};
    for (auto i = begin; i < end; ++i) {
        const auto x = static_cast<qreal>(data[i * 2]);
        const auto y = static_cast<qreal>(data[i * 2 + 1]);
        if (isValidSample(x, logScaleX)) {
            extents.xMin = std::min(extents.xMin, x);
            extents.xMax = std::max(extents.xMax, x);
        }
        if (isValidSample(y, logScaleY)) {
            extents.yMin = std::min(extents.yMin, y);
            extents.yMax = std::max(extents.yMax, y);
        }
    }
    return extents;
}

// Data-space culling box for the inclusive index range [first, last]. It covers
// every valid coordinate, which keeps it conservative for hit tests; a range
// without valid coordinates yields min > max and is skipped.
template <typename T> DataExtents computeChunkExtents(const T* data, const int first, const int last, const bool logScaleX, const bool logScaleY)
{
    return computeValidExtents(data, static_cast<std::size_t>(first), static_cast<std::size_t>(last) + 1, logScaleX, logScaleY);
}

// Returns the first enabled Effect with a valid payload. Effects snapshot their payloads on the
// GUI thread, so this does not touch QML objects and is safe in updatePaintNode().
template <typename Effect> const Effect* firstActiveGradientEffect(const QList<LineCurveEffect*>& effects)
{
    for (const auto effect : effects) {
        const auto gradientEffect = qobject_cast<const Effect*>(effect);
        if (gradientEffect && gradientEffect->enabled() && gradientEffect->payload().isValid()) {
            return gradientEffect;
        }
    }

    return nullptr;
}

// Returns the payload of the first active Effect with unset value bounds resolved by
// rangeFn(direction). Using the data range keeps gradient colors invariant to pan/zoom.
template <typename Effect, typename Payload, typename RangeFn> Payload resolveGradientPayload(const QList<LineCurveEffect*>& effects, const RangeFn& rangeFn)
{
    const auto gradientEffect = firstActiveGradientEffect<Effect>(effects);
    if (!gradientEffect) {
        return {};
    }

    auto payload = gradientEffect->payload();
    // Reading the data range can scan the records, so it is skipped when both bounds are fixed.
    if (!payload.gradientValueMin.has_value() || !payload.gradientValueMax.has_value()) {
        const auto [min, max] = rangeFn(payload.direction);
        resolveGradientValueRange(payload, min, max);
    }
    return payload;
}

constexpr auto kLineVerticesPerPoint = 2;
constexpr auto kMarkerVerticesPerPoint = 6;
constexpr auto kLineVertexBytesPerPoint = kLineVerticesPerPoint * static_cast<int>(sizeof(LineVertex));
// Markers carry their positions in the vertex buffer, which QSGGeometry sizes in int bytes.
constexpr auto kMaxMarkerPoints = std::numeric_limits<int>::max() / (kMarkerVerticesPerPoint * static_cast<int>(sizeof(PointVertex)));
// Float vertex ids, which the line shader reads its samples by, are exact up to 2^24.
constexpr auto kMaxLinePoints = 1 << 24;

// Number of points a curve with these parts draws in \a window.
int pointCapacity(QQuickWindow* window, const bool hasLine, const bool hasPoints)
{
    auto capacity = hasPoints ? kMaxMarkerPoints : std::numeric_limits<int>::max();
    if (hasLine) {
        capacity = std::min(capacity, Internal::recordCapacity(window, 2, kLineVertexBytesPerPoint));
    }
    return capacity;
}

}

LineCurve::LineCurve(QQuickItem* parent)
    : PlotSeries(DataPointStore::Layout{{DataPointStore::Dimension::X, DataPointStore::Dimension::Y}, DataPointStore::RangeRule::EachCoordinate, 0},
          DataPointWording{"data point", "points", "doubles", "floats"}, parent)
    , transition_(std::make_unique<TransitionRunner>(this))
{
    setFlag(ItemHasContents, true);
    setAcceptHoverEvents(Internal::hoverEnabled());
    connect(gaps_, &LineCurveGaps::nanModeChanged, this, &LineCurve::onNanGapModeChanged);
    connect(marker_, &SeriesMarker::shapeChanged, this, &LineCurve::onMarkerShapeChanged);
    // Size, fill, and outline width are material uniforms, so they need no vertex rebuild.
    connect(marker_, &SeriesMarker::sizeChanged, this, &QQuickItem::update);
    connect(marker_, &SeriesMarker::filledChanged, this, &QQuickItem::update);
    connect(marker_, &SeriesMarker::strokeWidthChanged, this, &QQuickItem::update);
    // A parent already in a window adds the curve to it from the QQuickItem constructor, where the
    // ItemSceneChange does not reach this class's itemChange().
    connectAnimationTicks(window());
    connect(transition_.get(), &TransitionRunner::frameDue, this, &LineCurve::advanceTransition);
    connect(transition_.get(), &TransitionRunner::interrupted, this, &LineCurve::finishTransition);
    connect(transition_.get(), &TransitionRunner::transitionDestroyed, this, &LineCurve::onTransitionDestroyed);
}

LineCurve::~LineCurve()
{
    // runningChanged handlers can still read the series until QObject destruction.
    transition_->cancel();
}

QColor LineCurve::color() const
{
    return color_;
}

void LineCurve::setColor(const QColor& c)
{
    if (color_ == c) {
        return;
    }
    color_ = c;
    emit colorChanged();
    update();
}

qreal LineCurve::lineWidth() const
{
    return lineWidth_;
}

void LineCurve::setLineWidth(qreal w)
{
    if (nearlyEqual(lineWidth_, w)) {
        return;
    }
    lineWidth_ = w;
    emit lineWidthChanged();
    // lineWidth is a material uniform — no vertex rebuild needed.
    update();
}

bool LineCurve::hovered() const
{
    return hovered_;
}

qreal LineCurve::hoverRadius() const
{
    return hoverRadius_;
}

void LineCurve::setHoverRadius(const qreal radius)
{
    const auto clamped = std::max(radius, qreal{0.0});
    if (nearlyEqual(hoverRadius_, clamped)) {
        return;
    }
    hoverRadius_ = clamped;
    emit hoverRadiusChanged();
    // Marks the item dirty so the window re-delivers hover at the cursor and updates hovered.
    update();
}

DataTransition* LineCurve::transition() const
{
    return transition_->transition();
}

void LineCurve::setTransition(DataTransition* transition)
{
    if (transition_->setTransition(transition)) {
        emit transitionChanged();
    }
}

LineStyle* LineCurve::lineStyle() const
{
    return lineStyle_;
}

void LineCurve::setLineStyle(LineStyle* style)
{
    if (lineStyle_ == style) {
        return;
    }
    if (lineStyle_) {
        disconnect(lineStyle_, &LineStyle::styleChanged, this, &LineCurve::onLineStyleChanged);
        disconnect(lineStyle_, &QObject::destroyed, this, &LineCurve::onLineStyleDestroyed);
    }
    lineStyle_ = style;
    if (lineStyle_) {
        connect(lineStyle_, &LineStyle::styleChanged, this, &LineCurve::onLineStyleChanged);
        connect(lineStyle_, &QObject::destroyed, this, &LineCurve::onLineStyleDestroyed);
    }
    styleChanged_ = true;
    emit lineStyleChanged();
    invalidateVertices();
    update();
}

bool LineCurve::antialiasingEnabled() const
{
    return antialiasingEnabled_;
}

void LineCurve::setAntialiasingEnabled(const bool enabled)
{
    if (antialiasingEnabled_ == enabled) {
        return;
    }
    antialiasingEnabled_ = enabled;
    emit antialiasingEnabledChanged();
    update();
}

qreal LineCurve::antialiasingFeather() const
{
    return antialiasingFeather_;
}

void LineCurve::setAntialiasingFeather(qreal feather)
{
    constexpr static auto kMinFeather = qreal{0.0};
    constexpr static auto kMaxFeather = qreal{10.0};
    const auto clamped = std::clamp(feather, kMinFeather, kMaxFeather);
    if (nearlyEqual(antialiasingFeather_, clamped)) {
        return;
    }
    antialiasingFeather_ = clamped;
    emit antialiasingFeatherChanged();
    update();
}

QQmlListProperty<LineCurveEffect> LineCurve::effects()
{
    return QQmlListProperty<LineCurveEffect>(this, this, &LineCurve::appendEffect, &LineCurve::effectCount, &LineCurve::effectAt, &LineCurve::clearEffects);
}

LineCurveGaps* LineCurve::gaps() const
{
    return gaps_;
}

SeriesMarker* LineCurve::marker() const
{
    return marker_;
}

void LineCurve::appendData(const qreal x, const qreal y)
{
    finishTransition();
    const auto point = std::array<double, 2>{x, y};
    appendDataPoint(point.data());
}

void LineCurve::setData(const QList<QPointF>& data)
{
    auto newCount = static_cast<int>(data.size());
    auto newData = std::vector<double>(static_cast<std::size_t>(newCount) * 2);
    for (int i = 0; i < newCount; ++i) {
        newData[static_cast<std::size_t>(i) * 2] = static_cast<double>(data[i].x());
        newData[static_cast<std::size_t>(i) * 2 + 1] = static_cast<double>(data[i].y());
    }
    applyNewData(std::move(newData), newCount);
}

void LineCurve::setData(const std::vector<double>& xs, const std::vector<double>& ys)
{
    const auto newCount = static_cast<int>(std::min(xs.size(), ys.size()));
    auto newData = std::vector<double>(static_cast<std::size_t>(newCount) * 2);
    for (auto i = int{0}; i < newCount; ++i) {
        newData[static_cast<std::size_t>(i) * 2] = xs[static_cast<std::size_t>(i)];
        newData[static_cast<std::size_t>(i) * 2 + 1] = ys[static_cast<std::size_t>(i)];
    }
    applyNewData(std::move(newData), newCount);
}

void LineCurve::setDataFWithCache(std::vector<float>&& data, const int pointCount, std::vector<char>&& vertexCache)
{
    if (!validateDataSize(data.size(), pointCount, true)) {
        return;
    }
    if (!transition_->enabled()) {
        pendingVertexCache_ = std::move(vertexCache);
    }
    applyNewData(std::move(data), pointCount);
}

void LineCurve::setDataFWithCache(const float* xyInterleaved, const int pointCount, std::vector<char>&& vertexCache)
{
    if (!validateRawDataArguments(xyInterleaved, pointCount)) {
        return;
    }
    if (!transition_->enabled()) {
        pendingVertexCache_ = std::move(vertexCache);
    }
    applyFloatArray(xyInterleaved, pointCount);
}

void LineCurve::clearData()
{
    const auto previousCount = dataPoints().count();
    cancelRunningTransition();
    dataPoints().clear();
    notifyDataPointsChanged(DataChange::Replaced, previousCount);
}

InspectionSource LineCurve::inspectionSource() const
{
    const auto points = dataPoints().view();
    auto source = InspectionSource{};
    source.count = points.count;
    source.logX = logScaleX();
    source.logY = logScaleY();
    source.doubles = points.precise;
    source.floats = points.floats;
    return source;
}

bool LineCurve::inspectionAvailable() const
{
    return !transition_->pending();
}

QSGNode* LineCurve::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData)

    if (!Internal::supportsCustomShaderRendering(window())) {
        static bool warned = false;
        if (!warned) {
            qCWarning(lcQAccelPlot) << "LineCurve custom rendering requires a hardware scene graph backend. Skipping updatePaintNode on the software backend.";
            warned = true;
        }
        delete oldNode;
        return nullptr;
    }

    const auto hasLine = lineStyle_ && lineStyle_->showLine();
    const auto hasPoints = marker_->shape() != MarkerShape::None;
    if (!hasLine && !hasPoints) {
        delete oldNode;
        return nullptr;
    }
    if (!xAxis() || !yAxis()) {
        qCDebug(lcQAccelPlot) << "missing xAxis or yAxis, returning nullptr";
        delete oldNode;
        return nullptr;
    }

    // When the style changes the node type must be recreated from scratch.
    if (styleChanged_) {
        delete oldNode;
        oldNode = nullptr;
        styleChanged_ = false;
    }

    const auto capacity = pointCapacity(window(), hasLine, hasPoints);
    overCapacity_ = Internal::warnIfOutgrowsCapacity(overCapacity_, renderPointCount(), capacity, "LineCurve", "points");
    const auto drawnPointCount = std::min(renderPointCount(), capacity);
    const auto minPoints = hasPoints ? 1 : 2;
    if (drawnPointCount < minPoints) {
        delete oldNode;
        return nullptr;
    }

    // The marker cache holds render positions, which a new render origin moves.
    const auto revision = dataPoints().renderRevision();
    if (revision != vertexCacheRevision_ && !vertexCache_.isReusableForDataChange(LineCurveVertexCache::Layout::Line, drawnPointCount)) {
        vertexCache_.invalidate();
    }
    const auto dataChanged = dataChanged_ || revision != uploadedRevision_;

    const auto pr = resolvePlotRect();
    const auto originX = dataPoints().renderOrigin(DataPointStore::Dimension::X);
    const auto originY = dataPoints().renderOrigin(DataPointStore::Dimension::Y);
    const auto domainMin = QVector2D(static_cast<float>(xAxis()->viewportMin() - originX), static_cast<float>(yAxis()->viewportMin() - originY));
    const auto domainMax = QVector2D(static_cast<float>(xAxis()->viewportMax() - originX), static_cast<float>(yAxis()->viewportMax() - originY));
    const auto viewportSize = QVector2D(static_cast<float>(pr.width()), static_cast<float>(pr.height()));
    const auto gradientPayload = resolveGradientColorPayload();
    const auto gradientFillPayload = resolveGradientFillPayload();
    auto renderGradientPayload = gradientPayload;
    if (renderGradientPayload.isValid()) {
        const auto origin = renderGradientPayload.direction == Qt::Horizontal ? originX : originY;
        if (renderGradientPayload.gradientValueMin.has_value()) {
            renderGradientPayload.gradientValueMin = *renderGradientPayload.gradientValueMin - origin;
        }
        if (renderGradientPayload.gradientValueMax.has_value()) {
            renderGradientPayload.gradientValueMax = *renderGradientPayload.gradientValueMax - origin;
        }
    }

    const auto* cache = vertexCache_.data();

    // A root container node holds both child nodes when both line and points are active.
    // When only one is active we return it directly (no container overhead).
    const auto dualMode = hasLine && hasPoints;

    QSGNode* rootNode = nullptr;
    QSGNode* lineOldNode = nullptr;
    QSGNode* pointOldNode = nullptr;

    if (dualMode) {
        if (!oldNode) {
            rootNode = new QSGNode;
        } else {
            rootNode = oldNode;
        }
        lineOldNode = rootNode->childCount() > 0 ? rootNode->firstChild() : nullptr;
        pointOldNode = rootNode->childCount() > 1 ? rootNode->firstChild()->nextSibling() : nullptr;
    } else {
        lineOldNode = hasLine ? oldNode : nullptr;
        pointOldNode = hasPoints ? oldNode : nullptr;
    }

    QSGNode* lineNode = nullptr;
    QSGNode* pointNode = nullptr;

    if (hasLine) {
        const auto lineParams = LineCurveRenderParams{window(), renderData(), sourceDataView(), drawnPointCount, dataChanged, color_, hovered_, lineWidth_,
            domainMin, domainMax, viewportSize, xAxis(), yAxis(), xAxis()->logScale(), yAxis()->logScale(), antialiasingEnabled_, antialiasingFeather_,
            renderGradientPayload, gradientFillPayload, hasPoints ? nullptr : cache, lineStyle_};
        lineNode = lineRenderer_.paint(lineOldNode, lineParams);
    }

    if (hasPoints) {
        const auto shapeType = static_cast<int>(marker_->shape()) - 1;
        const auto pointParams = PointCurveRenderParams{renderData(), sourceDataView(), drawnPointCount, dataChanged, color_, hovered_, marker_->size(),
            domainMin, domainMax, viewportSize, xAxis()->logScale(), yAxis()->logScale(), antialiasingEnabled_, antialiasingFeather_, gradientPayload,
            hasLine ? nullptr : cache, shapeType, marker_->strokeWidth(), marker_->filled()};
        pointNode = pointRenderer_.paint(pointOldNode, pointParams);
    }

    QSGNode* node = nullptr;
    if (dualMode) {
        // Sync children into rootNode: first=lineNode, second=pointNode
        if (rootNode->childCount() == 0) {
            rootNode->appendChildNode(lineNode);
            rootNode->appendChildNode(pointNode);
        } else {
            if (rootNode->firstChild() != lineNode) {
                rootNode->removeChildNode(rootNode->firstChild());
                rootNode->prependChildNode(lineNode);
            }
            auto* second = rootNode->firstChild()->nextSibling();
            if (second != pointNode) {
                if (second) {
                    rootNode->removeChildNode(second);
                }
                rootNode->appendChildNode(pointNode);
            }
        }
        node = rootNode;
    } else {
        node = hasLine ? lineNode : pointNode;
    }

    dataChanged_ = false;
    uploadedRevision_ = revision;
    return node;
}

void LineCurve::itemChange(const ItemChange change, const ItemChangeData& value)
{
    PlotSeries::itemChange(change, value);
    if (change == ItemSceneChange) {
        connectAnimationTicks(value.window);
    }
}

void LineCurve::hoverEnterEvent(QHoverEvent* event)
{
    hovered_ = true;
    emit hoveredChanged();
    // Color changes via hovered_ are handled by the material uniform (effectiveColor);
    // no vertex rebuild is needed since per-vertex RGBA is only used with gradient effects.
    update();
    QQuickItem::hoverEnterEvent(event);
}

void LineCurve::hoverLeaveEvent(QHoverEvent* event)
{
    hovered_ = false;
    emit hoveredChanged();
    update();
    QQuickItem::hoverLeaveEvent(event);
}

bool LineCurve::contains(const QPointF& point) const
{
    if (!xAxis() || !yAxis()) {
        qCDebug(lcQAccelPlot) << "missing xAxis or yAxis, returning false";
        return false;
    }
    if (!boundingRect().contains(point)) {
        return false;
    }

    if (!chunksValid_) {
        rebuildChunks();
    }

    const qreal w = width();
    const qreal h = height();
    const auto logX = logScaleX();
    const auto logY = logScaleY();

    if (marker_->shape() != MarkerShape::None) {
        // Pixel markers ignore the marker size, so hover them within a small fixed radius.
        constexpr static auto kPixelMarkerHitRadiusPx = qreal{3.0};
        const auto hitRadius = marker_->shape() == MarkerShape::Pixel ? kPixelMarkerHitRadiusPx : marker_->size();
        if (pointRenderer_.contains(point, CurveHitTestParams{sourceDataView(), renderPointCount(), chunks_, xAxis(), yAxis(), w, h, hitRadius, logX, logY})) {
            return true;
        }
    }

    if (lineStyle_ && lineStyle_->showLine() && hoverRadius_ > 0.0) {
        return lineRenderer_.contains(
            point, CurveHitTestParams{sourceDataView(), renderPointCount(), chunks_, xAxis(), yAxis(), w, h, hoverRadius_, logX, logY});
    }

    return false;
}

void LineCurve::applyDataPoints(DataPointInput&& input)
{
    if (input.floatArray) {
        applyFloatArray(input.floatArray, input.count);
    } else if (!input.precise) {
        applyNewData(std::move(input.floats), input.count);
    } else {
        const auto count = input.count;
        applyNewData(std::move(input).toDoubles(), count);
    }
}

void LineCurve::onAxisScaleChanged()
{
    invalidateData();
    refreshVertexCacheForDataChange();
    update();
}

void LineCurve::onDataPointsChanged(const DataChange change, const int /*previousCount*/)
{
    if (change == DataChange::Appended) {
        invalidateData();
        return;
    }
    dataChanged_ = true;
    if (pendingVertexCache_) {
        chunksValid_ = false;
        installVertexCache(*std::exchange(pendingVertexCache_, std::nullopt));
        return;
    }
    refreshVertexCacheForDataChange();
    // Pre-build chunk AABBs on the caller thread alongside the vertex cache so that
    // hover hit-tests never block on a lazy O(N) rebuild during live rendering.
    rebuildChunks();
}

void LineCurve::onTransitionDestroyed()
{
    emit transitionChanged();
    update();
}

void LineCurve::onLineStyleChanged()
{
    styleChanged_ = true;
    invalidateVertices();
    update();
}

void LineCurve::onLineStyleDestroyed()
{
    // lineStyle_ is a QPointer and has already been cleared; repaint without the style.
    emit lineStyleChanged();
    onLineStyleChanged();
}

void LineCurve::onNanGapModeChanged()
{
    dataPoints().setSkipInvalid(gaps_->nanMode() == NanGapMode::Connect);
    invalidateData();
    refreshVertexCacheForDataChange();
    update();
}

void LineCurve::onMarkerShapeChanged()
{
    styleChanged_ = true;
    invalidateVertices();
    update();
}

void LineCurve::appendEffect(QQmlListProperty<LineCurveEffect>* list, LineCurveEffect* effect)
{
    if (!list || !effect) {
        return;
    }

    const auto curve = static_cast<LineCurve*>(list->data);
    if (!curve || curve->effects_.contains(effect)) {
        return;
    }

    if (!effect->parent()) {
        effect->setParent(curve);
    }
    curve->effects_.append(effect);

    QObject::connect(effect, &LineCurveEffect::effectChanged, curve, [curve]() { curve->update(); });
    QObject::connect(effect, &QObject::destroyed, curve, [curve, effect]() {
        curve->effects_.removeAll(effect);
        curve->update();
    });

    curve->update();
}

qsizetype LineCurve::effectCount(QQmlListProperty<LineCurveEffect>* list)
{
    const auto curve = list ? static_cast<LineCurve*>(list->data) : nullptr;
    return curve ? curve->effects_.size() : 0;
}

LineCurveEffect* LineCurve::effectAt(QQmlListProperty<LineCurveEffect>* list, qsizetype index)
{
    const auto curve = list ? static_cast<LineCurve*>(list->data) : nullptr;
    if (!curve || index < 0 || index >= curve->effects_.size()) {
        qCDebug(lcQAccelPlot) << "invalid list or out-of-bounds index" << index << ", returning nullptr";
        return nullptr;
    }

    return curve->effects_.at(index);
}

void LineCurve::clearEffects(QQmlListProperty<LineCurveEffect>* list)
{
    const auto curve = list ? static_cast<LineCurve*>(list->data) : nullptr;
    if (!curve) {
        return;
    }

    for (const auto effect : std::as_const(curve->effects_)) {
        QObject::disconnect(effect, nullptr, curve, nullptr);
    }
    curve->effects_.clear();
    curve->update();
}

bool LineCurve::hasGradientEffect() const
{
    return firstActiveGradientEffect<GradientStroke>(effects_) || firstActiveGradientEffect<GradientFill>(effects_);
}

GradientColorPayload LineCurve::resolveGradientColorPayload() const
{
    return resolveGradientPayload<GradientStroke, GradientColorPayload>(
        effects_, [this](const Qt::Orientation direction) { return gradientDataRange(direction); });
}

GradientFillPayload LineCurve::resolveGradientFillPayload() const
{
    return resolveGradientPayload<GradientFill, GradientFillPayload>(
        effects_, [this](const Qt::Orientation direction) { return gradientDataRange(direction); });
}

std::pair<qreal, qreal> LineCurve::gradientDataRange(const Qt::Orientation direction) const
{
    const auto horizontal = direction == Qt::Horizontal;
    if (const auto range = horizontal ? xDataRange() : yDataRange()) {
        return {range->min, range->max};
    }
    const auto* axis = horizontal ? xAxis() : yAxis();
    return axis ? std::pair{axis->dataMin(), axis->dataMax()} : std::pair{kFallbackDataMin, kFallbackDataMax};
}

void LineCurve::applyNewData(std::vector<float>&& newData, const int newPointCount)
{
    if (transition_->enabled()) {
        // Transitions animate doubles.
        applyNewData(std::vector<double>(newData.begin(), newData.end()), newPointCount);
        return;
    }
    const auto previousCount = dataPoints().count();
    cancelRunningTransition();
    dataPoints().setFloat(std::move(newData), newPointCount);
    notifyDataPointsChanged(DataChange::Replaced, previousCount);
}

void LineCurve::applyNewData(std::vector<double>&& newData, const int newPointCount)
{
    const auto previousCount = dataPoints().count();
    if (!transition_->enabled()) {
        cancelRunningTransition();
        dataPoints().setPrecise(std::move(newData), newPointCount);
        notifyDataPointsChanged(DataChange::Replaced, previousCount);
        return;
    }
    // Copied before the data changes: the transition starts from the points drawn now.
    const auto drawn = dataPoints().drawnView();
    auto from = std::vector<double>(static_cast<std::size_t>(drawn.count) * 2);
    for (auto index = 0; index < drawn.count; ++index) {
        from[static_cast<std::size_t>(index) * 2] = drawn.value(index, 0);
        from[static_cast<std::size_t>(index) * 2 + 1] = drawn.value(index, 1);
    }
    dataPoints().setPrecise(std::vector<double>(newData), newPointCount);
    startTransition(std::move(from), drawn.count);
    notifyDataPointsChanged(DataChange::Replaced, previousCount);
}

void LineCurve::applyFloatArray(const float* xyInterleaved, const int pointCount)
{
    if (transition_->enabled()) {
        // Transitions animate doubles.
        applyNewData(std::vector<double>(xyInterleaved, xyInterleaved + static_cast<std::size_t>(pointCount) * 2), pointCount);
        return;
    }
    const auto previousCount = dataPoints().count();
    cancelRunningTransition();
    // Reuses the allocation of the stored points.
    dataPoints().assignFloat(xyInterleaved, pointCount);
    notifyDataPointsChanged(DataChange::Replaced, previousCount);
}

const std::vector<float>& LineCurve::renderData() const
{
    return dataPoints().renderData();
}

CurveDataView LineCurve::sourceDataView() const
{
    const auto points = dataPoints().renderedView();
    return CurveDataView{points.floats, points.precise};
}

int LineCurve::renderPointCount() const
{
    return dataPoints().renderCount();
}

void LineCurve::refreshVertexCacheForDataChange()
{
    const auto isDash = lineStyle_ && lineStyle_->showLine() && lineStyle_->dashParameters().enabled;
    const auto hasLine = lineStyle_ && lineStyle_->showLine();
    const auto hasPoints = marker_->shape() != MarkerShape::None;

    if (hasGradientEffect() || isDash || (hasLine && hasPoints)) {
        vertexCache_.invalidate();
        return;
    }

    // Solid-line cache vertices contain only point indices and ribbon sides, so
    // they remain valid across same-sized data updates. Marker cache vertices
    // contain XY positions and must be rebuilt for every update.
    if (hasLine && vertexCache_.isReusableForDataChange(LineCurveVertexCache::Layout::Line, renderPointCount())) {
        return;
    }

    rebuildVertexCache();
}

std::size_t LineCurve::expectedVertexCacheSize() const
{
    if (renderPointCount() <= 0) {
        return 0;
    }
    const auto hasLine = lineStyle_ && lineStyle_->showLine();
    const auto hasPoints = marker_->shape() != MarkerShape::None;
    const auto isDash = hasLine && lineStyle_->dashParameters().enabled;
    if (hasGradientEffect() || isDash || (hasLine && hasPoints)) {
        return 0;
    }
    if (hasLine && renderPointCount() >= 2) {
        return static_cast<std::size_t>(renderPointCount()) * 2 * sizeof(LineVertex);
    }
    if (hasPoints) {
        return static_cast<std::size_t>(renderPointCount()) * 6 * sizeof(PointVertex);
    }
    return 0;
}

void LineCurve::installVertexCache(std::vector<char>&& vertexCache)
{
    if (renderPointCount() != dataPoints().count()) {
        // Externally built caches describe the uncompacted samples. Connect mode
        // removed invalid samples, so the cache is rebuilt for the compacted data.
        vertexCache_.invalidate();
        refreshVertexCacheForDataChange();
        return;
    }

    const auto expectedSize = expectedVertexCacheSize();
    const auto actualSize = vertexCache.size();
    if (!vertexCache.empty() && actualSize != expectedSize) {
        qCWarning(lcQAccelPlot) << "Ignoring vertex cache with" << actualSize << "bytes; expected" << expectedSize;
    }

    const auto layout = (lineStyle_ && lineStyle_->showLine()) ? LineCurveVertexCache::Layout::Line : LineCurveVertexCache::Layout::Points;
    (void)vertexCache_.install(std::move(vertexCache), layout, renderPointCount(), expectedSize);
    vertexCacheRevision_ = dataPoints().renderRevision();
}

void LineCurve::rebuildVertexCache()
{
    if (hasGradientEffect() || transition_->active() || renderPointCount() == 0) {
        vertexCache_.invalidate();
        return;
    }
    const auto isDash = lineStyle_ && lineStyle_->showLine() && lineStyle_->dashParameters().enabled;
    // A cache of more points than the curve draws would not be used.
    if (lineStyle_ && lineStyle_->showLine() && !isDash && renderPointCount() >= 2 && renderPointCount() <= kMaxLinePoints) {
        vertexCache_.rebuild(LineCurveVertexCache::Layout::Line, renderPointCount(),
            [this](std::vector<char>& bytes) { lineRenderer_.buildVertexCache(renderData(), renderPointCount(), bytes); });
    } else if (marker_->shape() != MarkerShape::None && renderPointCount() >= 1 && renderPointCount() <= kMaxMarkerPoints) {
        vertexCache_.rebuild(LineCurveVertexCache::Layout::Points, renderPointCount(),
            [this](std::vector<char>& bytes) { pointRenderer_.buildVertexCache(renderData(), renderPointCount(), bytes); });
    } else {
        vertexCache_.invalidate();
    }
    vertexCacheRevision_ = dataPoints().renderRevision();
}

void LineCurve::rebuildChunks() const
{
    chunks_.clear();
    const auto pointCount = renderPointCount();
    if (pointCount == 0) {
        chunksValid_ = true;
        return;
    }

    constexpr static auto kChunkSize = int{512};
    const auto sourceData = sourceDataView();
    const auto logX = logScaleX();
    const auto logY = logScaleY();
    chunks_.reserve(static_cast<std::size_t>((pointCount + kChunkSize - 1) / kChunkSize));
    for (auto start = int{0}; start < pointCount; start += kChunkSize) {
        const auto count = std::min(kChunkSize, pointCount - start);
        // Extend AABB one point beyond the chunk boundary on each side so that
        // line segments bridging adjacent chunks are fully covered. Invalid samples
        // are excluded; a chunk without valid samples keeps min > max and is skipped.
        const auto aabbBegin = std::max(0, start - 1);
        const auto aabbEnd = std::min(start + count, pointCount - 1); // inclusive
        const auto extents = sourceData.doubleData ? computeChunkExtents(sourceData.doubleData, aabbBegin, aabbEnd, logX, logY)
                                                   : computeChunkExtents(sourceData.floatData, aabbBegin, aabbEnd, logX, logY);
        chunks_.push_back(CurveChunk{start, count, extents.xMin, extents.xMax, extents.yMin, extents.yMax});
    }
    chunksValid_ = true;
}

void LineCurve::invalidateVertices()
{
    dataChanged_ = true;
    vertexCache_.invalidate();
}

void LineCurve::invalidateData()
{
    dataChanged_ = true;
    vertexCache_.invalidate();
    chunksValid_ = false;
}

void LineCurve::cancelRunningTransition()
{
    transition_->cancel();
    dataPoints().clearDrawn();
    frameBuffer_ = std::vector<double>{};
}

void LineCurve::finishTransition()
{
    if (!transition_->pending()) {
        return;
    }
    // The stored points are already the target of the run.
    cancelRunningTransition();
    invalidateData();
    invalidateInspection();
    update();
}

void LineCurve::startTransition(std::vector<double>&& from, const int fromCount)
{
    const auto target = dataPoints().view();
    transition_->start(from, fromCount, std::vector<double>(target.precise, target.precise + static_cast<std::size_t>(target.count) * 2), target.count);
    frameBuffer_ = dataPoints().setDrawn(std::move(from), fromCount);
}

void LineCurve::connectAnimationTicks(QQuickWindow* window)
{
    if (animationTickWindow_ == window) {
        return;
    }
    if (animationTickWindow_) {
        disconnect(animationTickWindow_, &QQuickWindow::afterAnimating, this, &LineCurve::refreshEffects);
    }
    animationTickWindow_ = window;
    if (window) {
        connect(window, &QQuickWindow::afterAnimating, this, &LineCurve::refreshEffects);
    }
}

void LineCurve::refreshEffects()
{
    // Loop by index rather than range-for: refresh() emits effectChanged(), and a handler that
    // removes an effect would invalidate the iterators.
    for (auto i = qsizetype{0}; i < effects_.size(); ++i) {
        effects_.at(i)->refresh();
    }
}

void LineCurve::advanceTransition()
{
    auto frameCount = 0;
    transition_->advance(frameBuffer_, frameCount);
    if (transition_->pending()) {
        frameBuffer_ = dataPoints().setDrawn(std::move(frameBuffer_), frameCount);
    } else {
        // The run ended at its target, which the stored points already are.
        dataPoints().clearDrawn();
        frameBuffer_ = std::vector<double>{};
        invalidateInspection();
    }
    invalidateData();
    update();
}

} // namespace QAccelPlot
