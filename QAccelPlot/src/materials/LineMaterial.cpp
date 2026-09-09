//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "materials/LineMaterial.hpp"

#include <QByteArray>
#include <QMatrix4x4>
#include <QSGMaterialShader>

#include <algorithm>
#include <cstring>

namespace QAccelPlot {

// NOTE: line.vert has a different UBO layout from CommonUbo — lineWidth sits at
// offset 104 (before logScaleX), so we define a dedicated struct here instead of
// reusing DataTextureMaterial::writeCommonUniforms().
struct LineUbo {
    float matrix[16];          // 0–63
    float color[4];            // 64–79
    float domainMin[2];        // 80–87
    float domainMax[2];        // 88–95
    float viewportSize[2];     // 96–103
    float lineWidth;           // 104–107
    float logScaleX;           // 108–111
    float logScaleY;           // 112–115
    float useVertexColor;      // 116–119
    float pointCount;          // 120–123
    float antialiasingEnabled; // 124–127
    float antialiasingFeather; // 128–131
    float dashPeriod;          // 132–135
    float dashOffset;          // 136–139
    int dashPatternSize;       // 140–143
    float dashPattern[8];      // 144–175
};

static_assert(sizeof(LineUbo) == 176);

class LineShader : public QSGMaterialShader {
public:
    LineShader()
    {
        setShaderFileName(VertexStage, QString::fromLatin1(":/qaccelplot/src/shaders/line.vert.qsb"));
        setShaderFileName(FragmentStage, QString::fromLatin1(":/qaccelplot/src/shaders/line.frag.qsb"));
    }

    bool updateUniformData(RenderState& state, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        QByteArray* buf = state.uniformData();
        if (!buf || buf->size() < static_cast<int>(sizeof(LineUbo))) {
            return false;
        }

        const auto* mat = static_cast<const LineMaterial*>(newMaterial);

        auto ubo = LineUbo{};
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
        ubo.lineWidth = mat->lineWidth;
        ubo.logScaleX = mat->logScaleX;
        ubo.logScaleY = mat->logScaleY;
        ubo.useVertexColor = mat->useVertexColor;
        ubo.pointCount = mat->pointCount;
        ubo.antialiasingEnabled = mat->antialiasingEnabled;
        ubo.antialiasingFeather = mat->antialiasingFeather;
        ubo.dashPeriod = mat->dashPeriod;
        ubo.dashOffset = mat->dashOffset;
        ubo.dashPatternSize = mat->dashPatternSize;
        memcpy(ubo.dashPattern, mat->dashPattern, sizeof(ubo.dashPattern));
        memcpy(buf->data(), &ubo, sizeof(ubo));
        return true;
    }

    void updateSampledImage(RenderState& state, int binding, QSGTexture** texture, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        if (binding == 1) {
            auto* mat = static_cast<LineMaterial*>(newMaterial);
            DataTextureMaterial::commitTexture(state, binding, texture, mat->dataTexture.get());
        }
    }
};

LineMaterial::LineMaterial() = default;

QSGMaterialType* LineMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader* LineMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new LineShader;
}

int LineMaterial::compareExtra(const QSGMaterial* other) const
{
    const auto* m = static_cast<const LineMaterial*>(other);
    if (lineWidth != m->lineWidth) {
        return lineWidth < m->lineWidth ? -1 : 1;
    }
    if (pointCount != m->pointCount) {
        return pointCount < m->pointCount ? -1 : 1;
    }
    if (antialiasingEnabled != m->antialiasingEnabled) {
        return antialiasingEnabled < m->antialiasingEnabled ? -1 : 1;
    }
    if (antialiasingFeather != m->antialiasingFeather) {
        return antialiasingFeather < m->antialiasingFeather ? -1 : 1;
    }
    if (dashPatternSize != m->dashPatternSize) {
        return dashPatternSize < m->dashPatternSize ? -1 : 1;
    }
    if (dashPeriod != m->dashPeriod) {
        return dashPeriod < m->dashPeriod ? -1 : 1;
    }
    if (dashOffset != m->dashOffset) {
        return dashOffset < m->dashOffset ? -1 : 1;
    }
    const auto activePatternSize = std::clamp(dashPatternSize, 0, 8);
    for (auto index = int{0}; index < activePatternSize; ++index) {
        if (dashPattern[index] != m->dashPattern[index]) {
            return dashPattern[index] < m->dashPattern[index] ? -1 : 1;
        }
    }
    return 0;
}

} // namespace QAccelPlot
