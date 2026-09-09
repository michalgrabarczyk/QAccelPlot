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

/// \brief QSGMaterial for line rendering, extending DataTextureMaterial with line-specific uniforms.
///
/// Supplies line width, point count, anti-aliasing parameters, and dash pattern data
/// to the line fragment shader (std140 offsets 116–175 in the UBO).
class LineMaterial : public DataTextureMaterial {
public:
    /// \brief Constructs a LineMaterial with default uniform values.
    LineMaterial();

    /// \brief Returns the unique material type identifier for this subclass.
    QSGMaterialType* type() const override;
    /// \brief Creates and returns the line shader program.
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    float lineWidth{1.0f};           ///< \brief Rendered line width in pixels.
    float pointCount{0.0f};          ///< \brief Number of data points (used to index the data texture).
    float antialiasingEnabled{1.0f}; ///< \brief 1.0 when GPU anti-aliasing is active.
    float antialiasingFeather{1.0f}; ///< \brief Anti-aliasing feather width in pixels.

    // Dash line uniforms (std140 offsets 132–175 in the UBO).
    float dashPeriod{0.0f}; ///< \brief Total dash+gap cycle length in pixels.
    float dashOffset{0.0f}; ///< \brief Phase offset into the dash pattern.
    int dashPatternSize{0}; ///< \brief Number of valid entries in \c dashPattern.
    float dashPattern[8]{}; ///< \brief Alternating dash/gap lengths (up to 8 entries).

protected:
    /// \brief Compares line-specific uniform fields after the base-class comparison succeeds.
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
