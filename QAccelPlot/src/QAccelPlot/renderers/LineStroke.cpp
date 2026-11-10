//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/renderers/LineStroke.hpp"

#include "QAccelPlot/materials/LineMaterial.hpp"

#include <algorithm>
#include <iterator>

namespace QAccelPlot::LineStroke {

namespace {

QSGGeometry* createGeometry(const int vertexCount, const bool dashed)
{
    auto* geometry = new QSGGeometry(vertexAttributes(dashed), vertexCount);
    geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);
    return geometry;
}

} // namespace

const QSGGeometry::AttributeSet& vertexAttributes(const bool dashed)
{
    // layout(location = 0): the four bytes of a LineVertex, read as normalized bytes.
    // layout(location = 1): the arc length of a DashedLineVertex.
    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(0, 4, QSGGeometry::UnsignedByteType),
        QSGGeometry::Attribute::create(1, 1, QSGGeometry::FloatType),
    };
    static const QSGGeometry::AttributeSet solid = {1, static_cast<int>(sizeof(LineVertex)), attributes};
    static const QSGGeometry::AttributeSet dashedSet = {2, static_cast<int>(sizeof(DashedLineVertex)), attributes};
    return dashed ? dashedSet : solid;
}

QSGGeometryNode* createNode(const int vertexCount, LineMaterial* material)
{
    auto* node = new QSGGeometryNode;
    node->setGeometry(createGeometry(vertexCount, material->dashed()));
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(material);
    node->setFlag(QSGNode::OwnsMaterial);
    return node;
}

bool setMaterial(QSGGeometryNode* node, LineMaterial* material)
{
    const auto layoutChanged = node->geometry()->sizeOfVertex() != vertexAttributes(material->dashed()).stride;
    if (layoutChanged) {
        node->setGeometry(createGeometry(node->geometry()->vertexCount(), material->dashed()));
    }
    node->setMaterial(material);
    return layoutChanged;
}

void writeVertices(LineVertex* vertices, const int pointCount, const int cornerCount, const int firstPoint)
{
    const auto start = std::max(firstPoint, 0);
    constexpr auto kNegativeSide = LineVertex::make(0, 0, true).packed;
    auto* vertex = vertices + static_cast<std::size_t>(start) * static_cast<std::size_t>(cornerCount) * 2;
    for (auto index = start; index < pointCount; ++index) {
        for (auto corner = 0; corner < cornerCount; ++corner) {
            const auto packed = LineVertex::make(index, corner, false).packed;
            vertex[0].packed = packed;
            vertex[1].packed = packed | kNegativeSide;
            vertex += 2;
        }
    }
}

void writeDashedVertices(DashedLineVertex* vertices, const int pointCount, const int cornerCount, const std::vector<float>& arcLengths)
{
    // Field by field: MSVC assembles a whole vertex in memory and copies it, which stalls on every vertex.
    const auto* lengths = arcLengths.data();
    const auto lengthCount = arcLengths.size();
    constexpr auto kNegativeSide = LineVertex::make(0, 0, true).packed;
    auto* vertex = vertices;
    auto pathIndex = std::size_t{0};
    for (auto index = 0; index < pointCount; ++index) {
        for (auto corner = 0; corner < cornerCount; ++corner, ++pathIndex) {
            const auto arcLength = lengthCount == 0 ? 0.0f : lengths[std::min(pathIndex, lengthCount - 1)];
            const auto packed = LineVertex::make(index, corner, false).packed;
            vertex[0].vertex.packed = packed;
            vertex[0].arcLength = arcLength;
            vertex[1].vertex.packed = packed | kNegativeSide;
            vertex[1].arcLength = arcLength;
            vertex += 2;
        }
    }
}

void applyUniforms(LineMaterial& material, const Uniforms& uniforms)
{
    material.color = uniforms.color;
    material.lineWidth = static_cast<float>(uniforms.lineWidth);
    material.domainMin = uniforms.domainMin;
    material.domainMax = uniforms.domainMax;
    material.viewportSize = uniforms.viewportSize;
    material.logScaleX = uniforms.logScaleX ? 1.0f : 0.0f;
    material.logScaleY = uniforms.logScaleY ? 1.0f : 0.0f;
    material.pointCount = static_cast<float>(uniforms.pointCount);
    material.antialiasingEnabled = uniforms.antialiasingEnabled ? 1.0f : 0.0f;
    material.antialiasingFeather = static_cast<float>(uniforms.antialiasingFeather);
    material.dashPeriod = uniforms.dash.period;
    material.dashOffset = uniforms.dash.offset;
    material.dashPatternSize = uniforms.dash.enabled ? uniforms.dash.patternSize : 0;
    std::copy(std::begin(uniforms.dash.pattern), std::end(uniforms.dash.pattern), material.dashPattern);
}

} // namespace QAccelPlot::LineStroke
