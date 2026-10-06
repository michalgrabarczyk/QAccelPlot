//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "HoverEvents.hpp"
#include "QAccelPlot/series/PointCloud.hpp"
#include "QAccelPlot/series/RectangleSeries.hpp"

#include <QQuickWindow>
#include <QtTest/QtTest>

#include <array>
#include <limits>
#include <vector>

namespace {
class HoverableRectangles final : public QAccelPlot::RectangleSeries {
public:
    using RectangleSeries::hoverEnterEvent;
    using RectangleSeries::hoverLeaveEvent;
    using RectangleSeries::hoverMoveEvent;
};

// Data range 0..4 on both axes over a 400 × 400 plot: one data unit is 100 pixels.
struct AxisPair {
    QAccelPlot::Axis x;
    QAccelPlot::Axis y;

    AxisPair()
    {
        x.setSide(QAccelPlot::Axis::Bottom);
        y.setSide(QAccelPlot::Axis::Left);
        for (auto* axis : {&x, &y}) {
            axis->setViewportMin(0.0);
            axis->setViewportMax(4.0);
        }
    }
};

constexpr auto kPlotRect = QRectF{0.0, 0.0, 400.0, 400.0};

// Rectangle 0 spans data (0, 0)–(2, 2); rectangle 1 overlaps it at (1, 1)–(3, 3).
void setOverlappingRectangles(QAccelPlot::RectangleSeries& rectangles, AxisPair& axes)
{
    rectangles.setXAxis(&axes.x);
    rectangles.setYAxis(&axes.y);
    rectangles.setPlotRect(kPlotRect);
    const auto data = std::array<double, 8>{0.0, 0.0, 2.0, 2.0, 1.0, 1.0, 3.0, 3.0};
    rectangles.setData(data.data(), 2);
}
}

class RectangleHoverTest : public QObject {
    Q_OBJECT
private slots:
    void nearbyTimestampRectangles_data();
    void nearbyTimestampRectangles();
    void rectangleIndexAtReturnsTopmostRectangle();
    void rectangleIndexAtKeepsItsAnswersOnceIndexed();
    void rectangleIndexAtFollowsChangesAtTheSamePosition();
    void hoverEventsTrackRectangleUnderCursor();
    void removingHoveredRectangleClearsHover();
    void hitTestsFollowDataChanges();
    void floatDataIsHitTested();
    void spansAreHoveredAtAnyHeight();
    void narrowRectanglesAreHoveredAtMinimumSize();
    void seriesUnderneathReceiveHoverOutsideRectangles();
};

void RectangleHoverTest::nearbyTimestampRectangles_data()
{
    QTest::addColumn<bool>("horizontal");
    QTest::newRow("x-timestamps") << true;
    QTest::newRow("y-timestamps") << false;
}

void RectangleHoverTest::nearbyTimestampRectangles()
{
    QFETCH(bool, horizontal);
    constexpr auto epoch = 1'789'032'600'000.0;
    const auto baseX = horizontal ? epoch : 0.0;
    const auto baseY = horizontal ? 0.0 : epoch;
    auto xAxis = QAccelPlot::Axis{};
    auto yAxis = QAccelPlot::Axis{};
    xAxis.setOrientation(QAccelPlot::Axis::Horizontal);
    yAxis.setOrientation(QAccelPlot::Axis::Vertical);
    xAxis.setViewportMin(baseX);
    xAxis.setViewportMax(baseX + 4);
    yAxis.setViewportMin(baseY);
    yAxis.setViewportMax(baseY + 4);
    auto rectangles = HoverableRectangles{};
    rectangles.setXAxis(&xAxis);
    rectangles.setYAxis(&yAxis);
    rectangles.setPlotRect({0, 0, 400, 400});
    const auto secondX = baseX + (horizontal ? 2 : 0);
    const auto secondY = baseY + (horizontal ? 0 : 2);
    const auto data = std::array<double, 8>{baseX, baseY, baseX + 1, baseY + 1, secondX, secondY, secondX + 1, secondY + 1};
    rectangles.setData(data.data(), 2);

    for (const auto offset : {0.5, 2.5, 1.5}) {
        const auto position
            = QPointF{xAxis.coordToPixel(baseX + (horizontal ? offset : 0.5), 400), yAxis.coordToPixel(baseY + (horizontal ? 0.5 : offset), 400)};
        auto event = QAccelPlotTest::hoverEvent(QEvent::HoverMove, position, position);
        rectangles.hoverMoveEvent(&event);
        const auto expectedIndex = offset == 0.5 ? 0 : (offset == 2.5 ? 1 : -1);
        QCOMPARE(rectangles.hoveredIndex(), expectedIndex);
    }
}

void RectangleHoverTest::rectangleIndexAtReturnsTopmostRectangle()
{
    auto axes = AxisPair{};
    auto rectangles = QAccelPlot::RectangleSeries{};
    QCOMPARE(rectangles.rectangleIndexAt({50.0, 350.0}), -1);
    QVERIFY(!rectangles.contains({50.0, 350.0}));

    setOverlappingRectangles(rectangles, axes);

    QCOMPARE(rectangles.rectangleIndexAt({50.0, 350.0}), 0);
    QCOMPARE(rectangles.rectangleIndexAt({150.0, 250.0}), 1);
    QCOMPARE(rectangles.rectangleIndexAt({250.0, 150.0}), 1);
    QCOMPARE(rectangles.rectangleIndexAt({350.0, 50.0}), -1);
    QVERIFY(rectangles.contains({50.0, 350.0}));
    QVERIFY(!rectangles.contains({350.0, 50.0}));
    QVERIFY(!rectangles.contains({50.0, 450.0}));
}

void RectangleHoverTest::rectangleIndexAtKeepsItsAnswersOnceIndexed()
{
    auto axes = AxisPair{};
    auto rectangles = QAccelPlot::RectangleSeries{};
    setOverlappingRectangles(rectangles, axes);

    // Hit tests scan the rectangles until that has cost as much as building the spatial grid,
    // which for two rectangles takes at most 800 scans.
    for (auto query = 0; query < 250; ++query) {
        QCOMPARE(rectangles.rectangleIndexAt({50.0, 350.0}), 0);
        QCOMPARE(rectangles.rectangleIndexAt({150.0, 250.0}), 1);
        QCOMPARE(rectangles.rectangleIndexAt({250.0, 150.0}), 1);
        QCOMPARE(rectangles.rectangleIndexAt({350.0, 50.0}), -1);
    }

    // New data must not be answered from the grid of the old data.
    rectangles.clearData();
    for (auto query = 0; query < 40; ++query) {
        QCOMPARE(rectangles.rectangleIndexAt({50.0, 350.0}), -1);
    }
    setOverlappingRectangles(rectangles, axes);
    QCOMPARE(rectangles.rectangleIndexAt({150.0, 250.0}), 1);
}

// The last answer is reused for an equal hit test, so anything that can change it must be noticed.
void RectangleHoverTest::rectangleIndexAtFollowsChangesAtTheSamePosition()
{
    auto axes = AxisPair{};
    auto rectangles = QAccelPlot::RectangleSeries{};
    setOverlappingRectangles(rectangles, axes);
    // Data (0.5, 0.5), inside rectangle 0 only.
    const auto position = QPointF{50.0, 350.0};
    QCOMPARE(rectangles.rectangleIndexAt(position), 0);

    // Panning puts data x = -3 under the position.
    axes.x.setViewportMin(-4.0);
    QCOMPARE(rectangles.rectangleIndexAt(position), -1);
    axes.x.setViewportMin(0.0);
    QCOMPARE(rectangles.rectangleIndexAt(position), 0);

    // With the viewport starting at 0.25, this pixel shows data y = 2.59 on a linear axis,
    // above rectangle 0, and y = 1.41 on a logarithmic one.
    const auto upper = QPointF{50.0, 150.0};
    axes.y.setViewportMin(0.25);
    QCOMPARE(rectangles.rectangleIndexAt(upper), -1);
    axes.y.setLogScale(true);
    QCOMPARE(rectangles.rectangleIndexAt(upper), 0);
    axes.y.setLogScale(false);
    QCOMPARE(rectangles.rectangleIndexAt(upper), -1);
    axes.y.setViewportMin(0.0);
    QCOMPARE(rectangles.rectangleIndexAt(position), 0);

    // In a plot twice as tall the position shows data y = 2.25, above rectangle 0.
    rectangles.setPlotRect({0.0, 0.0, 400.0, 800.0});
    QCOMPARE(rectangles.rectangleIndexAt(position), -1);
    rectangles.setPlotRect(kPlotRect);
    QCOMPARE(rectangles.rectangleIndexAt(position), 0);

    // A rectangle one pixel wide at x = 100..101, four pixels left of the position.
    const auto thin = std::array<double, 4>{1.0, 0.0, 1.01, 4.0};
    rectangles.setData(thin.data(), 1);
    const auto beside = QPointF{105.0, 200.0};
    QCOMPARE(rectangles.rectangleIndexAt(beside), -1);
    rectangles.setMinimumWidth(10.0);
    QCOMPARE(rectangles.rectangleIndexAt(beside), 0);
    rectangles.setMinimumWidth(1.0);
    QCOMPARE(rectangles.rectangleIndexAt(beside), -1);

    const auto wide = std::array<double, 4>{1.0, 0.0, 1.1, 4.0};
    rectangles.setData(wide.data(), 1);
    QCOMPARE(rectangles.rectangleIndexAt(beside), 0);
    rectangles.clearData();
    QCOMPARE(rectangles.rectangleIndexAt(beside), -1);
}

void RectangleHoverTest::hoverEventsTrackRectangleUnderCursor()
{
    auto axes = AxisPair{};
    auto rectangles = HoverableRectangles{};
    setOverlappingRectangles(rectangles, axes);
    auto hoveredSpy = QSignalSpy{&rectangles, &QAccelPlot::RectangleSeries::hoveredIndexChanged};
    const auto hover = [](const QEvent::Type type, const QPointF& position) { return QAccelPlotTest::hoverEvent(type, position, position); };

    auto enter = hover(QEvent::HoverEnter, {50.0, 350.0});
    rectangles.hoverEnterEvent(&enter);
    QCOMPARE(rectangles.hoveredIndex(), 0);
    auto moveToTop = hover(QEvent::HoverMove, {150.0, 250.0});
    rectangles.hoverMoveEvent(&moveToTop);
    QCOMPARE(rectangles.hoveredIndex(), 1);
    auto moveOutside = hover(QEvent::HoverMove, {350.0, 50.0});
    rectangles.hoverMoveEvent(&moveOutside);
    QCOMPARE(rectangles.hoveredIndex(), -1);
    auto moveBack = hover(QEvent::HoverMove, {250.0, 150.0});
    rectangles.hoverMoveEvent(&moveBack);
    auto leave = hover(QEvent::HoverLeave, {250.0, 150.0});
    rectangles.hoverLeaveEvent(&leave);

    QCOMPARE(rectangles.hoveredIndex(), -1);
    QCOMPARE(hoveredSpy.count(), 5);
}

void RectangleHoverTest::removingHoveredRectangleClearsHover()
{
    auto axes = AxisPair{};
    auto rectangles = HoverableRectangles{};
    setOverlappingRectangles(rectangles, axes);
    auto move = QAccelPlotTest::hoverEvent(QEvent::HoverMove, {250.0, 150.0}, {250.0, 150.0});
    rectangles.hoverMoveEvent(&move);
    QCOMPARE(rectangles.hoveredIndex(), 1);

    rectangles.setData(std::vector<double>{0.0, 0.0, 2.0, 2.0}, 1);
    QCOMPARE(rectangles.hoveredIndex(), -1);

    rectangles.hoverMoveEvent(&move);
    QCOMPARE(rectangles.hoveredIndex(), -1);
    auto moveToFirst = QAccelPlotTest::hoverEvent(QEvent::HoverMove, {50.0, 350.0}, {50.0, 350.0});
    rectangles.hoverMoveEvent(&moveToFirst);
    QCOMPARE(rectangles.hoveredIndex(), 0);

    rectangles.clearData();
    QCOMPARE(rectangles.hoveredIndex(), -1);
}

void RectangleHoverTest::hitTestsFollowDataChanges()
{
    auto axes = AxisPair{};
    auto rectangles = QAccelPlot::RectangleSeries{};
    setOverlappingRectangles(rectangles, axes);
    QCOMPARE(rectangles.rectangleIndexAt({50.0, 350.0}), 0);

    // Same count, moved coordinates: rectangle 0 now spans (3, 3)–(4, 4).
    rectangles.setData(std::vector<double>{3.0, 3.0, 4.0, 4.0, 1.0, 1.0, 3.0, 3.0}, 2);
    QCOMPARE(rectangles.rectangleIndexAt({50.0, 350.0}), -1);
    QCOMPARE(rectangles.rectangleIndexAt({350.0, 50.0}), 0);

    rectangles.setDataNoRange(std::vector<double>{0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 3.0, 3.0}, {}, 3);
    QCOMPARE(rectangles.rectangleIndexAt({50.0, 350.0}), 0);
    QCOMPARE(rectangles.rectangleIndexAt({250.0, 150.0}), 2);
    QCOMPARE(rectangles.rectangleIndexAt({350.0, 50.0}), -1);

    rectangles.clearData();
    QCOMPARE(rectangles.rectangleIndexAt({50.0, 350.0}), -1);
    QVERIFY(!rectangles.contains({50.0, 350.0}));

    rectangles.setData(std::vector<double>{3.0, 0.0, 4.0, 1.0}, 1);
    QCOMPARE(rectangles.rectangleIndexAt({350.0, 350.0}), 0);
    QCOMPARE(rectangles.rectangleIndexAt({50.0, 350.0}), -1);
}

void RectangleHoverTest::floatDataIsHitTested()
{
    constexpr auto kInf = std::numeric_limits<float>::infinity();
    auto axes = AxisPair{};
    auto rectangles = QAccelPlot::RectangleSeries{};
    setOverlappingRectangles(rectangles, axes);

    rectangles.setDataF(std::vector<float>{0.0f, 0.0f, 2.0f, 2.0f, 1.0f, 1.0f, 3.0f, 3.0f, 3.5f, -kInf, 3.6f, kInf}, 3);
    QCOMPARE(rectangles.rectangleIndexAt({50.0, 350.0}), 0);
    QCOMPARE(rectangles.rectangleIndexAt({150.0, 250.0}), 1);
    QCOMPARE(rectangles.rectangleIndexAt({355.0, 10.0}), 2);
    QCOMPARE(rectangles.rectangleIndexAt({380.0, 10.0}), -1);

    const auto raw = std::array<float, 4>{3.0f, 3.0f, 4.0f, 4.0f};
    rectangles.setDataFNoRange(raw.data(), 1);
    QCOMPARE(rectangles.rectangleIndexAt({50.0, 350.0}), -1);
    QCOMPARE(rectangles.rectangleIndexAt({350.0, 50.0}), 0);
}

void RectangleHoverTest::spansAreHoveredAtAnyHeight()
{
    constexpr auto kInf = std::numeric_limits<double>::infinity();
    auto axes = AxisPair{};
    auto rectangles = QAccelPlot::RectangleSeries{};
    rectangles.setXAxis(&axes.x);
    rectangles.setYAxis(&axes.y);
    rectangles.setPlotRect(kPlotRect);
    rectangles.setData(std::vector<double>{1.0, -kInf, 2.0, kInf}, 1);

    QCOMPARE(rectangles.rectangleIndexAt({150.0, 1.0}), 0);
    QCOMPARE(rectangles.rectangleIndexAt({150.0, 399.0}), 0);
    QCOMPARE(rectangles.rectangleIndexAt({250.0, 200.0}), -1);

    axes.y.setViewportMin(1.0e9);
    axes.y.setViewportMax(1.0e9 + 4.0);
    QCOMPARE(rectangles.rectangleIndexAt({150.0, 200.0}), 0);
}

void RectangleHoverTest::narrowRectanglesAreHoveredAtMinimumSize()
{
    constexpr auto kInf = std::numeric_limits<double>::infinity();
    auto axes = AxisPair{};
    auto rectangles = QAccelPlot::RectangleSeries{};
    rectangles.setXAxis(&axes.x);
    rectangles.setYAxis(&axes.y);
    rectangles.setPlotRect(kPlotRect);
    // A 0.01 px wide span centered at x = 100 px, and a zero-height rectangle at y = 200 px.
    rectangles.setData(std::vector<double>{1.0, -kInf, 1.0001, kInf, 3.0, 2.0, 3.5, 2.0}, 2);

    QCOMPARE(rectangles.rectangleIndexAt({100.3, 50.0}), 0);
    QCOMPARE(rectangles.rectangleIndexAt({101.0, 50.0}), -1);
    QCOMPARE(rectangles.rectangleIndexAt({325.0, 200.3}), 1);
    QCOMPARE(rectangles.rectangleIndexAt({325.0, 201.0}), -1);

    rectangles.setMinimumWidth(10.0);
    rectangles.setMinimumHeight(10.0);
    QCOMPARE(rectangles.rectangleIndexAt({104.0, 50.0}), 0);
    QCOMPARE(rectangles.rectangleIndexAt({106.0, 50.0}), -1);
    QCOMPARE(rectangles.rectangleIndexAt({325.0, 196.0}), 1);
    QCOMPARE(rectangles.rectangleIndexAt({325.0, 206.0}), -1);

    rectangles.setMinimumWidth(-1.0);
    QCOMPARE(rectangles.minimumWidth(), 0.0);
    QCOMPARE(rectangles.rectangleIndexAt({100.3, 50.0}), -1);
}

void RectangleHoverTest::seriesUnderneathReceiveHoverOutsideRectangles()
{
    // The window stays hidden: synthetic events are still delivered, and the real mouse cannot interfere.
    auto window = QQuickWindow{};
    window.resize(400, 400);
    auto axes = AxisPair{};
    auto cloud = QAccelPlot::PointCloud{};
    cloud.setParentItem(window.contentItem());
    cloud.setXAxis(&axes.x);
    cloud.setYAxis(&axes.y);
    cloud.setPlotRect(kPlotRect);
    cloud.setHoverRadius(5.0);
    cloud.setDataF(std::vector<float>{3.5f, 3.5f}, 1);
    auto rectangles = QAccelPlot::RectangleSeries{};
    rectangles.setParentItem(window.contentItem());
    rectangles.setZ(1.0);
    setOverlappingRectangles(rectangles, axes);

    QTest::mouseMove(&window, QPoint{350, 50});
    QCOMPARE(cloud.hoveredIndex(), 0);
    QCOMPARE(rectangles.hoveredIndex(), -1);

    QTest::mouseMove(&window, QPoint{50, 350});
    QCOMPARE(cloud.hoveredIndex(), -1);
    QCOMPARE(rectangles.hoveredIndex(), 0);
}

QTEST_MAIN(RectangleHoverTest)
#include "tst_rectangle_hover.moc"
