//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "series/LineCurve.hpp"
#include "MathUtils.hpp"
#include "QAccelPlot.hpp"
#include "QAccelPlotLogging.hpp"
#include "axis/Axis.hpp"
#include "effects/GradientFill.hpp"
#include "effects/GradientStroke.hpp"
#include "transitions/DataTransition.hpp"

#include <QHoverEvent>
#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QSGRendererInterface>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace QAccelPlot {

namespace {

bool supportsCustomShaderRendering(const QQuickWindow* window)
{
    if (!window) {
        return false;
    }

    const auto* rendererInterface = window->rendererInterface();
    if (!rendererInterface) {
        return false;
    }

    return rendererInterface->graphicsApi() != QSGRendererInterface::Software;
}

bool hoverEnabled()
{
    auto isInteger = false;
    const auto value = qEnvironmentVariableIntValue("QACCELPLOT_HOVER_ENABLED", &isInteger);
    return !isInteger || value != 0;
}

// Fallback data-range bounds used when no axis is attached to resolve gradient normalization.
constexpr auto kFallbackDataMin = float{0.0f};
constexpr auto kFallbackDataMax = float{1.0f};

}

LineCurve::LineCurve(QQuickItem* parent)
    : PlotSeries(parent)
{
    setFlag(ItemHasContents, true);
    setAcceptHoverEvents(hoverEnabled());
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

DataTransition* LineCurve::transition() const
{
    return transition_;
}

void LineCurve::setTransition(DataTransition* transition)
{
    if (transition_ == transition) {
        return;
    }
    transition_ = transition;
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
    }
    lineStyle_ = style;
    if (lineStyle_) {
        connect(lineStyle_, &LineStyle::styleChanged, this, &LineCurve::onLineStyleChanged);
    }
    styleChanged_ = true;
    emit lineStyleChanged();
    invalidateVertices();
    update();
}

LineCurve::PointShape LineCurve::markerShape() const
{
    return markerShape_;
}

void LineCurve::setMarkerShape(PointShape shape)
{
    if (markerShape_ == shape) {
        return;
    }
    markerShape_ = shape;
    styleChanged_ = true;
    emit markerShapeChanged();
    invalidateVertices();
    update();
}

qreal LineCurve::markerSize() const
{
    return markerSize_;
}

void LineCurve::setMarkerSize(qreal r)
{
    if (nearly_equal(markerSize_, r)) {
        return;
    }
    markerSize_ = r;
    emit markerSizeChanged();
    // markerSize is a material uniform — no vertex rebuild needed.
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

void LineCurve::appendData(const qreal x, const qreal y)
{
    data_.push_back(static_cast<float>(x));
    data_.push_back(static_cast<float>(y));
    pointCount_++;
    updateDataRanges(data_, pointCount_);
    invalidateData();
    update();
}

void LineCurve::clearData()
{
    if (transition_) {
        transition_->cancel();
    }
    data_.clear();
    pointCount_ = 0;
    invalidateVertices();
    chunks_.clear();
    chunksValid_ = true;
    clearDataRanges();
    update();
}

void LineCurve::setData(const QList<QPointF>& data)
{
    auto newCount = static_cast<int>(data.size());
    std::vector<float> newData(newCount * 2);
    for (int i = 0; i < newCount; ++i) {
        newData[i * 2] = static_cast<float>(data[i].x());
        newData[i * 2 + 1] = static_cast<float>(data[i].y());
    }
    updateDataRanges(newData, newCount);
    applyNewData(std::move(newData), newCount);
}

void LineCurve::setData(const std::vector<double>& xs, const std::vector<double>& ys)
{
    const auto newCount = static_cast<int>(std::min(xs.size(), ys.size()));
    auto newData = std::vector<float>(static_cast<std::size_t>(newCount) * 2);
    for (auto i = int{0}; i < newCount; ++i) {
        newData[static_cast<std::size_t>(i * 2)] = static_cast<float>(xs[static_cast<std::size_t>(i)]);
        newData[static_cast<std::size_t>(i * 2 + 1)] = static_cast<float>(ys[static_cast<std::size_t>(i)]);
    }
    updateDataRanges(newData, newCount);
    applyNewData(std::move(newData), newCount);
}

void LineCurve::setDataF(const float* xyInterleaved, const int pointCount)
{
    if (!validateRawDataArguments(xyInterleaved, pointCount)) {
        return;
    }
    auto newData = std::vector<float>(static_cast<std::size_t>(pointCount) * 2);
    if (pointCount > 0) {
        std::memcpy(newData.data(), xyInterleaved, newData.size() * sizeof(float));
    }
    updateDataRanges(newData, pointCount);
    applyNewData(std::move(newData), pointCount);
}

void LineCurve::setDataF(std::vector<float>&& data, const int pointCount)
{
    if (!validateVectorDataArguments(data, pointCount)) {
        return;
    }
    updateDataRanges(data, pointCount);
    applyNewData(std::move(data), pointCount);
}

void LineCurve::setDataFNoRange(std::vector<float>&& data, const int pointCount)
{
    if (!validateVectorDataArguments(data, pointCount)) {
        return;
    }
    applyNewData(std::move(data), pointCount);
}

void LineCurve::setDataFNoRange(const float* xyInterleaved, const int pointCount)
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

    copyRawData(xyInterleaved, pointCount);
    pointCount_ = pointCount;
    refreshVertexCacheForDataChange();
    rebuildChunks();
    dataChanged_ = true;
    update();
}

void LineCurve::setDataFNoRangeWithCache(std::vector<float>&& data, const int pointCount, std::vector<char>&& vertexCache)
{
    if (!validateVectorDataArguments(data, pointCount)) {
        return;
    }
    if (transition_ && transition_->enabled()) {
        applyNewData(std::move(data), pointCount);
        return;
    }

    pointCount_ = pointCount;
    data_ = std::move(data);
    installVertexCache(std::move(vertexCache));
    dataChanged_ = true;
    chunksValid_ = false;
    update();
}

void LineCurve::setDataFNoRangeWithCache(const float* xyInterleaved, const int pointCount, std::vector<char>&& vertexCache)
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

    copyRawData(xyInterleaved, pointCount);
    pointCount_ = pointCount;
    installVertexCache(std::move(vertexCache));
    dataChanged_ = true;
    chunksValid_ = false;
    update();
}

void LineCurve::postData(std::vector<float>&& xyInterleaved, const int pointCount)
{
    QMetaObject::invokeMethod(
        this, [this, data = std::move(xyInterleaved), pointCount]() mutable { setDataF(std::move(data), pointCount); }, Qt::QueuedConnection);
}

static QRectF resolvePlotRect(LineCurve* curve)
{
    if (const auto fp = qobject_cast<QAccelPlot*>(curve->parentItem())) {
        const auto pr = fp->plotRect();
        if (curve->position() != pr.topLeft() || QSizeF(curve->width(), curve->height()) != pr.size()) {
            curve->setPosition(pr.topLeft());
            curve->setSize(pr.size());
        }
        return pr;
    }
    return QRectF(0, 0, curve->width(), curve->height());
}

QSGNode* LineCurve::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData)

    if (!supportsCustomShaderRendering(window())) {
        static bool warned = false;
        if (!warned) {
            qCWarning(lcQAccelPlot) << "LineCurve custom rendering requires a hardware scene graph backend. Skipping updatePaintNode on the software backend.";
            warned = true;
        }
        delete oldNode;
        return nullptr;
    }

    const auto hasLine = lineStyle_ && lineStyle_->showLine();
    const auto hasPoints = markerShape_ != PointShape::None;
    if (!hasLine && !hasPoints) {
        delete oldNode;
        return nullptr;
    }
    const auto minPoints = hasLine ? 2 : 1;
    if (pointCount_ < minPoints) {
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

    const auto pr = resolvePlotRect(this);
    const auto domainMin = QVector2D(xAxis()->viewportMin(), yAxis()->viewportMin());
    const auto domainMax = QVector2D(xAxis()->viewportMax(), yAxis()->viewportMax());
    const auto viewportSize = QVector2D(pr.width(), pr.height());
    const auto gradientPayload = resolveGradientColorPayload();
    const auto gradientFillPayload = resolveGradientFillPayload();

    auto stillAnimating = false;
    if (dataChanged_ && transition_ && transition_->running()) {
        stillAnimating = transition_->advance(data_, pointCount_);
        // Transition mutates data_ each frame; mark chunks stale so the main thread
        // rebuilds them on the next contains() call with the current interpolated data.
        chunksValid_ = false;
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
        const auto lineParams = LineCurveRenderParams{window(), data_, pointCount_, dataChanged_, color_, hovered_, lineWidth_, domainMin, domainMax,
            viewportSize, xAxis(), yAxis(), xAxis()->logScale(), yAxis()->logScale(), antialiasingEnabled_, antialiasingFeather_, gradientPayload,
            gradientFillPayload, hasPoints ? nullptr : cache, lineStyle_};
        lineNode = lineRenderer_.paint(lineOldNode, lineParams);
    }

    if (hasPoints) {
        const auto shapeType = static_cast<int>(markerShape_) - 1;
        const auto pointParams = PointCurveRenderParams{data_, pointCount_, dataChanged_, color_, hovered_, markerSize_, domainMin, domainMax, viewportSize,
            xAxis()->logScale(), yAxis()->logScale(), antialiasingEnabled_, antialiasingFeather_, gradientPayload, hasLine ? nullptr : cache, shapeType};
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
        // Single-mode: discard container if it was previously dual
        if (oldNode && oldNode != lineNode && oldNode != pointNode) {
            delete oldNode;
        }
        node = hasLine ? lineNode : pointNode;
    }

    if (dataChanged_) {
        if (stillAnimating) {
            update();
        } else {
            dataChanged_ = false;
        }
    }

    return node;
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

    if (markerShape_ != PointShape::None) {
        return pointRenderer_.contains(point, CurveHitTestParams{data_, pointCount_, chunks_, xAxis(), yAxis(), w, h, markerSize_});
    }

    if (lineStyle_ && lineStyle_->showLine()) {
        constexpr static auto kLineHitRadiusPx = qreal{10.0}; // Hit-test radius in pixels for line-based curves.
        return lineRenderer_.contains(point, CurveHitTestParams{data_, pointCount_, chunks_, xAxis(), yAxis(), w, h, kLineHitRadiusPx});
    }

    return false;
}

void LineCurve::onLineStyleChanged()
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

    effect->setParent(curve);
    curve->effects_.append(effect);

    QObject::connect(effect, &LineCurveEffect::effectChanged, curve, [curve]() { curve->update(); });

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

    for (const auto effect : curve->effects_) {
        if (effect) {
            effect->deleteLater();
        }
    }
    curve->effects_.clear();
    curve->update();
}

GradientColorPayload LineCurve::resolveGradientColorPayload() const
{
    for (const auto effect : effects_) {
        if (!effect || !effect->enabled()) {
            continue;
        }

        if (const auto gradientEffect = qobject_cast<GradientStroke*>(effect)) {
            auto payload = gradientEffect->payload();
            if (payload.isValid()) {
                // Resolve range from data range when source is DataRange.
                // This makes gradient colors data-relative and invariant to pan/zoom.
                if (!payload.gradientValueMin.has_value() || !payload.gradientValueMax.has_value()) {
                    if (payload.direction == GradientDirection::Horizontal) {
                        payload.gradientValueMin = xAxis() ? static_cast<float>(xAxis()->dataMin()) : kFallbackDataMin;
                        payload.gradientValueMax = xAxis() ? static_cast<float>(xAxis()->dataMax()) : kFallbackDataMax;
                    } else {
                        payload.gradientValueMin = yAxis() ? static_cast<float>(yAxis()->dataMin()) : kFallbackDataMin;
                        payload.gradientValueMax = yAxis() ? static_cast<float>(yAxis()->dataMax()) : kFallbackDataMax;
                    }
                }
                return payload;
            }
        }
    }

    return {};
}

GradientFillPayload LineCurve::resolveGradientFillPayload() const
{
    for (auto* effect : effects_) {
        if (!effect || !effect->enabled()) {
            continue;
        }

        if (const auto gradientFill = qobject_cast<GradientFill*>(effect)) {
            auto payload = gradientFill->payload();
            if (payload.isValid()) {
                if (!payload.gradientValueMin.has_value() || !payload.gradientValueMax.has_value()) {
                    if (payload.direction == GradientDirection::Horizontal) {
                        payload.gradientValueMin = xAxis() ? static_cast<float>(xAxis()->dataMin()) : kFallbackDataMin;
                        payload.gradientValueMax = xAxis() ? static_cast<float>(xAxis()->dataMax()) : kFallbackDataMax;
                    } else {
                        payload.gradientValueMin = yAxis() ? static_cast<float>(yAxis()->dataMin()) : kFallbackDataMin;
                        payload.gradientValueMax = yAxis() ? static_cast<float>(yAxis()->dataMax()) : kFallbackDataMax;
                    }
                }
                return payload;
            }
        }
    }

    return {};
}

void LineCurve::updateDataRanges(const std::vector<float>& buf, const int count)
{
    if (buf.empty() || count == 0) {
        clearDataRanges();
        return;
    }

    auto xMin = std::numeric_limits<qreal>::max();
    auto xMax = std::numeric_limits<qreal>::lowest();
    auto yMin = std::numeric_limits<qreal>::max();
    auto yMax = std::numeric_limits<qreal>::lowest();

    for (int i = 0; i < count; ++i) {
        const auto x = qreal{buf[i * 2]};
        const auto y = qreal{buf[i * 2 + 1]};
        xMin = std::min(xMin, x);
        xMax = std::max(xMax, x);
        yMin = std::min(yMin, y);
        yMax = std::max(yMax, y);
    }

    setDataRanges(xMin, xMax, yMin, yMax);
}

void LineCurve::applyNewData(std::vector<float>&& newData, const int newPointCount)
{
    if (transition_ && transition_->enabled()) {
        transition_->start(data_, pointCount_, std::move(newData), newPointCount);

        // Size working buffers so updatePaintNode doesn't discard the node
        const auto maxCount = std::max(pointCount_, newPointCount);
        pointCount_ = maxCount;
        data_.resize(maxCount * 2);

        invalidateData(); // transition mutates data_ each frame; can't pre-build vertex cache
        update();
    } else {
        pointCount_ = newPointCount;
        data_ = std::move(newData);
        refreshVertexCacheForDataChange();

        // Pre-build chunk AABBs on the caller thread alongside the vertex cache so that
        // hover hit-tests never block on a lazy O(N) rebuild during live rendering.
        rebuildChunks();

        dataChanged_ = true;
        update();
    }
}

bool LineCurve::validateRawDataArguments(const float* xyInterleaved, const int pointCount) const
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
    if (pointCount < 0) {
        qCWarning(lcQAccelPlot) << "LineCurve data point count cannot be negative:" << pointCount;
        return false;
    }

    const auto expectedFloatCount = static_cast<std::size_t>(pointCount) * 2;
    if (data.size() != expectedFloatCount) {
        qCWarning(lcQAccelPlot) << "LineCurve received" << data.size() << "floats for" << pointCount << "points; expected" << expectedFloatCount;
        return false;
    }
    return true;
}

void LineCurve::copyRawData(const float* xyInterleaved, const int pointCount)
{
    const auto floatCount = static_cast<std::size_t>(pointCount) * 2;
    data_.resize(floatCount);
    if (floatCount > 0) {
        std::memcpy(data_.data(), xyInterleaved, floatCount * sizeof(float));
    }
}

void LineCurve::refreshVertexCacheForDataChange()
{
    const auto hasGradient = resolveGradientColorPayload().isValid();
    const auto hasFillGradient = resolveGradientFillPayload().isValid();
    const auto isDash = lineStyle_ && lineStyle_->showLine() && lineStyle_->dashParameters().enabled;
    const auto hasLine = lineStyle_ && lineStyle_->showLine();
    const auto hasPoints = markerShape_ != PointShape::None;

    if (hasGradient || hasFillGradient || isDash || (hasLine && hasPoints)) {
        vertexCache_.invalidate();
        return;
    }

    // Solid-line cache vertices contain only point indices and ribbon sides, so
    // they remain valid across same-sized data updates. Marker cache vertices
    // contain XY positions and must be rebuilt for every update.
    if (hasLine && vertexCache_.isReusableForDataChange(LineCurveVertexCache::Layout::Line, pointCount_)) {
        return;
    }

    rebuildVertexCache();
}

std::size_t LineCurve::expectedVertexCacheSize() const
{
    if (pointCount_ <= 0) {
        return 0;
    }
    const auto hasLine = lineStyle_ && lineStyle_->showLine();
    const auto hasPoints = markerShape_ != PointShape::None;
    const auto isDash = hasLine && lineStyle_->dashParameters().enabled;
    const auto hasGradient = resolveGradientColorPayload().isValid() || resolveGradientFillPayload().isValid();
    if (hasGradient || isDash || (hasLine && hasPoints)) {
        return 0;
    }
    if (hasLine && pointCount_ >= 2) {
        return static_cast<std::size_t>(pointCount_) * 2 * sizeof(LineVertex);
    }
    if (hasPoints) {
        return static_cast<std::size_t>(pointCount_) * 6 * sizeof(PointVertex);
    }
    return 0;
}

void LineCurve::installVertexCache(std::vector<char>&& vertexCache)
{
    const auto expectedSize = expectedVertexCacheSize();
    const auto actualSize = vertexCache.size();
    if (!vertexCache.empty() && actualSize != expectedSize) {
        qCWarning(lcQAccelPlot) << "Ignoring vertex cache with" << actualSize << "bytes; expected" << expectedSize;
    }

    const auto layout = (lineStyle_ && lineStyle_->showLine()) ? LineCurveVertexCache::Layout::Line : LineCurveVertexCache::Layout::Points;
    (void)vertexCache_.install(std::move(vertexCache), layout, pointCount_, expectedSize);
}

void LineCurve::rebuildVertexCache()
{
    const auto hasGradient = resolveGradientColorPayload().isValid();
    const auto hasFillGradient = resolveGradientFillPayload().isValid();
    const auto isTransitioning = transition_ && transition_->running();
    if (hasGradient || hasFillGradient || isTransitioning || pointCount_ == 0) {
        vertexCache_.invalidate();
        return;
    }
    const auto isDash = lineStyle_ && lineStyle_->showLine() && lineStyle_->dashParameters().enabled;
    if (lineStyle_ && lineStyle_->showLine() && !isDash && pointCount_ >= 2) {
        vertexCache_.rebuild(
            LineCurveVertexCache::Layout::Line, pointCount_, [this](std::vector<char>& bytes) { lineRenderer_.buildVertexCache(data_, pointCount_, bytes); });
    } else if (markerShape_ != PointShape::None && pointCount_ >= 1) {
        vertexCache_.rebuild(LineCurveVertexCache::Layout::Points, pointCount_,
            [this](std::vector<char>& bytes) { pointRenderer_.buildVertexCache(data_, pointCount_, bytes); });
    } else {
        vertexCache_.invalidate();
    }
}

void LineCurve::rebuildChunks() const
{
    chunks_.clear();
    if (pointCount_ == 0) {
        chunksValid_ = true;
        return;
    }

    constexpr static auto kChunkSize = int{512};
    chunks_.reserve(static_cast<std::size_t>((pointCount_ + kChunkSize - 1) / kChunkSize));
    for (auto start = int{0}; start < pointCount_; start += kChunkSize) {
        const auto count = std::min(kChunkSize, pointCount_ - start);
        // Extend AABB one point beyond the chunk boundary on each side so that
        // line segments bridging adjacent chunks are fully covered.
        const auto aabbBegin = std::max(0, start - 1);
        const auto aabbEnd = std::min(start + count, pointCount_ - 1); // inclusive
        auto chunk = CurveChunk{start, count, std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max(),
            std::numeric_limits<float>::lowest()};
        for (auto i = aabbBegin; i <= aabbEnd; ++i) {
            chunk.minX = std::min(chunk.minX, data_[static_cast<std::size_t>(i * 2)]);
            chunk.maxX = std::max(chunk.maxX, data_[static_cast<std::size_t>(i * 2)]);
            chunk.minY = std::min(chunk.minY, data_[static_cast<std::size_t>(i * 2 + 1)]);
            chunk.maxY = std::max(chunk.maxY, data_[static_cast<std::size_t>(i * 2 + 1)]);
        }
        chunks_.push_back(chunk);
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

} // namespace QAccelPlot
