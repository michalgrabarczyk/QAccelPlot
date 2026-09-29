//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>

#include <functional>
#include <vector>

class QSGGeometry;

namespace QAccelPlot {

/// \brief Vertices of a series that draws each rectangle as a quad positioned from a data texture.
///
/// Each rectangle has 6 vertices (two triangles) in the \c DataTextureMaterial::attributeSet()
/// layout: rectangle index, quad corner (0–5), and color. The vertices hold no coordinates, so
/// only a rectangle count or color change requires a rebuild.
class RectVertexCache final {
public:
    /// \brief Returns the color of rectangle \a index.
    using ColorFunction = std::function<QColor(int index)>;

    /// \brief Number of vertices per rectangle.
    static constexpr int kVerticesPerRect{6};

    /// \brief Rebuilds the vertices of \a rectCount rectangles, keeping the allocation when it fits.
    ///
    /// Vertex colors come from \a colorAt, or are white when it is empty, e.g. when the shader
    /// uses a uniform color instead.
    void rebuild(int rectCount, const ColorFunction& colorAt);

    /// \brief Marks the vertices stale, keeping their allocation for the next rebuild.
    void invalidate() noexcept;

    /// \brief Returns whether the vertices match the last rebuild.
    [[nodiscard]] bool valid() const noexcept;

    /// \brief Returns the number of cached vertices.
    [[nodiscard]] int vertexCount() const noexcept;

    /// \brief Reallocates \a geometry to \c vertexCount vertices and copies the vertices into it.
    ///
    /// \a geometry must use the \c DataTextureMaterial::attributeSet() layout.
    void copyTo(QSGGeometry& geometry) const;

private:
    struct Vertex {
        float id;
        float corner;
        unsigned char r, g, b, a;
    };

    std::vector<Vertex> vertices_;
    bool valid_{false};
};

} // namespace QAccelPlot
