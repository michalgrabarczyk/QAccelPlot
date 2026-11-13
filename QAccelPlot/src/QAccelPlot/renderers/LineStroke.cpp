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
#include <cmath>
#include <iterator>

namespace QAccelPlot {

LineVertex LineVertex::make(const int index, const int corner, const bool negativeSide, const bool outgoing) noexcept
{
    return {static_cast<std::uint32_t>(index) | (static_cast<std::uint32_t>(corner) << 28U) | (outgoing ? 1U << 30U : 0U) | (negativeSide ? 1U << 31U : 0U)};
}

int LineVertex::index() const noexcept
{
    return static_cast<int>(packed & 0x0FFFFFFFU);
}

int LineVertex::corner() const noexcept
{
    return static_cast<int>((packed >> 28U) & 3U);
}

bool LineVertex::outgoing() const noexcept
{
    return ((packed >> 30U) & 1U) != 0U;
}

bool LineVertex::negativeSide() const noexcept
{
    return (packed >> 31U) != 0U;
}

namespace LineStroke {

namespace {

QSGGeometry* createGeometry(const int vertexCount, const bool dashed)
{
    auto* geometry = new QSGGeometry(vertexAttributes(dashed), vertexCount);
    geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);
    return geometry;
}

// Bits added to the packed path point of each of its vertices: the incoming pair, then the outgoing one.
const std::uint32_t kPathVertexBits[] = {
    0U,
    LineVertex::make(0, 0, true).packed,
    LineVertex::make(0, 0, false, true).packed,
    LineVertex::make(0, 0, true, true).packed,
};

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

int verticesPerPathPoint(const bool splitJoins) noexcept
{
    return splitJoins ? 4 : 2;
}

void writeVertices(LineVertex* vertices, const int pointCount, const int cornerCount, const bool splitJoins, const int firstPoint)
{
    const auto start = std::max(firstPoint, 0);
    const auto pathVertices = verticesPerPathPoint(splitJoins);
    auto* vertex = vertices + static_cast<std::size_t>(start) * static_cast<std::size_t>(cornerCount) * static_cast<std::size_t>(pathVertices);
    for (auto index = start; index < pointCount; ++index) {
        for (auto corner = 0; corner < cornerCount; ++corner) {
            const auto packed = LineVertex::make(index, corner, false).packed;
            for (auto i = 0; i < pathVertices; ++i) {
                vertex[i].packed = packed | kPathVertexBits[i];
            }
            vertex += pathVertices;
        }
    }
}

void writeDashedVertices(DashedLineVertex* vertices, const int pointCount, const int cornerCount, const bool splitJoins, const std::vector<float>& arcLengths)
{
    // Field by field: MSVC assembles a whole vertex in memory and copies it, which stalls on every vertex.
    const auto* lengths = arcLengths.data();
    const auto lengthCount = arcLengths.size();
    const auto pathVertices = verticesPerPathPoint(splitJoins);
    auto* vertex = vertices;
    auto pathIndex = std::size_t{0};
    for (auto index = 0; index < pointCount; ++index) {
        for (auto corner = 0; corner < cornerCount; ++corner, ++pathIndex) {
            const auto arcLength = lengthCount == 0 ? 0.0f : lengths[std::min(pathIndex, lengthCount - 1)];
            const auto packed = LineVertex::make(index, corner, false).packed;
            for (auto i = 0; i < pathVertices; ++i) {
                vertex[i].vertex.packed = packed | kPathVertexBits[i];
                vertex[i].arcLength = arcLength;
            }
            vertex += pathVertices;
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

std::vector<float> arcLengths(const int pointCount, const std::function<std::optional<QPointF>(int)>& pixelAt)
{
    auto lengths = std::vector<float>(static_cast<std::size_t>(std::max(pointCount, 0)));
    auto length = 0.0f;
    auto previousX = 0.0f;
    auto previousY = 0.0f;
    auto previousValid = false;
    for (auto i = 0; i < pointCount; ++i) {
        const auto current = pixelAt(i);
        if (current) {
            const auto x = static_cast<float>(current->x());
            const auto y = static_cast<float>(current->y());
            if (previousValid) {
                const auto dx = x - previousX;
                const auto dy = y - previousY;
                length += std::sqrt(dx * dx + dy * dy);
            }
            previousX = x;
            previousY = y;
        }
        previousValid = current.has_value();
        lengths[static_cast<std::size_t>(i)] = length;
    }
    return lengths;
}

} // namespace LineStroke

} // namespace QAccelPlot
