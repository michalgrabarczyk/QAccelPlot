//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/BandEdgeMaterial.hpp"

namespace QAccelPlot {

BandEdgeMaterial::BandEdgeMaterial(const Edge edge, const bool dashed)
    : LineMaterial(dashed)
    , edge_(edge)
{
}

BandEdgeMaterial::Edge BandEdgeMaterial::edge() const
{
    return edge_;
}

QSGMaterialType* BandEdgeMaterial::type() const
{
    // Indexed by edge, then by whether the vertices are dashed.
    static QSGMaterialType types[2][2];
    return &types[edge_ == Edge::Lower ? 0 : 1][dashed() ? 1 : 0];
}

QSGMaterialShader* BandEdgeMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    const auto* edgeName = edge_ == Edge::Lower ? "lower" : "upper";
    return createLineShader(
        QStringLiteral(":/qaccelplot/shaders/band_edge_%1%2.vert.qsb").arg(QLatin1String(edgeName), dashed() ? QStringLiteral("_dashed") : QString{}));
}

} // namespace QAccelPlot
