//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "PointSpatialIndex.hpp"

#include <QtTest/QtTest>

#include <array>
#include <cmath>
#include <limits>
#include <random>
#include <vector>

namespace QAccelPlot {

namespace {

int bruteForceNearest(const std::vector<float>& xy, const double x, const double y, const double radiusX, const double radiusY)
{
    auto bestIndex = -1;
    auto bestDistance = std::numeric_limits<double>::max();
    for (auto index = 0; index < static_cast<int>(xy.size() / 2); ++index) {
        const auto dx = (static_cast<double>(static_cast<float>(xy[static_cast<std::size_t>(index) * 2])) - x) / radiusX;
        const auto dy = (static_cast<double>(static_cast<float>(xy[static_cast<std::size_t>(index) * 2 + 1])) - y) / radiusY;
        const auto distance = dx * dx + dy * dy;
        if (distance <= 1.0 && (distance < bestDistance || (distance == bestDistance && index > bestIndex))) {
            bestDistance = distance;
            bestIndex = index;
        }
    }
    return bestIndex;
}

} // namespace

class PointSpatialIndexTest : public QObject {
    Q_OBJECT

private slots:
    void emptyIndexReturnsMinusOne();
    void nullDataAndInvalidStrideAreIgnored();
    void singlePointIsFoundInsideRadiusOnly();
    void duplicatePointsResolveToHighestIndex();
    void nearestMatchesBruteForce();
    void nonFinitePointsAreSkipped();
    void logarithmicMappingSkipsNonPositivePoints();
    void anisotropicRadiusUsesNormalizedDistance();
    void strideSkipsExtraComponents();
    void nonPositiveRadiusReturnsMinusOne();
};

void PointSpatialIndexTest::emptyIndexReturnsMinusOne()
{
    auto index = PointSpatialIndex{};
    QVERIFY(index.isEmpty());
    QCOMPARE(index.nearest(0.0, 0.0, 1.0, 1.0), -1);
}

void PointSpatialIndexTest::nullDataAndInvalidStrideAreIgnored()
{
    const auto data = std::array<float, 2>{1.0f, 1.0f};
    auto index = PointSpatialIndex{};
    index.build(nullptr, 10);
    QVERIFY(index.isEmpty());
    index.build(data.data(), 1, 1);
    QVERIFY(index.isEmpty());
    index.build(data.data(), -1);
    QVERIFY(index.isEmpty());
}

void PointSpatialIndexTest::singlePointIsFoundInsideRadiusOnly()
{
    const auto data = std::array<float, 2>{2.0f, 3.0f};
    auto index = PointSpatialIndex{};
    index.build(data.data(), 1);

    QCOMPARE(index.validPointCount(), 1);
    QCOMPARE(index.nearest(2.0, 3.0, 0.1, 0.1), 0);
    QCOMPARE(index.nearest(2.5, 3.0, 0.6, 0.6), 0);
    QCOMPARE(index.nearest(2.5, 3.0, 0.4, 0.4), -1);
    QCOMPARE(index.nearest(-100.0, -100.0, 1.0, 1.0), -1);
}

void PointSpatialIndexTest::duplicatePointsResolveToHighestIndex()
{
    const auto data = std::array<float, 6>{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    auto index = PointSpatialIndex{};
    index.build(data.data(), 3);

    QCOMPARE(index.nearest(1.0, 1.0, 0.5, 0.5), 2);
}

void PointSpatialIndexTest::nearestMatchesBruteForce()
{
    constexpr auto pointCount = 10000;
    auto engine = std::mt19937{42u};
    auto coordinate = std::uniform_real_distribution<float>{-50.0f, 50.0f};
    auto xy = std::vector<float>(pointCount * 2);
    for (auto& value : xy) {
        value = coordinate(engine);
    }
    // A dense clump stresses cells with many entries.
    for (auto i = 0; i < 500; ++i) {
        xy[static_cast<std::size_t>(i) * 2] = 10.0f + coordinate(engine) * 0.01f;
        xy[static_cast<std::size_t>(i) * 2 + 1] = -5.0f + coordinate(engine) * 0.01f;
    }

    auto index = PointSpatialIndex{};
    index.build(xy.data(), pointCount);

    for (auto query = 0; query < 2000; ++query) {
        const auto x = static_cast<double>(coordinate(engine)) * 1.1;
        const auto y = static_cast<double>(coordinate(engine)) * 1.1;
        const auto radiusX = 0.05 + std::abs(coordinate(engine)) * 0.05;
        const auto radiusY = 0.05 + std::abs(coordinate(engine)) * 0.05;
        QCOMPARE(index.nearest(x, y, radiusX, radiusY), bruteForceNearest(xy, x, y, radiusX, radiusY));
    }
    QCOMPARE(index.nearest(10.0, -5.0, 1.0, 1.0), bruteForceNearest(xy, 10.0, -5.0, 1.0, 1.0));
}

void PointSpatialIndexTest::nonFinitePointsAreSkipped()
{
    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();
    constexpr auto inf = std::numeric_limits<float>::infinity();
    const auto data = std::array<float, 8>{nan, 0.0f, 0.0f, inf, 0.0f, 0.0f, 1e30f, 1e30f};
    auto index = PointSpatialIndex{};
    index.build(data.data(), 4);

    QCOMPARE(index.validPointCount(), 2);
    QCOMPARE(index.nearest(0.0, 0.0, 0.5, 0.5), 2);
    QCOMPARE(index.nearest(1e30, 1e30, 1e25, 1e25), 3);
}

void PointSpatialIndexTest::logarithmicMappingSkipsNonPositivePoints()
{
    const auto data = std::array<float, 8>{0.0f, 10.0f, -1.0f, 10.0f, 100.0f, 1000.0f, 10.0f, 0.0f};
    auto index = PointSpatialIndex{};
    index.build(data.data(), 4, 2, PointSpatialIndex::Mapping{true, true});

    QCOMPARE(index.validPointCount(), 1);
    QVERIFY(index.mapping() == (PointSpatialIndex::Mapping{true, true}));
    // Point 2 sits at (2, 3) in log10 space.
    QCOMPARE(index.nearest(2.0, 3.0, 0.01, 0.01), 2);

    auto mapped = 0.0;
    QVERIFY(!PointSpatialIndex::mapCoordinate(0.0, true, mapped));
    QVERIFY(PointSpatialIndex::mapCoordinate(-4.0, false, mapped));
    QCOMPARE(mapped, -4.0);
}

void PointSpatialIndexTest::anisotropicRadiusUsesNormalizedDistance()
{
    // Point 0 is closer in raw distance, point 1 is closer relative to the radii.
    const auto data = std::array<float, 4>{0.0f, 0.9f, 0.9f, 0.0f};
    auto index = PointSpatialIndex{};
    index.build(data.data(), 2);

    QCOMPARE(index.nearest(0.0, 0.0, 10.0, 1.0), 1);
    QCOMPARE(index.nearest(0.0, 0.0, 1.0, 10.0), 0);
}

void PointSpatialIndexTest::strideSkipsExtraComponents()
{
    const auto data = std::array<float, 6>{1.0f, 1.0f, 99.0f, 5.0f, 5.0f, -99.0f};
    auto index = PointSpatialIndex{};
    index.build(data.data(), 2, 3);

    QCOMPARE(index.validPointCount(), 2);
    QCOMPARE(index.nearest(5.0, 5.0, 0.1, 0.1), 1);
    QCOMPARE(index.nearest(99.0, 5.0, 0.1, 0.1), -1);
}

void PointSpatialIndexTest::nonPositiveRadiusReturnsMinusOne()
{
    const auto data = std::array<float, 2>{0.0f, 0.0f};
    auto index = PointSpatialIndex{};
    index.build(data.data(), 1);

    QCOMPARE(index.nearest(0.0, 0.0, 0.0, 1.0), -1);
    QCOMPARE(index.nearest(0.0, 0.0, 1.0, -1.0), -1);
}

} // namespace QAccelPlot

using QAccelPlot::PointSpatialIndexTest;
QTEST_GUILESS_MAIN(PointSpatialIndexTest)
#include "tst_point_spatial_index.moc"
