//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "materials/DataTextureMaterial.hpp"

namespace QAccelPlot {

/// \brief QSGMaterial for rectangle list rendering, extending DataTextureMaterial with a rect-count uniform.
class RectMaterial : public DataTextureMaterial {
public:
    /// \brief Constructs a RectMaterial with default uniform values.
    RectMaterial();

    /// \brief Returns the unique material type identifier for this class.
    QSGMaterialType* type() const override;
    /// \brief Creates and returns the rectangle shader program.
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    float rectCount{0.0f}; ///< \brief Number of rectangles in the data texture (used to index the sampler).

protected:
    /// \brief Compares the \c rectCount field after the base-class comparison succeeds.
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
