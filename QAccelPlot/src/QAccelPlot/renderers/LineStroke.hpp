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

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace QAccelPlot {

class LineMaterial;

/// \brief Vertex of a solid line ribbon, shared with the main thread for pre-built vertex caches.
///
/// Packs a data point index, a path corner, a join half, and a ribbon side into four bytes, which the
/// shaders read as normalized bytes (see line_vertex_input.glsl).
struct LineVertex {
    /// \brief Largest data point index a vertex holds.
    static constexpr int kMaximumIndex = (1 << 28) - 1;
    /// \brief Largest path corner a vertex holds.
    static constexpr int kMaximumCorner = 3;

    /// \brief Bits 0–27 hold the data point index, bits 28–29 the path corner, bit 30 marks the outgoing join half, and bit 31 the negative ribbon side.
    std::uint32_t packed;

    /// \brief Returns the vertex of data point \a index at step corner \a corner (0 for the point itself) on the negative ribbon side when \a negativeSide.
    ///
    /// \a outgoing marks a vertex that starts the segment after a split join; see verticesPerPathPoint().
    static LineVertex make(int index, int corner, bool negativeSide, bool outgoing = false) noexcept;

    /// \brief Returns the data point index.
    int index() const noexcept;

    /// \brief Returns the step corner after the data point; 0 for the point itself.
    int corner() const noexcept;

    /// \brief Returns whether the vertex starts the segment after a split join.
    bool outgoing() const noexcept;

    /// \brief Returns whether the vertex lies on the negative ribbon side.
    bool negativeSide() const noexcept;
};

static_assert(sizeof(LineVertex) == 4);

/// \brief Vertex of a dashed line ribbon: a \c LineVertex and the screen-space arc length the dash pattern follows.
struct DashedLineVertex {
    LineVertex vertex; ///< \brief Data point index, path corner, join half, and ribbon side.
    float arcLength;   ///< \brief Cumulative screen-space arc length in pixels.
};

static_assert(sizeof(DashedLineVertex) == 8);

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

/// \brief Returns the vertex attribute set matching \c DashedLineVertex when \a dashed, or \c LineVertex otherwise.
const QSGGeometry::AttributeSet& vertexAttributes(bool dashed);

/// \brief Returns a triangle-strip node with \a vertexCount line vertices that owns \a material.
///
/// The vertices have the dashed layout when \a material is dashed.
QSGGeometryNode* createNode(int vertexCount, LineMaterial* material);

/// \brief Gives \a node, a node from createNode(), the vertex layout of \a material, which it takes ownership of.
///
/// Replaces the geometry of \a node with one of the same vertex count, whose vertices are undefined,
/// when the layout differs from the current one, and returns \c true then.
bool setMaterial(QSGGeometryNode* node, LineMaterial* material);

/// \brief Returns the ribbon vertices of a path point: four with \a splitJoins, or two otherwise.
///
/// Split joins give a path point one vertex pair that ends the segment before it and one that starts the
/// segment after it, so a join too sharp for a miter draws as a bevel. Without them, both segments share a
/// mitered pair, which suits paths that only turn by right angles.
int verticesPerPathPoint(bool splitJoins) noexcept;

/// \brief Writes the ribbon vertices of data points [\a firstPoint, \a pointCount) of \a cornerCount path points each into \a vertices.
///
/// Each path point gets verticesPerPathPoint(\a splitJoins) vertices. \a vertices holds the vertices of all
/// \a pointCount data points; those before \a firstPoint are left unchanged.
void writeVertices(LineVertex* vertices, int pointCount, int cornerCount, bool splitJoins, int firstPoint = 0);

/// \brief Writes the ribbon vertices of \a pointCount data points of \a cornerCount path points each into \a vertices.
///
/// Each path point gets verticesPerPathPoint(\a splitJoins) vertices. \a arcLengths holds cumulative lengths of
/// the leading path points. Path points past its end, such as vertices reserved for appended data, repeat its
/// last length, or 0 when it is empty.
void writeDashedVertices(DashedLineVertex* vertices, int pointCount, int cornerCount, bool splitJoins, const std::vector<float>& arcLengths);

/// \brief Copies \a uniforms into \a material.
void applyUniforms(LineMaterial& material, const Uniforms& uniforms);

/// \brief Returns the cumulative pixel length at each of \a pointCount data points, for dash patterns.
///
/// \a pixelAt returns the pixel position of a data point, or \c std::nullopt for an invalid data point.
/// Segments touching an invalid data point add no length, so the dash phase continues across a gap.
std::vector<float> arcLengths(int pointCount, const std::function<std::optional<QPointF>(int)>& pixelAt);

} // namespace LineStroke

} // namespace QAccelPlot
