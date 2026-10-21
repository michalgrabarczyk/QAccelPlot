//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/ErrorBarMaterial.hpp"

#include <QByteArray>
#include <QSGMaterialShader>

#include <array>
#include <cstddef>
#include <cstring>
#include <iterator>

namespace QAccelPlot {

namespace {

// Must match the uniform block in error_bar.vert. The first 116 bytes are the DataTextureMaterial uniforms.
struct ErrorBarUbo {
    char common[116];    //   0–115
    float opacity;       // 116–119
    float hoveredIndex;  // 120–123
    float lineWidth;     // 124–127
    float hoverColor[4]; // 128–143
    float capLength;     // 144–147
    float offset;        // 148–151
    float horizontal;    // 152–155
    float errorBarCount; // 156–159
};

static_assert(offsetof(ErrorBarUbo, hoverColor) == 128);
static_assert(sizeof(ErrorBarUbo) == 160);

class ErrorBarShader : public QSGMaterialShader {
public:
    ErrorBarShader()
    {
        setShaderFileName(VertexStage, QString::fromLatin1(":/qaccelplot/shaders/error_bar.vert.qsb"));
        setShaderFileName(FragmentStage, QString::fromLatin1(":/qaccelplot/shaders/error_bar.frag.qsb"));
    }

    bool updateUniformData(RenderState& state, QSGMaterial* newMaterial, QSGMaterial* /*oldMaterial*/) override
    {
        auto* buf = state.uniformData();
        if (!buf || buf->size() < static_cast<int>(sizeof(ErrorBarUbo))) {
            return false;
        }
        const auto* mat = static_cast<const ErrorBarMaterial*>(newMaterial);
        auto ubo = ErrorBarUbo{};
        DataTextureMaterial::writeCommonUniforms(ubo.common, sizeof(ubo.common), state.combinedMatrix(), mat);
        ubo.opacity = state.opacity();
        ubo.hoveredIndex = mat->hoveredIndex;
        ubo.lineWidth = mat->lineWidth;
        ubo.hoverColor[0] = static_cast<float>(mat->hoverColor.redF());
        ubo.hoverColor[1] = static_cast<float>(mat->hoverColor.greenF());
        ubo.hoverColor[2] = static_cast<float>(mat->hoverColor.blueF());
        ubo.hoverColor[3] = static_cast<float>(mat->hoverColor.alphaF());
        ubo.capLength = mat->capLength;
        ubo.offset = mat->offset;
        ubo.horizontal = mat->horizontal;
        ubo.errorBarCount = mat->errorBarCount;
        memcpy(buf->data(), &ubo, sizeof(ubo));
        return true;
    }

    void updateSampledImage(RenderState& state, int binding, QSGTexture** texture, QSGMaterial* newMaterial, QSGMaterial* /*oldMaterial*/) override
    {
        const auto* mat = static_cast<const ErrorBarMaterial*>(newMaterial);
        DataTextureMaterial::commitTexture(state, binding, texture, mat->dataTexture.get());
    }
};

} // namespace

ErrorBarMaterial::ErrorBarMaterial() = default;

QSGMaterialType* ErrorBarMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader* ErrorBarMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new ErrorBarShader;
}

const QSGGeometry::AttributeSet& ErrorBarMaterial::attributeSet()
{
    static constexpr int kVertexStride{4};        // uchar4 placeholder; Metal needs a multiple of 4
    static constexpr int kAttrLocationPadding{0}; // layout(location = 0) in error_bar.vert

    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(kAttrLocationPadding, 4, QSGGeometry::UnsignedByteType),
    };
    static const QSGGeometry::AttributeSet attrSet = {static_cast<int>(std::size(attributes)), kVertexStride, attributes};
    return attrSet;
}

int ErrorBarMaterial::compareExtra(const QSGMaterial* other) const
{
    const auto* m = static_cast<const ErrorBarMaterial*>(other);
    if (hoverColor != m->hoverColor) {
        return hoverColor.rgba() < m->hoverColor.rgba() ? -1 : 1;
    }
    const auto mine = std::array<float, 6>{hoveredIndex, lineWidth, capLength, offset, horizontal, errorBarCount};
    const auto theirs = std::array<float, 6>{m->hoveredIndex, m->lineWidth, m->capLength, m->offset, m->horizontal, m->errorBarCount};
    for (std::size_t i = 0; i < mine.size(); ++i) {
        if (mine[i] != theirs[i]) {
            return mine[i] < theirs[i] ? -1 : 1;
        }
    }
    return 0;
}

} // namespace QAccelPlot
