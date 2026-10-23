//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "QAccelPlot/linestyles/LineStyle.hpp"

#include <QColor>
#include <QPointF>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QVector2D>

#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

namespace QAccelPlot {

class LineMaterial;

/// \brief Vertex layout for line geometry, shared with the main thread for pre-built vertex caches.
struct LineVertex {
    float id;        ///< \brief Point index in the data buffer.
    float side;      ///< \brief <tt>±(1 + corner)</tt>: the sign is the ribbon side, \c corner the step corner after the point (0 for the point itself).
    unsigned char r; ///< \brief Red channel (used when \c useVertexColor is true).
    unsigned char g; ///< \brief Green channel (used when \c useVertexColor is true).
    unsigned char b; ///< \brief Blue channel (used when \c useVertexColor is true).
    unsigned char a; ///< \brief Alpha channel (used when \c useVertexColor is true).
    float arcLength; ///< \brief Cumulative screen-space arc length in pixels; 0 for solid lines.
};

/// \brief Building blocks for the line ribbon drawn by the line shaders, shared by the line renderers.
namespace LineStroke {

/// \brief Uniforms of a line material that do not depend on its shader variant.
struct Uniforms {
    QColor color;              ///< \brief Line color.
    qreal lineWidth;           ///< \brief Line width in pixels.
    QVector2D domainMin;       ///< \brief Minimum data-space coordinate, relative to the render origin.
    QVector2D domainMax;       ///< \brief Maximum data-space coordinate, relative to the render origin.
    QVector2D viewportSize;    ///< \brief Viewport size in pixels.
    bool logScaleX;            ///< \brief Whether the X axis uses log scale.
    bool logScaleY;            ///< \brief Whether the Y axis uses log scale.
    int pointCount;            ///< \brief Number of data points in the data texture.
    bool antialiasingEnabled;  ///< \brief Whether GPU-side anti-aliasing is active.
    qreal antialiasingFeather; ///< \brief Anti-aliasing feather width in pixels.
    DashParameters dash;       ///< \brief Dash pattern; disabled for solid lines.
};

/// \brief Returns the vertex attribute set matching \c LineVertex.
const QSGGeometry::AttributeSet& vertexAttributes();

/// \brief Returns a triangle-strip node with \a vertexCount line vertices that owns \a material.
QSGGeometryNode* createNode(int vertexCount, QSGMaterial* material);

/// \brief Writes two ribbon vertices per path point for \a pointCount data points of \a cornerCount path points each into \a vertices.
///
/// \a arcLengths holds cumulative lengths of the leading path points, or is empty for solid lines. Path points
/// past its end, such as vertices reserved for appended data, repeat its last length.
void writeVertices(LineVertex* vertices, int pointCount, int cornerCount, const QColor& color, const std::vector<float>& arcLengths);

/// \brief Copies \a uniforms into \a material.
void applyUniforms(LineMaterial& material, const Uniforms& uniforms);

/// \brief Returns the cumulative pixel length at each of \a pointCount data points, for dash patterns.
///
/// \a pixelAt returns the pixel position of a data point, or \c std::nullopt for an invalid data point.
/// Segments touching an invalid data point add no length, so the dash phase continues across a gap.
template <typename PixelAt> std::vector<float> arcLengths(const int pointCount, const PixelAt& pixelAt)
{
    auto lengths = std::vector<float>(static_cast<std::size_t>(std::max(pointCount, 0)));
    auto length = 0.0f;
    auto previousX = 0.0f;
    auto previousY = 0.0f;
    auto previousValid = false;
    for (auto i = 0; i < pointCount; ++i) {
        const auto current = std::optional<QPointF>{pixelAt(i)};
        if (current) {
            const auto x = static_cast<float>(current->x());
            const auto y = static_cast<float>(current->y());
            if (previousValid) {
                const auto dx = x - previousX;
                const auto dy = y - previousY;
                length += std::sqrt(dx * dx + dy * dy);
            }
            previousX = x;
            previousY = y;
        }
        previousValid = current.has_value();
        lengths[static_cast<std::size_t>(i)] = length;
    }
    return lengths;
}

} // namespace LineStroke

} // namespace QAccelPlot
