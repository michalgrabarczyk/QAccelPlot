//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "renderers/LineCurvePointRenderer.hpp"

#include "materials/PointMaterial.hpp"
#include "renderers/CurveRendererParams.hpp"

#include <QSGGeometry>
#include <QSGGeometryNode>

#include <algorithm>
#include <cstring>

namespace QAccelPlot {

namespace {
// Minimum gradient stop/axis range used as a divisor guard to prevent division by zero during interpolation.
constexpr auto kMinGradientRangeEpsilon = float{1e-6f};

QColor evaluateGradientColor(const GradientColorPayload& gradientPayload, const float normalizedValue)
{
    if (!gradientPayload.isValid()) {
        return Qt::transparent;
    }

    if (normalizedValue <= gradientPayload.stops.front().position) {
        return gradientPayload.stops.front().color;
    }
    if (normalizedValue >= gradientPayload.stops.back().position) {
        return gradientPayload.stops.back().color;
    }

    for (size_t stopIndex = 1; stopIndex < gradientPayload.stops.size(); ++stopIndex) {
        const auto& leftStop = gradientPayload.stops[stopIndex - 1];
        const auto& rightStop = gradientPayload.stops[stopIndex];
        if (normalizedValue > rightStop.position) {
            continue;
        }

        const auto stopRange = std::max(rightStop.position - leftStop.position, kMinGradientRangeEpsilon);
        const auto interpolation = std::clamp((normalizedValue - leftStop.position) / stopRange, 0.0f, 1.0f);

        const auto red = leftStop.color.redF() + (rightStop.color.redF() - leftStop.color.redF()) * interpolation;
        const auto green = leftStop.color.greenF() + (rightStop.color.greenF() - leftStop.color.greenF()) * interpolation;
        const auto blue = leftStop.color.blueF() + (rightStop.color.blueF() - leftStop.color.blueF()) * interpolation;
        const auto alpha = leftStop.color.alphaF() + (rightStop.color.alphaF() - leftStop.color.alphaF()) * interpolation;
        return QColor::fromRgbF(red, green, blue, alpha);
    }

    return gradientPayload.stops.back().color;
}

float normalizedGradientValue(const GradientColorPayload& gradientPayload, const float dataX, const float dataY)
{
    const auto valueMin = *gradientPayload.gradientValueMin;
    const auto valueMax = *gradientPayload.gradientValueMax;
    const auto axisRange = std::max(valueMax - valueMin, kMinGradientRangeEpsilon);
    const auto value = (gradientPayload.direction == GradientDirection::Horizontal) ? dataX : dataY;
    return std::clamp((value - valueMin) / axisRange, 0.0f, 1.0f);
}

QSGGeometryNode* createPointNode(const int vertexCount)
{
    auto* node = new QSGGeometryNode;

    static constexpr int kVertexStride{32};      // float pos[2] (8) + float corner[2] (8) + float rgba[4] (16)
    static constexpr int kAttrLocationPos{0};    // layout(location = 0): data-space point position (x, y)
    static constexpr int kAttrLocationCorner{1}; // layout(location = 1): marker quad corner offset (-1..+1, -1..+1)
    static constexpr int kAttrLocationColor{2};  // layout(location = 2): per-vertex RGBA color (float4)
    static constexpr int kComponentsVec2{2};     // 2 floats
    static constexpr int kComponentsVec4{4};     // 4 floats

    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(kAttrLocationPos, kComponentsVec2, QSGGeometry::FloatType),
        QSGGeometry::Attribute::create(kAttrLocationCorner, kComponentsVec2, QSGGeometry::FloatType),
        QSGGeometry::Attribute::create(kAttrLocationColor, kComponentsVec4, QSGGeometry::FloatType),
    };
    static const QSGGeometry::AttributeSet attrSet = {static_cast<int>(std::size(attributes)), kVertexStride, attributes};

    auto* geometry = new QSGGeometry(attrSet, vertexCount);
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);

    node->setMaterial(new PointMaterial);
    node->setFlag(QSGNode::OwnsMaterial);

    return node;
}

void assemblePointVertices(
    QSGGeometry* geometry, const std::vector<float>& data, const int pointCount, const QColor& effectiveColor, const GradientColorPayload& gradientPayload)
{
    auto* vertices = static_cast<PointVertex*>(geometry->vertexData());
    const auto* src = data.data();
    const auto hasGradient = gradientPayload.isValid();

    // Hoist base color conversion — avoids 4 QColor::xF() calls per vertex in the common no-gradient path.
    const auto baseR = static_cast<float>(effectiveColor.redF());
    const auto baseG = static_cast<float>(effectiveColor.greenF());
    const auto baseB = static_cast<float>(effectiveColor.blueF());
    const auto baseA = static_cast<float>(effectiveColor.alphaF());

    for (int index = 0; index < pointCount; ++index) {
        const auto px = src[index * 2];
        const auto py = src[index * 2 + 1];

        auto r = baseR;
        auto g = baseG;
        auto b = baseB;
        auto a = baseA;
        if (hasGradient) {
            const auto normalizedValue = normalizedGradientValue(gradientPayload, px, py);
            const auto c = evaluateGradientColor(gradientPayload, normalizedValue);
            r = static_cast<float>(c.redF());
            g = static_cast<float>(c.greenF());
            b = static_cast<float>(c.blueF());
            a = static_cast<float>(c.alphaF());
        }

        vertices[index * 6 + 0] = {px, py, -1.0f, -1.0f, r, g, b, a};
        vertices[index * 6 + 1] = {px, py, 1.0f, -1.0f, r, g, b, a};
        vertices[index * 6 + 2] = {px, py, 1.0f, 1.0f, r, g, b, a};
        vertices[index * 6 + 3] = {px, py, -1.0f, -1.0f, r, g, b, a};
        vertices[index * 6 + 4] = {px, py, 1.0f, 1.0f, r, g, b, a};
        vertices[index * 6 + 5] = {px, py, -1.0f, 1.0f, r, g, b, a};
    }
}

} // namespace

void LineCurvePointRenderer::buildVertexCache(const std::vector<float>& data, const int pointCount, std::vector<char>& cache) const
{
    const auto byteSize = static_cast<std::size_t>(pointCount) * 6 * sizeof(PointVertex);
    cache.resize(byteSize);
    auto* vertices = reinterpret_cast<PointVertex*>(cache.data());
    const auto* src = data.data();

    // Writes neutral RGBA — the shader uses the material color uniform when useVertexColor==0.
    for (int index = 0; index < pointCount; ++index) {
        const auto px = src[index * 2];
        const auto py = src[index * 2 + 1];
        vertices[index * 6 + 0] = {px, py, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        vertices[index * 6 + 1] = {px, py, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        vertices[index * 6 + 2] = {px, py, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        vertices[index * 6 + 3] = {px, py, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        vertices[index * 6 + 4] = {px, py, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        vertices[index * 6 + 5] = {px, py, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    }
}

bool LineCurvePointRenderer::contains(const QPointF& point, const CurveHitTestParams& params) const
{
    const auto radiusSquared = params.hitThreshold * params.hitThreshold;

    for (const auto& chunk : params.chunks) {
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

        const auto pointEnd = chunk.start + chunk.count;
        for (auto index = chunk.start; index < pointEnd; ++index) {
            const auto delta = QPointF{params.xAxis->coordToPixel(params.data[index * 2], params.width),
                                   params.yAxis->coordToPixel(params.data[index * 2 + 1], params.height)}
                - point;
            if (QPointF::dotProduct(delta, delta) < radiusSquared) {
                return true;
            }
        }
    }

    // Suppress unused-parameter warning when chunks covers all points.
    Q_UNUSED(params.pointCount)
    return false;
}

QSGNode* LineCurvePointRenderer::paint(QSGNode* oldNode, const PointCurveRenderParams& params) const
{
    const auto vertexCount = params.pointCount * 6;
    auto* node = oldNode ? static_cast<QSGGeometryNode*>(oldNode) : createPointNode(vertexCount);

    // Only reallocate when the vertex count actually changes — avoids GPU buffer
    // churn (and the associated CPU stall) every frame during steady-state rendering.
    if (node->geometry()->vertexCount() != vertexCount) {
        node->geometry()->allocate(vertexCount);
    }

    const auto effectiveColor = params.hovered ? params.color.lighter(150) : params.color;
    const auto useVertexColor = params.gradientPayload.isValid();

    auto* material = static_cast<PointMaterial*>(node->material());
    material->color = effectiveColor;
    material->markerSize = static_cast<float>(params.markerSize);
    material->domainMin = params.domainMin;
    material->domainMax = params.domainMax;
    material->viewportSize = params.viewportSize;
    material->logScaleX = params.logScaleX ? 1.0f : 0.0f;
    material->logScaleY = params.logScaleY ? 1.0f : 0.0f;
    material->useVertexColor = useVertexColor ? 1.0f : 0.0f;
    material->antialiasingEnabled = params.antialiasingEnabled ? 1.0f : 0.0f;
    material->antialiasingFeather = static_cast<float>(params.antialiasingFeather);
    material->shapeType = params.shapeType;

    if (params.dataChanged || !oldNode || useVertexColor) {
        const auto expectedCacheBytes = static_cast<std::size_t>(node->geometry()->vertexCount()) * sizeof(PointVertex);
        if (params.vertexCache && params.vertexCache->size() == expectedCacheBytes && !useVertexColor) {
            std::memcpy(node->geometry()->vertexData(), params.vertexCache->data(), expectedCacheBytes);
        } else {
            assemblePointVertices(node->geometry(), params.data, params.pointCount, effectiveColor, params.gradientPayload);
        }
        node->markDirty(QSGNode::DirtyGeometry);
    }

    node->markDirty(QSGNode::DirtyMaterial);
    return node;
}

} // namespace QAccelPlot
