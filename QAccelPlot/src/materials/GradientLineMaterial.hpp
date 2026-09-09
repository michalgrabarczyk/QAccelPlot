//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "materials/GradientTexture.hpp"
#include "materials/LineMaterial.hpp"

namespace QAccelPlot {

/// \brief Line material variant that samples a one-dimensional gradient texture.
class GradientLineMaterial : public LineMaterial {
public:
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
