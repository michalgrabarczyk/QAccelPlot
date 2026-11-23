//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include "QAccelPlot/materials/GradientTexture.hpp"

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
    bool logGradient{false};       ///< The vertex coordinate is a log value, see Internal::LogGradientMapping.
    float logGradientScale{0.0f};  ///< Internal::LogGradientMapping::scale.
    float logGradientOffset{0.0f}; ///< Internal::LogGradientMapping::offset.
    GradientTexture gradientTexture;
};

} // namespace QAccelPlot
