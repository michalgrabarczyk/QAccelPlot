//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/shapes/RectangleList.hpp"

#include <QtTest/QtTest>

#include <array>

namespace {
class HoverableRectangles final : public QAccelPlot::RectangleList {
public:
    using RectangleList::hoverMoveEvent;
};
}

class RectangleHoverTest : public QObject {
    Q_OBJECT
private slots:
    void nearbyTimestampRectangles_data();
    void nearbyTimestampRectangles();
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

QTEST_MAIN(RectangleHoverTest)
#include "tst_rectangle_hover.moc"
