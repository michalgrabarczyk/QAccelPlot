//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/RectVertexCache.hpp"

#include <QSGGeometry>

#include <cstring>

namespace QAccelPlot {

void RectVertexCache::rebuild(const int rectCount, const ColorFunction& colorAt)
{
    static_assert(sizeof(Vertex) == 12, "Must match DataTextureMaterial::attributeSet()");
    vertices_.resize(static_cast<size_t>(rectCount) * kVerticesPerRect);
    for (auto i = 0; i < rectCount; ++i) {
        const auto base = static_cast<size_t>(i) * kVerticesPerRect;
        const auto id = static_cast<float>(i);
        const auto rgba = colorAt ? colorAt(i).toRgb() : QColor{Qt::white};
        for (auto c = 0; c < kVerticesPerRect; ++c) {
            auto& v = vertices_[base + static_cast<size_t>(c)];
            v.id = id;
            v.corner = static_cast<float>(c);
            v.r = static_cast<unsigned char>(rgba.red());
            v.g = static_cast<unsigned char>(rgba.green());
            v.b = static_cast<unsigned char>(rgba.blue());
            v.a = static_cast<unsigned char>(rgba.alpha());
        }
    }
    valid_ = true;
}

void RectVertexCache::invalidate() noexcept
{
    valid_ = false;
}

bool RectVertexCache::valid() const noexcept
{
    return valid_;
}

int RectVertexCache::vertexCount() const noexcept
{
    return static_cast<int>(vertices_.size());
}

void RectVertexCache::copyTo(QSGGeometry& geometry) const
{
    Q_ASSERT(geometry.sizeOfVertex() == static_cast<int>(sizeof(Vertex)));
    geometry.allocate(vertexCount());
    if (!vertices_.empty()) {
        memcpy(geometry.vertexData(), vertices_.data(), vertices_.size() * sizeof(Vertex));
    }
}

} // namespace QAccelPlot
