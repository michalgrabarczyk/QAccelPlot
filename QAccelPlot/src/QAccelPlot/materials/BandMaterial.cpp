//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/BandMaterial.hpp"

#include <QByteArray>
#include <QSGMaterialShader>

#include <cstring>
#include <iterator>

namespace QAccelPlot {

// Mirrors the std140 block in band.vert.
struct BandUbo {
    float matrix[16];      // 0–63
    float color[4];        // 64–79
    float domainMin[2];    // 80–87
    float domainMax[2];    // 88–95
    float viewportSize[2]; // 96–103
    float logScaleX;       // 104–107
    float logScaleY;       // 108–111
    float sampleCount;     // 112–115
    float opacity;         // 116–119
};

static_assert(sizeof(BandUbo) == 120);
static_assert(sizeof(BandMaterial::Vertex) == 8);

class BandShader : public QSGMaterialShader {
public:
    BandShader()
    {
        setShaderFileName(VertexStage, QString::fromLatin1(":/qaccelplot/shaders/band.vert.qsb"));
        setShaderFileName(FragmentStage, QString::fromLatin1(":/qaccelplot/shaders/band.frag.qsb"));
    }

    bool updateUniformData(RenderState& state, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        QByteArray* buf = state.uniformData();
        if (!buf || buf->size() < static_cast<int>(sizeof(BandUbo))) {
            return false;
        }

        const auto* mat = static_cast<const BandMaterial*>(newMaterial);

        auto ubo = BandUbo{};
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
        ubo.sampleCount = mat->sampleCount;
        ubo.opacity = state.opacity();
        memcpy(buf->data(), &ubo, sizeof(ubo));
        return true;
    }

    void updateSampledImage(RenderState& state, int binding, QSGTexture** texture, QSGMaterial* newMaterial, QSGMaterial* /*oldMaterial*/) override
    {
        if (binding == 1) {
            auto* mat = static_cast<BandMaterial*>(newMaterial);
            DataTextureMaterial::commitTexture(state, binding, texture, mat->dataTexture.get());
        }
    }
};

BandMaterial::BandMaterial() = default;

QSGMaterialType* BandMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader* BandMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new BandShader;
}

const QSGGeometry::AttributeSet& BandMaterial::vertexAttributes()
{
    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(0, 1, QSGGeometry::FloatType),
        QSGGeometry::Attribute::create(1, 1, QSGGeometry::FloatType),
    };
    static const QSGGeometry::AttributeSet attributeSet = {static_cast<int>(std::size(attributes)), static_cast<int>(sizeof(Vertex)), attributes};
    return attributeSet;
}

int BandMaterial::compareExtra(const QSGMaterial* other) const
{
    const auto* m = static_cast<const BandMaterial*>(other);
    if (sampleCount != m->sampleCount) {
        return sampleCount < m->sampleCount ? -1 : 1;
    }
    return 0;
}

} // namespace QAccelPlot
