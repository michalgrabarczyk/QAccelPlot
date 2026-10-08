//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/materials/RectMaterial.hpp"

namespace QAccelPlot {

/// \brief QSGMaterial for bar series rendering, extending RectMaterial with the bar geometry uniforms.
///
/// The data texture holds (position, value) pairs, or (from, to, value) triples when \c ranged is set;
/// the shader turns each into a rectangle.
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

protected:
    /// \brief Compares the bar-specific uniforms after the rectangle uniforms compare equal.
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
