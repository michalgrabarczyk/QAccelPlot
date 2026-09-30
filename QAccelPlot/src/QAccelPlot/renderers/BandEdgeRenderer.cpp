//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/renderers/BandEdgeRenderer.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/axis/Axis.hpp"

namespace QAccelPlot {

BandEdgeRenderer::BandEdgeRenderer(const BandEdgeMaterial::Edge edge)
    : edge_(edge)
{
}

QSGGeometryNode* BandEdgeRenderer::paint(QSGGeometryNode* oldNode, const BandEdgeRenderParams& params) const
{
    auto* node = oldNode ? oldNode : LineStroke::createNode(0, new BandEdgeMaterial(edge_));
    auto* geometry = node->geometry();
    const auto vertexCount = params.samples.count * 2;
    const auto countChanged = geometry->vertexCount() != vertexCount;
    if (countChanged) {
        geometry->allocate(vertexCount);
    }

    auto* material = static_cast<BandEdgeMaterial*>(node->material());
    LineStroke::applyUniforms(*material, params.uniforms);
    material->useVertexColor = 0.0f;
    material->dataTexture = params.dataTexture;

    // Vertices hold only sample indices unless a dash pattern needs screen-space arc lengths.
    const auto dashed = params.uniforms.dash.enabled;
    if (countChanged || dashed) {
        LineStroke::writeVertices(
            static_cast<LineVertex*>(geometry->vertexData()), params.samples.count, params.uniforms.color, dashed ? arcLengths(params) : std::vector<float>{});
        node->markDirty(QSGNode::DirtyGeometry);
    }
    node->markDirty(QSGNode::DirtyMaterial);
    return node;
}

std::vector<float> BandEdgeRenderer::arcLengths(const BandEdgeRenderParams& params) const
{
    if (!params.xAxis || !params.yAxis) {
        return {};
    }
    const auto component = edge_ == BandEdgeMaterial::Edge::Lower ? 1 : 2;
    const auto& samples = params.samples;
    const auto& uniforms = params.uniforms;
    return LineStroke::arcLengths(samples.count, [&](const int i) -> std::optional<QPointF> {
        const auto x = samples.value(i, 0);
        const auto y = samples.value(i, component);
        if (!isValidSample(x, uniforms.logScaleX) || !isValidSample(y, uniforms.logScaleY)) {
            return std::nullopt;
        }
        return QPointF{params.xAxis->coordToPixel(x, uniforms.viewportSize.x()), params.yAxis->coordToPixel(y, uniforms.viewportSize.y())};
    });
}

} // namespace QAccelPlot
