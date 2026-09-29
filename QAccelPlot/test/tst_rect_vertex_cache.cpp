//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/DataTextureMaterial.hpp"
#include "QAccelPlot/series/RectVertexCache.hpp"

#include <QSGGeometry>
#include <QtTest/QtTest>

#include <array>
#include <memory>

namespace QAccelPlot {

class RectVertexCacheTest : public QObject {
    Q_OBJECT

private slots:
    void valid_beforeRebuild_isFalse();
    void rebuild_withoutColorFunction_writesIdsCornersAndWhite();
    void rebuild_withColorFunction_writesRectangleColors();
    void invalidate_keepsVertexCount();
    void copyTo_reallocatesGeometry();
    void rebuild_zeroRectangles_copiesEmptyGeometry();
};

namespace {

struct Vertex {
    float id;
    float corner;
    unsigned char r, g, b, a;
};

std::unique_ptr<QSGGeometry> copiedGeometry(const RectVertexCache& cache, const int initialVertexCount = 0)
{
    auto geometry = std::make_unique<QSGGeometry>(DataTextureMaterial::attributeSet(), initialVertexCount);
    cache.copyTo(*geometry);
    return geometry;
}

const Vertex& vertexAt(const QSGGeometry& geometry, const int index)
{
    return static_cast<const Vertex*>(geometry.vertexData())[index];
}

QColor vertexColor(const Vertex& vertex)
{
    return QColor{vertex.r, vertex.g, vertex.b, vertex.a};
}

}

void RectVertexCacheTest::valid_beforeRebuild_isFalse()
{
    const auto cache = RectVertexCache{};
    QVERIFY(!cache.valid());
    QCOMPARE(cache.vertexCount(), 0);
}

void RectVertexCacheTest::rebuild_withoutColorFunction_writesIdsCornersAndWhite()
{
    auto cache = RectVertexCache{};
    cache.rebuild(2, {});
    QVERIFY(cache.valid());
    QCOMPARE(cache.vertexCount(), 2 * RectVertexCache::kVerticesPerRect);

    const auto geometry = copiedGeometry(cache);
    for (auto i = 0; i < cache.vertexCount(); ++i) {
        const auto& vertex = vertexAt(*geometry, i);
        QCOMPARE(vertex.id, static_cast<float>(i / RectVertexCache::kVerticesPerRect));
        QCOMPARE(vertex.corner, static_cast<float>(i % RectVertexCache::kVerticesPerRect));
        QCOMPARE(vertexColor(vertex), QColor{Qt::white});
    }
}

void RectVertexCacheTest::rebuild_withColorFunction_writesRectangleColors()
{
    const auto colors = std::array<QColor, 2>{QColor{10, 20, 30, 40}, QColor::fromHsv(120, 255, 255)};
    auto cache = RectVertexCache{};
    cache.rebuild(2, [&colors](const int index) { return colors[static_cast<size_t>(index)]; });

    const auto geometry = copiedGeometry(cache);
    for (auto i = 0; i < cache.vertexCount(); ++i) {
        const auto rect = static_cast<size_t>(i / RectVertexCache::kVerticesPerRect);
        QCOMPARE(vertexColor(vertexAt(*geometry, i)), colors[rect].toRgb());
    }
}

void RectVertexCacheTest::invalidate_keepsVertexCount()
{
    auto cache = RectVertexCache{};
    cache.rebuild(3, {});
    cache.invalidate();
    QVERIFY(!cache.valid());
    QCOMPARE(cache.vertexCount(), 3 * RectVertexCache::kVerticesPerRect);
}

void RectVertexCacheTest::copyTo_reallocatesGeometry()
{
    auto cache = RectVertexCache{};
    cache.rebuild(1, {});
    const auto geometry = copiedGeometry(cache, 100);
    QCOMPARE(geometry->vertexCount(), RectVertexCache::kVerticesPerRect);
}

void RectVertexCacheTest::rebuild_zeroRectangles_copiesEmptyGeometry()
{
    auto cache = RectVertexCache{};
    cache.rebuild(4, {});
    cache.rebuild(0, {});
    QVERIFY(cache.valid());
    QCOMPARE(copiedGeometry(cache, 6)->vertexCount(), 0);
}

} // namespace QAccelPlot

using QAccelPlot::RectVertexCacheTest;
QTEST_GUILESS_MAIN(RectVertexCacheTest)
#include "tst_rect_vertex_cache.moc"
