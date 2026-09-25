//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/PointCloudMaterial.hpp"

#include <QByteArray>
#include <QSGGeometry>
#include <QSGMaterialShader>

#include <cstring>
#include <iterator>

namespace QAccelPlot {

namespace {

constexpr auto kCommonUniformSize = 116;

struct PointCloudUboTail {
    float markerSize;          // 116–119
    float antialiasingEnabled; // 120–123
    float antialiasingFeather; // 124–127
    float valueMin;            // 128–131
    float valueMax;            // 132–135
    float stride;              // 136–139
    int shapeType;             // 140–143
    float markerStrokeWidth;   // 144–147
    float markerFilled;        // 148–151
    float valueLogScale;       // 152–155
    float opacity;             // 156–159
};

constexpr auto kPointCloudUboSize = kCommonUniformSize + static_cast<int>(sizeof(PointCloudUboTail));

static_assert(sizeof(PointCloudUboTail) == 44);
static_assert(kPointCloudUboSize == 160);

int compareValues(const float left, const float right)
{
    if (left == right) {
        return 0;
    }
    return left < right ? -1 : 1;
}

class PointCloudShader : public QSGMaterialShader {
public:
    PointCloudShader()
    {
        setShaderFileName(VertexStage, QString::fromLatin1(":/qaccelplot/shaders/point_cloud.vert.qsb"));
        setShaderFileName(FragmentStage, QString::fromLatin1(":/qaccelplot/shaders/point_cloud.frag.qsb"));
    }

    bool updateUniformData(RenderState& state, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        auto* buffer = state.uniformData();
        if (!buffer || buffer->size() < kPointCloudUboSize) {
            return false;
        }

        const auto* material = static_cast<const PointCloudMaterial*>(newMaterial);
        if (!DataTextureMaterial::writeCommonUniforms(buffer->data(), static_cast<int>(buffer->size()), state.combinedMatrix(), material)) {
            return false;
        }

        auto tail = PointCloudUboTail{};
        tail.markerSize = material->markerSize;
        tail.antialiasingEnabled = material->antialiasingEnabled;
        tail.antialiasingFeather = material->antialiasingFeather;
        tail.valueMin = material->valueMin;
        tail.valueMax = material->valueMax;
        tail.stride = material->stride;
        tail.shapeType = material->shapeType;
        tail.markerStrokeWidth = material->markerStrokeWidth;
        tail.markerFilled = material->markerFilled;
        tail.valueLogScale = material->valueLogScale;
        tail.opacity = state.opacity();
        std::memcpy(buffer->data() + kCommonUniformSize, &tail, sizeof(tail));
        return true;
    }

    void updateSampledImage(RenderState& state, const int binding, QSGTexture** texture, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        auto* material = static_cast<PointCloudMaterial*>(newMaterial);
        if (binding == 1) {
            DataTextureMaterial::commitTexture(state, binding, texture, material->dataTexture.get());
        } else if (binding == 2) {
            material->colorMap.commit(state, binding, texture);
        }
    }
};

} // namespace

PointCloudMaterial::PointCloudMaterial() = default;

QSGMaterialType* PointCloudMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader* PointCloudMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new PointCloudShader;
}

const QSGGeometry::AttributeSet& PointCloudMaterial::attributeSet()
{
    static constexpr int kVertexStride{8};       // float pointId (4) + float corner (4)
    static constexpr int kAttrLocationId{0};     // layout(location = 0) in point_cloud.vert
    static constexpr int kAttrLocationCorner{1}; // layout(location = 1)

    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(kAttrLocationId, 1, QSGGeometry::FloatType),
        QSGGeometry::Attribute::create(kAttrLocationCorner, 1, QSGGeometry::FloatType),
    };
    static const QSGGeometry::AttributeSet attrSet = {static_cast<int>(std::size(attributes)), kVertexStride, attributes};
    return attrSet;
}

int PointCloudMaterial::compareExtra(const QSGMaterial* other) const
{
    const auto* material = static_cast<const PointCloudMaterial*>(other);
    const auto fields = {
        compareValues(markerSize, material->markerSize),
        compareValues(antialiasingEnabled, material->antialiasingEnabled),
        compareValues(antialiasingFeather, material->antialiasingFeather),
        compareValues(valueMin, material->valueMin),
        compareValues(valueMax, material->valueMax),
        compareValues(stride, material->stride),
        compareValues(markerStrokeWidth, material->markerStrokeWidth),
        compareValues(markerFilled, material->markerFilled),
        compareValues(valueLogScale, material->valueLogScale),
    };
    for (const auto result : fields) {
        if (result != 0) {
            return result;
        }
    }
    if (shapeType != material->shapeType) {
        return shapeType < material->shapeType ? -1 : 1;
    }
    const auto colorMapKey = colorMap.comparisonKey();
    const auto otherColorMapKey = material->colorMap.comparisonKey();
    if (colorMapKey != otherColorMapKey) {
        return colorMapKey < otherColorMapKey ? -1 : 1;
    }
    return 0;
}

} // namespace QAccelPlot
