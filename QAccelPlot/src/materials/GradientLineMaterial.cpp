//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "materials/GradientLineMaterial.hpp"

#include <QByteArray>
#include <QSGMaterialShader>

#include <cstring>

namespace QAccelPlot {

struct GradientLineUbo {
    float matrix[16];
    float color[4];
    float domainMin[2];
    float domainMax[2];
    float viewportSize[2];
    float lineWidth;
    float logScaleX;
    float logScaleY;
    float useVertexColor;
    float pointCount;
    float antialiasingEnabled;
    float antialiasingFeather;
    float dashPeriod;
    float dashOffset;
    int dashPatternSize;
    float dashPattern[8];
    float gradientParameters[4];
};

static_assert(sizeof(GradientLineUbo) == 192);

class GradientLineShader : public QSGMaterialShader {
public:
    GradientLineShader()
    {
        setShaderFileName(VertexStage, QString::fromLatin1(":/qaccelplot/src/shaders/gradient_line.vert.qsb"));
        setShaderFileName(FragmentStage, QString::fromLatin1(":/qaccelplot/src/shaders/gradient_line.frag.qsb"));
    }

    bool updateUniformData(RenderState& state, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        auto* buffer = state.uniformData();
        if (!buffer || buffer->size() < static_cast<int>(sizeof(GradientLineUbo))) {
            return false;
        }
        const auto* material = static_cast<const GradientLineMaterial*>(newMaterial);
        auto ubo = GradientLineUbo{};
        const auto matrix = state.combinedMatrix();
        std::memcpy(ubo.matrix, matrix.constData(), sizeof(ubo.matrix));
        ubo.color[0] = float(material->color.redF());
        ubo.color[1] = float(material->color.greenF());
        ubo.color[2] = float(material->color.blueF());
        ubo.color[3] = float(material->color.alphaF());
        ubo.domainMin[0] = material->domainMin.x();
        ubo.domainMin[1] = material->domainMin.y();
        ubo.domainMax[0] = material->domainMax.x();
        ubo.domainMax[1] = material->domainMax.y();
        ubo.viewportSize[0] = material->viewportSize.x();
        ubo.viewportSize[1] = material->viewportSize.y();
        ubo.lineWidth = material->lineWidth;
        ubo.logScaleX = material->logScaleX;
        ubo.logScaleY = material->logScaleY;
        ubo.useVertexColor = 0.0f;
        ubo.pointCount = material->pointCount;
        ubo.antialiasingEnabled = material->antialiasingEnabled;
        ubo.antialiasingFeather = material->antialiasingFeather;
        ubo.dashPeriod = material->dashPeriod;
        ubo.dashOffset = material->dashOffset;
        ubo.dashPatternSize = material->dashPatternSize;
        std::memcpy(ubo.dashPattern, material->dashPattern, sizeof(ubo.dashPattern));
        ubo.gradientParameters[0] = material->gradientDirection;
        ubo.gradientParameters[1] = material->gradientValueMin;
        ubo.gradientParameters[2] = material->gradientValueMax;
        std::memcpy(buffer->data(), &ubo, sizeof(ubo));
        return true;
    }

    void updateSampledImage(RenderState& state, int binding, QSGTexture** texture, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        auto* material = static_cast<GradientLineMaterial*>(newMaterial);
        if (binding == 1) {
            DataTextureMaterial::commitTexture(state, binding, texture, material->dataTexture.get());
        } else if (binding == 2) {
            material->gradientTexture.commit(state, binding, texture);
        }
    }
};

QSGMaterialType* GradientLineMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader* GradientLineMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new GradientLineShader;
}

int GradientLineMaterial::compareExtra(const QSGMaterial* other) const
{
    const auto baseComparison = LineMaterial::compareExtra(other);
    if (baseComparison != 0) {
        return baseComparison;
    }
    const auto* material = static_cast<const GradientLineMaterial*>(other);
    if (gradientDirection != material->gradientDirection) {
        return gradientDirection < material->gradientDirection ? -1 : 1;
    }
    if (gradientValueMin != material->gradientValueMin) {
        return gradientValueMin < material->gradientValueMin ? -1 : 1;
    }
    if (gradientValueMax != material->gradientValueMax) {
        return gradientValueMax < material->gradientValueMax ? -1 : 1;
    }
    const auto textureKey = gradientTexture.comparisonKey();
    const auto otherTextureKey = material->gradientTexture.comparisonKey();
    if (textureKey != otherTextureKey) {
        return textureKey < otherTextureKey ? -1 : 1;
    }
    return 0;
}

} // namespace QAccelPlot
