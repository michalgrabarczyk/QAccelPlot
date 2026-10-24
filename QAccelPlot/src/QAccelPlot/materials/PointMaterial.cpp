//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/PointMaterial.hpp"

#include <QByteArray>
#include <QSGMaterialShader>

#include <cstring>

namespace QAccelPlot {

namespace {

constexpr auto kCommonUniformSize = 116;

struct PointUboTail {
    float markerSize;          // 116–119
    float antialiasingEnabled; // 120–123
    float antialiasingFeather; // 124–127
    int shapeType;             // 128–131
    float markerStrokeWidth;   // 132–135
    float markerFilled;        // 136–139
    float gradientDirection;   // 140–143
    float gradientValueMin;    // 144–147
    float gradientValueMax;    // 148–151
    float opacity;             // 152–155
};

constexpr auto kPointUboSize = kCommonUniformSize + static_cast<int>(sizeof(PointUboTail));

static_assert(sizeof(PointUboTail) == 40);
static_assert(kPointUboSize == 156);

int compareValues(const float left, const float right)
{
    if (left == right) {
        return 0;
    }
    return left < right ? -1 : 1;
}

class PointShader : public QSGMaterialShader {
public:
    PointShader()
    {
        setShaderFileName(VertexStage, QString::fromLatin1(":/qaccelplot/shaders/point.vert.qsb"));
        setShaderFileName(FragmentStage, QString::fromLatin1(":/qaccelplot/shaders/point.frag.qsb"));
    }

    bool updateUniformData(RenderState& state, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        auto* buffer = state.uniformData();
        if (!buffer || buffer->size() < kPointUboSize) {
            return false;
        }

        const auto* material = static_cast<const PointMaterial*>(newMaterial);
        if (!DataTextureMaterial::writeCommonUniforms(buffer->data(), static_cast<int>(buffer->size()), state.combinedMatrix(), material)) {
            return false;
        }

        auto tail = PointUboTail{};
        tail.markerSize = material->markerSize;
        tail.antialiasingEnabled = material->antialiasingEnabled;
        tail.antialiasingFeather = material->antialiasingFeather;
        tail.shapeType = material->shapeType;
        tail.markerStrokeWidth = material->markerStrokeWidth;
        tail.markerFilled = material->markerFilled;
        tail.gradientDirection = material->gradientDirection;
        tail.gradientValueMin = material->gradientValueMin;
        tail.gradientValueMax = material->gradientValueMax;
        tail.opacity = state.opacity();
        std::memcpy(buffer->data() + kCommonUniformSize, &tail, sizeof(tail));
        return true;
    }

    void updateSampledImage(RenderState& state, const int binding, QSGTexture** texture, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        auto* material = static_cast<PointMaterial*>(newMaterial);
        if (binding == 1) {
            DataTextureMaterial::commitTexture(state, binding, texture, material->dataTexture.get());
        } else if (binding == 2) {
            material->gradientTexture.commit(state, binding, texture);
        }
    }
};

} // namespace

PointMaterial::PointMaterial() = default;

QSGMaterialType* PointMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader* PointMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new PointShader;
}

int PointMaterial::compareExtra(const QSGMaterial* other) const
{
    const auto* material = static_cast<const PointMaterial*>(other);
    if (shapeType != material->shapeType) {
        return shapeType < material->shapeType ? -1 : 1;
    }
    const auto fields = {
        compareValues(markerSize, material->markerSize),
        compareValues(antialiasingEnabled, material->antialiasingEnabled),
        compareValues(antialiasingFeather, material->antialiasingFeather),
        compareValues(markerStrokeWidth, material->markerStrokeWidth),
        compareValues(markerFilled, material->markerFilled),
        compareValues(gradientDirection, material->gradientDirection),
        compareValues(gradientValueMin, material->gradientValueMin),
        compareValues(gradientValueMax, material->gradientValueMax),
    };
    for (const auto result : fields) {
        if (result != 0) {
            return result;
        }
    }
    const auto textureKey = gradientTexture.comparisonKey();
    const auto otherTextureKey = material->gradientTexture.comparisonKey();
    if (textureKey != otherTextureKey) {
        return textureKey < otherTextureKey ? -1 : 1;
    }
    return 0;
}

} // namespace QAccelPlot
