//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/series/PointCloud.hpp"

#include <QtTest/QtTest>

#include <cmath>
#include <limits>
#include <thread>
#include <vector>

namespace QAccelPlot {

namespace {

constexpr auto kNaN = std::numeric_limits<float>::quiet_NaN();
constexpr auto kInf = std::numeric_limits<float>::infinity();

struct AxisPair {
    Axis x;
    Axis y;

    AxisPair(const qreal min, const qreal max)
    {
        x.setSide(Axis::Bottom);
        y.setSide(Axis::Left);
        for (auto* axis : {&x, &y}) {
            axis->setViewportMin(min);
            axis->setViewportMax(max);
        }
    }
};

} // namespace

class PointCloudDataTest : public QObject {
    Q_OBJECT

private slots:
    void defaults();
    void hoverEnvironmentControlsAcceptance();
    void invalidArgumentsAreRejected();
    void countChangedTracksCountAndValuePresence();
    void dataRangesSkipNonFinitePoints();
    void dataRangesFollowLogarithmicAxes();
    void noRangeUpdateDoesNotReportRanges();
    void valueRangeResolvesDataAndFixedSources();
    void setValuesExpandsAndClearsInPlace();
    void accessorsReturnNaNOutOfRange();
    void clearDataResetsState();
    void postDataIsAppliedFromWorkerThread();
    void pointIndexAtFindsNearestPointWithinRadius();
    void pointIndexAtUsesLogarithmicMapping();
    void propertySettersClampAndNotify();
    void doubleDataKeepsPrecisionForLargeCoordinates();
    void doubleDataSurvivesLogScaleChange();
};

void PointCloudDataTest::defaults()
{
    const auto cloud = PointCloud{};
    QCOMPARE(cloud.count(), 0);
    QCOMPARE(cloud.hoveredIndex(), -1);
    QVERIFY(!cloud.hasValues());
    QVERIFY(cloud.marker());
    QCOMPARE(cloud.marker()->parent(), &cloud);
    QCOMPARE(cloud.marker()->shape(), PlotSeries::MarkerShape::Circle);
    QCOMPARE(cloud.marker()->size(), 3.0);
    QVERIFY(cloud.marker()->filled());
    QCOMPARE(cloud.marker()->strokeWidth(), 1.0);
    QVERIFY(cloud.colormap() == nullptr);
    QCOMPARE(cloud.legendSymbol(), PlotSeries::LegendSymbol::Marker);
    QCOMPARE(cloud.dataValueMin(), 0.0);
    QCOMPARE(cloud.dataValueMax(), 1.0);
}

void PointCloudDataTest::hoverEnvironmentControlsAcceptance()
{
    constexpr auto variableName = "QACCELPLOT_HOVER_ENABLED";
    const auto wasSet = qEnvironmentVariableIsSet(variableName);
    const auto previousValue = qgetenv(variableName);

    qunsetenv(variableName);
    const auto defaultCloud = PointCloud{};
    qputenv(variableName, "0");
    const auto hoverDisabledCloud = PointCloud{};

    if (wasSet) {
        qputenv(variableName, previousValue);
    } else {
        qunsetenv(variableName);
    }

    QVERIFY(defaultCloud.acceptHoverEvents());
    QVERIFY(!hoverDisabledCloud.acceptHoverEvents());
}

void PointCloudDataTest::invalidArgumentsAreRejected()
{
    auto cloud = PointCloud{};
    cloud.setDataF(std::vector<float>{1.0f, 2.0f}, 1);
    auto countSpy = QSignalSpy{&cloud, &PointCloud::countChanged};

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("PointCloud data point count cannot be negative.*"));
    cloud.setDataF(std::vector<float>{}, -1);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("PointCloud received 3 coordinates for 2 points.*"));
    cloud.setDataF(std::vector<float>{1.0f, 2.0f, 3.0f}, 2);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("PointCloud received 1 values for 2 points.*"));
    cloud.setDataF(std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f}, std::vector<float>{1.0f}, 2);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("PointCloud received a null data pointer.*"));
    cloud.setDataF(nullptr, 3);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("PointCloud received 3 values for 1 points.*"));
    cloud.setValues({1.0, 2.0, 3.0});

    QCOMPARE(cloud.count(), 1);
    QVERIFY(!cloud.hasValues());
    QCOMPARE(countSpy.count(), 0);
    QCOMPARE(cloud.pointAt(0), QPointF(1.0, 2.0));
}

void PointCloudDataTest::countChangedTracksCountAndValuePresence()
{
    auto cloud = PointCloud{};
    auto countSpy = QSignalSpy{&cloud, &PointCloud::countChanged};

    cloud.setDataF(std::vector<float>{0.0f, 0.0f, 1.0f, 1.0f}, 2);
    QCOMPARE(countSpy.count(), 1);

    // Same count, same value presence: streaming updates must not re-emit.
    cloud.setDataF(std::vector<float>{2.0f, 2.0f, 3.0f, 3.0f}, 2);
    QCOMPARE(countSpy.count(), 1);

    cloud.setDataF(std::vector<float>{2.0f, 2.0f, 3.0f, 3.0f}, std::vector<float>{0.5f, 0.7f}, 2);
    QCOMPARE(countSpy.count(), 2);
    QVERIFY(cloud.hasValues());

    const float raw[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
    cloud.setDataF(raw, 3);
    QCOMPARE(countSpy.count(), 3);
    QCOMPARE(cloud.count(), 3);
    QVERIFY(!cloud.hasValues());
    QCOMPARE(cloud.pointAt(2), QPointF(5.0, 6.0));
}

void PointCloudDataTest::dataRangesSkipNonFinitePoints()
{
    auto axes = AxisPair{0.0, 10.0};
    auto cloud = PointCloud{};
    cloud.setXAxis(&axes.x);
    cloud.setYAxis(&axes.y);
    auto xRangeSpy = QSignalSpy{&cloud, &PlotSeries::xDataRangeChanged};
    auto yRangeSpy = QSignalSpy{&cloud, &PlotSeries::yDataRangeChanged};

    cloud.setDataF(std::vector<float>{kNaN, 100.0f, -2.0f, 4.0f, 5.0f, kInf, 3.0f, -1.0f}, 4);

    QCOMPARE(xRangeSpy.count(), 1);
    QCOMPARE(xRangeSpy.last().at(0).toReal(), -2.0);
    QCOMPARE(xRangeSpy.last().at(1).toReal(), 3.0);
    QCOMPARE(yRangeSpy.last().at(0).toReal(), -1.0);
    QCOMPARE(yRangeSpy.last().at(1).toReal(), 4.0);
    // Invalid points keep their indices.
    QVERIFY(std::isnan(cloud.pointAt(0).x()));
    QCOMPARE(cloud.count(), 4);
}

void PointCloudDataTest::dataRangesFollowLogarithmicAxes()
{
    auto axes = AxisPair{1.0, 1000.0};
    auto cloud = PointCloud{};
    cloud.setXAxis(&axes.x);
    cloud.setYAxis(&axes.y);
    cloud.setDataF(std::vector<float>{-5.0f, 10.0f, 0.0f, 20.0f, 2.0f, 30.0f, 400.0f, 0.5f}, 4);

    auto xRangeSpy = QSignalSpy{&cloud, &PlotSeries::xDataRangeChanged};
    axes.x.setLogScale(true);
    QCOMPARE(xRangeSpy.count(), 1);
    QCOMPARE(xRangeSpy.last().at(0).toReal(), 2.0);
    QCOMPARE(xRangeSpy.last().at(1).toReal(), 400.0);

    auto yRangeSpy = QSignalSpy{&cloud, &PlotSeries::yDataRangeChanged};
    axes.x.setLogScale(false);
    QCOMPARE(xRangeSpy.last().at(0).toReal(), -5.0);
    QCOMPARE(yRangeSpy.count(), 0);
}

void PointCloudDataTest::noRangeUpdateDoesNotReportRanges()
{
    auto axes = AxisPair{0.0, 10.0};
    auto cloud = PointCloud{};
    cloud.setXAxis(&axes.x);
    cloud.setYAxis(&axes.y);
    auto xRangeSpy = QSignalSpy{&cloud, &PlotSeries::xDataRangeChanged};

    cloud.setDataFNoRange(std::vector<float>{1.0f, 1.0f, 9.0f, 9.0f}, {}, 2);
    QCOMPARE(xRangeSpy.count(), 0);
    QCOMPARE(cloud.count(), 2);
}

void PointCloudDataTest::valueRangeResolvesDataAndFixedSources()
{
    auto cloud = PointCloud{};
    auto rangeSpy = QSignalSpy{&cloud, &PointCloud::valueRangeChanged};

    cloud.setDataF(std::vector<float>{0.0f, 0.0f, 1.0f, 1.0f, 2.0f, 2.0f}, std::vector<float>{-4.0f, kNaN, 12.0f}, 3);
    QCOMPARE(cloud.dataValueMin(), -4.0);
    QCOMPARE(cloud.dataValueMax(), 12.0);
    QCOMPARE(rangeSpy.count(), 1);

    // An unset bound stays resolved from the data; setting one pins just that end.
    auto colormap = Colormap{};
    cloud.setColormap(&colormap);
    QCOMPARE(cloud.dataValueMin(), -4.0);
    QCOMPARE(cloud.dataValueMax(), 12.0);

    colormap.setMin(-10.0);
    QCOMPARE(cloud.dataValueMin(), -10.0);
    QCOMPARE(cloud.dataValueMax(), 12.0);

    colormap.setMax(20.0);
    QCOMPARE(cloud.dataValueMax(), 20.0);

    // Clearing the values falls back to the default [0, 1] for a bound that is not pinned.
    colormap.setMin(std::numeric_limits<qreal>::quiet_NaN());
    cloud.setValues({});
    QCOMPARE(cloud.dataValueMin(), 0.0);
    QCOMPARE(cloud.dataValueMax(), 20.0);
    QVERIFY(std::isnan(cloud.valueAt(0)));
}

void PointCloudDataTest::setValuesExpandsAndClearsInPlace()
{
    auto cloud = PointCloud{};
    cloud.setData({QPointF(1.0, 2.0), QPointF(3.0, 4.0), QPointF(5.0, 6.0)});
    auto countSpy = QSignalSpy{&cloud, &PointCloud::countChanged};

    cloud.setValues({0.25, 0.5, 0.75});
    QVERIFY(cloud.hasValues());
    QCOMPARE(countSpy.count(), 1);
    QCOMPARE(cloud.pointAt(0), QPointF(1.0, 2.0));
    QCOMPARE(cloud.pointAt(2), QPointF(5.0, 6.0));
    QCOMPARE(cloud.valueAt(1), 0.5);

    cloud.setValues({1.0, 2.0, 3.0});
    QCOMPARE(countSpy.count(), 1);
    QCOMPARE(cloud.valueAt(2), 3.0);
    QCOMPARE(cloud.pointAt(1), QPointF(3.0, 4.0));

    cloud.setValues({});
    QVERIFY(!cloud.hasValues());
    QCOMPARE(countSpy.count(), 2);
    QCOMPARE(cloud.pointAt(0), QPointF(1.0, 2.0));
    QCOMPARE(cloud.pointAt(2), QPointF(5.0, 6.0));
}

void PointCloudDataTest::accessorsReturnNaNOutOfRange()
{
    auto cloud = PointCloud{};
    cloud.setDataF(std::vector<float>{1.0f, 1.0f}, std::vector<float>{7.0f}, 1);

    QVERIFY(std::isnan(cloud.pointAt(-1).x()));
    QVERIFY(std::isnan(cloud.pointAt(1).y()));
    QVERIFY(std::isnan(cloud.valueAt(1)));
    QCOMPARE(cloud.valueAt(0), 7.0);
}

void PointCloudDataTest::clearDataResetsState()
{
    auto axes = AxisPair{0.0, 10.0};
    auto cloud = PointCloud{};
    cloud.setXAxis(&axes.x);
    cloud.setYAxis(&axes.y);
    cloud.setDataF(std::vector<float>{1.0f, 1.0f}, std::vector<float>{7.0f}, 1);
    auto countSpy = QSignalSpy{&cloud, &PointCloud::countChanged};

    cloud.clearData();
    QCOMPARE(cloud.count(), 0);
    QVERIFY(!cloud.hasValues());
    QCOMPARE(countSpy.count(), 1);
    QCOMPARE(cloud.pointIndexAt(QPointF(10.0, 10.0)), -1);
}

void PointCloudDataTest::postDataIsAppliedFromWorkerThread()
{
    auto cloud = PointCloud{};
    auto worker = std::thread{[&cloud]() {
        cloud.postData(std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f}, std::vector<float>{0.1f, 0.2f}, 2);
        cloud.postData(std::vector<float>{5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f}, 3);
    }};
    worker.join();

    // Queued: nothing is applied until the event loop runs.
    QCOMPARE(cloud.count(), 0);
    QTRY_COMPARE(cloud.count(), 3);
    QVERIFY(!cloud.hasValues());
    QCOMPARE(cloud.pointAt(2), QPointF(9.0, 10.0));
}

void PointCloudDataTest::pointIndexAtFindsNearestPointWithinRadius()
{
    auto axes = AxisPair{0.0, 100.0};
    auto cloud = PointCloud{};
    cloud.setSize({200.0, 100.0});
    cloud.setXAxis(&axes.x);
    cloud.setYAxis(&axes.y);
    cloud.setHoverRadius(5.0);
    // Pixel positions: (20, 90), (40, 90), (100, 50).
    cloud.setDataF(std::vector<float>{10.0f, 10.0f, 20.0f, 10.0f, 50.0f, 50.0f}, 3);

    QCOMPARE(cloud.pointIndexAt(QPointF(21.0, 91.0)), 0);
    QCOMPARE(cloud.pointIndexAt(QPointF(37.0, 90.0)), 1);
    QCOMPARE(cloud.pointIndexAt(QPointF(100.0, 50.0)), 2);
    QCOMPARE(cloud.pointIndexAt(QPointF(30.0, 90.0)), -1);
    QVERIFY(cloud.contains(QPointF(100.0, 52.0)));
    QVERIFY(!cloud.contains(QPointF(150.0, 20.0)));

    // Zooming in makes the same data-space gap wider in pixels.
    axes.x.setViewportMax(25.0);
    QCOMPARE(cloud.pointIndexAt(QPointF(80.0, 90.0)), 0);
    QCOMPARE(cloud.pointIndexAt(QPointF(95.0, 90.0)), -1);

    // New data invalidates the index.
    cloud.setDataF(std::vector<float>{15.0f, 10.0f}, 1);
    QCOMPARE(cloud.pointIndexAt(QPointF(120.0, 90.0)), 0);
}

void PointCloudDataTest::pointIndexAtUsesLogarithmicMapping()
{
    auto axes = AxisPair{1.0, 10000.0};
    axes.x.setLogScale(true);
    axes.y.setLogScale(true);
    auto cloud = PointCloud{};
    cloud.setSize({400.0, 400.0});
    cloud.setXAxis(&axes.x);
    cloud.setYAxis(&axes.y);
    cloud.setHoverRadius(4.0);
    // (100, 100) sits in the middle of a 4-decade log axis; the second point is not drawable.
    cloud.setDataF(std::vector<float>{100.0f, 100.0f, -1.0f, 100.0f}, 2);

    QCOMPARE(cloud.pointIndexAt(QPointF(200.0, 200.0)), 0);
    QCOMPARE(cloud.pointIndexAt(QPointF(203.0, 198.0)), 0);
    QCOMPARE(cloud.pointIndexAt(QPointF(210.0, 200.0)), -1);

    axes.x.setLogScale(false);
    axes.x.setViewportMin(0.0);
    axes.x.setViewportMax(200.0);
    // On a linear X axis the point at x = -1 becomes valid (pixel x = -2) and pickable.
    QCOMPARE(cloud.pointIndexAt(QPointF(0.0, 200.0)), 1);
    QCOMPARE(cloud.pointIndexAt(QPointF(200.0, 200.0)), 0);
    QCOMPARE(cloud.pointIndexAt(QPointF(100.0, 200.0)), -1);
}

void PointCloudDataTest::propertySettersClampAndNotify()
{
    auto cloud = PointCloud{};
    auto* marker = cloud.marker();
    auto sizeSpy = QSignalSpy{marker, &SeriesMarker::sizeChanged};
    auto featherSpy = QSignalSpy{&cloud, &PointCloud::antialiasingFeatherChanged};
    auto shapeSpy = QSignalSpy{marker, &SeriesMarker::shapeChanged};

    marker->setSize(-3.0);
    QCOMPARE(marker->size(), 0.0);
    marker->setSize(0.0);
    QCOMPARE(sizeSpy.count(), 1);

    cloud.setAntialiasingFeather(50.0);
    QCOMPARE(cloud.antialiasingFeather(), 10.0);
    QCOMPARE(featherSpy.count(), 1);

    marker->setShape(PlotSeries::MarkerShape::Cross);
    marker->setShape(PlotSeries::MarkerShape::Cross);
    QCOMPARE(shapeSpy.count(), 1);

    cloud.setHoverRadius(-1.0);
    QCOMPARE(cloud.hoverRadius(), 0.0);

    auto filledSpy = QSignalSpy{marker, &SeriesMarker::filledChanged};
    marker->setFilled(false);
    marker->setFilled(false);
    QVERIFY(!marker->filled());
    QCOMPARE(filledSpy.count(), 1);

    auto strokeSpy = QSignalSpy{marker, &SeriesMarker::strokeWidthChanged};
    marker->setStrokeWidth(-2.0);
    QCOMPARE(marker->strokeWidth(), 0.0);
    marker->setStrokeWidth(2.5);
    QCOMPARE(marker->strokeWidth(), 2.5);
    QCOMPARE(strokeSpy.count(), 2);
}

void PointCloudDataTest::doubleDataKeepsPrecisionForLargeCoordinates()
{
    // Epoch-second timestamps one millisecond apart: a float mantissa cannot separate these,
    // so a single-precision path would collapse them onto one coordinate.
    constexpr auto baseTime = 1758000000.0;
    constexpr auto stepSeconds = 0.001;
    constexpr auto pointCount = 4;

    auto xy = std::vector<double>{};
    for (auto index = 0; index < pointCount; ++index) {
        xy.push_back(baseTime + index * stepSeconds);
        xy.push_back(static_cast<double>(index));
    }

    auto cloud = PointCloud{};
    cloud.setData(std::move(xy), pointCount);
    QCOMPARE(cloud.count(), pointCount);

    // pointAt() reports the value that was supplied, not a rounded one.
    for (auto index = 0; index < pointCount; ++index) {
        QCOMPARE(cloud.pointAt(index).x(), baseTime + index * stepSeconds);
    }
    // Distinct inputs stay distinct.
    QVERIFY(cloud.pointAt(0).x() != cloud.pointAt(1).x());

    // Confirm the float path really does lose them, so this test would fail without the change.
    QCOMPARE(static_cast<float>(baseTime), static_cast<float>(baseTime + stepSeconds));
}

void PointCloudDataTest::doubleDataSurvivesLogScaleChange()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    xAxis.setOrientation(Axis::Horizontal);
    yAxis.setOrientation(Axis::Vertical);

    auto cloud = PointCloud{};
    cloud.setXAxis(&xAxis);
    cloud.setYAxis(&yAxis);
    cloud.setData(std::vector<double>{10.0, 100.0, 20.0, 200.0}, std::vector<float>{1.0f, 2.0f}, 2);

    QCOMPARE(cloud.count(), 2);
    QVERIFY(cloud.hasValues());
    QCOMPARE(cloud.valueAt(1), 2.0);

    // A log axis must not be origin-shifted; the values must survive the rebuild.
    yAxis.setLogScale(true);
    QCOMPARE(cloud.pointAt(1), QPointF(20.0, 200.0));
    QCOMPARE(cloud.valueAt(1), 2.0);
    QVERIFY(cloud.hasValues());
}

} // namespace QAccelPlot

using QAccelPlot::PointCloudDataTest;
QTEST_MAIN(PointCloudDataTest)
#include "tst_point_cloud_data.moc"
