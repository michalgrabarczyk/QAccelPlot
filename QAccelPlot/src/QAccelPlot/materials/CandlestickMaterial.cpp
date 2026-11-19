//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/materials/CandlestickMaterial.hpp"

#include "QAccelPlot/materials/internal/GlowUniforms.hpp"
#include "QAccelPlot/materials/internal/ItemOpacityUniforms.hpp"

#include <QByteArray>
#include <QSGMaterialShader>

#include <array>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <utility>

namespace QAccelPlot {

namespace {

// Must match the uniform block in candlestick.vert. The first 116 bytes are the DataTextureMaterial uniforms.
struct CandlestickUbo {
    char common[116];                          //   0–115
    float opacity;                             // 116–119
    float hoveredIndex;                        // 120–123
    float lineWidth;                           // 124–127
    float hoverColor[4];                       // 128–143
    float downColor[4];                        // 144–159
    float halfWidth;                           // 160–163
    float minimumWidth;                        // 164–167
    float ohlc;                                // 168–171
    float candleCount;                         // 172–175
    float antialiasing;                        // 176–179
    float padding[3];                          // 180–191
    Internal::ItemOpacityUniforms itemOpacity; // 192–719
    Internal::GlowUniforms glow;               // 720–751
    float glowDownColor[4];                    // 752–767
};

static_assert(offsetof(CandlestickUbo, hoverColor) == 128);
static_assert(offsetof(CandlestickUbo, itemOpacity) == 192);
static_assert(sizeof(CandlestickUbo) == 768);

void writeColor(float (&out)[4], const QColor& color)
{
    out[0] = static_cast<float>(color.redF());
    out[1] = static_cast<float>(color.greenF());
    out[2] = static_cast<float>(color.blueF());
    out[3] = static_cast<float>(color.alphaF());
}

class CandlestickShader : public QSGMaterialShader {
public:
    CandlestickShader()
    {
        setShaderFileName(VertexStage, QString::fromLatin1(":/qaccelplot/shaders/candlestick.vert.qsb"));
        // Candle parts are quads like error bar lines and caps, so they share the fragment shader.
        setShaderFileName(FragmentStage, QString::fromLatin1(":/qaccelplot/shaders/error_bar.frag.qsb"));
    }

    bool updateUniformData(RenderState& state, QSGMaterial* newMaterial, [[maybe_unused]] QSGMaterial* oldMaterial) override
    {
        auto* buf = state.uniformData();
        if (!buf || buf->size() < static_cast<int>(sizeof(CandlestickUbo))) {
            return false;
        }
        const auto* mat = static_cast<const CandlestickMaterial*>(newMaterial);
        auto ubo = CandlestickUbo{};
        DataTextureMaterial::writeCommonUniforms(ubo.common, sizeof(ubo.common), state.combinedMatrix(), mat);
        ubo.opacity = state.opacity();
        ubo.hoveredIndex = mat->hoveredIndex;
        ubo.lineWidth = mat->lineWidth;
        writeColor(ubo.hoverColor, mat->hoverColor);
        writeColor(ubo.downColor, mat->downColor);
        ubo.halfWidth = mat->halfWidth;
        ubo.minimumWidth = mat->minimumWidth;
        ubo.ohlc = mat->ohlc;
        ubo.candleCount = mat->candleCount;
        ubo.antialiasing = mat->antialiasing;
        ubo.itemOpacity = Internal::itemOpacityUniforms(mat->itemFade, mat->itemTrail);
        ubo.glow = Internal::glowUniforms(mat->glow);
        writeColor(ubo.glowDownColor, mat->glowDownColor);
        memcpy(buf->data(), &ubo, sizeof(ubo));
        return true;
    }

    void updateSampledImage(RenderState& state, int binding, QSGTexture** texture, QSGMaterial* newMaterial, [[maybe_unused]] QSGMaterial* oldMaterial) override
    {
        const auto* mat = static_cast<const CandlestickMaterial*>(newMaterial);
        DataTextureMaterial::commitTexture(state, binding, texture, mat->dataTexture.get());
    }
};

int compareColors(const QColor& left, const QColor& right)
{
    if (left == right) {
        return 0;
    }
    return left.rgba64() < right.rgba64() ? -1 : 1;
}

} // namespace

CandlestickMaterial::CandlestickMaterial() = default;

QSGMaterialType* CandlestickMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader* CandlestickMaterial::createShader([[maybe_unused]] QSGRendererInterface::RenderMode renderMode) const
{
    return new CandlestickShader;
}

const QSGGeometry::AttributeSet& CandlestickMaterial::attributeSet()
{
    static constexpr int kVertexStride{4};        // uchar4 placeholder; Metal needs a multiple of 4
    static constexpr int kAttrLocationPadding{0}; // layout(location = 0) in candlestick.vert

    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(kAttrLocationPadding, 4, QSGGeometry::UnsignedByteType),
    };
    static const QSGGeometry::AttributeSet attrSet = {static_cast<int>(std::size(attributes)), kVertexStride, attributes};
    return attrSet;
}

int CandlestickMaterial::compareExtra(const QSGMaterial* other) const
{
    const auto* m = static_cast<const CandlestickMaterial*>(other);
    for (const auto& [mine, theirs] : {std::pair{downColor, m->downColor}, std::pair{hoverColor, m->hoverColor}, std::pair{glowDownColor, m->glowDownColor}}) {
        if (const auto order = compareColors(mine, theirs)) {
            return order;
        }
    }
    const auto mine = std::array<float, 7>{hoveredIndex, lineWidth, halfWidth, minimumWidth, ohlc, candleCount, antialiasing};
    const auto theirs = std::array<float, 7>{m->hoveredIndex, m->lineWidth, m->halfWidth, m->minimumWidth, m->ohlc, m->candleCount, m->antialiasing};
    for (std::size_t i = 0; i < mine.size(); ++i) {
        if (mine[i] != theirs[i]) {
            return mine[i] < theirs[i] ? -1 : 1;
        }
    }
    return 0;
}

} // namespace QAccelPlot
