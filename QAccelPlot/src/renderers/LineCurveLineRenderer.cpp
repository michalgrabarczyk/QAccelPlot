//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "renderers/LineCurveLineRenderer.hpp"

#include "effects/GradientCoordinateUtils.hpp"
#include "materials/DataTextureMaterial.hpp"
#include "materials/GradientFillMaterial.hpp"
#include "materials/GradientLineMaterial.hpp"
#include "materials/LineMaterial.hpp"
#include "renderers/CurveRendererParams.hpp"

#include <QSGGeometry>
#include <QSGGeometryNode>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace QAccelPlot {

namespace {
constexpr int kMaxFillVertices = 60000;

// Opaque-white byte used to initialise per-vertex RGBA when the shader ignores vertex color.
constexpr auto kMaxColorChannelByte = static_cast<unsigned char>(255);
// Ribbon side values: each data point generates two vertices extruded to opposite sides of the line.
constexpr auto kSidePositive = float{1.0f};
constexpr auto kSideNegative = float{-1.0f};

struct GradientFillVertex {
    float x;
    float y;
    float gradientCoordinate;
};

float normalizedGradientFillValue(const GradientFillPayload& gradientPayload, const float dataX, const float dataY)
{
    const auto valueMin = *gradientPayload.gradientValueMin;
    const auto valueMax = *gradientPayload.gradientValueMax;
    const auto value = (gradientPayload.direction == GradientDirection::Horizontal) ? dataX : dataY;
    // Keep the coordinate unbounded while the GPU interpolates it across the fill.
    // Clamping here would stretch the complete palette between the baseline and
    // curve whenever an endpoint lies outside the configured gradient range.
    // The fragment shader clamps the interpolated coordinate immediately before
    // sampling, preserving endpoint-color saturation beyond the configured range.
    return unboundedGradientCoordinate(gradientPayload.direction, value, valueMin, valueMax);
}

QSGGeometryNode* createLineNode(const int vertexCount, const bool gradientStroke)
{
    auto* node = new QSGGeometryNode;

    static constexpr int kAttrLocationId{0};        // layout(location = 0): point index
    static constexpr int kAttrLocationSide{1};      // layout(location = 1): ribbon side (+1 / -1)
    static constexpr int kAttrLocationColor{2};     // layout(location = 2): per-vertex RGBA color
    static constexpr int kAttrLocationArcLength{3}; // layout(location = 3): cumulative arc length
    static constexpr int kComponentsScalar{1};      // 1 float
    static constexpr int kComponentsColor{4};       // 4 bytes: RGBA
    static constexpr int kLineAttributeCount{4};    // total number of vertex attributes

    // Line-specific attribute set: {float id, float side, uchar4 rgba, float arcLength} = 16 bytes.
    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(kAttrLocationId, kComponentsScalar, QSGGeometry::FloatType),
        QSGGeometry::Attribute::create(kAttrLocationSide, kComponentsScalar, QSGGeometry::FloatType),
        QSGGeometry::Attribute::createWithAttributeType(kAttrLocationColor, kComponentsColor, QSGGeometry::UnsignedByteType, QSGGeometry::ColorAttribute),
        QSGGeometry::Attribute::create(kAttrLocationArcLength, kComponentsScalar, QSGGeometry::FloatType),
    };
    static const QSGGeometry::AttributeSet lineAttrSet = {kLineAttributeCount, static_cast<int>(sizeof(LineVertex)), attributes};

    auto* geometry = new QSGGeometry(lineAttrSet, vertexCount);
    geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);

    node->setMaterial(gradientStroke ? static_cast<QSGMaterial*>(new GradientLineMaterial) : static_cast<QSGMaterial*>(new LineMaterial));
    node->setFlag(QSGNode::OwnsMaterial);

    return node;
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

void assembleFillVertices(QSGGeometry* geometry, const std::vector<float>& data, const int pointCount, const int sampledPointCount, Axis* xAxis, Axis* yAxis,
    const qreal width, const qreal height, const GradientFillPayload& gradientFillPayload)
{
    auto* vertices = static_cast<GradientFillVertex*>(geometry->vertexData());
    const auto* src = data.data();
    const auto baselineData
        = (gradientFillPayload.baseline == GradientFillBaseline::AxisMinimum) ? static_cast<float>(yAxis->viewportMin()) : gradientFillPayload.baselineValue;
    const auto baselinePixel = static_cast<float>(yAxis->coordToPixel(baselineData, height));

    for (int index = 0; index < sampledPointCount; ++index) {
        auto sourceIndex = index;
        if (sampledPointCount < pointCount && sampledPointCount > 1) {
            sourceIndex = static_cast<int>((static_cast<long long>(index) * (pointCount - 1)) / (sampledPointCount - 1));
        }

        const auto px = src[sourceIndex * 2];
        const auto py = src[sourceIndex * 2 + 1];
        const auto xPixel = static_cast<float>(xAxis->coordToPixel(px, width));
        const auto yPixel = static_cast<float>(yAxis->coordToPixel(py, height));

        const auto normalizedCurve = normalizedGradientFillValue(gradientFillPayload, px, py);
        const auto normalizedBaseline = normalizedGradientFillValue(gradientFillPayload, px, baselineData);
        vertices[index * 2] = {xPixel, baselinePixel, normalizedBaseline};
        vertices[index * 2 + 1] = {xPixel, yPixel, normalizedCurve};
    }
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
        const auto hasGradientLineMaterial = dynamic_cast<const GradientLineMaterial*>(lineGeometryNode->material()) != nullptr;
        return hasGradientFillMaterial && hasGradientLineMaterial == gradientStroke;
    }();

    if (isExpectedRoot) {
        recreated = false;
        return oldNode;
    }

    delete oldNode;
    recreated = true;
    return createLineRootNode(strokeVertexCount, gradientStroke);
}

void assembleVertices(
    QSGGeometry* geometry, const std::vector<float>& data, const int pointCount, const QColor& effectiveColor, const std::vector<float>& arcLengths)
{
    auto* vertices = static_cast<LineVertex*>(geometry->vertexData());
    const auto* src = data.data();

    // Hoist base color conversion — avoids 4 QColor::xF() calls per vertex in the common no-gradient path.
    const auto baseR = static_cast<unsigned char>(effectiveColor.red());
    const auto baseG = static_cast<unsigned char>(effectiveColor.green());
    const auto baseB = static_cast<unsigned char>(effectiveColor.blue());
    const auto baseA = static_cast<unsigned char>(effectiveColor.alpha());

    for (int index = 0; index < pointCount; ++index) {
        const auto px = src[index * 2];
        const auto py = src[index * 2 + 1];
        const auto prevX = (index == 0) ? px : src[(index - 1) * 2];
        const auto prevY = (index == 0) ? py : src[(index - 1) * 2 + 1];
        const auto nextX = (index == pointCount - 1) ? px : src[(index + 1) * 2];
        const auto nextY = (index == pointCount - 1) ? py : src[(index + 1) * 2 + 1];

        vertices[index * 2] = {static_cast<float>(index), kSidePositive, baseR, baseG, baseB, baseA, arcLengths.empty() ? 0.0f : arcLengths[index]};
        vertices[index * 2 + 1] = {static_cast<float>(index), kSideNegative, baseR, baseG, baseB, baseA, arcLengths.empty() ? 0.0f : arcLengths[index]};
    }
}

} // namespace

void LineCurveLineRenderer::buildVertexCache(const std::vector<float>& data, const int pointCount, std::vector<char>& cache) const
{
    const auto byteSize = static_cast<std::size_t>(pointCount) * 2 * sizeof(LineVertex);
    cache.resize(byteSize);
    auto* vertices = reinterpret_cast<LineVertex*>(cache.data());
    const auto* src = data.data();

    // Writes neutral RGBA — the shader uses the material color uniform (useVertexColor==0)
    // for non-gradient rendering, so per-vertex color is irrelevant in that path.
    for (int index = 0; index < pointCount; ++index) {
        vertices[index * 2]
            = {static_cast<float>(index), kSidePositive, kMaxColorChannelByte, kMaxColorChannelByte, kMaxColorChannelByte, kMaxColorChannelByte, 0.0f};
        vertices[index * 2 + 1]
            = {static_cast<float>(index), kSideNegative, kMaxColorChannelByte, kMaxColorChannelByte, kMaxColorChannelByte, kMaxColorChannelByte, 0.0f};
    }
}

bool LineCurveLineRenderer::contains(const QPointF& point, const CurveHitTestParams& params) const
{
    const auto thresholdSquared = params.hitThreshold * params.hitThreshold;

    for (const auto& chunk : params.chunks) {
        // Map chunk data-space AABB to screen space and reject the chunk when the
        // mouse is farther than hitThreshold from it. coordToPixel is monotone for
        // linear scale and for log scale, so taking min/max of the two mapped extremes
        // always produces the correct screen-space interval.
        const auto screenLeft = params.xAxis->coordToPixel(static_cast<qreal>(chunk.minX), params.width);
        const auto screenRight = params.xAxis->coordToPixel(static_cast<qreal>(chunk.maxX), params.width);
        const auto screenTop = params.yAxis->coordToPixel(static_cast<qreal>(chunk.minY), params.height);
        const auto screenBottom = params.yAxis->coordToPixel(static_cast<qreal>(chunk.maxY), params.height);
        const auto screenMinX = std::min(screenLeft, screenRight);
        const auto screenMaxX = std::max(screenLeft, screenRight);
        const auto screenMinY = std::min(screenTop, screenBottom);
        const auto screenMaxY = std::max(screenTop, screenBottom);
        if (point.x() < screenMinX - params.hitThreshold || point.x() > screenMaxX + params.hitThreshold || point.y() < screenMinY - params.hitThreshold
            || point.y() > screenMaxY + params.hitThreshold) {
            continue;
        }

        // Inner loop: test all segments whose first endpoint is in this chunk, including
        // the bridging segment to the following chunk (segEnd is clamped to pointCount-1).
        const auto segStart = std::max(0, chunk.start - 1);
        const auto segEnd = std::min(chunk.start + chunk.count, params.pointCount - 1);
        for (auto index = segStart; index < segEnd; ++index) {
            const auto p1 = QPointF{
                params.xAxis->coordToPixel(params.data[index * 2], params.width), params.yAxis->coordToPixel(params.data[index * 2 + 1], params.height)};
            const auto p2 = QPointF{params.xAxis->coordToPixel(params.data[(index + 1) * 2], params.width),
                params.yAxis->coordToPixel(params.data[(index + 1) * 2 + 1], params.height)};
            const auto lineLengthSquared = QPointF::dotProduct(p2 - p1, p2 - p1);
            const auto t = (lineLengthSquared == 0.0) ? 0.0 : std::max(0.0, std::min(1.0, QPointF::dotProduct(point - p1, p2 - p1) / lineLengthSquared));
            const auto projection = p1 + t * (p2 - p1);
            if (QPointF::dotProduct(point - projection, point - projection) < thresholdSquared) {
                return true;
            }
        }
    }

    return false;
}

QSGNode* LineCurveLineRenderer::paint(QSGNode* oldNode, const LineCurveRenderParams& params) const
{
    const auto vertexCount = params.pointCount * 2;
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
        material->uploadTexture(material->dataTexture, params.window, params.data.data(), params.pointCount * 2);
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

void LineCurveLineRenderer::updateFillGeometry(QSGGeometryNode* fillNode, const LineCurveRenderParams& params) const
{
    const auto fillEnabled = params.gradientFillPayload.isValid() && params.xAxis && params.yAxis;
    const auto maxFillPointCount = std::max(2, kMaxFillVertices / 2);
    const auto sampledFillPointCount = fillEnabled ? std::clamp(params.pointCount, 2, maxFillPointCount) : 0;
    const auto fillVertexCount = sampledFillPointCount * 2;
    if (fillNode->geometry()->vertexCount() != fillVertexCount) {
        fillNode->geometry()->allocate(fillVertexCount);
    }
    if (fillEnabled && fillVertexCount > 0) {
        assembleFillVertices(fillNode->geometry(), params.data, params.pointCount, sampledFillPointCount, params.xAxis, params.yAxis, params.viewportSize.x(),
            params.viewportSize.y(), params.gradientFillPayload);
        fillNode->markDirty(QSGNode::DirtyGeometry);
        auto* material = static_cast<GradientFillMaterial*>(fillNode->material());
        material->opacity = params.gradientFillPayload.opacity;
        material->gradientTexture.upload(params.window, params.gradientFillPayload.stops);
        fillNode->markDirty(QSGNode::DirtyMaterial);
    }
}

void LineCurveLineRenderer::updateLineMaterial(LineMaterial* material, const LineCurveRenderParams& params, const QColor& effectiveColor,
    const bool useVertexColor, const DashParameters& dashParams) const
{
    material->color = effectiveColor;
    material->lineWidth = static_cast<float>(params.lineWidth);
    material->domainMin = params.domainMin;
    material->domainMax = params.domainMax;
    material->viewportSize = params.viewportSize;
    material->logScaleX = params.logScaleX ? 1.0f : 0.0f;
    material->logScaleY = params.logScaleY ? 1.0f : 0.0f;
    material->useVertexColor = useVertexColor ? 1.0f : 0.0f;
    material->pointCount = static_cast<float>(params.pointCount);
    material->antialiasingEnabled = params.antialiasingEnabled ? 1.0f : 0.0f;
    material->antialiasingFeather = static_cast<float>(params.antialiasingFeather);
    material->dashPeriod = dashParams.period;
    material->dashOffset = dashParams.offset;
    material->dashPatternSize = dashParams.enabled ? dashParams.patternSize : 0;
    std::copy(std::begin(dashParams.pattern), std::end(dashParams.pattern), material->dashPattern);
    if (auto* gradientMaterial = dynamic_cast<GradientLineMaterial*>(material)) {
        gradientMaterial->gradientDirection = params.gradientPayload.direction == GradientDirection::Vertical ? 1.0f : 0.0f;
        gradientMaterial->gradientValueMin = *params.gradientPayload.gradientValueMin;
        gradientMaterial->gradientValueMax = *params.gradientPayload.gradientValueMax;
        gradientMaterial->gradientTexture.upload(params.window, params.gradientPayload.stops);
    }
}

std::vector<float> LineCurveLineRenderer::computeArcLengths(const LineCurveRenderParams& params, const DashParameters& dashParams) const
{
    if (!dashParams.enabled || !params.xAxis || !params.yAxis || params.pointCount <= 0) {
        return {};
    }
    auto arcLengths = std::vector<float>(params.pointCount);
    auto cumLen = 0.0f;
    auto prevPx = static_cast<float>(params.xAxis->coordToPixel(params.data[0], params.viewportSize.x()));
    auto prevPy = static_cast<float>(params.yAxis->coordToPixel(params.data[1], params.viewportSize.y()));
    arcLengths[0] = 0.0f;
    for (auto i = 1; i < params.pointCount; ++i) {
        const auto px = static_cast<float>(params.xAxis->coordToPixel(params.data[i * 2], params.viewportSize.x()));
        const auto py = static_cast<float>(params.yAxis->coordToPixel(params.data[i * 2 + 1], params.viewportSize.y()));
        const auto dx = px - prevPx;
        const auto dy = py - prevPy;
        cumLen += std::sqrt(dx * dx + dy * dy);
        arcLengths[i] = cumLen;
        prevPx = px;
        prevPy = py;
    }
    return arcLengths;
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
        assembleVertices(geometry, params.data, params.pointCount, effectiveColor, arcLengths);
    }
}

} // namespace QAccelPlot
