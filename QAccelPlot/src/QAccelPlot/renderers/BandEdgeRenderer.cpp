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

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

BandEdgeRenderer::BandEdgeRenderer(const BandEdgeMaterial::Edge edge)
    : edge_(edge)
{
}

QSGGeometryNode* BandEdgeRenderer::paint(QSGGeometryNode* oldNode, const BandEdgeRenderParams& params) const
{
    auto* node = oldNode ? oldNode : LineStroke::createNode(0, new BandEdgeMaterial(edge_));
    auto* geometry = node->geometry();
    const auto vertexCount = params.reservedCount * 2;
    const auto countChanged = geometry->vertexCount() != vertexCount;
    if (countChanged) {
        geometry->allocate(vertexCount);
    }

    auto* material = static_cast<BandEdgeMaterial*>(node->material());
    LineStroke::applyUniforms(*material, params.uniforms);
    material->useVertexColor = 0.0f;
    material->dataTexture = params.dataTexture;

    // Vertices hold only data point indices unless a dash pattern needs screen-space arc lengths.
    // Those change with the data and the zoom, but not while panning.
    const auto dashed = params.uniforms.dash.enabled;
    auto lengthsStale = false;
    if (dashed) {
        const auto scale = arcLengthScale(params);
        lengthsStale = params.dataChanged || !arcLengthScale_ || !arcLengthScale_->matches(scale);
        arcLengthScale_ = scale;
    } else {
        arcLengthScale_.reset();
    }
    if (countChanged || lengthsStale) {
        LineStroke::writeVertices(static_cast<LineVertex*>(geometry->vertexData()), params.reservedCount, 1, params.uniforms.color,
            dashed ? arcLengths(params) : std::vector<float>{});
        node->markDirty(QSGNode::DirtyGeometry);
    }
    node->markDirty(QSGNode::DirtyMaterial);
    return node;
}

bool BandEdgeRenderer::ArcLengthScale::matches(const ArcLengthScale& other) const
{
    const auto close = [](const qreal a, const qreal b) {
        constexpr auto kRelativeTolerance = 1e-9;
        return std::abs(a - b) <= kRelativeTolerance * std::max(std::abs(a), std::abs(b));
    };
    return close(xSpan, other.xSpan) && close(ySpan, other.ySpan) && viewportSize == other.viewportSize && logScaleX == other.logScaleX
        && logScaleY == other.logScaleY;
}

BandEdgeRenderer::ArcLengthScale BandEdgeRenderer::arcLengthScale(const BandEdgeRenderParams& params)
{
    const auto span = [](const Axis* axis, const bool logScale) {
        if (!axis) {
            return qreal{0.0};
        }
        return logScale ? std::log10(axis->viewportMax()) - std::log10(axis->viewportMin()) : axis->viewportMax() - axis->viewportMin();
    };
    const auto& uniforms = params.uniforms;
    return ArcLengthScale{
        span(params.xAxis, uniforms.logScaleX), span(params.yAxis, uniforms.logScaleY), uniforms.viewportSize, uniforms.logScaleX, uniforms.logScaleY};
}

std::vector<float> BandEdgeRenderer::arcLengths(const BandEdgeRenderParams& params) const
{
    if (!params.xAxis || !params.yAxis) {
        return {};
    }
    const auto upper = edge_ == BandEdgeMaterial::Edge::Upper;
    const auto& points = params.dataPoints;
    const auto& uniforms = params.uniforms;
    // Like band_edge.glsl: the drawn bound of each data point, invalid wherever the fill has a gap.
    return LineStroke::arcLengths(points.count, [&](const int i) -> std::optional<QPointF> {
        const auto x = points.value(i, 0);
        const auto low = points.value(i, 1);
        const auto high = points.value(i, 2);
        if (!isValidCoordinate(x, uniforms.logScaleX) || !isValidCoordinate(low, uniforms.logScaleY) || !isValidCoordinate(high, uniforms.logScaleY)) {
            return std::nullopt;
        }
        const auto y = upper ? std::max(low, high) : std::min(low, high);
        return QPointF{params.xAxis->coordToPixel(x, uniforms.viewportSize.x()), params.yAxis->coordToPixel(y, uniforms.viewportSize.y())};
    });
}

} // namespace QAccelPlot
