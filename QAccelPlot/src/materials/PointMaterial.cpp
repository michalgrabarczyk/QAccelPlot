//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "materials/PointMaterial.hpp"

#include <QByteArray>
#include <QMatrix4x4>
#include <QSGMaterialShader>

namespace QAccelPlot {

struct PointUbo {
    float matrix[16];          // 0–63
    float color[4];            // 64–79
    float domainMin[2];        // 80–87
    float domainMax[2];        // 88–95
    float viewportSize[2];     // 96–103
    float markerSize;          // 104–107
    float logScaleX;           // 108–111
    float logScaleY;           // 112–115
    float useVertexColor;      // 116–119
    float antialiasingEnabled; // 120–123
    float antialiasingFeather; // 124–127
    int shapeType;             // 128–131
};

static_assert(sizeof(PointUbo) == 132);

class PointShader : public QSGMaterialShader {
public:
    PointShader()
    {
        setShaderFileName(VertexStage, QString::fromLatin1(":/qaccelplot/src/shaders/point.vert.qsb"));
        setShaderFileName(FragmentStage, QString::fromLatin1(":/qaccelplot/src/shaders/point.frag.qsb"));
    }

    bool updateUniformData(RenderState& state, QSGMaterial* newMaterial, QSGMaterial* oldMaterial) override
    {
        Q_UNUSED(oldMaterial)
        QByteArray* buf = state.uniformData();

        if (!buf || buf->size() < static_cast<int>(sizeof(PointUbo))) {
            return false;
        }

        const auto* mat = static_cast<const PointMaterial*>(newMaterial);

        auto ubo = PointUbo{};
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
        ubo.markerSize = mat->markerSize;
        ubo.logScaleX = mat->logScaleX;
        ubo.logScaleY = mat->logScaleY;
        ubo.useVertexColor = mat->useVertexColor;
        ubo.antialiasingEnabled = mat->antialiasingEnabled;
        ubo.antialiasingFeather = mat->antialiasingFeather;
        ubo.shapeType = mat->shapeType;
        memcpy(buf->data(), &ubo, sizeof(ubo));
        return true;
    }
};

PointMaterial::PointMaterial()
{
    setFlag(Blending | RequiresFullMatrix);
}

QSGMaterialType* PointMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader* PointMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new PointShader;
}

int PointMaterial::compare(const QSGMaterial* other) const
{
    const auto* m = static_cast<const PointMaterial*>(other);
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
    if (markerSize != m->markerSize) {
        return markerSize < m->markerSize ? -1 : 1;
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
    if (antialiasingEnabled != m->antialiasingEnabled) {
        return antialiasingEnabled < m->antialiasingEnabled ? -1 : 1;
    }
    if (antialiasingFeather != m->antialiasingFeather) {
        return antialiasingFeather < m->antialiasingFeather ? -1 : 1;
    }
    if (shapeType != m->shapeType) {
        return shapeType < m->shapeType ? -1 : 1;
    }
    return 0;
}

} // namespace QAccelPlot
