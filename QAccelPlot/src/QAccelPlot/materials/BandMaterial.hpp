//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/materials/DataTextureMaterial.hpp"

#include <QSGGeometry>

namespace QAccelPlot {

/// \brief QSGMaterial that fills a band between the low and high values of <tt>(x, low, high)</tt> samples.
///
/// The data texture holds three floats per sample. Vertices carry only a sample index and an edge
/// selector, so panning and zooming change uniforms only.
class BandMaterial : public DataTextureMaterial {
public:
    /// \brief Vertex layout of the band triangle strip: two vertices per sample.
    struct Vertex {
        float id;   ///< \brief Sample index in the data texture.
        float edge; ///< \brief 0 for the lower edge, 1 for the upper edge.
    };

    /// \brief Constructs a BandMaterial with default uniform values.
    BandMaterial();

    /// \brief Returns the unique material type identifier for this class.
    QSGMaterialType* type() const override;
    /// \brief Creates and returns the band shader program.
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    /// \brief Returns the vertex attribute set matching \c Vertex: \c {float id, float edge} (8 bytes).
    static const QSGGeometry::AttributeSet& vertexAttributes();

    float sampleCount{0.0f}; ///< \brief Number of samples in the data texture (used to clamp neighbor lookups).

protected:
    /// \brief Compares the band-specific uniforms after the base-class comparison succeeds.
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
