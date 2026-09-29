//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/internal/RectUniforms.hpp"

#include "QAccelPlot/materials/RectMaterial.hpp"

#include <cstring>

namespace QAccelPlot::Internal {

namespace {

void writeColor(float (&target)[4], const QColor& color)
{
    target[0] = static_cast<float>(color.redF());
    target[1] = static_cast<float>(color.greenF());
    target[2] = static_cast<float>(color.blueF());
    target[3] = static_cast<float>(color.alphaF());
}

void writeVector(float (&target)[2], const QVector2D& vector)
{
    target[0] = vector.x();
    target[1] = vector.y();
}

}

void writeRectUniforms(RectUbo& ubo, const QSGMaterialShader::RenderState& state, const RectMaterial& material)
{
    const auto matrix = state.combinedMatrix();
    memcpy(ubo.matrix, matrix.constData(), sizeof(ubo.matrix));
    writeColor(ubo.color, material.color);
    writeColor(ubo.borderColor, material.borderColor);
    writeColor(ubo.hoverColor, material.hoverColor);
    writeVector(ubo.domainMin, material.domainMin);
    writeVector(ubo.domainMax, material.domainMax);
    writeVector(ubo.viewportSize, material.viewportSize);
    writeVector(ubo.minimumSize, material.minimumSize);
    ubo.logScaleX = material.logScaleX;
    ubo.logScaleY = material.logScaleY;
    ubo.useVertexColor = material.useVertexColor;
    ubo.rectCount = material.rectCount;
    ubo.opacity = state.opacity();
    ubo.borderWidth = material.borderWidth;
    ubo.hoveredIndex = material.hoveredIndex;
}

} // namespace QAccelPlot::Internal
