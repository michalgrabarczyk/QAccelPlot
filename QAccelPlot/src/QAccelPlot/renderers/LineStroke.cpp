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

// Ribbon side values: each data point generates two vertices extruded to opposite sides of the line.
constexpr auto kSidePositive = float{1.0f};
constexpr auto kSideNegative = float{-1.0f};

} // namespace

const QSGGeometry::AttributeSet& vertexAttributes()
{
    static constexpr int kAttrLocationId{0};        // layout(location = 0): point index
    static constexpr int kAttrLocationSide{1};      // layout(location = 1): ribbon side (+1 / -1)
    static constexpr int kAttrLocationColor{2};     // layout(location = 2): per-vertex RGBA color
    static constexpr int kAttrLocationArcLength{3}; // layout(location = 3): cumulative arc length
    static constexpr int kComponentsScalar{1};      // 1 float
    static constexpr int kComponentsColor{4};       // 4 bytes: RGBA

    // {float id, float side, uchar4 rgba, float arcLength} = 16 bytes.
    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(kAttrLocationId, kComponentsScalar, QSGGeometry::FloatType),
        QSGGeometry::Attribute::create(kAttrLocationSide, kComponentsScalar, QSGGeometry::FloatType),
        QSGGeometry::Attribute::createWithAttributeType(kAttrLocationColor, kComponentsColor, QSGGeometry::UnsignedByteType, QSGGeometry::ColorAttribute),
        QSGGeometry::Attribute::create(kAttrLocationArcLength, kComponentsScalar, QSGGeometry::FloatType),
    };
    static const QSGGeometry::AttributeSet attributeSet = {static_cast<int>(std::size(attributes)), static_cast<int>(sizeof(LineVertex)), attributes};
    return attributeSet;
}

QSGGeometryNode* createNode(const int vertexCount, QSGMaterial* material)
{
    auto* geometry = new QSGGeometry(vertexAttributes(), vertexCount);
    geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);
    auto* node = new QSGGeometryNode;
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(material);
    node->setFlag(QSGNode::OwnsMaterial);
    return node;
}

void writeVertices(LineVertex* vertices, const int pointCount, const QColor& color, const std::vector<float>& arcLengths)
{
    // Hoist the color conversion out of the per-vertex loop.
    const auto r = static_cast<unsigned char>(color.red());
    const auto g = static_cast<unsigned char>(color.green());
    const auto b = static_cast<unsigned char>(color.blue());
    const auto a = static_cast<unsigned char>(color.alpha());
    for (auto index = 0; index < pointCount; ++index) {
        const auto arcLength = arcLengths.empty() ? 0.0f : arcLengths[static_cast<std::size_t>(index)];
        vertices[index * 2] = {static_cast<float>(index), kSidePositive, r, g, b, a, arcLength};
        vertices[index * 2 + 1] = {static_cast<float>(index), kSideNegative, r, g, b, a, arcLength};
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
