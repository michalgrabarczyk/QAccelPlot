//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/materials/DataTextureMaterial.hpp"

#include <QSGGeometry>

namespace QAccelPlot {

/// \brief Material that draws error bars from (x, y, low, high) data points in a data texture.
///
/// Each error bar takes \c kVerticesPerErrorBar vertices: two triangles each for the line and the two
/// caps. The shader derives the error bar, part, and corner from the vertex index.
class ErrorBarMaterial : public DataTextureMaterial {
public:
    /// \brief Vertices per error bar: a quad of two triangles for the line and for each cap.
    static constexpr int kVerticesPerErrorBar{18};

    /// \brief Constructs an ErrorBarMaterial with default uniform values.
    ErrorBarMaterial();

    /// \brief Returns the unique material type identifier for this class.
    QSGMaterialType* type() const override;
    /// \brief Creates and returns the error bar shader program.
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;

    /// \brief Returns the vertex attribute set: a 4-byte placeholder the shader ignores.
    static const QSGGeometry::AttributeSet& attributeSet();

    QColor hoverColor{Qt::transparent}; ///< \brief Color of the highlighted error bar.
    float hoveredIndex{-1.0f};          ///< \brief Index of the highlighted error bar, or -1 for none.
    float lineWidth{1.5f};              ///< \brief Width of the lines and caps in pixels.
    float capLength{8.0f};              ///< \brief Length of the caps in pixels; 0 draws none.
    float offset{0.0f};                 ///< \brief Shift of every error bar across its bounds, in data units.
    float horizontal{0.0f};             ///< \brief 1.0 when the bounds lie on the X axis (float for std140 UBO compatibility).
    float errorBarCount{0.0f};          ///< \brief Number of error bars drawn; vertices beyond them are culled.

protected:
    /// \brief Compares the error-bar-specific uniforms after the base-class comparison succeeds.
    int compareExtra(const QSGMaterial* other) const override;
};

} // namespace QAccelPlot
