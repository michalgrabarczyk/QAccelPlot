//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/BarMaterial.hpp"

#include "QAccelPlot/materials/internal/RectUniforms.hpp"

#include <QByteArray>
#include <QSGMaterialShader>

#include <array>
#include <cstring>

namespace QAccelPlot {

namespace {

// Mirrors the std140 block in bar.vert: the rect.vert block followed by the bar geometry.
struct BarUbo {
    Internal::RectUbo rect; // 0–171
    float barWidth;         // 172–175
    float barOffset;        // 176–179
    float baseline;         // 180–183
    float horizontal;       // 184–187
};

static_assert(sizeof(BarUbo) == 188);

class BarShader : public QSGMaterialShader {
public:
    BarShader()
    {
        setShaderFileName(VertexStage, QString::fromLatin1(":/qaccelplot/shaders/bar.vert.qsb"));
        setShaderFileName(FragmentStage, QString::fromLatin1(":/qaccelplot/shaders/rect.frag.qsb"));
    }

    bool updateUniformData(RenderState& state, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        QByteArray* buf = state.uniformData();
        if (!buf || buf->size() < static_cast<int>(sizeof(BarUbo))) {
            return false;
        }

        const auto* mat = static_cast<const BarMaterial*>(newMaterial);
        auto ubo = BarUbo{};
        Internal::writeRectUniforms(ubo.rect, state, *mat);
        ubo.barWidth = mat->barWidth;
        ubo.barOffset = mat->barOffset;
        ubo.baseline = mat->baseline;
        ubo.horizontal = mat->horizontal;
        memcpy(buf->data(), &ubo, sizeof(ubo));
        return true;
    }

    void updateSampledImage(RenderState& state, int binding, QSGTexture** texture, QSGMaterial* newMaterial, QSGMaterial* /*oldMaterial*/) override
    {
        if (binding == 1) {
            auto* mat = static_cast<BarMaterial*>(newMaterial);
            DataTextureMaterial::commitTexture(state, binding, texture, mat->dataTexture.get());
        }
    }
};

}

BarMaterial::BarMaterial() = default;

QSGMaterialType* BarMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader* BarMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new BarShader;
}

int BarMaterial::compareExtra(const QSGMaterial* other) const
{
    const auto rectResult = RectMaterial::compareExtra(other);
    if (rectResult != 0) {
        return rectResult;
    }
    const auto* m = static_cast<const BarMaterial*>(other);
    const auto mine = std::array<float, 4>{barWidth, barOffset, baseline, horizontal};
    const auto theirs = std::array<float, 4>{m->barWidth, m->barOffset, m->baseline, m->horizontal};
    for (size_t i = 0; i < mine.size(); ++i) {
        if (mine[i] != theirs[i]) {
            return mine[i] < theirs[i] ? -1 : 1;
        }
    }
    return 0;
}

} // namespace QAccelPlot
