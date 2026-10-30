//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/materials/DataTextureMaterial.hpp"
#include "QAccelPlot/materials/GradientTexture.hpp"

namespace QAccelPlot {

/// \brief QSGMaterial for \c LineCurve marker rendering.
///
/// Marker positions are read from the inherited data texture as interleaved XY floats, so the
/// vertex buffer depends only on the point count. With \c useVertexColor == 1 the markers take
/// their color from \c gradientTexture at their position along \c gradientDirection.
class PointMaterial : public DataTextureMaterial {
public:
    /// \brief Constructs a PointMaterial with default uniform values.
    PointMaterial();

    /// \brief Returns the unique material type identifier for this class.
    QSGMaterialType* type() const override;
    /// \brief Creates and returns the point shader program.
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    float markerSize{4.0f};          ///< \brief Marker radius in pixels.
    float antialiasingEnabled{1.0f}; ///< \brief 1.0 when GPU anti-aliasing is active.
    float antialiasingFeather{1.0f}; ///< \brief Anti-aliasing feather width in pixels.
    int shapeType{0};                ///< \brief Marker shape: \c MarkerShape value minus one (0 = Circle).
    float markerStrokeWidth{1.0f};   ///< \brief Outline width in pixels for hollow markers.
    float markerFilled{1.0f};        ///< \brief 1.0 for filled markers, 0.0 for hollow outlines.
    float gradientDirection{0.0f};   ///< \brief 0.0 for a horizontal gradient, 1.0 for a vertical one.
    float gradientValueMin{0.0f};    ///< \brief Render-space data value at gradient position 0.
    float gradientValueMax{1.0f};    ///< \brief Render-space data value at gradient position 1.
    GradientTexture gradientTexture; ///< \brief Gradient lookup texture sampled at binding 2.

protected:
    /// \brief Compares marker-specific uniforms and the gradient texture identity.
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
