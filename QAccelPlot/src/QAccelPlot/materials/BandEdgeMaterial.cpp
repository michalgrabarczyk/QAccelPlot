//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/BandEdgeMaterial.hpp"

namespace QAccelPlot {

BandEdgeMaterial::BandEdgeMaterial(const Edge edge)
    : edge_(edge)
{
}

BandEdgeMaterial::Edge BandEdgeMaterial::edge() const
{
    return edge_;
}

QSGMaterialType* BandEdgeMaterial::type() const
{
    static QSGMaterialType lowerType;
    static QSGMaterialType upperType;
    return edge_ == Edge::Lower ? &lowerType : &upperType;
}

QSGMaterialShader* BandEdgeMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return createLineShader(
        QString::fromLatin1(edge_ == Edge::Lower ? ":/qaccelplot/shaders/band_edge_lower.vert.qsb" : ":/qaccelplot/shaders/band_edge_upper.vert.qsb"));
}

} // namespace QAccelPlot
