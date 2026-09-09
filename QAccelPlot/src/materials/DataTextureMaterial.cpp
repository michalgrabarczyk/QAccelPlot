//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "materials/DataTextureMaterial.hpp"
#include "materials/internal/DataTextureUpload.hpp"

#include <QColorSpace>
#include <QImage>
#include <QSGGeometry>

#include <cstring>
#include <iterator>

namespace QAccelPlot {

constexpr int kTextureWidth{2048};
constexpr int kCommonUniformSize{116};

struct CommonUbo {
    float matrix[16];      // 0–63
    float color[4];        // 64–79
    float domainMin[2];    // 80–87
    float domainMax[2];    // 88–95
    float viewportSize[2]; // 96–103
    float logScaleX;       // 104–107
    float logScaleY;       // 108–111
    float useVertexColor;  // 112–115
};

static_assert(sizeof(CommonUbo) == kCommonUniformSize);

DataTextureMaterial::DataTextureMaterial()
{
    setFlag(QSGMaterial::Blending | QSGMaterial::RequiresFullMatrix);
}

int DataTextureMaterial::compare(const QSGMaterial* other) const
{
    const auto* m = static_cast<const DataTextureMaterial*>(other);
    if (color != m->color) {
        return color.rgba() < m->color.rgba() ? -1 : 1;
    }
    if (domainMin != m->domainMin) {
        return domainMin.x() != m->domainMin.x() ? (domainMin.x() < m->domainMin.x() ? -1 : 1) : (domainMin.y() < m->domainMin.y() ? -1 : 1);
    }
    if (domainMax != m->domainMax) {
        return domainMax.x() != m->domainMax.x() ? (domainMax.x() < m->domainMax.x() ? -1 : 1) : (domainMax.y() < m->domainMax.y() ? -1 : 1);
    }
    if (viewportSize != m->viewportSize) {
        return viewportSize.x() != m->viewportSize.x() ? (viewportSize.x() < m->viewportSize.x() ? -1 : 1) : (viewportSize.y() < m->viewportSize.y() ? -1 : 1);
    }
    if (logScaleX != m->logScaleX) {
        return logScaleX < m->logScaleX ? -1 : 1;
    }
    if (logScaleY != m->logScaleY) {
        return logScaleY < m->logScaleY ? -1 : 1;
    }
    if (useVertexColor != m->useVertexColor) {
        return useVertexColor < m->useVertexColor ? -1 : 1;
    }

    const auto textureKey = dataTexture ? dataTexture->comparisonKey() : qint64{};
    const auto otherTextureKey = m->dataTexture ? m->dataTexture->comparisonKey() : qint64{};
    if (textureKey != otherTextureKey) {
        return textureKey < otherTextureKey ? -1 : 1;
    }
    return compareExtra(other);
}

bool DataTextureMaterial::writeCommonUniforms(char* buf, const int bufSize, const QMatrix4x4& matrix, const DataTextureMaterial* mat)
{
    if (bufSize < kCommonUniformSize) {
        return false;
    }

    auto ubo = CommonUbo{};
    memcpy(ubo.matrix, matrix.constData(), sizeof(ubo.matrix));
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
    memcpy(buf, &ubo, sizeof(ubo));
    return true;
}

void DataTextureMaterial::uploadTexture(std::unique_ptr<QSGTexture>& texture, QQuickWindow* window, const float* data, const int floatCount)
{
    if (!window || floatCount <= 0) {
        return;
    }

    const auto texHeight = (floatCount + kTextureWidth - 1) / kTextureWidth;

    // Reuse QImage storage across frames. The upload helper either updates the
    // scene-graph texture in place or recreates it through public Qt API,
    // depending on the configured build mode.
    const auto dimensionsChanged = imageBuffer_.width() != kTextureWidth || imageBuffer_.height() != texHeight;
    if (dimensionsChanged) {
        imageBuffer_ = QImage(kTextureWidth, texHeight, QImage::Format_RGBA8888_Premultiplied);
        imageBuffer_.setColorSpace(QColorSpace());
    }

    if (imageBuffer_.isNull()) {
        return;
    }

    const auto dataBytes = static_cast<size_t>(floatCount) * sizeof(float);
    memcpy(imageBuffer_.bits(), data, dataBytes);

    if (dimensionsChanged) {
        const auto tailBytes = imageBuffer_.sizeInBytes() - static_cast<qsizetype>(dataBytes);
        if (tailBytes > 0) {
            memset(imageBuffer_.bits() + dataBytes, 0, static_cast<size_t>(tailBytes));
        }
    }

    Internal::uploadDataTexture(texture, window, imageBuffer_);
}

void DataTextureMaterial::commitTexture(QSGMaterialShader::RenderState& state, const int binding, QSGTexture** texture, QSGTexture* dataTexture)
{
    if (binding != 1 || !dataTexture) {
        return;
    }

    auto* rhi = state.rhi();
    auto* resourceUpdateBatch = state.resourceUpdateBatch();
    if (!rhi || !resourceUpdateBatch) {
        *texture = nullptr;
        return;
    }

    dataTexture->commitTextureOperations(rhi, resourceUpdateBatch);
    *texture = dataTexture;
}

const QSGGeometry::AttributeSet& DataTextureMaterial::attributeSet()
{
    static constexpr int kVertexStride{12};     // float id (4) + float param (4) + uchar4 rgba (4)
    static constexpr int kAttrLocationId{0};    // layout(location = 0) in the vertex shader
    static constexpr int kAttrLocationParam{1}; // layout(location = 1)
    static constexpr int kAttrLocationColor{2}; // layout(location = 2)
    static constexpr int kComponentsId{1};      // 1 float
    static constexpr int kComponentsParam{1};   // 1 float
    static constexpr int kComponentsColor{4};   // 4 bytes: RGBA

    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(kAttrLocationId, kComponentsId, QSGGeometry::FloatType),
        QSGGeometry::Attribute::create(kAttrLocationParam, kComponentsParam, QSGGeometry::FloatType),
        QSGGeometry::Attribute::createWithAttributeType(kAttrLocationColor, kComponentsColor, QSGGeometry::UnsignedByteType, QSGGeometry::ColorAttribute),
    };

    static const QSGGeometry::AttributeSet attrSet = {static_cast<int>(std::size(attributes)), kVertexStride, attributes};
    return attrSet;
}

int DataTextureMaterial::compareExtra(const QSGMaterial* /*other*/) const
{
    return 0;
}

} // namespace QAccelPlot
