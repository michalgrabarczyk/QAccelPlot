//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/PointCloud.hpp"
#include "QAccelPlot/series/RectangleList.hpp"

#include <QQuickWindow>
#include <QtTest/QtTest>

#include <array>

namespace {
class HoverableRectangles final : public QAccelPlot::RectangleList {
public:
    using RectangleList::hoverEnterEvent;
    using RectangleList::hoverLeaveEvent;
    using RectangleList::hoverMoveEvent;
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
void setOverlappingRectangles(QAccelPlot::RectangleList& rectangles, AxisPair& axes)
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
    void hoverEventsTrackRectangleUnderCursor();
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
        auto event = QHoverEvent{QEvent::HoverMove, position, position};
        rectangles.hoverMoveEvent(&event);
        const auto expectedIndex = offset == 0.5 ? 0 : (offset == 2.5 ? 1 : -1);
        QCOMPARE(rectangles.hoveredIndex(), expectedIndex);
    }
}

void RectangleHoverTest::rectangleIndexAtReturnsTopmostRectangle()
{
    auto axes = AxisPair{};
    auto rectangles = QAccelPlot::RectangleList{};
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

void RectangleHoverTest::hoverEventsTrackRectangleUnderCursor()
{
    auto axes = AxisPair{};
    auto rectangles = HoverableRectangles{};
    setOverlappingRectangles(rectangles, axes);
    auto hoveredSpy = QSignalSpy{&rectangles, &QAccelPlot::RectangleList::hoveredIndexChanged};
    const auto hover = [](const QEvent::Type type, const QPointF& position) { return QHoverEvent{type, position, position, position}; };

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
    auto rectangles = QAccelPlot::RectangleList{};
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
