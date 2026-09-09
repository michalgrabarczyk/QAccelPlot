//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>
#include <QSGMaterial>
#include <QSGMaterialShader>
#include <QVector2D>

namespace QAccelPlot {

/// \brief QSGMaterial for marker (point) rendering.
///
/// Passes per-point color, domain bounds, marker size, and shape type to the
/// point fragment shader. Unlike \c LineMaterial this class does not inherit from
/// \c DataTextureMaterial because markers do not require a data texture.
class PointMaterial : public QSGMaterial {
public:
    /// \brief Constructs a PointMaterial with default uniform values.
    PointMaterial();
    ~PointMaterial() override = default;

    /// \brief Returns the unique material type identifier for this class.
    QSGMaterialType* type() const override;
    /// \brief Creates and returns the point shader program.
    QSGMaterialShader* createShader(QSGRendererInterface::RenderMode) const override;
    /// \brief Compares all uniform fields for equality.
    int compare(const QSGMaterial* other) const override;

    QColor color{Qt::blue};                 ///< \brief Base marker color.
    QVector2D domainMin{0.0f, 0.0f};        ///< \brief Minimum data-space coordinate.
    QVector2D domainMax{1.0f, 1.0f};        ///< \brief Maximum data-space coordinate.
    QVector2D viewportSize{800.0f, 600.0f}; ///< \brief Viewport size in pixels.
    float markerSize{4.0f};                 ///< \brief Marker radius in pixels.
    float logScaleX{0.0f};                  ///< \brief 1.0 when the X axis uses log scale.
    float logScaleY{0.0f};                  ///< \brief 1.0 when the Y axis uses log scale.
    float useVertexColor{0.0f};             ///< \brief 1.0 when per-vertex color overrides \c color.
    float antialiasingEnabled{1.0f};        ///< \brief 1.0 when GPU anti-aliasing is active.
    float antialiasingFeather{1.0f};        ///< \brief Anti-aliasing feather width in pixels.
    int shapeType{0};                       ///< \brief Marker shape: 0=Circle, 1=Square, 2=Diamond, 3=TriangleUp, 4=TriangleDown, 5=Cross.
};

} // namespace QAccelPlot
