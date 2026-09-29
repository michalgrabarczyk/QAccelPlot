//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/RectMaterial.hpp"

#include "QAccelPlot/materials/internal/RectUniforms.hpp"

#include <QByteArray>
#include <QSGMaterialShader>

#include <cstring>

namespace QAccelPlot {

class RectShader : public QSGMaterialShader {
public:
    RectShader()
    {
        setShaderFileName(VertexStage, QString::fromLatin1(":/qaccelplot/shaders/rect.vert.qsb"));
        setShaderFileName(FragmentStage, QString::fromLatin1(":/qaccelplot/shaders/rect.frag.qsb"));
    }

    bool updateUniformData(RenderState& state, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        QByteArray* buf = state.uniformData();
        if (!buf || buf->size() < static_cast<int>(sizeof(Internal::RectUbo))) {
            return false;
        }

        auto ubo = Internal::RectUbo{};
        Internal::writeRectUniforms(ubo, state, *static_cast<const RectMaterial*>(newMaterial));
        memcpy(buf->data(), &ubo, sizeof(ubo));
        return true;
    }

    void updateSampledImage(RenderState& state, int binding, QSGTexture** texture, QSGMaterial* newMaterial, QSGMaterial* /*oldMaterial*/) override
    {
        if (binding == 1) {
            auto* mat = static_cast<RectMaterial*>(newMaterial);
            DataTextureMaterial::commitTexture(state, binding, texture, mat->dataTexture.get());
        }
    }
};

RectMaterial::RectMaterial() = default;

QSGMaterialType* RectMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader* RectMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new RectShader;
}

int RectMaterial::compareExtra(const QSGMaterial* other) const
{
    const auto* m = static_cast<const RectMaterial*>(other);
    if (rectCount != m->rectCount) {
        return rectCount < m->rectCount ? -1 : 1;
    }
    for (auto i = 0; i < 2; ++i) {
        if (minimumSize[i] != m->minimumSize[i]) {
            return minimumSize[i] < m->minimumSize[i] ? -1 : 1;
        }
    }
    if (borderWidth != m->borderWidth) {
        return borderWidth < m->borderWidth ? -1 : 1;
    }
    if (borderColor != m->borderColor) {
        return borderColor.rgba() < m->borderColor.rgba() ? -1 : 1;
    }
    if (hoverColor != m->hoverColor) {
        return hoverColor.rgba() < m->hoverColor.rgba() ? -1 : 1;
    }
    if (hoveredIndex != m->hoveredIndex) {
        return hoveredIndex < m->hoveredIndex ? -1 : 1;
    }
    return 0;
}

} // namespace QAccelPlot
