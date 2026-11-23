//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/materials/GradientFillMaterial.hpp"

#include <QByteArray>
#include <QSGMaterialShader>

#include <cstring>

namespace QAccelPlot {

struct GradientFillUbo {
    float matrix[16];
    float parameters[4];
    float logGradient[4];
};

static_assert(sizeof(GradientFillUbo) == 96);

class GradientFillShader : public QSGMaterialShader {
public:
    GradientFillShader()
    {
        setShaderFileName(VertexStage, QString::fromLatin1(":/qaccelplot/shaders/gradient_fill.vert.qsb"));
        setShaderFileName(FragmentStage, QString::fromLatin1(":/qaccelplot/shaders/gradient_fill.frag.qsb"));
    }

    bool updateUniformData(RenderState& state, QSGMaterial* newMaterial, [[maybe_unused]] QSGMaterial* oldMaterial) override
    {
        auto* buffer = state.uniformData();
        if (!buffer || buffer->size() < static_cast<int>(sizeof(GradientFillUbo))) {
            return false;
        }
        const auto* material = static_cast<const GradientFillMaterial*>(newMaterial);
        auto ubo = GradientFillUbo{};
        const auto matrix = state.combinedMatrix();
        std::memcpy(ubo.matrix, matrix.constData(), sizeof(ubo.matrix));
        ubo.parameters[0] = material->opacity;
        ubo.parameters[1] = state.opacity();
        ubo.logGradient[0] = material->logGradient ? 1.0f : 0.0f;
        ubo.logGradient[1] = material->logGradientScale;
        ubo.logGradient[2] = material->logGradientOffset;
        std::memcpy(buffer->data(), &ubo, sizeof(ubo));
        return true;
    }

    void updateSampledImage(RenderState& state, int binding, QSGTexture** texture, QSGMaterial* newMaterial, [[maybe_unused]] QSGMaterial* oldMaterial) override
    {
        if (binding == 1) {
            static_cast<GradientFillMaterial*>(newMaterial)->gradientTexture.commit(state, texture);
        }
    }
};

GradientFillMaterial::GradientFillMaterial()
{
    setFlag(Blending | RequiresFullMatrix);
}

QSGMaterialType* GradientFillMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader* GradientFillMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new GradientFillShader;
}

int GradientFillMaterial::compare(const QSGMaterial* other) const
{
    const auto* material = static_cast<const GradientFillMaterial*>(other);
    if (opacity != material->opacity) {
        return opacity < material->opacity ? -1 : 1;
    }
    if (logGradient != material->logGradient) {
        return logGradient ? 1 : -1;
    }
    if (logGradientScale != material->logGradientScale) {
        return logGradientScale < material->logGradientScale ? -1 : 1;
    }
    if (logGradientOffset != material->logGradientOffset) {
        return logGradientOffset < material->logGradientOffset ? -1 : 1;
    }
    const auto textureKey = gradientTexture.comparisonKey();
    const auto otherTextureKey = material->gradientTexture.comparisonKey();
    if (textureKey != otherTextureKey) {
        return textureKey < otherTextureKey ? -1 : 1;
    }
    return 0;
}

} // namespace QAccelPlot
