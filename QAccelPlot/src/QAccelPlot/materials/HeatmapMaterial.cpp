//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/HeatmapMaterial.hpp"

#include "QAccelPlot/materials/DataTextureMaterial.hpp"

#include <QByteArray>
#include <QSGMaterialShader>
#include <QSGTexture>

#include <cstring>

namespace QAccelPlot {

namespace {

struct HeatmapUbo {
    float matrix[16];    // 0–63
    float cellScale[2];  // 64–71
    float cellOffset[2]; // 72–79
    float valueMin;      // 80–83
    float valueMax;      // 84–87
    float valueLogScale; // 88–91
    float opacity;       // 92–95
    int columns;         // 96–99
    int rows;            // 100–103
};

static_assert(sizeof(HeatmapUbo) == 104);

template <typename T> int compareValues(const T left, const T right)
{
    if (left == right) {
        return 0;
    }
    return left < right ? -1 : 1;
}

qint64 textureKey(const DataTexture& texture)
{
    const auto* sampled = texture.texture();
    return sampled ? sampled->comparisonKey() : qint64{};
}

class HeatmapShader : public QSGMaterialShader {
public:
    HeatmapShader()
    {
        setShaderFileName(VertexStage, QString::fromLatin1(":/qaccelplot/shaders/heatmap.vert.qsb"));
        setShaderFileName(FragmentStage, QString::fromLatin1(":/qaccelplot/shaders/heatmap.frag.qsb"));
    }

    bool updateUniformData(RenderState& state, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        auto* buffer = state.uniformData();
        if (!buffer || buffer->size() < static_cast<qsizetype>(sizeof(HeatmapUbo))) {
            return false;
        }

        const auto* material = static_cast<const HeatmapMaterial*>(newMaterial);
        auto ubo = HeatmapUbo{};
        std::memcpy(ubo.matrix, state.combinedMatrix().constData(), sizeof(ubo.matrix));
        ubo.cellScale[0] = material->cellScale.x();
        ubo.cellScale[1] = material->cellScale.y();
        ubo.cellOffset[0] = material->cellOffset.x();
        ubo.cellOffset[1] = material->cellOffset.y();
        ubo.valueMin = material->valueMin;
        ubo.valueMax = material->valueMax;
        ubo.valueLogScale = material->valueLogScale;
        ubo.opacity = state.opacity();
        ubo.columns = material->columns;
        ubo.rows = material->rows;
        std::memcpy(buffer->data(), &ubo, sizeof(ubo));
        return true;
    }

    void updateSampledImage(RenderState& state, const int binding, QSGTexture** texture, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        auto* material = static_cast<HeatmapMaterial*>(newMaterial);
        if (binding == 1) {
            DataTextureMaterial::commitTexture(state, binding, texture, &material->values);
        } else if (binding == 2) {
            material->colorMap.commit(state, binding, texture);
        }
    }
};

} // namespace

HeatmapMaterial::HeatmapMaterial()
{
    // A batchable node would need a batchable vertex shader variant, which the shaders are not built with.
    setFlag(QSGMaterial::Blending | QSGMaterial::RequiresFullMatrix);
}

QSGMaterialType* HeatmapMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader* HeatmapMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new HeatmapShader;
}

int HeatmapMaterial::compare(const QSGMaterial* other) const
{
    const auto* material = static_cast<const HeatmapMaterial*>(other);
    const auto fields = {
        compareValues(cellScale.x(), material->cellScale.x()),
        compareValues(cellScale.y(), material->cellScale.y()),
        compareValues(cellOffset.x(), material->cellOffset.x()),
        compareValues(cellOffset.y(), material->cellOffset.y()),
        compareValues(valueMin, material->valueMin),
        compareValues(valueMax, material->valueMax),
        compareValues(valueLogScale, material->valueLogScale),
        compareValues(columns, material->columns),
        compareValues(rows, material->rows),
        compareValues(textureKey(values), textureKey(material->values)),
        compareValues(colorMap.comparisonKey(), material->colorMap.comparisonKey()),
    };
    for (const auto result : fields) {
        if (result != 0) {
            return result;
        }
    }
    return 0;
}

} // namespace QAccelPlot
