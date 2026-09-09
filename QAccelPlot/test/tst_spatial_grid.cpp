//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "SpatialGrid.hpp"

#include <QtTest/QtTest>

class TestSpatialGrid : public QObject {
    Q_OBJECT

private slots:
    void query_emptyGrid_returnsMinusOne();
    void query_insideSingleRect_returnsZero();
    void query_outsideSingleRect_returnsMinusOne();
    void query_outsideBounds_returnsMinusOne();
    void query_fourRectsGrid_returnsCorrectIndex();
    void query_withStride_readsCorrectFloats();

    // Corner cases
    void build_invertedRect_isNormalized();
    void build_zeroAreaRect_isQueryable();
    void query_overlappingRects_returnsLastInserted();
    void query_skipsTopCandidateThatDoesNotContainPoint();
    void build_nullData_returnsMinusOne();
};

void TestSpatialGrid::query_emptyGrid_returnsMinusOne()
{
    auto grid = QAccelPlot::SpatialGrid{};
    QCOMPARE(grid.query(0.5f, 0.5f), -1);
}

void TestSpatialGrid::query_insideSingleRect_returnsZero()
{
    const auto data = std::array<float, 4>{0.0f, 0.0f, 1.0f, 1.0f};
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 1);

    QCOMPARE(grid.query(0.5f, 0.5f), 0);
    QCOMPARE(grid.query(0.1f, 0.9f), 0);
}

void TestSpatialGrid::query_outsideSingleRect_returnsMinusOne()
{
    const auto data = std::array<float, 4>{0.0f, 0.0f, 1.0f, 1.0f};
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 1);

    // SpatialGrid bounds are extended by viewportMax(x1,x2)/viewportMin(x1,x2) — outside those bounds
    QCOMPARE(grid.query(2.0f, 0.5f), -1);
    QCOMPARE(grid.query(0.5f, 2.0f), -1);
}

void TestSpatialGrid::query_outsideBounds_returnsMinusOne()
{
    const auto data = std::array<float, 4>{10.0f, 20.0f, 30.0f, 40.0f};
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 1);

    QCOMPARE(grid.query(0.0f, 0.0f), -1);
    QCOMPARE(grid.query(50.0f, 50.0f), -1);
    QCOMPARE(grid.query(9.9f, 25.0f), -1);
}

void TestSpatialGrid::query_fourRectsGrid_returnsCorrectIndex()
{
    // 4 non-overlapping rects in a 2x2 arrangement.
    // With itemCount=4, dim=sqrt(4)=2, so the spatial grid is 2x2.
    // cellW = (3-0)/2 = 1.5, cellH = (3-0)/2 = 1.5
    const auto data = std::array<float, 16>{
        0.0f, 0.0f, 1.0f, 1.0f, // index 0: bottom-left cell
        2.0f, 0.0f, 3.0f, 1.0f, // index 1: bottom-right cell
        0.0f, 2.0f, 1.0f, 3.0f, // index 2: top-left cell
        2.0f, 2.0f, 3.0f, 3.0f, // index 3: top-right cell
    };
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 4);

    QCOMPARE(grid.query(0.5f, 0.5f), 0);
    QCOMPARE(grid.query(2.5f, 0.5f), 1);
    QCOMPARE(grid.query(0.5f, 2.5f), 2);
    QCOMPARE(grid.query(2.5f, 2.5f), 3);
}

void TestSpatialGrid::query_withStride_readsCorrectFloats()
{
    // floatsPerItem=6: first 4 floats are x1,y1,x2,y2; last 2 are padding.
    // The padded values (99.0) should be outside the query bounds if only
    // x1,y1,x2,y2 are used to build the grid.
    const auto data = std::array<float, 6>{0.0f, 0.0f, 1.0f, 1.0f, 99.0f, 99.0f};
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 1, 6);

    // Bounds come from x1,y1,x2,y2 = [0,0,1,1], not from 99.0
    QCOMPARE(grid.query(0.5f, 0.5f), 0);
    QCOMPARE(grid.query(50.0f, 50.0f), -1);
}

void TestSpatialGrid::build_invertedRect_isNormalized()
{
    // Rect with x1>x2, y1>y2 — build() normalises with viewportMin/viewportMax, so query still works.
    const auto data = std::array<float, 4>{1.0f, 1.0f, 0.0f, 0.0f};
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 1);

    QCOMPARE(grid.query(0.5f, 0.5f), 0);
}

void TestSpatialGrid::build_zeroAreaRect_isQueryable()
{
    // Single-point rect (x1==x2, y1==y2). Build pads the bounds by +1 to avoid
    // a zero-size grid, but the point itself should still be found.
    const auto data = std::array<float, 4>{2.0f, 3.0f, 2.0f, 3.0f};
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 1);

    QCOMPARE(grid.query(2.0f, 3.0f), 0);
    QCOMPARE(grid.query(0.0f, 0.0f), -1);
}

void TestSpatialGrid::query_overlappingRects_returnsLastInserted()
{
    // Two identical rects share the same cell. The implementation iterates the
    // cell list in reverse and returns the first match — i.e. the highest index.
    const auto data = std::array<float, 8>{
        0.0f, 0.0f, 1.0f, 1.0f, // index 0
        0.0f, 0.0f, 1.0f, 1.0f, // index 1
    };
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 2);

    QCOMPARE(grid.query(0.5f, 0.5f), 1);
}

void TestSpatialGrid::query_skipsTopCandidateThatDoesNotContainPoint()
{
    // Two disjoint rectangles share the single grid cell used for two items.
    // The later rectangle is topmost, but must not hide an earlier rectangle
    // when it does not contain the queried point.
    const auto data = std::array<float, 8>{
        0.0f, 0.0f, 1.0f, 1.0f, // index 0
        2.0f, 2.0f, 3.0f, 3.0f, // index 1
    };
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 2);

    QCOMPARE(grid.query(0.5f, 0.5f), 0);
    QCOMPARE(grid.query(1.5f, 1.5f), -1);
}

void TestSpatialGrid::build_nullData_returnsMinusOne()
{
    // nullptr data must not crash; the grid stays empty and all queries return -1.
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(nullptr, 5);

    QCOMPARE(grid.query(0.5f, 0.5f), -1);
}

QTEST_GUILESS_MAIN(TestSpatialGrid)
#include "tst_spatial_grid.moc"
