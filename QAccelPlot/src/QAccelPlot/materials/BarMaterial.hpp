//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/materials/GradientTexture.hpp"
#include "QAccelPlot/materials/RectMaterial.hpp"

namespace QAccelPlot {

/// \brief QSGMaterial for bar series rendering, extending RectMaterial with the bar geometry and colormap uniforms.
///
/// The data texture holds (position, value) pairs, or (from, to, value) triples when \c ranged is set;
/// the shader turns each into a rectangle. When \c colorSource is set, the vertex shader colors each
/// bar from \c colorMap.
class BarMaterial : public RectMaterial {
public:
    /// \brief Constructs a BarMaterial with default uniform values.
    BarMaterial();

    /// \brief Returns the unique material type identifier for this class.
    QSGMaterialType* type() const override;
    /// \brief Creates and returns the bar shader program.
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    float barWidth{0.8f};   ///< \brief Bar width in position-axis data units.
    float barOffset{0.0f};  ///< \brief Shift of every bar along the position axis in data units.
    float baseline{0.0f};   ///< \brief Value the bars start from, relative to the value-axis render origin.
    float horizontal{0.0f}; ///< \brief 1.0 when positions lie on the Y axis (float for std140 UBO compatibility).
    float ranged{0.0f};     ///< \brief 1.0 when each bar holds its own extent along the position axis.
    /// \brief What \c colorMap maps: 0 nothing (the rectangle colors apply), 1 the value, 2 the position.
    float colorSource{0.0f};
    float valueMin{0.0f};      ///< \brief Value at colormap position 0, shifted like the data unless \c valueLogScale is set.
    float valueMax{1.0f};      ///< \brief Value at colormap position 1, shifted like \c valueMin.
    float valueOrigin{0.0f};   ///< \brief Added to the mapped data before a log normalization, which needs the absolute value.
    float valueLogScale{0.0f}; ///< \brief 1.0 to place values on the colormap in log10.
    GradientTexture colorMap;  ///< \brief Colormap lookup texture sampled at binding 2.

protected:
    /// \brief Compares the bar-specific uniforms and the colormap texture after the rectangle uniforms compare equal.
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
