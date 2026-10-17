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
/// The vertex shader maps item-local pixels to fractional cell coordinates with \c cellScale and
/// \c cellOffset; the fragment shader reads the cell value from \c values and looks up its color
/// in \c colorMap.
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

    QVector2D cellScale{1.0f, 1.0f};  ///< \brief Cells per item-local pixel along X and Y.
    QVector2D cellOffset{0.0f, 0.0f}; ///< \brief Cell coordinate at item-local pixel (0, 0).
    float valueMin{0.0f};             ///< \brief Value mapped to colormap position 0.
    float valueMax{1.0f};             ///< \brief Value mapped to colormap position 1.
    float valueLogScale{0.0f};        ///< \brief 1.0 to place values on the ramp in log10 instead of linearly.
    int columns{0};                   ///< \brief Cells per row.
    int rows{0};                      ///< \brief Number of rows.
    DataTexture values;               ///< \brief Cell values, row by row, sampled at binding 1.
    GradientTexture colorMap;         ///< \brief Colormap lookup texture sampled at binding 2.
};

} // namespace QAccelPlot
