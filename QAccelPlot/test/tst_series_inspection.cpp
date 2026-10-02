//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include <QAccelPlot/series/LineCurve.hpp>
#include <QAccelPlot/series/PointCloud.hpp>
#include <QAccelPlot/transitions/MorphTransition.hpp>

#include <QtTest>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <vector>

using namespace QAccelPlot;

namespace {

constexpr auto kInfinity = std::numeric_limits<double>::infinity();
constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();

// Binds a standalone series to axes with a given viewport and a pixel size.
void bind(PlotSeries& series, Axis& x, Axis& y, const QRectF& viewport, const QSizeF& size)
{
    y.setSide(Axis::Left);
    x.setViewportMin(viewport.left());
    x.setViewportMax(viewport.right());
    y.setViewportMin(viewport.top());
    y.setViewportMax(viewport.bottom());
    series.setXAxis(&x);
    series.setYAxis(&y);
    series.setSize(size);
}

std::vector<double> randomPoints(const int count, const unsigned seed)
{
    auto generator = std::mt19937{seed};
    auto distribution = std::uniform_real_distribution<double>{-100, 100};
    auto data = std::vector<double>(static_cast<std::size_t>(count) * 2);
    for (auto& value : data) {
        value = distribution(generator);
    }
    return data;
}

// Compares every query of a point cloud against a linear scan of its records.
void compareAgainstScan(PointCloud& cloud, const std::vector<double>& data, const int count)
{
    auto* inspection = cloud.inspection();
    auto generator = std::mt19937{7};
    auto distribution = std::uniform_real_distribution<double>{0, 1};
    for (auto query = 0; query < 60; ++query) {
        const auto position = QPointF{distribution(generator) * 800, distribution(generator) * 200};
        auto nearest = -1;
        auto nearestDistance = 20.0;
        auto nearestX = -1;
        auto nearestXDistance = kInfinity;
        auto inside = 0;
        for (auto i = 0; i < count; ++i) {
            const auto pixel = inspection->sampleAt(i).pixelPosition;
            const auto distance = std::hypot(pixel.x() - position.x(), pixel.y() - position.y());
            if (distance <= nearestDistance) {
                nearestDistance = distance;
                nearest = i;
            }
            if (std::abs(pixel.x() - position.x()) <= nearestXDistance) {
                nearestXDistance = std::abs(pixel.x() - position.x());
                nearestX = i;
            }
            const auto x = data[static_cast<std::size_t>(i) * 2];
            const auto y = data[static_cast<std::size_t>(i) * 2 + 1];
            inside += x >= -30 && x <= 40 && y >= -20 && y <= 30 ? 1 : 0;
        }
        QCOMPARE(inspection->nearest(position, 20).index, nearest);
        QCOMPARE(inspection->nearestByX(position.x()).index, nearestX);
        QCOMPARE(inspection->summarize(-30, 40, -20, 30).count, inside);
    }
}

} // namespace

class TestSeriesInspection : public QObject {
    Q_OBJECT
private slots:
    void precisionAndGaps();
    void summariesAndPages();
    void inclusiveLimits();
    void replacementAndScale();
    void bracketEnds();
    void interpolation();
    void argumentsAndAxisLifetime();
    void transition();
    void unorderedScan();
    void unorderedIndex();
    void coincidentPoints();
    void zeroRadiusMapping();
    void appendedRecords();
};

void TestSeriesInspection::precisionAndGaps()
{
    auto curve = LineCurve{};
    auto x = Axis{};
    auto y = Axis{};
    bind(curve, x, y, {QPointF{1e12, 0}, QPointF{1e12 + 1, 10}}, {1000, 200});
    curve.setData(std::vector<double>{1e12, 1, 1e12 + .25, kNaN, 1e12 + .5, 4, 1e12 + .5, 5}, 4);
    curve.gaps()->setNanMode(NanGapMode::Connect);
    auto* inspection = curve.inspection();
    // Ordered X is searched in place: there is nothing to prepare.
    QCOMPARE(inspection->status(), InspectionStatus::Ready);
    QCOMPARE(inspection->indexBytes(), quint64{0});
    QCOMPARE(inspection->sampleAt(2).index, 2);
    QCOMPARE(inspection->sampleAt(2).x(), 1e12 + .5);
    const auto gap = inspection->sampleAt(1);
    QCOMPARE(gap.status, InspectionStatus::NoMatch);
    QCOMPARE(gap.x(), 1e12 + .25);
    QVERIFY(std::isnan(gap.y()));
    QCOMPARE(inspection->sampleAt(4).status, InspectionStatus::InvalidArgument);
    // Equal distances resolve to the highest index.
    QCOMPARE(inspection->nearestByX(500, 0).index, 3);
    QCOMPARE(inspection->nearest({500, 120}, 1).index, 2);
    const auto bracket = inspection->bracketByX(300);
    QCOMPARE(bracket.left.index, 0);
    QCOMPARE(bracket.right.index, 2);
    QVERIFY(!bracket.adjacent);
    QVERIFY(!bracket.interpolated.valid());
}

void TestSeriesInspection::summariesAndPages()
{
    auto cloud = PointCloud{};
    cloud.setData(std::vector<double>{0, 1, 1, 2, 2, 3, 3, 4}, 4);
    auto* inspection = cloud.inspection();
    const auto stats = inspection->summarize(0, 3, 0, 4);
    QVERIFY(stats.valid());
    QCOMPARE(stats.count, 4);
    QCOMPARE(stats.minimumIndex, 0);
    QCOMPARE(stats.maximumIndex, 3);
    QCOMPARE(stats.mean, 2.5);
    QVERIFY(std::abs(stats.standardDeviation - std::sqrt(1.25)) < 1e-12);
    // Reversed limits are swapped and a degenerate region is still a region.
    QCOMPARE(inspection->summarizeRange(2, 1).count, 2);
    QCOMPARE(inspection->summarize(2, 2, 3, 3).count, 1);
    const auto empty = inspection->summarize(20, 20, 0, 0);
    QCOMPARE(empty.status, InspectionStatus::NoMatch);
    QCOMPARE(empty.count, 0);
    QVERIFY(std::isnan(empty.mean));

    const auto first = inspection->indices(0, 3, 0, 4, 0, 3);
    QVERIFY(first.valid());
    QCOMPARE(first.total, 4);
    QCOMPARE(first.limit, 3);
    QVERIFY(first.hasMore);
    QVERIFY(first.sourceOrder);
    QCOMPARE(first.indices, (QList<int>{0, 1, 2}));
    const auto second = inspection->indices(0, 3, 0, 4, 3, 3);
    QVERIFY(!second.hasMore);
    QCOMPARE(second.indices, (QList<int>{3}));
    // The page size is clamped and the effective value reported.
    QCOMPARE(inspection->indices(0, 3, 0, 4, 0, 1000000).limit, inspection->maximumPageSize());
    QCOMPARE(inspection->indices(0, 3, 0, 4, -1, 10).status, InspectionStatus::InvalidArgument);
    QCOMPARE(inspection->indices(0, 3, 0, 4, 0, 0).status, InspectionStatus::InvalidArgument);
    QCOMPARE(inspection->indices(kNaN, 3, 0, 4).status, InspectionStatus::InvalidArgument);
}

void TestSeriesInspection::inclusiveLimits()
{
    // 0.2 + (0.9 - 0.2) < 0.9, so limits stored as position and size would drop the sample at 0.9.
    auto cloud = PointCloud{};
    cloud.setData(std::vector<double>{0.2, 1, 0.9, 1}, 2);
    QCOMPARE(cloud.inspection()->summarize(0.2, 0.9, 0, 2).count, 2);
    QCOMPARE(cloud.inspection()->summarizeRange(0.2, 0.9).count, 2);
    QCOMPARE(cloud.inspection()->indices(0.2, 0.9, 1, 1).total, 2);
}

void TestSeriesInspection::replacementAndScale()
{
    auto cloud = PointCloud{};
    auto x = Axis{};
    auto y = Axis{};
    bind(cloud, x, y, {QPointF{0, 0}, QPointF{1, 1}}, {100, 100});
    auto revisions = QSignalSpy{&cloud, &PlotSeries::dataRevisionChanged};
    cloud.setDataF(std::vector<float>{-1, 2, 1, 3}, 2);
    const auto oldRevision = cloud.dataRevision();
    cloud.setDataFNoRange(std::vector<float>{-1, 8, 1, 9}, 2);
    QVERIFY(cloud.dataRevision() > oldRevision);
    QCOMPARE(revisions.count(), 2);
    auto* inspection = cloud.inspection();
    QCOMPARE(inspection->indices(-2, 2, 0, 10, 0, 10, oldRevision).status, InspectionStatus::Stale);
    QCOMPARE(inspection->indices(-2, 2, 0, 10, 0, 10, cloud.dataRevision()).total, 2);
    QCOMPARE(inspection->summarizeRange(-2, 2).maximum, 9.0);
    // A logarithmic axis invalidates the nonpositive sample.
    auto status = QSignalSpy{inspection, &SeriesInspection::statusChanged};
    x.setLogScale(true);
    QVERIFY(status.count() > 0);
    QCOMPARE(inspection->summarizeRange(-2, 2).count, 1);
    QCOMPARE(inspection->sampleAt(0).status, InspectionStatus::NoMatch);
    cloud.clearData();
    QCOMPARE(inspection->summarizeRange(-2, 2).count, 0);
    QCOMPARE(inspection->status(), InspectionStatus::Ready);
}

void TestSeriesInspection::bracketEnds()
{
    for (const auto ordered : {true, false}) {
        auto cloud = PointCloud{};
        auto x = Axis{};
        auto y = Axis{};
        bind(cloud, x, y, {QPointF{0, 0}, QPointF{4, 10}}, {400, 100});
        // The unordered variant holds the same samples and exercises the scanning queries.
        cloud.setData(ordered ? std::vector<double>{1, 1, 2, 2, 3, 3} : std::vector<double>{3, 3, 1, 1, 2, 2}, 3);
        auto* inspection = cloud.inspection();
        const auto xAt = [&](const double dataX, const bool left) {
            const auto bracket = inspection->bracketByX(x.coordToPixel(dataX, 400));
            return left ? bracket.left.x() : bracket.right.x();
        };
        // Before the first sample only a right neighbor exists.
        const auto before = inspection->bracketByX(x.coordToPixel(0.5, 400));
        QVERIFY(before.valid());
        QVERIFY(!before.left.valid());
        QCOMPARE(before.right.x(), 1.0);
        // A sample exactly under the cursor is the left neighbor, at both ends alike.
        QCOMPARE(xAt(1.0, true), 1.0);
        QCOMPARE(xAt(1.0, false), 2.0);
        QCOMPARE(xAt(1.5, true), 1.0);
        QCOMPARE(xAt(1.5, false), 2.0);
        QCOMPARE(xAt(3.0, true), 3.0);
        const auto last = inspection->bracketByX(x.coordToPixel(3.0, 400));
        QVERIFY(last.valid());
        QVERIFY(!last.right.valid());
        const auto after = inspection->bracketByX(x.coordToPixel(3.5, 400));
        QCOMPARE(after.left.x(), 3.0);
        QVERIFY(!after.right.valid());
    }
    auto empty = PointCloud{};
    auto x = Axis{};
    auto y = Axis{};
    bind(empty, x, y, {QPointF{0, 0}, QPointF{4, 10}}, {400, 100});
    QCOMPARE(empty.inspection()->bracketByX(100).status, InspectionStatus::NoMatch);
}

void TestSeriesInspection::interpolation()
{
    auto curve = LineCurve{};
    auto x = Axis{};
    auto y = Axis{};
    bind(curve, x, y, {QPointF{0, 1}, QPointF{10, 1000}}, {1000, 300});
    curve.setData(std::vector<double>{2, 10, 4, 1000}, 2);
    auto* inspection = curve.inspection();
    auto bracket = inspection->bracketByX(300);
    QVERIFY(bracket.adjacent);
    QVERIFY(bracket.interpolated.valid());
    QVERIFY(bracket.interpolated.interpolated);
    QCOMPARE(bracket.interpolated.index, 0);
    QCOMPARE(bracket.interpolated.x(), 3.0);
    QCOMPARE(bracket.interpolated.y(), 505.0);
    // On a logarithmic axis the drawn segment is straight in log space.
    y.setLogScale(true);
    bracket = inspection->bracketByX(300);
    QVERIFY(std::abs(bracket.interpolated.y() - 100.0) < 1e-9);
    QCOMPARE(bracket.interpolated.pixelPosition.x(), 300.0);
}

void TestSeriesInspection::argumentsAndAxisLifetime()
{
    auto cloud = PointCloud{};
    auto x = std::make_unique<Axis>();
    auto y = Axis{};
    bind(cloud, *x, y, {QPointF{-2, 0}, QPointF{2, 1}}, {100, 100});
    cloud.setData(std::vector<double>{-1, 2, 1, 3}, 2);
    auto* inspection = cloud.inspection();
    QCOMPARE(inspection->bracketByX(kNaN).status, InspectionStatus::InvalidArgument);
    QCOMPARE(inspection->nearestByX(kNaN).status, InspectionStatus::InvalidArgument);
    QCOMPARE(inspection->nearest({1, 1}, -1).status, InspectionStatus::InvalidArgument);
    QCOMPARE(inspection->summarize(0, 1, kNaN, 1).status, InspectionStatus::InvalidArgument);
    // A no-match result carries no coordinates.
    const auto miss = inspection->nearest({50, 50}, 0);
    QCOMPARE(miss.status, InspectionStatus::NoMatch);
    QCOMPARE(miss.index, -1);
    QVERIFY(std::isnan(miss.x()) && std::isnan(miss.distance));
    x->setLogScale(true);
    QCOMPARE(inspection->summarizeRange(-2, 2).count, 1);
    // Without an X axis pixel queries cannot run, but data-space queries still do.
    x.reset();
    QCOMPARE(inspection->nearestByX(50).status, InspectionStatus::Unavailable);
    QCOMPARE(inspection->summarizeRange(-2, 2).count, 2);
    auto oldAxis = Axis{};
    auto newAxis = Axis{};
    cloud.setXAxis(&oldAxis);
    cloud.setXAxis(&newAxis);
    auto changed = QSignalSpy{inspection, &SeriesInspection::statusChanged};
    oldAxis.setLogScale(true);
    QCOMPARE(changed.count(), 0);
}

void TestSeriesInspection::transition()
{
    auto curve = LineCurve{};
    auto animation = MorphTransition{};
    animation.setDuration(10000);
    curve.setTransition(&animation);
    curve.setData(std::vector<double>{1, 2, 3, 4}, 2);
    auto* inspection = curve.inspection();
    QCOMPARE(inspection->sampleAt(0).status, InspectionStatus::Unavailable);
    QCOMPARE(inspection->status(), InspectionStatus::Unavailable);
    QCOMPARE(inspection->summarizeRange(0, 10).status, InspectionStatus::Unavailable);
    animation.cancel();
    QCOMPARE(inspection->status(), InspectionStatus::Ready);
    QCOMPARE(inspection->sampleAt(1).position, QPointF(3, 4));
    QCOMPARE(inspection->summarizeRange(0, 10).count, 2);
}

void TestSeriesInspection::unorderedScan()
{
    auto cloud = PointCloud{};
    auto x = Axis{};
    auto y = Axis{};
    bind(cloud, x, y, {QPointF{-100, -100}, QPointF{100, 100}}, {800, 200});
    const auto count = 2000;
    const auto data = randomPoints(count, 42);
    cloud.setData(data.data(), count);
    // Small unordered series are scanned per query and never report Preparing.
    QCOMPARE(cloud.inspection()->status(), InspectionStatus::Ready);
    compareAgainstScan(cloud, data, count);
    QVERIFY(cloud.inspection()->indices(-30, 40, -20, 30).sourceOrder);
}

void TestSeriesInspection::unorderedIndex()
{
    auto cloud = PointCloud{};
    auto x = Axis{};
    auto y = Axis{};
    bind(cloud, x, y, {QPointF{-100, -100}, QPointF{100, 100}}, {800, 200});
    const auto count = 30000;
    const auto data = randomPoints(count, 43);
    cloud.setData(data.data(), count);
    auto* inspection = cloud.inspection();
    QCOMPARE(inspection->status(), InspectionStatus::Idle);
    QCOMPARE(inspection->nearestByX(100).status, InspectionStatus::Preparing);
    QCOMPARE(inspection->status(), InspectionStatus::Preparing);
    QTRY_COMPARE(inspection->status(), InspectionStatus::Ready);
    // The index stays well below the 44 bytes per point of the first implementation.
    QVERIFY(inspection->indexBytes() > 0);
    QVERIFY(inspection->indexBytes() < quint64{40} * count);
    compareAgainstScan(cloud, data, count);

    // Pages cover every match exactly once, in traversal order.
    auto collected = QList<int>{};
    auto page = InspectionPage{};
    do {
        page = inspection->indices(-30, 40, -20, 30, static_cast<int>(collected.size()), 1000, cloud.dataRevision());
        QVERIFY(page.valid());
        QVERIFY(!page.sourceOrder);
        collected += page.indices;
    } while (page.hasMore);
    QCOMPARE(collected.size(), page.total);
    std::sort(collected.begin(), collected.end());
    QVERIFY(std::adjacent_find(collected.cbegin(), collected.cend()) == collected.cend());

    // Replacing the data drops the index; an ordered replacement needs none and builds none.
    cloud.setData(std::vector<double>{0, 0, 1, 1}, 2);
    QCOMPARE(inspection->status(), InspectionStatus::Ready);
    QCOMPARE(inspection->nearestByX(100).index, 0);
    QTest::qWait(50);
    QCOMPARE(inspection->indexBytes(), quint64{0});
}

void TestSeriesInspection::coincidentPoints()
{
    auto cloud = PointCloud{};
    auto x = Axis{};
    auto y = Axis{};
    bind(cloud, x, y, {QPointF{0, 0}, QPointF{10, 10}}, {100, 100});
    const auto count = 100000;
    cloud.setData(std::vector<double>(static_cast<std::size_t>(count) * 2, 5), count);
    auto* inspection = cloud.inspection();
    QCOMPARE(inspection->nearest({50, 50}, 0).index, count - 1);
    QCOMPARE(inspection->nearest({53, 54}, 5).index, count - 1);
    QCOMPARE(inspection->nearestByX(50, 0).index, count - 1);
    QCOMPARE(inspection->summarize(5, 5, 5, 5).count, count);
    QCOMPARE(inspection->indices(5, 5, 5, 5, count - 10, 20).indices.size(), 10);
}

void TestSeriesInspection::zeroRadiusMapping()
{
    auto cloud = PointCloud{};
    auto x = Axis{};
    auto y = Axis{};
    bind(cloud, x, y, {QPointF{.001, .001}, QPointF{10000, 10000}}, {801, 397});
    cloud.setData(std::vector<double>{.003, 3.14, 7.13, .011, 1234.56, 4.78}, 3);
    for (const auto logarithmic : {false, true}) {
        x.setLogScale(logarithmic);
        y.setLogScale(logarithmic);
        for (auto index = 0; index < 3; ++index) {
            const auto position = cloud.inspection()->sampleAt(index).pixelPosition;
            QCOMPARE(cloud.inspection()->nearest(position, 0).index, index);
            QCOMPARE(cloud.inspection()->nearestByX(position.x(), 0).index, index);
        }
    }
}

void TestSeriesInspection::appendedRecords()
{
    auto curve = LineCurve{};
    auto x = Axis{};
    auto y = Axis{};
    const auto count = 200000;
    bind(curve, x, y, {QPointF{0, -2}, QPointF{count + 200.0, 2}}, {1000, 200});
    auto data = std::vector<double>(static_cast<std::size_t>(count) * 2);
    for (auto i = 0; i < count; ++i) {
        data[static_cast<std::size_t>(i) * 2] = i;
        data[static_cast<std::size_t>(i) * 2 + 1] = std::sin(i * 0.01);
    }
    curve.setData(std::move(data), count);
    auto* inspection = curve.inspection();
    const auto before = inspection->summarizeRange(-kInfinity, kInfinity);
    QCOMPARE(before.count, count);
    // A live stream stays queryable on every append, without the event loop running.
    for (auto i = 0; i < 100; ++i) {
        curve.appendData(count + i, 5);
        QCOMPARE(inspection->status(), InspectionStatus::Ready);
        QCOMPARE(inspection->nearestByX(x.coordToPixel(count + i, 1000)).index, count + i);
    }
    const auto after = inspection->summarizeRange(-kInfinity, kInfinity);
    QCOMPARE(after.count, count + 100);
    QCOMPARE(after.maximum, 5.0);
    QCOMPARE(after.maximumIndex, count + 99);
    QCOMPARE(inspection->indices(count, kInfinity, -kInfinity, kInfinity).total, 100);
    // An append that breaks the order falls back to an index.
    curve.appendData(0.5, 0);
    QCOMPARE(inspection->status(), InspectionStatus::Idle);
    QCOMPARE(inspection->summarizeRange(0, 1).status, InspectionStatus::Preparing);
    QTRY_COMPARE(inspection->status(), InspectionStatus::Ready);
    QCOMPARE(inspection->summarizeRange(0, 1).count, 3);
}

QTEST_MAIN(TestSeriesInspection)
#include "tst_series_inspection.moc"
