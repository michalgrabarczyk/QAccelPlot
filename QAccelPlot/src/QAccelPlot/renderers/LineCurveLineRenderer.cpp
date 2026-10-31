//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/renderers/LineCurveLineRenderer.hpp"

#include "QAccelPlot/effects/internal/GradientCoordinates.hpp"
#include "QAccelPlot/materials/DataTextureMaterial.hpp"
#include "QAccelPlot/materials/GradientFillMaterial.hpp"
#include "QAccelPlot/materials/GradientLineMaterial.hpp"
#include "QAccelPlot/materials/LineMaterial.hpp"
#include "QAccelPlot/renderers/CurveRendererParams.hpp"
#include "QAccelPlot/renderers/LineStroke.hpp"
#include "QAccelPlot/series/LineCurveGapFilter.hpp"
#include "QAccelPlot/series/internal/StepPath.hpp"

#include <QSGGeometry>
#include <QSGGeometryNode>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace QAccelPlot {

namespace {
constexpr int kMaxFillVertices = 60000;

struct GradientFillVertex {
    float x;
    float y;
    float gradientCoordinate;
};

float normalizedGradientFillValue(const GradientFillPayload& gradientPayload, const qreal dataX, const qreal dataY)
{
    const auto valueMin = gradientPayload.gradientValueMin.value_or(0.0);
    const auto valueMax = gradientPayload.gradientValueMax.value_or(1.0);
    const auto value = (gradientPayload.direction == Qt::Horizontal) ? dataX : dataY;
    // Keep the coordinate unbounded while the GPU interpolates it across the fill.
    // Clamping here would stretch the complete palette between the baseline and
    // curve whenever an endpoint lies outside the configured gradient range.
    // The fragment shader clamps the interpolated coordinate immediately before
    // sampling, preserving endpoint-color saturation beyond the configured range.
    return Internal::unboundedGradientCoordinate(gradientPayload.direction, value, valueMin, valueMax);
}

QSGGeometryNode* createLineNode(const int vertexCount, const bool gradientStroke)
{
    return LineStroke::createNode(vertexCount, gradientStroke ? static_cast<QSGMaterial*>(new GradientLineMaterial) : new LineMaterial);
}

QSGGeometryNode* createFillNode(const int vertexCount)
{
    auto* node = new QSGGeometryNode;

    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(0, 2, QSGGeometry::FloatType, true),
        QSGGeometry::Attribute::create(1, 1, QSGGeometry::FloatType),
    };
    static const QSGGeometry::AttributeSet attributeSet = {2, static_cast<int>(sizeof(GradientFillVertex)), attributes};
    auto* geometry = new QSGGeometry(attributeSet, vertexCount);
    geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);

    node->setMaterial(new GradientFillMaterial);
    node->setFlag(QSGNode::OwnsMaterial);

    return node;
}

// Vertex count of a gap-aware gradient fill: one triangle strip per sampled run, joined by two
// degenerate vertices so the strip never spans a gap.
int fillVertexCount(const FillSamples& samples)
{
    auto vertexCount = int{0};
    auto drawnRuns = int{0};
    for (const auto runPoints : samples.pointCounts) {
        if (runPoints > 0) {
            vertexCount += runPoints * 2;
            ++drawnRuns;
        }
    }
    // Two restart vertices (the previous run's last vertex and the next run's first vertex) per join.
    return vertexCount + std::max(0, drawnRuns - 1) * 2;
}

void assembleFillVertices(QSGGeometry* geometry, const FillSamples& samples, Axis* xAxis, Axis* yAxis, const qreal width, const qreal height,
    const GradientFillPayload& gradientFillPayload)
{
    auto* vertices = static_cast<GradientFillVertex*>(geometry->vertexData());
    const auto baselineData = (gradientFillPayload.baseline == GradientFillBaseline::AxisMinimum) ? yAxis->viewportMin() : gradientFillPayload.baselineValue;
    const auto baselinePixel = static_cast<float>(yAxis->coordToPixel(baselineData, height));

    auto written = int{0};
    auto nextPoint = std::size_t{0};
    for (const auto runPoints : samples.pointCounts) {
        if (runPoints <= 0) {
            continue;
        }
        const auto runStartVertex = written;
        if (written > 0) {
            // Restart the strip: repeat the previous vertex now and this run's first vertex below.
            vertices[written] = vertices[written - 1];
            written += 2;
        }
        for (auto point = int{0}; point < runPoints; ++point) {
            const auto& dataPoint = samples.points[nextPoint++];
            const auto px = dataPoint.x();
            const auto py = dataPoint.y();
            const auto xPixel = static_cast<float>(xAxis->coordToPixel(px, width));
            const auto yPixel = static_cast<float>(yAxis->coordToPixel(py, height));

            const auto normalizedCurve = normalizedGradientFillValue(gradientFillPayload, px, py);
            const auto normalizedBaseline = normalizedGradientFillValue(gradientFillPayload, px, baselineData);
            vertices[written++] = {xPixel, baselinePixel, normalizedBaseline};
            vertices[written++] = {xPixel, yPixel, normalizedCurve};
        }
        if (runStartVertex > 0) {
            vertices[runStartVertex + 1] = vertices[runStartVertex + 2];
        }
    }
}

// Appends the fill outline of \a run to \a points: the data points \a sampled and, with a step mode, the step
// corners between them and those of the steps reaching into the gaps on either side of the run.
void appendRunOutline(const CurveDataView& data, const int pointCount, const DataPointRun& run, const std::vector<int>& sampled, const StepMode mode,
    const bool logScaleX, const bool logScaleY, std::vector<QPointF>& points)
{
    const auto cornerCount = Internal::stepCornerCount(mode);
    const auto appendCorners = [&](const int from, const int to) {
        for (auto corner = 1; corner < cornerCount; ++corner) {
            if (const auto point = Internal::stepCorner(data, from, to, corner, mode, logScaleX, logScaleY)) {
                points.push_back(*point);
            }
        }
    };
    if (run.start > 0) {
        appendCorners(run.start - 1, run.start);
    }
    for (auto i = std::size_t{0}; i < sampled.size(); ++i) {
        const auto index = sampled[i];
        points.emplace_back(data.x(index), data.y(index));
        const auto next = i + 1 < sampled.size() ? sampled[i + 1] : index + 1;
        if (next < pointCount) {
            appendCorners(index, next);
        }
    }
}

// Returns the squared pixel distance from \a point to the segment from \a from to \a to.
qreal squaredDistanceToSegment(const QPointF& point, const QPointF& from, const QPointF& to)
{
    const auto lengthSquared = QPointF::dotProduct(to - from, to - from);
    const auto t = (lengthSquared == 0.0) ? 0.0 : std::clamp(QPointF::dotProduct(point - from, to - from) / lengthSquared, 0.0, 1.0);
    const auto projection = from + t * (to - from);
    return QPointF::dotProduct(point - projection, point - projection);
}

// Returns true when \a point is closer than \a thresholdSquared to the drawn line from data point \a index to the next one.
bool segmentsFromPointHit(const QPointF& point, const CurveHitTestParams& params, const int index, const qreal thresholdSquared)
{
    const auto pixelAt = [&params](const int pointIndex, const int corner) -> std::optional<QPointF> {
        const auto dataPoint = Internal::linePathPoint(
            params.data, params.pointCount, pointIndex, corner, params.stepMode, params.nonPositiveXInvalid, params.nonPositiveYInvalid);
        if (!dataPoint) {
            return std::nullopt;
        }
        return QPointF{params.xAxis->coordToPixel(dataPoint->x(), params.width), params.yAxis->coordToPixel(dataPoint->y(), params.height)};
    };
    const auto cornerCount = Internal::stepCornerCount(params.stepMode);
    auto from = pixelAt(index, 0);
    for (auto corner = 1; corner <= cornerCount; ++corner) {
        const auto to = corner < cornerCount ? pixelAt(index, corner) : pixelAt(index + 1, 0);
        if (from && to && squaredDistanceToSegment(point, *from, *to) < thresholdSquared) {
            return true;
        }
        from = to;
    }
    return false;
}

QSGNode* createLineRootNode(const int strokeVertexCount, const bool gradientStroke)
{
    auto* rootNode = new QSGNode;
    rootNode->appendChildNode(createFillNode(0));
    rootNode->appendChildNode(createLineNode(strokeVertexCount, gradientStroke));
    return rootNode;
}

QSGNode* ensureLineRootNode(QSGNode* oldNode, const int strokeVertexCount, const bool gradientStroke, bool& recreated)
{
    const auto isExpectedRoot = [&]() {
        if (!oldNode || oldNode->type() != QSGNode::BasicNodeType || oldNode->childCount() != 2) {
            return false;
        }

        auto* fillNode = oldNode->firstChild();
        auto* lineNode = fillNode ? fillNode->nextSibling() : nullptr;
        if (!fillNode || !lineNode) {
            return false;
        }

        if (fillNode->type() != QSGNode::GeometryNodeType || lineNode->type() != QSGNode::GeometryNodeType) {
            return false;
        }

        const auto* fillGeometryNode = static_cast<QSGGeometryNode*>(fillNode);
        const auto* lineGeometryNode = static_cast<QSGGeometryNode*>(lineNode);
        const auto hasGradientFillMaterial = dynamic_cast<const GradientFillMaterial*>(fillGeometryNode->material()) != nullptr;
        const auto hasLineMaterial = dynamic_cast<const LineMaterial*>(lineGeometryNode->material()) != nullptr;
        return hasGradientFillMaterial && hasLineMaterial;
    }();

    if (isExpectedRoot) {
        auto* lineNode = static_cast<QSGGeometryNode*>(oldNode->lastChild());
        const auto hasGradient = dynamic_cast<const GradientLineMaterial*>(lineNode->material()) != nullptr;
        recreated = hasGradient != gradientStroke;
        if (recreated) {
            lineNode->setMaterial(gradientStroke ? static_cast<QSGMaterial*>(new GradientLineMaterial) : static_cast<QSGMaterial*>(new LineMaterial));
        }
        return oldNode;
    }

    delete oldNode;
    recreated = true;
    return createLineRootNode(strokeVertexCount, gradientStroke);
}

} // namespace

std::size_t LineCurveLineRenderer::vertexCacheBytesPerPoint(const StepMode stepMode)
{
    return static_cast<std::size_t>(Internal::stepCornerCount(stepMode)) * 2 * sizeof(LineVertex);
}

void LineCurveLineRenderer::writeVertexCache(std::vector<char>& cache, const int firstPoint, const int pointCount, const StepMode stepMode)
{
    // Neutral RGBA: the shader uses the material color uniform (useVertexColor == 0) for
    // non-gradient rendering, so per-vertex color is irrelevant in that path.
    LineStroke::writeVertices(reinterpret_cast<LineVertex*>(cache.data()), pointCount, Internal::stepCornerCount(stepMode), Qt::white, {}, firstPoint);
}

bool LineCurveLineRenderer::contains(const QPointF& point, const CurveHitTestParams& params) const
{
    const auto thresholdSquared = params.hitThreshold * params.hitThreshold;

    for (const auto& chunk : params.chunks) {
        if (isEmptyChunk(chunk)) {
            continue;
        }
        // Map chunk data-space AABB to screen space and reject the chunk when the
        // mouse is farther than hitThreshold from it. coordToPixel is monotone for
        // linear scale and for log scale, so taking min/max of the two mapped extremes
        // always produces the correct screen-space interval.
        const auto screenLeft = params.xAxis->coordToPixel(chunk.minX, params.width);
        const auto screenRight = params.xAxis->coordToPixel(chunk.maxX, params.width);
        const auto screenTop = params.yAxis->coordToPixel(chunk.minY, params.height);
        const auto screenBottom = params.yAxis->coordToPixel(chunk.maxY, params.height);
        const auto screenMinX = std::min(screenLeft, screenRight);
        const auto screenMaxX = std::max(screenLeft, screenRight);
        const auto screenMinY = std::min(screenTop, screenBottom);
        const auto screenMaxY = std::max(screenTop, screenBottom);
        if (point.x() < screenMinX - params.hitThreshold || point.x() > screenMaxX + params.hitThreshold || point.y() < screenMinY - params.hitThreshold
            || point.y() > screenMaxY + params.hitThreshold) {
            continue;
        }

        // Inner loop: test the line from every data point in this chunk, including the
        // bridging line to the following chunk (segEnd is clamped to pointCount-1). Step
        // corners lie within the bounding box of the two data points they join.
        const auto segStart = std::max(0, chunk.start - 1);
        const auto segEnd = std::min(chunk.start + chunk.count, params.pointCount - 1);
        for (auto index = segStart; index < segEnd; ++index) {
            if (segmentsFromPointHit(point, params, index, thresholdSquared)) {
                return true;
            }
        }
    }

    return false;
}

QSGNode* LineCurveLineRenderer::paint(QSGNode* oldNode, const LineCurveRenderParams& params) const
{
    const auto vertexCount = params.pointCount * 2 * Internal::stepCornerCount(params.stepMode);
    const auto gradientStroke = params.gradientPayload.isValid();
    auto rootRecreated = false;
    auto* rootNode = ensureLineRootNode(oldNode, vertexCount, gradientStroke, rootRecreated);
    auto* fillNode = static_cast<QSGGeometryNode*>(rootNode->firstChild());
    auto* node = static_cast<QSGGeometryNode*>(fillNode->nextSibling());

    updateFillGeometry(fillNode, params);

    // Detect whether the vertex count changed BEFORE reallocating, so we know
    // whether the vertex buffer content (sequential indices + ±1 sides) needs
    // to be re-uploaded. The vertex data is deterministic from pointCount alone.
    const auto geometryCountChanged = node->geometry()->vertexCount() != vertexCount;

    // Only reallocate when the vertex count actually changes — avoids GPU buffer
    // churn (and the associated CPU stall) every frame during steady-state rendering.
    if (geometryCountChanged) {
        node->geometry()->allocate(vertexCount);
    }

    const auto effectiveColor = params.hovered ? params.color.lighter(150) : params.color;
    const auto useVertexColor = false;
    const auto dashParams = params.lineStyle ? params.lineStyle->dashParameters() : DashParameters{};

    auto* material = static_cast<LineMaterial*>(node->material());
    updateLineMaterial(material, params, effectiveColor, useVertexColor, dashParams);

    if (params.dataChanged || rootRecreated) {
        material->uploadTexture(params.window, params.data.data(), params.pointCount * 2);
    }

    // The vertex buffer stores {id, side, rgba, arcLength}. For solid lines the
    // data is deterministic from pointCount alone. For dash lines the arcLengths
    // are screen-space and must be recomputed whenever axis range or viewport changes.
    const auto arcLengths = computeArcLengths(params, dashParams);
    const auto needVertexUpload = geometryCountChanged || rootRecreated || useVertexColor || dashParams.enabled;
    if (needVertexUpload) {
        updateLineVertices(node->geometry(), params, useVertexColor, effectiveColor, arcLengths);
        node->markDirty(QSGNode::DirtyGeometry);
    }

    // Uniforms can change without data updates (lineWidth, hover, axis range, AA controls).
    node->markDirty(QSGNode::DirtyMaterial);

    return rootNode;
}

std::shared_ptr<DataTexture> LineCurveLineRenderer::dataTexture(QSGNode* node)
{
    auto* lineNode = node ? static_cast<QSGGeometryNode*>(node->lastChild()) : nullptr;
    return lineNode ? static_cast<LineMaterial*>(lineNode->material())->dataTexture : nullptr;
}

void LineCurveLineRenderer::updateFillGeometry(QSGGeometryNode* fillNode, const LineCurveRenderParams& params) const
{
    const auto fillEnabled = params.gradientFillPayload.isValid() && params.xAxis && params.yAxis;
    const auto* samples = fillEnabled ? &fillSamples(params) : nullptr;
    const auto vertexCount = samples ? fillVertexCount(*samples) : 0;
    if (fillNode->geometry()->vertexCount() != vertexCount) {
        fillNode->geometry()->allocate(vertexCount);
    }
    if (samples && vertexCount > 0) {
        assembleFillVertices(
            fillNode->geometry(), *samples, params.xAxis, params.yAxis, params.viewportSize.x(), params.viewportSize.y(), params.gradientFillPayload);
        fillNode->markDirty(QSGNode::DirtyGeometry);
        auto* material = static_cast<GradientFillMaterial*>(fillNode->material());
        material->opacity = params.gradientFillPayload.opacity;
        material->gradientTexture.upload(params.window, params.gradientFillPayload.stops);
        fillNode->markDirty(QSGNode::DirtyMaterial);
    }
}

const FillSamples& LineCurveLineRenderer::fillSamples(const LineCurveRenderParams& params) const
{
    // The fill is rebuilt every frame (it is assembled in pixel space), but its data points
    // only change with the data, so scanning every data point is limited to data updates.
    auto& cache = fillSampleCache_;
    const auto* dataIdentity
        = params.sourceData.doubleData ? static_cast<const void*>(params.sourceData.doubleData) : static_cast<const void*>(params.sourceData.floatData);
    const auto stale = params.dataChanged || cache.data != dataIdentity || cache.pointCount != params.pointCount || cache.logScaleX != params.logScaleX
        || cache.logScaleY != params.logScaleY || cache.stepMode != params.stepMode;
    if (stale) {
        rebuildFillSamples(params, cache.samples);
        cache.data = dataIdentity;
        cache.pointCount = params.pointCount;
        cache.logScaleX = params.logScaleX;
        cache.logScaleY = params.logScaleY;
        cache.stepMode = params.stepMode;
    }
    return cache.samples;
}

void LineCurveLineRenderer::rebuildFillSamples(const LineCurveRenderParams& params, FillSamples& samples)
{
    const auto runs = LineCurveGapFilter::findValidRuns(params.sourceData, params.pointCount, params.logScaleX, params.logScaleY);
    const auto stepped = params.stepMode != StepMode::None;
    const auto sampledCounts = LineCurveGapFilter::planRunSampling(runs, kMaxFillVertices / (2 * Internal::stepCornerCount(params.stepMode)));
    samples.pointCounts.clear();
    samples.points.clear();
    auto sampled = std::vector<int>{};
    for (auto run = std::size_t{0}; run < runs.size(); ++run) {
        // A lone data point still fills the step it holds.
        const auto sampledCount = stepped && runs[run].count == 1 ? 1 : sampledCounts[run];
        const auto runStart = samples.points.size();
        if (sampledCount > 0) {
            sampled.clear();
            LineCurveGapFilter::appendEnvelopeSamples(params.sourceData, runs[run], sampledCount, sampled);
            appendRunOutline(params.sourceData, params.pointCount, runs[run], sampled, params.stepMode, params.logScaleX, params.logScaleY, samples.points);
        }
        auto runPoints = static_cast<int>(samples.points.size() - runStart);
        if (runPoints < 2) {
            samples.points.resize(runStart);
            runPoints = 0;
        }
        samples.pointCounts.push_back(runPoints);
    }
}

void LineCurveLineRenderer::updateLineMaterial(LineMaterial* material, const LineCurveRenderParams& params, const QColor& effectiveColor,
    const bool useVertexColor, const DashParameters& dashParams) const
{
    LineStroke::applyUniforms(*material,
        LineStroke::Uniforms{effectiveColor, params.lineWidth, params.domainMin, params.domainMax, params.viewportSize, params.logScaleX, params.logScaleY,
            params.pointCount, params.antialiasingEnabled, params.antialiasingFeather, dashParams});
    material->useVertexColor = useVertexColor ? 1.0f : 0.0f;
    material->stepMode = static_cast<float>(params.stepMode);
    if (auto* gradientMaterial = dynamic_cast<GradientLineMaterial*>(material)) {
        gradientMaterial->gradientDirection = params.gradientPayload.direction == Qt::Vertical ? 1.0f : 0.0f;
        gradientMaterial->gradientValueMin = static_cast<float>(params.gradientPayload.gradientValueMin.value_or(0.0));
        gradientMaterial->gradientValueMax = static_cast<float>(params.gradientPayload.gradientValueMax.value_or(1.0));
        gradientMaterial->gradientTexture.upload(params.window, params.gradientPayload.stops);
    }
}

std::vector<float> LineCurveLineRenderer::computeArcLengths(const LineCurveRenderParams& params, const DashParameters& dashParams) const
{
    if (!dashParams.enabled || !params.xAxis || !params.yAxis || params.pointCount <= 0) {
        return {};
    }
    const auto cornerCount = Internal::stepCornerCount(params.stepMode);
    return LineStroke::arcLengths(params.pointCount * cornerCount, [&params, cornerCount](const int pathIndex) -> std::optional<QPointF> {
        const auto point = Internal::linePathPoint(
            params.sourceData, params.pointCount, pathIndex / cornerCount, pathIndex % cornerCount, params.stepMode, params.logScaleX, params.logScaleY);
        if (!point) {
            return std::nullopt;
        }
        return QPointF{params.xAxis->coordToPixel(point->x(), params.viewportSize.x()), params.yAxis->coordToPixel(point->y(), params.viewportSize.y())};
    });
}

void LineCurveLineRenderer::updateLineVertices(QSGGeometry* geometry, const LineCurveRenderParams& params, const bool useVertexColor,
    const QColor& effectiveColor, const std::vector<float>& arcLengths) const
{
    const auto expectedCacheBytes = static_cast<std::size_t>(geometry->vertexCount()) * sizeof(LineVertex);
    if (params.vertexCache && params.vertexCache->size() == expectedCacheBytes && !useVertexColor) {
        // Fast path: memcpy the pre-built cache (main-thread work) to the GPU buffer.
        // A single large memcpy is significantly more efficient than the equivalent
        // per-struct loop when writing to GPU-mapped (write-combining) memory.
        std::memcpy(geometry->vertexData(), params.vertexCache->data(), expectedCacheBytes);
    } else {
        // Fallback: assemble on the render thread (dash lines or no cache).
        LineStroke::writeVertices(
            static_cast<LineVertex*>(geometry->vertexData()), params.pointCount, Internal::stepCornerCount(params.stepMode), effectiveColor, arcLengths);
    }
}

} // namespace QAccelPlot
