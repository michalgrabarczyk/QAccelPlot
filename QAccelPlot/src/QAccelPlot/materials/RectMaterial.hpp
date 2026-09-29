//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/materials/DataTextureMaterial.hpp"

namespace QAccelPlot {

/// \brief QSGMaterial for rectangle series rendering, extending DataTextureMaterial with a rect-count uniform.
class RectMaterial : public DataTextureMaterial {
public:
    /// \brief Constructs a RectMaterial with default uniform values.
    RectMaterial();

    /// \brief Returns the unique material type identifier for this class.
    QSGMaterialType* type() const override;
    /// \brief Creates and returns the rectangle shader program.
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    float rectCount{0.0f};              ///< \brief Number of rectangles in the data texture (used to index the sampler).
    QVector2D minimumSize{0.0f, 0.0f};  ///< \brief Minimum drawn rectangle width and height in pixels.
    float borderWidth{0.0f};            ///< \brief Outline width in pixels, drawn inside each rectangle.
    QColor borderColor{Qt::black};      ///< \brief Outline color.
    QColor hoverColor{Qt::transparent}; ///< \brief Fill color of the highlighted rectangle.
    float hoveredIndex{-1.0f};          ///< \brief Index of the highlighted rectangle, or -1 for none.

protected:
    /// \brief Compares the rectangle-specific uniforms after the base-class comparison succeeds.
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
