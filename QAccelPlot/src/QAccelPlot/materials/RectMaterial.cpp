//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/RectMaterial.hpp"

#include <QByteArray>
#include <QSGMaterialShader>

#include <cstring>

namespace QAccelPlot {

// Mirrors the std140 block in rect.vert. Members are ordered by alignment (mat4, vec4, vec2,
// float), so every offset is already aligned and the block needs no padding.
struct RectUbo {
    float matrix[16];      // 0–63
    float color[4];        // 64–79
    float borderColor[4];  // 80–95
    float hoverColor[4];   // 96–111
    float domainMin[2];    // 112–119
    float domainMax[2];    // 120–127
    float viewportSize[2]; // 128–135
    float minimumSize[2];  // 136–143
    float logScaleX;       // 144–147
    float logScaleY;       // 148–151
    float useVertexColor;  // 152–155
    float rectCount;       // 156–159
    float opacity;         // 160–163
    float borderWidth;     // 164–167
    float hoveredIndex;    // 168–171
};

static_assert(sizeof(RectUbo) == 172);

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
        if (!buf || buf->size() < static_cast<int>(sizeof(RectUbo))) {
            return false;
        }

        const auto* mat = static_cast<const RectMaterial*>(newMaterial);

        auto ubo = RectUbo{};
        const auto m = state.combinedMatrix();
        memcpy(ubo.matrix, m.constData(), sizeof(ubo.matrix));
        ubo.color[0] = float(mat->color.redF());
        ubo.color[1] = float(mat->color.greenF());
        ubo.color[2] = float(mat->color.blueF());
        ubo.color[3] = float(mat->color.alphaF());
        ubo.domainMin[0] = mat->domainMin.x();
        ubo.domainMin[1] = mat->domainMin.y();
        ubo.domainMax[0] = mat->domainMax.x();
        ubo.domainMax[1] = mat->domainMax.y();
        ubo.viewportSize[0] = mat->viewportSize.x();
        ubo.viewportSize[1] = mat->viewportSize.y();
        ubo.logScaleX = mat->logScaleX;
        ubo.logScaleY = mat->logScaleY;
        ubo.useVertexColor = mat->useVertexColor;
        ubo.rectCount = mat->rectCount;
        ubo.opacity = state.opacity();
        ubo.minimumSize[0] = mat->minimumSize.x();
        ubo.minimumSize[1] = mat->minimumSize.y();
        ubo.borderWidth = mat->borderWidth;
        ubo.borderColor[0] = float(mat->borderColor.redF());
        ubo.borderColor[1] = float(mat->borderColor.greenF());
        ubo.borderColor[2] = float(mat->borderColor.blueF());
        ubo.borderColor[3] = float(mat->borderColor.alphaF());
        ubo.hoverColor[0] = float(mat->hoverColor.redF());
        ubo.hoverColor[1] = float(mat->hoverColor.greenF());
        ubo.hoverColor[2] = float(mat->hoverColor.blueF());
        ubo.hoverColor[3] = float(mat->hoverColor.alphaF());
        ubo.hoveredIndex = mat->hoveredIndex;
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
