//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/DataPointStore.hpp"

#include <QtTest/QtTest>

#include <array>
#include <limits>
#include <tuple>
#include <vector>

namespace QAccelPlot {

namespace {

using Dimension = DataPointStore::Dimension;
using RangeRule = DataPointStore::RangeRule;

constexpr auto kInf = std::numeric_limits<double>::infinity();
constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();
constexpr auto kEpochMs = 1.7e12;

DataPointStore xyStore(const RangeRule rule = RangeRule::EachCoordinate)
{
    return DataPointStore{DataPointStore::Layout{{Dimension::X, Dimension::Y}, rule, 0}};
}

DataPointStore::DimensionState linear(const double min, const double max)
{
    return DataPointStore::DimensionState{false, DataPointStore::Viewport{min, max}};
}

DataPointStore::DimensionState logarithmic(const double min, const double max)
{
    return DataPointStore::DimensionState{true, DataPointStore::Viewport{min, max}};
}

std::vector<float> renderValues(DataPointStore& store)
{
    return store.renderData();
}

} // namespace

class DataPointStoreTest : public QObject {
    Q_OBJECT

private slots:
    void movedFloatDataIsTheRenderData();
    void copiedFloatDataReusesItsAllocation();
    void doubleDataIsDrawnRelativeToTheFirstFiniteCoordinate();
    void renderOriginMovesToTheViewportWhenFarAway();
    void logarithmicDimensionsAreNotShifted();
    void scaleChangeRebuildsTheRenderData();
    void eachCoordinateRuleJudgesCoordinatesAlone();
    void wholeDataPointRuleDropsInvalidDataPoints();
    void skipOnNaNRuleIgnoresInfiniteEdgesOnly();
    void extentsCoverARangeOfDataPoints();
    void columnsShareTheirDimensionsExtent();
    void appendPromotesFloatData();
    void appendExtendsTheRenderDataInPlace();
    void appendRebuildsWhenTheOriginIsUnsettled();
    void floatAttributesAreInterleaved();
    void attributesCanBeReplacedOnFloatData();
    void doubleAttributesFollowTheCoordinatesInRenderData();
    void drawnDataPointsReplaceTheRenderedOnes();
    void skipInvalidLeavesInvalidDataPointsOut();
    void skipInvalidFollowsTheScale();
    void categoriesAreDroppedWhenTheCountChanges();
    void clearEmptiesTheStore();
};

void DataPointStoreTest::movedFloatDataIsTheRenderData()
{
    auto store = xyStore();
    auto data = std::vector<float>{1.0F, 2.0F, 3.0F, 4.0F};
    const auto* buffer = data.data();

    store.setFloat(std::move(data), 2);

    QCOMPARE(store.count(), 2);
    QVERIFY(!store.isPrecise());
    QCOMPARE(store.renderData().data(), buffer);
    QCOMPARE(store.renderStride(), 2);
    QCOMPARE(store.renderCount(), 2);
    QCOMPARE(store.renderOrigin(Dimension::X), 0.0);
    QCOMPARE(store.value(1, 1), 4.0);
}

void DataPointStoreTest::copiedFloatDataReusesItsAllocation()
{
    auto store = xyStore();
    const auto first = std::vector<float>{1.0F, 2.0F, 3.0F, 4.0F};
    store.assignFloat(first.data(), 2);
    const auto* buffer = store.renderData().data();
    const auto revision = store.renderRevision();

    const auto second = std::vector<float>{5.0F, 6.0F};
    store.assignFloat(second.data(), 1);

    QCOMPARE(store.renderData().data(), buffer);
    QCOMPARE(renderValues(store), second);
    QVERIFY(store.renderRevision() != revision);
}

void DataPointStoreTest::doubleDataIsDrawnRelativeToTheFirstFiniteCoordinate()
{
    auto store = xyStore();
    store.setDimensionState(Dimension::X, linear(kEpochMs, kEpochMs + 10.0));
    store.setDimensionState(Dimension::Y, linear(0.0, 10.0));

    store.setPrecise({kNaN, kInf, kEpochMs + 1.0, 5.0, kEpochMs + 2.0, 6.0}, 3);

    QVERIFY(store.isPrecise());
    QCOMPARE(store.renderOrigin(Dimension::X), kEpochMs + 1.0);
    QCOMPARE(store.renderOrigin(Dimension::Y), 5.0);
    const auto render = renderValues(store);
    QVERIFY(std::isnan(render[0]));
    QCOMPARE(render[2], 0.0F);
    QCOMPARE(render[4], 1.0F);
    QCOMPARE(render[5], 1.0F);
}

void DataPointStoreTest::renderOriginMovesToTheViewportWhenFarAway()
{
    auto store = xyStore();
    store.setDimensionState(Dimension::X, linear(0.0, 10.0));
    store.setPrecise({0.0, 0.0, kEpochMs, 1.0}, 2);
    QCOMPARE(store.renderOrigin(Dimension::X), 0.0);
    const auto revision = store.renderRevision();

    store.setDimensionState(Dimension::X, linear(5.0, 15.0));
    QCOMPARE(store.renderRevision(), revision);

    store.setDimensionState(Dimension::X, linear(kEpochMs - 5.0, kEpochMs + 5.0));
    QVERIFY(store.renderRevision() != revision);
    QCOMPARE(renderValues(store)[2], 0.0F);
    QCOMPARE(store.renderOrigin(Dimension::X), kEpochMs);
}

void DataPointStoreTest::logarithmicDimensionsAreNotShifted()
{
    auto store = xyStore();
    store.setDimensionState(Dimension::Y, logarithmic(1.0, 1000.0));
    store.setPrecise({100.0, 10.0, 200.0, 100.0}, 2);

    const auto render = renderValues(store);

    QCOMPARE(store.renderOrigin(Dimension::X), 100.0);
    QCOMPARE(store.renderOrigin(Dimension::Y), 0.0);
    QCOMPARE(render[1], 10.0F);
    QCOMPARE(render[3], 100.0F);
}

void DataPointStoreTest::scaleChangeRebuildsTheRenderData()
{
    auto store = xyStore();
    store.setPrecise({100.0, 10.0}, 1);
    QCOMPARE(renderValues(store)[1], 0.0F);
    const auto revision = store.renderRevision();

    store.setDimensionState(Dimension::Y, logarithmic(1.0, 1000.0));

    QVERIFY(store.renderRevision() != revision);
    QCOMPARE(renderValues(store)[1], 10.0F);
}

void DataPointStoreTest::eachCoordinateRuleJudgesCoordinatesAlone()
{
    auto store = xyStore(RangeRule::EachCoordinate);
    store.setPrecise({1.0, kNaN, 2.0, -3.0, kInf, 4.0}, 3);

    const auto linearExtents = store.extents(false, false);
    QCOMPARE(linearExtents.x->min, 1.0);
    QCOMPARE(linearExtents.x->max, 2.0);
    QCOMPARE(linearExtents.y->min, -3.0);
    QCOMPARE(linearExtents.y->max, 4.0);

    const auto logExtents = store.extents(false, true);
    QCOMPARE(logExtents.y->min, 4.0);
    QCOMPARE(logExtents.y->max, 4.0);

    auto floats = xyStore(RangeRule::EachCoordinate);
    floats.setFloat({1.0F, 2.0F, 3.0F, 4.0F}, 2);
    QCOMPARE(floats.extents(false, false).y->max, 4.0);
}

void DataPointStoreTest::wholeDataPointRuleDropsInvalidDataPoints()
{
    auto store = xyStore(RangeRule::WholeDataPoint);
    store.setPrecise({1.0, kNaN, 2.0, -3.0, 5.0, 4.0}, 3);

    const auto linearExtents = store.extents(false, false);
    QCOMPARE(linearExtents.x->min, 2.0);
    QCOMPARE(linearExtents.x->max, 5.0);

    const auto logExtents = store.extents(false, true);
    QCOMPARE(logExtents.x->min, 5.0);
    QCOMPARE(logExtents.y->min, 4.0);

    store.setPrecise({1.0, kNaN}, 1);
    const auto none = store.extents(false, false);
    QVERIFY(!none.x);
    QVERIFY(!none.y);
}

void DataPointStoreTest::skipOnNaNRuleIgnoresInfiniteEdgesOnly()
{
    auto store = DataPointStore{DataPointStore::Layout{{Dimension::X, Dimension::Y, Dimension::X, Dimension::Y}, RangeRule::SkipOnNaN, 0}};
    store.setPrecise({8.0, -kInf, 12.0, kInf, -50.0, -50.0, 50.0, kNaN, 0.0, 5.0, 1.0, 6.0}, 3);

    const auto linearExtents = store.extents(false, false);
    QCOMPARE(linearExtents.x->min, 0.0);
    QCOMPARE(linearExtents.x->max, 12.0);
    QCOMPARE(linearExtents.y->min, 5.0);
    QCOMPARE(linearExtents.y->max, 6.0);

    const auto logExtents = store.extents(true, false);
    QCOMPARE(logExtents.x->min, 1.0);
}

void DataPointStoreTest::extentsCoverARangeOfDataPoints()
{
    auto store = xyStore();
    store.setPrecise({1.0, 10.0, 2.0, 20.0, 3.0, 30.0, 4.0, kInf}, 4);

    const auto middle = store.extents(false, false, 1, 3);
    QCOMPARE(middle.x->min, 2.0);
    QCOMPARE(middle.x->max, 3.0);
    QCOMPARE(middle.y->max, 30.0);

    const auto tail = store.extents(false, false, 3);
    QCOMPARE(tail.x->min, 4.0);
    QVERIFY(!tail.y);

    QVERIFY(!store.extents(false, false, 2, 2).x);
}

void DataPointStoreTest::columnsShareTheirDimensionsExtent()
{
    auto store = DataPointStore{DataPointStore::Layout{{Dimension::X, Dimension::Y, Dimension::Y}, RangeRule::EachCoordinate, 0}};
    store.setPrecise({1.0, -2.0, 3.0, 2.0, 0.5, 9.0}, 2);

    const auto extents = store.extents(false, false);

    QCOMPARE(extents.y->min, -2.0);
    QCOMPARE(extents.y->max, 9.0);
}

void DataPointStoreTest::appendPromotesFloatData()
{
    auto store = xyStore();
    store.setFloat({1.0F, 2.0F}, 1);
    const auto point = std::array<double, 2>{kEpochMs, 3.0};

    store.append(point.data());

    QVERIFY(store.isPrecise());
    QCOMPARE(store.count(), 2);
    QCOMPARE(store.value(0, 0), 1.0);
    QCOMPARE(store.value(1, 0), kEpochMs);
}

void DataPointStoreTest::appendExtendsTheRenderDataInPlace()
{
    auto store = xyStore();
    store.setDimensionState(Dimension::X, linear(kEpochMs, kEpochMs + 10.0));
    store.setPrecise({kEpochMs, 1.0}, 1);
    std::ignore = store.renderData();
    const auto revision = store.renderRevision();

    const auto point = std::array<double, 2>{kEpochMs + 1.0, 2.0};
    store.append(point.data());

    QVERIFY(store.renderRevision() != revision);
    QCOMPARE(store.renderCount(), 2);
    const auto render = renderValues(store);
    QCOMPARE(render.size(), std::size_t{4});
    QCOMPARE(render[2], 1.0F);
    QCOMPARE(render[3], 1.0F);
}

void DataPointStoreTest::appendRebuildsWhenTheOriginIsUnsettled()
{
    auto store = xyStore();
    store.setPrecise({kNaN, kNaN}, 1);
    std::ignore = store.renderData();

    const auto point = std::array<double, 2>{kEpochMs, 2.0};
    store.append(point.data());

    const auto render = renderValues(store);
    QCOMPARE(store.renderOrigin(Dimension::X), kEpochMs);
    QCOMPARE(render[2], 0.0F);
}

void DataPointStoreTest::floatAttributesAreInterleaved()
{
    auto store = DataPointStore{DataPointStore::Layout{{Dimension::X, Dimension::Y}, RangeRule::WholeDataPoint, 1}};

    store.setFloat({1.0F, 2.0F, 3.0F, 4.0F}, 2, {10.0F, 20.0F});

    QVERIFY(store.hasAttributes());
    QCOMPARE(store.renderStride(), 3);
    QCOMPARE(renderValues(store), (std::vector<float>{1.0F, 2.0F, 10.0F, 3.0F, 4.0F, 20.0F}));
    QCOMPARE(store.attribute(1), 20.0F);
    QCOMPARE(store.view().value(1, 1), 4.0);
    QCOMPARE(store.attributes().stride, 3);
}

void DataPointStoreTest::attributesCanBeReplacedOnFloatData()
{
    auto store = DataPointStore{DataPointStore::Layout{{Dimension::X, Dimension::Y}, RangeRule::WholeDataPoint, 1}};
    store.setFloat({1.0F, 2.0F, 3.0F, 4.0F}, 2);
    QVERIFY(!store.hasAttributes());
    QVERIFY(std::isnan(store.attribute(0)));

    store.setAttributes({7.0F, 8.0F});
    QCOMPARE(renderValues(store), (std::vector<float>{1.0F, 2.0F, 7.0F, 3.0F, 4.0F, 8.0F}));

    store.setAttributes({});
    QCOMPARE(renderValues(store), (std::vector<float>{1.0F, 2.0F, 3.0F, 4.0F}));
}

void DataPointStoreTest::doubleAttributesFollowTheCoordinatesInRenderData()
{
    auto store = DataPointStore{DataPointStore::Layout{{Dimension::X, Dimension::Y}, RangeRule::WholeDataPoint, 1}};
    store.setPrecise({kEpochMs, 1.0, kEpochMs + 2.0, 3.0}, 2, {10.0F, 20.0F});

    QCOMPARE(renderValues(store), (std::vector<float>{0.0F, 0.0F, 10.0F, 2.0F, 2.0F, 20.0F}));
    QCOMPARE(store.attributes().stride, 1);

    store.promoteToPrecise();
    QCOMPARE(store.attribute(1), 20.0F);
}

void DataPointStoreTest::drawnDataPointsReplaceTheRenderedOnes()
{
    auto store = xyStore();
    store.setFloat({1.0F, 2.0F}, 1);

    store.setDrawn({kEpochMs, 5.0, kEpochMs + 1.0, 6.0}, 2);

    QVERIFY(store.hasDrawn());
    QCOMPARE(store.count(), 1);
    QCOMPARE(store.view().count, 1);
    QCOMPARE(store.drawnView().count, 2);
    QCOMPARE(store.renderCount(), 2);
    QCOMPARE(store.renderOrigin(Dimension::X), kEpochMs);
    QCOMPARE(renderValues(store), (std::vector<float>{0.0F, 0.0F, 1.0F, 1.0F}));
    QCOMPARE(store.extents(false, false).x->max, 1.0);

    store.clearDrawn();
    QCOMPARE(renderValues(store), (std::vector<float>{1.0F, 2.0F}));
    QCOMPARE(store.renderOrigin(Dimension::X), 0.0);
}

void DataPointStoreTest::skipInvalidLeavesInvalidDataPointsOut()
{
    auto floats = xyStore();
    floats.setFloat({1.0F, 2.0F, std::numeric_limits<float>::quiet_NaN(), 3.0F, 4.0F, 5.0F}, 3);
    floats.setSkipInvalid(true);
    QCOMPARE(floats.renderCount(), 2);
    QCOMPARE(renderValues(floats), (std::vector<float>{1.0F, 2.0F, 4.0F, 5.0F}));
    QCOMPARE(floats.drawnView().value(1, 0), 4.0);
    QCOMPARE(floats.view().count, 3);

    auto doubles = xyStore();
    doubles.setPrecise({kEpochMs, 1.0, kEpochMs + 1.0, kInf, kEpochMs + 2.0, 3.0}, 3);
    doubles.setSkipInvalid(true);
    QCOMPARE(renderValues(doubles), (std::vector<float>{0.0F, 0.0F, 2.0F, 2.0F}));
    QCOMPARE(doubles.drawnView().value(1, 0), kEpochMs + 2.0);

    auto valid = xyStore();
    valid.setFloat({1.0F, 2.0F}, 1);
    const auto* buffer = valid.renderData().data();
    valid.setSkipInvalid(true);
    QCOMPARE(valid.renderData().data(), buffer);
}

void DataPointStoreTest::skipInvalidFollowsTheScale()
{
    auto store = xyStore();
    store.setFloat({1.0F, 0.0F, 2.0F, 3.0F}, 2);
    store.setSkipInvalid(true);
    QCOMPARE(store.renderCount(), 2);

    store.setDimensionState(Dimension::Y, logarithmic(1.0, 10.0));
    QCOMPARE(store.renderCount(), 1);
    QCOMPARE(renderValues(store), (std::vector<float>{2.0F, 3.0F}));

    store.setSkipInvalid(false);
    QCOMPARE(store.renderCount(), 2);
}

void DataPointStoreTest::categoriesAreDroppedWhenTheCountChanges()
{
    auto store = xyStore();
    store.setPrecise({1.0, 2.0, 3.0, 4.0}, 2);
    store.setCategories({0, 1});
    QVERIFY(store.hasCategories());

    store.setPrecise({5.0, 6.0, 7.0, 8.0}, 2);
    QCOMPARE(store.categories(), (std::vector<int>{0, 1}));

    store.setPrecise({5.0, 6.0}, 1);
    QVERIFY(!store.hasCategories());
}

void DataPointStoreTest::clearEmptiesTheStore()
{
    auto store = DataPointStore{DataPointStore::Layout{{Dimension::X, Dimension::Y}, RangeRule::WholeDataPoint, 1}};
    store.setPrecise({1.0, 2.0}, 1, {3.0F});
    const auto revision = store.renderRevision();

    store.clear();

    QCOMPARE(store.count(), 0);
    QVERIFY(!store.isPrecise());
    QVERIFY(!store.hasAttributes());
    QCOMPARE(store.renderCount(), 0);
    QVERIFY(store.renderData().empty());
    QVERIFY(store.renderRevision() != revision);
}

} // namespace QAccelPlot

QTEST_APPLESS_MAIN(QAccelPlot::DataPointStoreTest)
#include "tst_data_point_store.moc"
