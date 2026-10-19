//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/materials/DataTexture.hpp"
#include "QAccelPlot/materials/GradientTexture.hpp"

#include <QSGMaterial>
#include <QVector2D>

namespace QAccelPlot {

/// \brief Material that colors each fragment by the value of the heatmap cell under it.
///
/// Item-local pixels map to fractional cell coordinates per dimension, by the formula that
/// \c columnMapping and \c rowMapping select from \c Internal::HeatmapShaderMapping::Mode. The
/// fragment shader reads the cell value from \c values and looks up its color in \c colorMap.
class HeatmapMaterial : public QSGMaterial {
public:
    /// \brief Constructs a HeatmapMaterial with blending enabled and batching disabled.
    HeatmapMaterial();

    /// \brief Returns the unique material type identifier for this class.
    QSGMaterialType* type() const override;
    /// \brief Creates and returns the heatmap shader program.
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;
    /// \brief Compares the uniforms and the texture identities.
    int compare(const QSGMaterial* other) const override;

    QVector2D cellScale{1.0f, 1.0f};  ///< \brief Factor of the cell mapping along X and Y.
    QVector2D cellOffset{0.0f, 0.0f}; ///< \brief Offset of the cell mapping along X and Y.
    QVector2D cellRate{0.0f, 0.0f};   ///< \brief Per-pixel rate of the non-affine cell mappings along X and Y.
    QVector2D cellShift{0.0f, 0.0f};  ///< \brief \c log1p argument at pixel 0 of the logarithmic cell mappings along X and Y.
    float valueMin{0.0f};             ///< \brief Value mapped to colormap position 0.
    float valueMax{1.0f};             ///< \brief Value mapped to colormap position 1.
    float valueLogScale{0.0f};        ///< \brief 1.0 to place values on the ramp in log10 instead of linearly.
    int columns{0};                   ///< \brief Cells per row.
    int rows{0};                      ///< \brief Number of rows.
    int columnMapping{0};             ///< \brief \c Internal::HeatmapShaderMapping::Mode along X.
    int rowMapping{0};                ///< \brief \c Internal::HeatmapShaderMapping::Mode along Y.
    DataTexture values;               ///< \brief Cell values, row by row, sampled at binding 1.
    GradientTexture colorMap;         ///< \brief Colormap lookup texture sampled at binding 2.
};

} // namespace QAccelPlot
