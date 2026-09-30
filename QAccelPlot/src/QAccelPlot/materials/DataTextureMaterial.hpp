//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/materials/DataTexture.hpp"

#include <QColor>
#include <QMatrix4x4>
#include <QQuickWindow>
#include <QSGMaterial>
#include <QSGTexture>
#include <QVector2D>

#include <memory>

namespace QAccelPlot {

/// \brief Base QSGMaterial that samples series data from a \c DataTexture and exposes shared shader uniforms.
///
/// Subclasses (\c LineMaterial, \c RectMaterial) extend the UBO (Uniform Buffer Object) with type-specific fields.
/// The shared uniform block occupies the first 116 bytes of the std140 UBO:
/// \par UBO layout (std140)
/// -  0–63  mat4  transform matrix
/// - 64–79  vec4  color
/// - 80–87  vec2  domainMin
/// - 88–95  vec2  domainMax
/// - 96–103 vec2  viewportSize
/// - 104–107 float logScaleX
/// - 108–111 float logScaleY
/// - 112–115 float useVertexColor
class DataTextureMaterial : public QSGMaterial {
public:
    /// \brief Constructs an empty DataTextureMaterial.
    DataTextureMaterial();
    ~DataTextureMaterial() override = default;

    /// \brief Compares shared uniform fields; delegates type-specific fields to \c compareExtra().
    int compare(const QSGMaterial* other) const override;

    /// \brief Writes the shared UBO prefix (transform + color + domain + flags) into \a buf.
    ///
    /// Returns \c false only if \a bufSize is too small; always writes all fields otherwise.
    static bool writeCommonUniforms(char* buf, int bufSize, const QMatrix4x4& matrix, const DataTextureMaterial* mat);

    /// \brief Uploads \a floatCount raw floats from \a data into \c dataTexture, creating it when it is \c nullptr.
    void uploadTexture(QQuickWindow* window, const float* data, int floatCount);

    /// \brief Commits the texture of \a data to sampler \a binding (call from \c updateSampledImage). Does nothing before an upload.
    static void commitTexture(QSGMaterialShader::RenderState& state, int binding, QSGTexture** texture, const DataTexture* data);

    /// \brief Returns the shared vertex attribute set: \c {float id, float param, uchar4 color} (12 bytes).
    static const QSGGeometry::AttributeSet& attributeSet();

    /// \brief Returns the texture the shader samples, or \c nullptr before \c dataTexture holds an upload.
    QSGTexture* sampledTexture() const;

protected:
    /// \brief Subclass hook for \c compare() — called after the shared fields compare equal.
    virtual int compareExtra(const QSGMaterial* other) const;

public:
    QColor color{Qt::blue};                 ///< \brief Line/fill color uniform.
    QVector2D domainMin{0.0f, 0.0f};        ///< \brief Minimum data-space coordinate.
    QVector2D domainMax{1.0f, 1.0f};        ///< \brief Maximum data-space coordinate.
    QVector2D viewportSize{800.0f, 600.0f}; ///< \brief Viewport size in pixels.
    float logScaleX{0.0f};                  ///< \brief 1.0 when the X axis uses log scale (float for std140 UBO compatibility).
    float logScaleY{0.0f};                  ///< \brief 1.0 when the Y axis uses log scale (float for std140 UBO compatibility).
    float useVertexColor{0.0f};             ///< \brief 1.0 when per-vertex color overrides \c color (float for std140 UBO compatibility).
    /// \brief Data texture sampled by the shader. Materials drawing the same samples may share one.
    std::shared_ptr<DataTexture> dataTexture;
};

} // namespace QAccelPlot
