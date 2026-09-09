//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "materials/GradientFillMaterial.hpp"

#include <QByteArray>
#include <QSGMaterialShader>

#include <cstring>

namespace QAccelPlot {

struct GradientFillUbo {
    float matrix[16];
    float parameters[4];
};

static_assert(sizeof(GradientFillUbo) == 80);

class GradientFillShader : public QSGMaterialShader {
public:
    GradientFillShader()
    {
        setShaderFileName(VertexStage, QString::fromLatin1(":/qaccelplot/src/shaders/gradient_fill.vert.qsb"));
        setShaderFileName(FragmentStage, QString::fromLatin1(":/qaccelplot/src/shaders/gradient_fill.frag.qsb"));
    }

    bool updateUniformData(RenderState& state, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        auto* buffer = state.uniformData();
        if (!buffer || buffer->size() < static_cast<int>(sizeof(GradientFillUbo))) {
            return false;
        }
        const auto* material = static_cast<const GradientFillMaterial*>(newMaterial);
        auto ubo = GradientFillUbo{};
        const auto matrix = state.combinedMatrix();
        std::memcpy(ubo.matrix, matrix.constData(), sizeof(ubo.matrix));
        ubo.parameters[0] = material->opacity;
        std::memcpy(buffer->data(), &ubo, sizeof(ubo));
        return true;
    }

    void updateSampledImage(RenderState& state, int binding, QSGTexture** texture, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        if (binding == 1) {
            static_cast<GradientFillMaterial*>(newMaterial)->gradientTexture.commit(state, binding, texture);
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
    const auto textureKey = gradientTexture.comparisonKey();
    const auto otherTextureKey = material->gradientTexture.comparisonKey();
    if (textureKey != otherTextureKey) {
        return textureKey < otherTextureKey ? -1 : 1;
    }
    return 0;
}

} // namespace QAccelPlot
