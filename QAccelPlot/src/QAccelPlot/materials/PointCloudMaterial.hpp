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

/// \brief QSGMaterial for \c PointCloud rendering.
///
/// Point positions (and optional per-point values) are read from the inherited data texture
/// with \c stride floats per point. In value-color mode (\c useVertexColor == 1) the fragment
/// shader maps each value through \c colorMap.
///
/// \par UBO layout (std140), after the shared 116-byte prefix
/// - 116–119 float markerSize
/// - 120–123 float antialiasingEnabled
/// - 124–127 float antialiasingFeather
/// - 128–131 float valueMin
/// - 132–135 float valueMax
/// - 136–139 float stride
/// - 140–143 int   shapeType
/// - 144–147 float markerStrokeWidth
/// - 148–151 float markerFilled
class PointCloudMaterial : public DataTextureMaterial {
public:
    /// \brief Constructs a PointCloudMaterial with default uniform values.
    PointCloudMaterial();

    /// \brief Returns the unique material type identifier for this class.
    QSGMaterialType* type() const override;
    /// \brief Creates and returns the point cloud shader program.
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    /// \brief Returns the per-vertex attribute set: \c {float pointId, float corner} (8 bytes).
    static const QSGGeometry::AttributeSet& attributeSet();

    float markerSize{3.0f};          ///< \brief Marker radius in pixels.
    float antialiasingEnabled{1.0f}; ///< \brief 1.0 when GPU anti-aliasing is active.
    float antialiasingFeather{1.0f}; ///< \brief Anti-aliasing feather width in pixels.
    float valueMin{0.0f};            ///< \brief Value mapped to colormap position 0.
    float valueMax{1.0f};            ///< \brief Value mapped to colormap position 1.
    float stride{2.0f};              ///< \brief Floats per point in the data texture: 2 (x, y) or 3 (x, y, value).
    int shapeType{0};                ///< \brief Marker shape: \c PlotSeries::MarkerShape value minus one (0 = Circle).
    float markerStrokeWidth{1.0f};   ///< \brief Outline width in pixels for hollow markers.
    float markerFilled{1.0f};        ///< \brief 1.0 for filled markers, 0.0 for hollow outlines.
    float valueLogScale{0.0f};       ///< \brief 1.0 to place values on the ramp in log10 instead of linearly.
    GradientTexture colorMap;        ///< \brief Colormap lookup texture sampled at binding 2.

protected:
    /// \brief Compares point-cloud-specific uniforms and the colormap texture identity.
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
