//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/materials/GradientTexture.hpp"
#include "QAccelPlot/materials/LineMaterial.hpp"

namespace QAccelPlot {

/// \brief Line material variant that samples a one-dimensional gradient texture.
class GradientLineMaterial : public LineMaterial {
public:
    /// \brief Constructs a gradient line material, for dashed vertices when \a dashed.
    explicit GradientLineMaterial(bool dashed = false);

    QSGMaterialType* type() const override;
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    float gradientDirection{0.0f};
    float gradientValueMin{0.0f};
    float gradientValueMax{1.0f};
    GradientTexture gradientTexture;

protected:
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
