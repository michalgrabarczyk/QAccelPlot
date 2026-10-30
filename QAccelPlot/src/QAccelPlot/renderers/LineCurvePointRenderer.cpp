//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/renderers/LineCurvePointRenderer.hpp"

#include "QAccelPlot/materials/PointMaterial.hpp"
#include "QAccelPlot/renderers/CurveRendererParams.hpp"
#include "QAccelPlot/series/LineCurveGapFilter.hpp"

#include <QSGGeometry>
#include <QSGGeometryNode>

#include <algorithm>
#include <cstring>

namespace QAccelPlot {

namespace {

QSGGeometryNode* createPointNode()
{
    auto* geometry = new QSGGeometry(DataTextureMaterial::vertexIndexAttributeSet(), 0);
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);
    // The buffer only changes with the point count; let the backend keep it in static GPU memory.
    geometry->setVertexDataPattern(QSGGeometry::StaticPattern);

    auto* node = new QSGGeometryNode;
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(new PointMaterial);
    node->setFlag(QSGNode::OwnsMaterial);
    return node;
}

void writePointVertices(QSGGeometry* geometry, const std::vector<char>* vertexCache)
{
    const auto byteCount = static_cast<std::size_t>(geometry->vertexCount()) * sizeof(PointVertex);
    if (vertexCache && vertexCache->size() == byteCount) {
        std::memcpy(geometry->vertexData(), vertexCache->data(), byteCount);
    } else {
        std::memset(geometry->vertexData(), 0, byteCount);
    }
}

void updatePointMaterial(PointMaterial& material, const PointCurveRenderParams& params)
{
    material.color = params.hovered ? params.color.lighter(150) : params.color;
    material.markerSize = static_cast<float>(params.markerSize);
    material.domainMin = params.domainMin;
    material.domainMax = params.domainMax;
    material.viewportSize = params.viewportSize;
    material.logScaleX = params.logScaleX ? 1.0f : 0.0f;
    material.logScaleY = params.logScaleY ? 1.0f : 0.0f;
    material.antialiasingEnabled = params.antialiasingEnabled ? 1.0f : 0.0f;
    material.antialiasingFeather = static_cast<float>(params.antialiasingFeather);
    material.shapeType = params.shapeType;
    material.markerStrokeWidth = static_cast<float>(params.markerStrokeWidth);
    material.markerFilled = params.markerFilled ? 1.0f : 0.0f;

    const auto& gradient = params.gradientPayload;
    const auto useGradient = gradient.isValid();
    material.useVertexColor = useGradient ? 1.0f : 0.0f;
    material.gradientDirection = gradient.direction == Qt::Vertical ? 1.0f : 0.0f;
    material.gradientValueMin = static_cast<float>(gradient.gradientValueMin.value_or(0.0));
    material.gradientValueMax = static_cast<float>(gradient.gradientValueMax.value_or(1.0));
    // The fragment shader always samples the gradient, so a neutral texture keeps the sampler valid.
    material.gradientTexture.upload(params.window, useGradient ? gradient.stops : GradientTexture::neutralStops());
}

} // namespace

void LineCurvePointRenderer::writeVertexCache(std::vector<char>& cache, const int firstPoint, const int pointCount)
{
    const auto begin = static_cast<std::size_t>(std::max(firstPoint, 0)) * kVertexCacheBytesPerPoint;
    const auto end = static_cast<std::size_t>(std::max(pointCount, 0)) * kVertexCacheBytesPerPoint;
    if (begin < end) {
        std::fill(cache.begin() + static_cast<std::ptrdiff_t>(begin), cache.begin() + static_cast<std::ptrdiff_t>(end), char{0});
    }
}

bool LineCurvePointRenderer::contains(const QPointF& point, const CurveHitTestParams& params) const
{
    const auto radiusSquared = params.hitThreshold * params.hitThreshold;

    for (const auto& chunk : params.chunks) {
        if (isEmptyChunk(chunk)) {
            continue;
        }
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

        const auto pointEnd = std::min(chunk.start + chunk.count, params.pointCount);
        for (auto index = chunk.start; index < pointEnd; ++index) {
            if (!LineCurveGapFilter::isValidPoint(params.data, index, params.nonPositiveXInvalid, params.nonPositiveYInvalid)) {
                continue;
            }
            const auto delta
                = QPointF{params.xAxis->coordToPixel(params.data.x(index), params.width), params.yAxis->coordToPixel(params.data.y(index), params.height)}
                - point;
            if (QPointF::dotProduct(delta, delta) < radiusSquared) {
                return true;
            }
        }
    }
    return false;
}

QSGNode* LineCurvePointRenderer::paint(QSGNode* oldNode, const PointCurveRenderParams& params) const
{
    auto* node = oldNode ? static_cast<QSGGeometryNode*>(oldNode) : createPointNode();
    const auto vertexCount = params.pointCount * kVerticesPerPoint;
    if (node->geometry()->vertexCount() != vertexCount) {
        node->geometry()->allocate(vertexCount);
        writePointVertices(node->geometry(), params.vertexCache);
        node->markDirty(QSGNode::DirtyGeometry);
    }

    auto* material = static_cast<PointMaterial*>(node->material());
    if (params.dataTexture) {
        material->dataTexture = params.dataTexture;
    } else if (params.dataChanged || !oldNode || !material->dataTexture) {
        // A freshly created node owns a new material, so it always needs the data texture.
        material->uploadTexture(params.window, params.data.data(), params.pointCount * 2);
    }
    updatePointMaterial(*material, params);

    node->markDirty(QSGNode::DirtyMaterial);
    return node;
}

} // namespace QAccelPlot
