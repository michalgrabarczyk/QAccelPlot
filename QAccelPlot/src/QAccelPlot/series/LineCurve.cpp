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

#include <QHoverEvent>
#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <cstring>
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

template <typename T> bool validateInterleavedVector(const std::vector<T>& data, const int pointCount)
{
    static_assert(std::is_same_v<T, float> || std::is_same_v<T, double>, "LineCurve data must be float or double");
    if (pointCount < 0) {
        qCWarning(lcQAccelPlot) << "LineCurve data point count cannot be negative:" << pointCount;
        return false;
    }

    const auto expectedValueCount = static_cast<std::size_t>(pointCount) * 2;
    if (data.size() != expectedValueCount) {
        if constexpr (std::is_same_v<T, float>) {
            qCWarning(lcQAccelPlot) << "LineCurve received" << data.size() << "floats for" << pointCount << "points; expected" << expectedValueCount;
        } else {
            qCWarning(lcQAccelPlot) << "LineCurve received" << data.size() << "doubles for" << pointCount << "points; expected" << expectedValueCount;
        }
        return false;
    }
    return true;
}

template <typename T> DataExtents computeDataExtents(const std::vector<T>& buf, const int count, const bool logScaleX, const bool logScaleY)
{
    const auto available = std::min(static_cast<std::size_t>(std::max(count, 0)), buf.size() / 2);
    return computeValidExtents(buf.data(), 0, available, logScaleX, logScaleY);
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

}

LineCurve::LineCurve(QQuickItem* parent)
    : PlotSeries(parent)
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
}

LineCurve::~LineCurve()
{
    // Ending the run emits runningChanged, which must not reach this partly destroyed curve.
    if (transition_) {
        disconnect(transition_, nullptr, this, nullptr);
    }
    transitionRun_.cancel();
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
    if (nearly_equal(lineWidth_, w)) {
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
    if (nearly_equal(hoverRadius_, clamped)) {
        return;
    }
    hoverRadius_ = clamped;
    emit hoverRadiusChanged();
    // Marks the item dirty so the window re-delivers hover at the cursor and updates hovered.
    update();
}

DataTransition* LineCurve::transition() const
{
    return transition_;
}

void LineCurve::setTransition(DataTransition* transition)
{
    if (transition_ == transition) {
        return;
    }
    finishTransition();
    if (transition_) {
        disconnect(transition_, &QObject::destroyed, this, &LineCurve::onTransitionDestroyed);
        disconnect(transition_, &DataTransition::runningChanged, this, &LineCurve::onTransitionRunningChanged);
    }
    transition_ = transition;
    if (transition_) {
        connect(transition_, &QObject::destroyed, this, &LineCurve::onTransitionDestroyed);
        connect(transition_, &DataTransition::runningChanged, this, &LineCurve::onTransitionRunningChanged);
    }
    emit transitionChanged();
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
    if (nearly_equal(antialiasingFeather_, clamped)) {
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
    promoteFloatDataToDouble();

    const auto logX = logScaleX();
    const auto logY = logScaleY();
    const auto canExtendRenderData = renderOriginXSettled_ && renderOriginYSettled_ && renderLogScaleX_ == logX && renderLogScaleY_ == logY
        && renderData_.size() == static_cast<std::size_t>(pointCount_) * 2;

    data_.push_back(static_cast<double>(x));
    data_.push_back(static_cast<double>(y));
    pointCount_++;

    // Each coordinate is judged on its own, matching computeDataRanges().
    if (isValidSample(x, logX)) {
        extendXDataRange(x);
    }
    if (isValidSample(y, logY)) {
        extendYDataRange(y);
    }
    if (canExtendRenderData) {
        renderData_.push_back(static_cast<float>(x - renderOriginX_));
        renderData_.push_back(static_cast<float>(y - renderOriginY_));
    } else {
        rebuildDoubleRenderData(logX, logY);
    }
    rebuildGapConnectData();

    invalidateData();
    inspectionDataChanged(DataChange::Appended);
    update();
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

void LineCurve::setData(const double* xyInterleaved, const int pointCount)
{
    if (!validateRawDataArguments(xyInterleaved, pointCount)) {
        return;
    }
    auto data = std::vector<double>{};
    if (pointCount > 0) {
        data.assign(xyInterleaved, xyInterleaved + static_cast<std::size_t>(pointCount) * 2);
    }
    setData(std::move(data), pointCount);
}

void LineCurve::setData(std::vector<double>&& xyInterleaved, const int pointCount)
{
    if (!validateVectorDataArguments(xyInterleaved, pointCount)) {
        return;
    }
    applyNewData(std::move(xyInterleaved), pointCount);
}

void LineCurve::setDataF(const float* xyInterleaved, const int pointCount)
{
    if (!validateRawDataArguments(xyInterleaved, pointCount)) {
        return;
    }

    if (transition_ && transition_->enabled()) {
        auto newData = std::vector<float>(static_cast<std::size_t>(pointCount) * 2);
        if (pointCount > 0) {
            std::memcpy(newData.data(), xyInterleaved, newData.size() * sizeof(float));
        }
        applyNewData(std::move(newData), pointCount);
        return;
    }

    cancelRunningTransition();
    copyRawData(xyInterleaved, pointCount);
    pointCount_ = pointCount;
    rebuildGapConnectData();
    refreshVertexCacheForDataChange();
    rebuildChunks();
    dataChanged_ = true;
    invalidateDataRanges();
    inspectionDataChanged();
    update();
}

void LineCurve::setDataF(std::vector<float>&& data, const int pointCount)
{
    if (!validateVectorDataArguments(data, pointCount)) {
        return;
    }
    applyNewData(std::move(data), pointCount);
}

void LineCurve::setDataFWithCache(std::vector<float>&& data, const int pointCount, std::vector<char>&& vertexCache)
{
    if (!validateVectorDataArguments(data, pointCount)) {
        return;
    }
    if (transition_ && transition_->enabled()) {
        applyNewData(std::move(data), pointCount);
        return;
    }

    cancelRunningTransition();
    pointCount_ = pointCount;
    dataType_ = DataType::Float;
    data_.clear();
    renderData_.clear();
    dataF_ = std::move(data);
    renderOriginX_ = 0.0;
    renderOriginY_ = 0.0;
    renderLogScaleX_ = false;
    renderLogScaleY_ = false;
    rebuildGapConnectData();
    installVertexCache(std::move(vertexCache));
    dataChanged_ = true;
    chunksValid_ = false;
    invalidateDataRanges();
    inspectionDataChanged();
    update();
}

void LineCurve::setDataFWithCache(const float* xyInterleaved, const int pointCount, std::vector<char>&& vertexCache)
{
    if (!validateRawDataArguments(xyInterleaved, pointCount)) {
        return;
    }

    if (transition_ && transition_->enabled()) {
        auto newData = std::vector<float>(static_cast<std::size_t>(pointCount) * 2);
        if (pointCount > 0) {
            std::memcpy(newData.data(), xyInterleaved, newData.size() * sizeof(float));
        }
        applyNewData(std::move(newData), pointCount);
        return;
    }

    cancelRunningTransition();
    copyRawData(xyInterleaved, pointCount);
    pointCount_ = pointCount;
    rebuildGapConnectData();
    installVertexCache(std::move(vertexCache));
    dataChanged_ = true;
    chunksValid_ = false;
    invalidateDataRanges();
    inspectionDataChanged();
    update();
}

void LineCurve::postData(std::vector<double>&& xyInterleaved, const int pointCount)
{
    QMetaObject::invokeMethod(
        this, [this, data = std::move(xyInterleaved), pointCount]() mutable { setData(std::move(data), pointCount); }, Qt::QueuedConnection);
}

void LineCurve::postData(std::vector<float>&& xyInterleaved, const int pointCount)
{
    QMetaObject::invokeMethod(
        this, [this, data = std::move(xyInterleaved), pointCount]() mutable { setDataF(std::move(data), pointCount); }, Qt::QueuedConnection);
}

void LineCurve::clearData()
{
    cancelRunningTransition();
    data_.clear();
    dataF_.clear();
    renderData_.clear();
    renderOriginX_ = 0.0;
    renderOriginY_ = 0.0;
    renderLogScaleX_ = false;
    renderLogScaleY_ = false;
    renderOriginXSettled_ = false;
    renderOriginYSettled_ = false;
    pointCount_ = 0;
    releaseGapConnectData();
    invalidateVertices();
    chunks_.clear();
    chunksValid_ = true;
    invalidateDataRanges();
    inspectionDataChanged();
    update();
}

InspectionSource LineCurve::inspectionSource() const
{
    auto source = InspectionSource{};
    source.count = pointCount_;
    source.logX = logScaleX();
    source.logY = logScaleY();
    if (dataType_ == DataType::Double) {
        source.doubles = data_.data();
    } else {
        source.floats = dataF_.data();
    }
    return source;
}

bool LineCurve::inspectionAvailable() const
{
    return !transitionRun_.pending();
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

    const auto drawnPointCount = renderPointCount();
    const auto minPoints = hasPoints ? 1 : 2;
    if (drawnPointCount < minPoints) {
        delete oldNode;
        return nullptr;
    }

    const auto pr = resolvePlotRect();
    const auto domainMin = QVector2D(static_cast<float>(xAxis()->viewportMin() - renderOriginX_), static_cast<float>(yAxis()->viewportMin() - renderOriginY_));
    const auto domainMax = QVector2D(static_cast<float>(xAxis()->viewportMax() - renderOriginX_), static_cast<float>(yAxis()->viewportMax() - renderOriginY_));
    const auto viewportSize = QVector2D(static_cast<float>(pr.width()), static_cast<float>(pr.height()));
    const auto gradientPayload = resolveGradientColorPayload();
    const auto gradientFillPayload = resolveGradientFillPayload();
    auto renderGradientPayload = gradientPayload;
    if (dataType_ == DataType::Double && renderGradientPayload.isValid()) {
        const auto origin = renderGradientPayload.direction == GradientDirection::Horizontal ? renderOriginX_ : renderOriginY_;
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
        const auto lineParams = LineCurveRenderParams{window(), renderData(), sourceDataView(), drawnPointCount, dataChanged_, color_, hovered_, lineWidth_,
            domainMin, domainMax, viewportSize, xAxis(), yAxis(), xAxis()->logScale(), yAxis()->logScale(), antialiasingEnabled_, antialiasingFeather_,
            renderGradientPayload, gradientFillPayload, hasPoints ? nullptr : cache, lineStyle_};
        lineNode = lineRenderer_.paint(lineOldNode, lineParams);
    }

    if (hasPoints) {
        const auto shapeType = static_cast<int>(marker_->shape()) - 1;
        const auto pointParams = PointCurveRenderParams{renderData(), sourceDataView(), drawnPointCount, dataChanged_, color_, hovered_, marker_->size(),
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

void LineCurve::onAxisScaleChanged()
{
    // Log scale changes which samples are valid.
    invalidateDataRanges();
    rebuildDoubleRenderData(logScaleX(), logScaleY());
    rebuildGapConnectData();
    invalidateData();
    refreshVertexCacheForDataChange();
    update();
}

PlotSeries::DataRanges LineCurve::computeDataRanges() const
{
    const auto logX = logScaleX();
    const auto logY = logScaleY();
    const auto extents = [this, logX, logY] {
        // A running transition ends at its target, so the axes fit the target from the start.
        if (transitionRun_.pending()) {
            return computeDataExtents(transitionRun_.targetData(), transitionRun_.targetPointCount(), logX, logY);
        }
        if (dataType_ == DataType::Double) {
            return computeDataExtents(data_, pointCount_, logX, logY);
        }
        return computeDataExtents(dataF_, pointCount_, logX, logY);
    }();
    // A dimension without a valid coordinate has min > max, which the base class treats as unset.
    return {DataExtent{extents.xMin, extents.xMax}, DataExtent{extents.yMin, extents.yMax}};
}

void LineCurve::onAxisRangeChanged()
{
    PlotSeries::onAxisRangeChanged();
    if (dataType_ != DataType::Double) {
        return;
    }
    const auto rebaseX = !renderLogScaleX_ && xAxis() && renderOriginTooFar(renderOriginX_, xAxis()->viewportMin(), xAxis()->viewportMax());
    const auto rebaseY = !renderLogScaleY_ && yAxis() && renderOriginTooFar(renderOriginY_, yAxis()->viewportMin(), yAxis()->viewportMax());
    if (!rebaseX && !rebaseY) {
        return;
    }
    rebuildDoubleRenderData(logScaleX(), logScaleY());
    rebuildGapConnectData();
    dataChanged_ = true;
    refreshVertexCacheForDataChange();
}

void LineCurve::onTransitionDestroyed()
{
    finishTransition();
    emit transitionChanged();
    update();
}

void LineCurve::onTransitionRunningChanged()
{
    // DataTransition::cancel() ends every run but leaves its data pending for the host.
    if (!transitionRun_.active()) {
        finishTransition();
    }
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
    rebuildGapConnectData();
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
        effects_, [this](const GradientDirection direction) { return gradientDataRange(direction); });
}

GradientFillPayload LineCurve::resolveGradientFillPayload() const
{
    return resolveGradientPayload<GradientFill, GradientFillPayload>(
        effects_, [this](const GradientDirection direction) { return gradientDataRange(direction); });
}

std::pair<qreal, qreal> LineCurve::gradientDataRange(const GradientDirection direction) const
{
    const auto horizontal = direction == GradientDirection::Horizontal;
    if (const auto range = horizontal ? xDataRange() : yDataRange()) {
        return {range->min, range->max};
    }
    const auto* axis = horizontal ? xAxis() : yAxis();
    return axis ? std::pair{axis->dataMin(), axis->dataMax()} : std::pair{kFallbackDataMin, kFallbackDataMax};
}

bool LineCurve::logScaleX() const
{
    return xAxis() && xAxis()->logScale();
}

bool LineCurve::logScaleY() const
{
    return yAxis() && yAxis()->logScale();
}

void LineCurve::applyNewData(std::vector<float>&& newData, const int newPointCount)
{
    if (transition_ && transition_->enabled()) {
        auto preciseData = std::vector<double>(newData.begin(), newData.end());
        applyNewData(std::move(preciseData), newPointCount);
    } else {
        cancelRunningTransition();
        dataType_ = DataType::Float;
        pointCount_ = newPointCount;
        data_.clear();
        renderData_.clear();
        dataF_ = std::move(newData);
        renderOriginX_ = 0.0;
        renderOriginY_ = 0.0;
        renderLogScaleX_ = false;
        renderLogScaleY_ = false;
        rebuildGapConnectData();
        refreshVertexCacheForDataChange();

        // Pre-build chunk AABBs on the caller thread alongside the vertex cache so that
        // hover hit-tests never block on a lazy O(N) rebuild during live rendering.
        rebuildChunks();

        dataChanged_ = true;
        invalidateDataRanges();
        inspectionDataChanged();
        update();
    }
}

void LineCurve::applyNewData(std::vector<double>&& newData, const int newPointCount)
{
    if (transition_ && transition_->enabled()) {
        promoteFloatDataToDouble();
        dataType_ = DataType::Double;
        transition_->start(transitionRun_, data_, pointCount_, std::move(newData), newPointCount);

        // Do not update pointCount_ here. transition_->advance() updates it through its
        // output argument when it produces data_; changing only the count now would
        // make it refer to a buffer that has not been produced yet.
        invalidateData();
        invalidateDataRanges();
        inspectionDataChanged();
        update();
        return;
    }

    cancelRunningTransition();
    dataType_ = DataType::Double;
    pointCount_ = newPointCount;
    dataF_.clear();
    data_ = std::move(newData);
    rebuildDoubleRenderData(logScaleX(), logScaleY());
    rebuildGapConnectData();
    refreshVertexCacheForDataChange();
    rebuildChunks();
    dataChanged_ = true;
    invalidateDataRanges();
    inspectionDataChanged();
    update();
}

bool LineCurve::validateRawDataArguments(const void* xyInterleaved, const int pointCount) const
{
    if (pointCount < 0) {
        qCWarning(lcQAccelPlot) << "LineCurve data point count cannot be negative:" << pointCount;
        return false;
    }
    if (pointCount > 0 && !xyInterleaved) {
        qCWarning(lcQAccelPlot) << "LineCurve received a null data pointer for" << pointCount << "points";
        return false;
    }
    return true;
}

bool LineCurve::validateVectorDataArguments(const std::vector<float>& data, const int pointCount) const
{
    return validateInterleavedVector(data, pointCount);
}

bool LineCurve::validateVectorDataArguments(const std::vector<double>& data, const int pointCount) const
{
    return validateInterleavedVector(data, pointCount);
}

void LineCurve::copyRawData(const float* xyInterleaved, const int pointCount)
{
    const auto floatCount = static_cast<std::size_t>(pointCount) * 2;
    dataType_ = DataType::Float;
    data_.clear();
    renderData_.clear();
    dataF_.resize(floatCount);
    if (floatCount > 0) {
        std::memcpy(dataF_.data(), xyInterleaved, floatCount * sizeof(float));
    }
    renderOriginX_ = 0.0;
    renderOriginY_ = 0.0;
    renderLogScaleX_ = false;
    renderLogScaleY_ = false;
}

void LineCurve::promoteFloatDataToDouble()
{
    if (dataType_ == DataType::Double) {
        return;
    }

    data_.assign(dataF_.begin(), dataF_.end());
    dataF_.clear();
    dataType_ = DataType::Double;
    rebuildDoubleRenderData(xAxis() && xAxis()->logScale(), yAxis() && yAxis()->logScale());
}

void LineCurve::rebuildDoubleRenderData(const bool logScaleX, const bool logScaleY)
{
    if (dataType_ != DataType::Double) {
        return;
    }

    renderLogScaleX_ = logScaleX;
    renderLogScaleY_ = logScaleY;
    renderOriginX_ = 0.0;
    renderOriginY_ = 0.0;
    auto foundOriginX = logScaleX;
    auto foundOriginY = logScaleY;

    for (auto i = int{0}; i < pointCount_; ++i) {
        const auto x = data_[static_cast<std::size_t>(i) * 2];
        const auto y = data_[static_cast<std::size_t>(i) * 2 + 1];
        if (!foundOriginX && std::isfinite(x)) {
            renderOriginX_ = x;
            foundOriginX = true;
        }
        if (!foundOriginY && std::isfinite(y)) {
            renderOriginY_ = y;
            foundOriginY = true;
        }
        if (foundOriginX && foundOriginY) {
            break;
        }
    }
    // Log dimensions are never shifted: the vertex shader takes log10 of the uploaded value.
    if (!logScaleX && xAxis()) {
        renderOriginX_ = renderOriginForViewport(renderOriginX_, xAxis()->viewportMin(), xAxis()->viewportMax());
    }
    if (!logScaleY && yAxis()) {
        renderOriginY_ = renderOriginForViewport(renderOriginY_, yAxis()->viewportMin(), yAxis()->viewportMax());
    }

    renderOriginXSettled_ = foundOriginX;
    renderOriginYSettled_ = foundOriginY;

    renderData_.resize(static_cast<std::size_t>(pointCount_) * 2);
    for (auto i = int{0}; i < pointCount_; ++i) {
        const auto offset = static_cast<std::size_t>(i) * 2;
        renderData_[offset] = static_cast<float>(data_[offset] - renderOriginX_);
        renderData_[offset + 1] = static_cast<float>(data_[offset + 1] - renderOriginY_);
    }
}

const std::vector<float>& LineCurve::renderData() const
{
    if (gapConnectCompacted_) {
        return gapConnectRenderData_;
    }
    return dataType_ == DataType::Double ? renderData_ : dataF_;
}

CurveDataView LineCurve::sourceDataView() const
{
    if (dataType_ == DataType::Double) {
        return CurveDataView{nullptr, gapConnectCompacted_ ? gapConnectData_.data() : data_.data()};
    }
    return CurveDataView{gapConnectCompacted_ ? gapConnectRenderData_.data() : dataF_.data(), nullptr};
}

int LineCurve::renderPointCount() const
{
    return gapConnectCompacted_ ? gapConnectPointCount_ : pointCount_;
}

void LineCurve::rebuildGapConnectData()
{
    const auto renderDataReady = dataType_ == DataType::Float || renderData_.size() >= static_cast<std::size_t>(pointCount_) * 2;
    if (gaps_->nanMode() != NanGapMode::Connect || pointCount_ <= 0 || !renderDataReady) {
        releaseGapConnectData();
        return;
    }

    const auto logX = logScaleX();
    const auto logY = logScaleY();
    const auto fullSource = dataType_ == DataType::Double ? CurveDataView{nullptr, data_.data()} : CurveDataView{dataF_.data(), nullptr};
    if (LineCurveGapFilter::countInvalidPoints(fullSource, pointCount_, logX, logY) == 0) {
        // Nothing to remove: render the original buffers without copying.
        releaseGapConnectData();
        return;
    }

    if (dataType_ == DataType::Double) {
        gapConnectPointCount_ = LineCurveGapFilter::compactValidPoints(data_, renderData_, pointCount_, logX, logY, gapConnectData_, gapConnectRenderData_);
    } else {
        gapConnectData_.clear();
        gapConnectPointCount_ = LineCurveGapFilter::compactValidPoints(dataF_, pointCount_, logX, logY, gapConnectRenderData_);
    }
    gapConnectCompacted_ = true;
}

void LineCurve::releaseGapConnectData()
{
    gapConnectCompacted_ = false;
    gapConnectPointCount_ = 0;
    gapConnectData_.clear();
    gapConnectRenderData_.clear();
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
    if (gapConnectCompacted_) {
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
}

void LineCurve::rebuildVertexCache()
{
    if (hasGradientEffect() || transitionRun_.active() || renderPointCount() == 0) {
        vertexCache_.invalidate();
        return;
    }
    const auto isDash = lineStyle_ && lineStyle_->showLine() && lineStyle_->dashParameters().enabled;
    if (lineStyle_ && lineStyle_->showLine() && !isDash && renderPointCount() >= 2) {
        vertexCache_.rebuild(LineCurveVertexCache::Layout::Line, renderPointCount(),
            [this](std::vector<char>& bytes) { lineRenderer_.buildVertexCache(renderData(), renderPointCount(), bytes); });
    } else if (marker_->shape() != MarkerShape::None && renderPointCount() >= 1) {
        vertexCache_.rebuild(LineCurveVertexCache::Layout::Points, renderPointCount(),
            [this](std::vector<char>& bytes) { pointRenderer_.buildVertexCache(renderData(), renderPointCount(), bytes); });
    } else {
        vertexCache_.invalidate();
    }
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
    transitionRun_.cancel();
}

void LineCurve::finishTransition()
{
    if (transitionRun_.finish(data_, pointCount_)) {
        applyTransitionData();
    }
}

void LineCurve::connectAnimationTicks(QQuickWindow* window)
{
    if (animationTickWindow_ == window) {
        return;
    }
    if (animationTickWindow_) {
        disconnect(animationTickWindow_, &QQuickWindow::afterAnimating, this, &LineCurve::refreshEffects);
        disconnect(animationTickWindow_, &QQuickWindow::afterAnimating, this, &LineCurve::advanceTransition);
    }
    animationTickWindow_ = window;
    if (window) {
        connect(window, &QQuickWindow::afterAnimating, this, &LineCurve::refreshEffects);
        connect(window, &QQuickWindow::afterAnimating, this, &LineCurve::advanceTransition);
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
    if (!transition_ || !transitionRun_.active()) {
        return;
    }
    // afterAnimating is emitted on the GUI thread before the scene graph syncs, so runningChanged
    // reaches QML there, and this frame renders the new step.
    transition_->advance(transitionRun_, data_, pointCount_);
    applyTransitionData();
    // The item is still dirty from this frame, so update() alone does not schedule the next one.
    // A runningChanged handler may have started a new run, so the run is checked again here.
    if (transitionRun_.active() && animationTickWindow_) {
        animationTickWindow_->update();
    }
}

void LineCurve::applyTransitionData()
{
    dataType_ = DataType::Double;
    rebuildDoubleRenderData(logScaleX(), logScaleY());
    rebuildGapConnectData();
    invalidateData();
    if (!transitionRun_.pending()) {
        invalidateInspection();
    }
    update();
}

} // namespace QAccelPlot
