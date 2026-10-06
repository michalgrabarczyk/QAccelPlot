//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/PointSpatialIndex.hpp"

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
    void scanMatchesIndex();
    void scanMatchesIndexOnLogarithmicAxes();
    void scanIgnoresInvalidArguments();
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

void PointSpatialIndexTest::scanMatchesIndex()
{
    constexpr auto pointCount = 10000;
    constexpr auto stride = 3;
    auto engine = std::mt19937{7u};
    auto coordinate = std::uniform_real_distribution<float>{-50.0f, 50.0f};
    auto data = std::vector<float>(pointCount * stride);
    for (auto& value : data) {
        value = coordinate(engine);
    }
    // Duplicates exercise the tie rule; non-finite points must be skipped by both paths.
    for (auto i = std::size_t{0}; i < 200; ++i) {
        data[(i + 200) * stride] = data[i * stride];
        data[(i + 200) * stride + 1] = data[i * stride + 1];
    }
    data[400 * stride] = std::numeric_limits<float>::quiet_NaN();
    data[401 * stride + 1] = std::numeric_limits<float>::infinity();
    data[402 * stride] = -std::numeric_limits<float>::infinity();

    auto index = PointSpatialIndex{};
    index.build(data.data(), pointCount, stride);

    auto hits = 0;
    for (auto query = 0; query < 4000; ++query) {
        // Half of the queries sit exactly on a point, which also lands duplicates at distance zero.
        const auto onPoint = static_cast<std::size_t>(query % 1000) * stride;
        const auto x = query % 2 == 0 ? static_cast<double>(data[onPoint]) : static_cast<double>(coordinate(engine)) * 1.1;
        const auto y = query % 2 == 0 ? static_cast<double>(data[onPoint + 1]) : static_cast<double>(coordinate(engine)) * 1.1;
        const auto radiusX = 0.05 + std::abs(coordinate(engine)) * 0.05;
        const auto radiusY = 0.05 + std::abs(coordinate(engine)) * 0.05;
        const auto expected = index.nearest(x, y, radiusX, radiusY);
        QCOMPARE(PointSpatialIndex::nearestByScan(data.data(), pointCount, stride, {}, x, y, radiusX, radiusY), expected);
        hits += expected >= 0 ? 1 : 0;
    }
    QVERIFY(hits > 1000);
}

void PointSpatialIndexTest::scanMatchesIndexOnLogarithmicAxes()
{
    constexpr auto pointCount = 5000;
    auto engine = std::mt19937{11u};
    auto exponent = std::uniform_real_distribution<float>{-6.0f, 6.0f};
    auto data = std::vector<float>(pointCount * 2);
    for (auto& value : data) {
        value = std::pow(10.0f, exponent(engine));
    }
    // Not drawable on a logarithmic axis.
    data[0] = 0.0f;
    data[3] = -5.0f;
    data[4] = std::numeric_limits<float>::quiet_NaN();

    for (const auto mapping : {PointSpatialIndex::Mapping{true, true}, PointSpatialIndex::Mapping{true, false}, PointSpatialIndex::Mapping{false, true}}) {
        auto index = PointSpatialIndex{};
        index.build(data.data(), pointCount, 2, mapping);

        auto hits = 0;
        for (auto query = 0; query < 3000; ++query) {
            const auto onPoint = static_cast<std::size_t>(10 + query) * 2;
            auto x = 0.0;
            auto y = 0.0;
            QVERIFY(PointSpatialIndex::mapCoordinate(data[onPoint], mapping.logX, x));
            QVERIFY(PointSpatialIndex::mapCoordinate(data[onPoint + 1], mapping.logY, y));
            // A linear dimension spans twelve decades, so its radius scales with the value.
            const auto radiusX = mapping.logX ? 0.02 : 0.02 * std::abs(x);
            const auto radiusY = mapping.logY ? 0.02 : 0.02 * std::abs(y);
            // Offsets up to and just past the radius probe the boundary of the ellipse.
            const auto offset = 1.2 * (query % 7) / 6.0;
            const auto expected = index.nearest(x + offset * radiusX, y, radiusX, radiusY);
            QCOMPARE(PointSpatialIndex::nearestByScan(data.data(), pointCount, 2, mapping, x + offset * radiusX, y, radiusX, radiusY), expected);
            hits += expected >= 0 ? 1 : 0;
        }
        QVERIFY(hits > 1000);
    }
}

void PointSpatialIndexTest::scanIgnoresInvalidArguments()
{
    const auto data = std::array<float, 2>{0.0f, 0.0f};
    constexpr auto nan = std::numeric_limits<double>::quiet_NaN();

    QCOMPARE(PointSpatialIndex::nearestByScan(data.data(), 1, 2, {}, 0.0, 0.0, 1.0, 1.0), 0);
    QCOMPARE(PointSpatialIndex::nearestByScan(nullptr, 1, 2, {}, 0.0, 0.0, 1.0, 1.0), -1);
    QCOMPARE(PointSpatialIndex::nearestByScan(data.data(), 0, 2, {}, 0.0, 0.0, 1.0, 1.0), -1);
    QCOMPARE(PointSpatialIndex::nearestByScan(data.data(), 1, 1, {}, 0.0, 0.0, 1.0, 1.0), -1);
    QCOMPARE(PointSpatialIndex::nearestByScan(data.data(), 1, 2, {}, 0.0, 0.0, 0.0, 1.0), -1);
    QCOMPARE(PointSpatialIndex::nearestByScan(data.data(), 1, 2, {}, nan, 0.0, 1.0, 1.0), -1);
}

} // namespace QAccelPlot

using QAccelPlot::PointSpatialIndexTest;
QTEST_GUILESS_MAIN(PointSpatialIndexTest)
#include "tst_point_spatial_index.moc"
