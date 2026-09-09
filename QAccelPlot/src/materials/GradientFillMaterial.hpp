//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "materials/GradientTexture.hpp"

#include <QSGMaterial>

namespace QAccelPlot {

/// \brief Scene-graph material that evaluates fill gradients per fragment.
class GradientFillMaterial : public QSGMaterial {
public:
    GradientFillMaterial();

    QSGMaterialType* type() const override;
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;
    int compare(const QSGMaterial* other) const override;

    float opacity{1.0f};
    GradientTexture gradientTexture;
};

} // namespace QAccelPlot
