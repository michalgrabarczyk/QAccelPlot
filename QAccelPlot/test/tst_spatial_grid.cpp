//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/SpatialGrid.hpp"

#include <QtTest/QtTest>

#include <array>
#include <limits>
#include <vector>

class TestSpatialGrid : public QObject {
    Q_OBJECT

private slots:
    void query_emptyGrid_returnsMinusOne();
    void query_insideSingleRect_returnsZero();
    void query_outsideSingleRect_returnsMinusOne();
    void query_outsideBounds_returnsMinusOne();
    void query_fourRectsGrid_returnsCorrectIndex();
    void query_withStride_readsCorrectCoordinates();

    // Corner cases
    void build_invertedRect_isNormalized();
    void build_zeroAreaRect_isQueryable();
    void query_overlappingRects_returnsLastInserted();
    void query_skipsTopCandidateThatDoesNotContainPoint();
    void build_nullData_returnsMinusOne();

    // Unbounded and invalid edges
    void query_fullHeightSpans_hitAtAnyY();
    void query_halfOpenRect_extendsToInfinity();
    void query_fullPlaneRect_hitsEverywhere();
    void query_spanAboveBox_returnsTopmost();
    void build_nanRect_isNeverHit();
    void query_nanPoint_returnsMinusOne();

    // Box queries
    void queryTopmost_returnsHighestAcceptedOverlap();
};

namespace {
constexpr auto kInf = std::numeric_limits<double>::infinity();
constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();
}

void TestSpatialGrid::query_emptyGrid_returnsMinusOne()
{
    auto grid = QAccelPlot::SpatialGrid{};
    QCOMPARE(grid.query(0.5, 0.5), -1);
}

void TestSpatialGrid::query_insideSingleRect_returnsZero()
{
    const auto data = std::array<double, 4>{0.0, 0.0, 1.0, 1.0};
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 1);

    QCOMPARE(grid.query(0.5, 0.5), 0);
    QCOMPARE(grid.query(0.1, 0.9), 0);
}

void TestSpatialGrid::query_outsideSingleRect_returnsMinusOne()
{
    const auto data = std::array<double, 4>{0.0, 0.0, 1.0, 1.0};
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 1);

    // SpatialGrid bounds are extended by viewportMax(x1,x2)/viewportMin(x1,x2) — outside those bounds
    QCOMPARE(grid.query(2.0, 0.5), -1);
    QCOMPARE(grid.query(0.5, 2.0), -1);
}

void TestSpatialGrid::query_outsideBounds_returnsMinusOne()
{
    const auto data = std::array<double, 4>{10.0, 20.0, 30.0, 40.0};
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 1);

    QCOMPARE(grid.query(0.0, 0.0), -1);
    QCOMPARE(grid.query(50.0, 50.0), -1);
    QCOMPARE(grid.query(9.9, 25.0), -1);
}

void TestSpatialGrid::query_fourRectsGrid_returnsCorrectIndex()
{
    // 4 non-overlapping rects in a 2x2 arrangement.
    // With itemCount=4, dim=sqrt(4)=2, so the spatial grid is 2x2.
    // cellW = (3-0)/2 = 1.5, cellH = (3-0)/2 = 1.5
    const auto data = std::array<double, 16>{
        0.0, 0.0, 1.0, 1.0, // index 0: bottom-left cell
        2.0, 0.0, 3.0, 1.0, // index 1: bottom-right cell
        0.0, 2.0, 1.0, 3.0, // index 2: top-left cell
        2.0, 2.0, 3.0, 3.0, // index 3: top-right cell
    };
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 4);

    QCOMPARE(grid.query(0.5, 0.5), 0);
    QCOMPARE(grid.query(2.5, 0.5), 1);
    QCOMPARE(grid.query(0.5, 2.5), 2);
    QCOMPARE(grid.query(2.5, 2.5), 3);
}

void TestSpatialGrid::query_withStride_readsCorrectCoordinates()
{
    // valuesPerItem=6: first 4 doubles are x1,y1,x2,y2; last 2 are padding.
    // The padded values (99.0) should be outside the query bounds if only
    // x1,y1,x2,y2 are used to build the grid.
    const auto data = std::array<double, 6>{0.0, 0.0, 1.0, 1.0, 99.0, 99.0};
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 1, 6);

    // Bounds come from x1,y1,x2,y2 = [0,0,1,1], not from 99.0
    QCOMPARE(grid.query(0.5, 0.5), 0);
    QCOMPARE(grid.query(50.0, 50.0), -1);
}

void TestSpatialGrid::build_invertedRect_isNormalized()
{
    // Rect with x1>x2, y1>y2 — build() normalises with viewportMin/viewportMax, so query still works.
    const auto data = std::array<double, 4>{1.0, 1.0, 0.0, 0.0};
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 1);

    QCOMPARE(grid.query(0.5, 0.5), 0);
}

void TestSpatialGrid::build_zeroAreaRect_isQueryable()
{
    // Single-point rect (x1==x2, y1==y2). Build pads the bounds by +1 to avoid
    // a zero-size grid, but the point itself should still be found.
    const auto data = std::array<double, 4>{2.0, 3.0, 2.0, 3.0};
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 1);

    QCOMPARE(grid.query(2.0, 3.0), 0);
    QCOMPARE(grid.query(0.0, 0.0), -1);
}

void TestSpatialGrid::query_overlappingRects_returnsLastInserted()
{
    // Two identical rects share the same cell. The implementation iterates the
    // cell list in reverse and returns the first match — i.e. the highest index.
    const auto data = std::array<double, 8>{
        0.0, 0.0, 1.0, 1.0, // index 0
        0.0, 0.0, 1.0, 1.0, // index 1
    };
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 2);

    QCOMPARE(grid.query(0.5, 0.5), 1);
}

void TestSpatialGrid::query_skipsTopCandidateThatDoesNotContainPoint()
{
    // Two disjoint rectangles share the single grid cell used for two items.
    // The later rectangle is topmost, but must not hide an earlier rectangle
    // when it does not contain the queried point.
    const auto data = std::array<double, 8>{
        0.0, 0.0, 1.0, 1.0, // index 0
        2.0, 2.0, 3.0, 3.0, // index 1
    };
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 2);

    QCOMPARE(grid.query(0.5, 0.5), 0);
    QCOMPARE(grid.query(1.5, 1.5), -1);
}

void TestSpatialGrid::build_nullData_returnsMinusOne()
{
    // nullptr data must not crash; the grid stays empty and all queries return -1.
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(nullptr, 5);

    QCOMPARE(grid.query(0.5, 0.5), -1);
}

void TestSpatialGrid::query_fullHeightSpans_hitAtAnyY()
{
    // Enough spans that a square grid would push each full-height column past the per-item cell limit.
    constexpr auto kSpanCount = 10'000;
    auto data = std::vector<double>{};
    for (auto i = 0; i < kSpanCount; ++i) {
        data.insert(data.end(), {2.0 * i, -kInf, 2.0 * i + 1.0, kInf});
    }
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), kSpanCount);

    for (const auto i : {0, 1, 4'999, 9'999}) {
        QCOMPARE(grid.query(2.0 * i + 0.5, 0.0), i);
        QCOMPARE(grid.query(2.0 * i + 0.5, 1.0e12), i);
        QCOMPARE(grid.query(2.0 * i + 0.5, -1.0e12), i);
        QCOMPARE(grid.query(2.0 * i + 1.5, 0.0), -1);
    }
    QCOMPARE(grid.query(-1.0, 0.0), -1);
}

void TestSpatialGrid::query_halfOpenRect_extendsToInfinity()
{
    const auto data = std::array<double, 8>{
        -kInf, 0.0, 5.0, 1.0, // index 0: everything left of x = 5
        10.0, 0.0, 20.0, 1.0, // index 1: bounded
    };
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 2);

    QCOMPARE(grid.query(-1.0e15, 0.5), 0);
    QCOMPARE(grid.query(4.9, 0.5), 0);
    QCOMPARE(grid.query(7.0, 0.5), -1);
    QCOMPARE(grid.query(-1.0e15, 2.0), -1);
    QCOMPARE(grid.query(15.0, 0.5), 1);
}

void TestSpatialGrid::query_fullPlaneRect_hitsEverywhere()
{
    const auto data = std::array<double, 4>{-kInf, -kInf, kInf, kInf};
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 1);

    QCOMPARE(grid.query(0.0, 0.0), 0);
    QCOMPARE(grid.query(-1.0e300, 1.0e300), 0);
}

void TestSpatialGrid::query_spanAboveBox_returnsTopmost()
{
    const auto data = std::array<double, 8>{
        0.0, 0.0, 1.0, 1.0,    // index 0: box
        0.5, -kInf, 0.6, kInf, // index 1: span drawn over the box
    };
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 2);

    QCOMPARE(grid.query(0.55, 0.5), 1);
    QCOMPARE(grid.query(0.55, 100.0), 1);
    QCOMPARE(grid.query(0.2, 0.5), 0);
}

void TestSpatialGrid::build_nanRect_isNeverHit()
{
    const auto data = std::array<double, 8>{
        0.0, 0.0, 10.0, kNaN, // index 0: invalid
        2.0, 2.0, 3.0, 3.0,   // index 1
    };
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 2);

    QCOMPARE(grid.query(5.0, 0.5), -1);
    QCOMPARE(grid.query(2.5, 2.5), 1);
}

void TestSpatialGrid::query_nanPoint_returnsMinusOne()
{
    const auto data = std::array<double, 4>{-kInf, -kInf, kInf, kInf};
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 1);

    QCOMPARE(grid.query(kNaN, 0.0), -1);
    QCOMPARE(grid.query(0.0, kNaN), -1);
}

void TestSpatialGrid::queryTopmost_returnsHighestAcceptedOverlap()
{
    // Index 2 is large enough to be checked outside the cells.
    auto data = std::vector<double>{
        0.0, 0.0, 1.0, 1.0,           // index 0
        1.5, 0.0, 2.0, 1.0,           // index 1
        -100.0, -100.0, 100.0, 100.0, // index 2
    };
    for (auto i = 0; i < 97; ++i) {
        data.insert(data.end(), {50.0 + i, 50.0, 50.5 + i, 50.5});
    }
    auto grid = QAccelPlot::SpatialGrid{};
    grid.build(data.data(), 100);
    const auto acceptAll = [](int) { return true; };
    const auto rejectLarge = [](const int index) { return index != 2; };

    QCOMPARE(grid.queryTopmost(0.9, 0.5, 1.6, 0.5, acceptAll), 2);
    QCOMPARE(grid.queryTopmost(0.9, 0.5, 1.6, 0.5, rejectLarge), 1);
    QCOMPARE(grid.queryTopmost(0.9, 0.5, 1.2, 0.5, rejectLarge), 0);
    QCOMPARE(grid.queryTopmost(1.1, 0.5, 1.2, 0.5, rejectLarge), -1);
    QCOMPARE(grid.queryTopmost(200.0, 200.0, 300.0, 300.0, acceptAll), -1);
}

QTEST_GUILESS_MAIN(TestSpatialGrid)
#include "tst_spatial_grid.moc"
