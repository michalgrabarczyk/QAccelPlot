//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "HoverEvents.hpp"
#include "QAccelPlot/series/BarSeries.hpp"

#include <QElapsedTimer>
#include <QtTest/QtTest>

#include <limits>
#include <vector>

namespace {

class HoverableBars final : public QAccelPlot::BarSeries {
public:
    using BarSeries::hoverEnterEvent;
    using BarSeries::hoverLeaveEvent;
    using BarSeries::hoverMoveEvent;
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

    QPointF pixel(const double dataX, const double dataY) const
    {
        return {x.coordToPixel(dataX, 400.0), y.coordToPixel(dataY, 400.0)};
    }
};

// Bar 0 spans x 0.5..1.5 and y 1..2; bar 1 spans x 2.5..3.5 and y 1..3.
void setBars(QAccelPlot::BarSeries& bars, AxisPair& axes)
{
    bars.setXAxis(&axes.x);
    bars.setYAxis(&axes.y);
    bars.setPlotRect({0.0, 0.0, 400.0, 400.0});
    bars.setBarWidth(1.0);
    bars.setBaselineValue(1.0);
    bars.setData(std::vector<double>{1.0, 2.0, 3.0, 3.0}, 2);
}

}

class BarHoverTest : public QObject {
    Q_OBJECT
private slots:
    void barIndexAtFindsBarsBetweenBaselineAndValue();
    void horizontalBarsAreHitAlongX();
    void narrowBarsAreHitAtMinimumWidth();
    void geometryChangesMoveHitTests();
    void unboundedBaselineAndInvalidBars();
    void tallBarsKeepLookupsLocal();
    void hoverEventsTrackBarUnderCursor();
};

void BarHoverTest::barIndexAtFindsBarsBetweenBaselineAndValue()
{
    auto axes = AxisPair{};
    auto bars = QAccelPlot::BarSeries{};
    QCOMPARE(bars.barIndexAt(axes.pixel(1.0, 1.5)), -1);
    setBars(bars, axes);

    QCOMPARE(bars.barIndexAt(axes.pixel(1.0, 1.5)), 0);
    QCOMPARE(bars.barIndexAt(axes.pixel(1.4, 1.9)), 0);
    QCOMPARE(bars.barIndexAt(axes.pixel(1.0, 2.5)), -1);
    QCOMPARE(bars.barIndexAt(axes.pixel(1.0, 0.5)), -1);
    QCOMPARE(bars.barIndexAt(axes.pixel(2.0, 1.5)), -1);
    QCOMPARE(bars.barIndexAt(axes.pixel(3.0, 2.5)), 1);
    QVERIFY(bars.contains(axes.pixel(3.0, 2.5)));
    QVERIFY(!bars.contains(axes.pixel(2.0, 1.5)));
}

void BarHoverTest::horizontalBarsAreHitAlongX()
{
    auto axes = AxisPair{};
    auto bars = QAccelPlot::BarSeries{};
    setBars(bars, axes);
    bars.setOrientation(Qt::Horizontal);

    QCOMPARE(bars.barIndexAt(axes.pixel(1.5, 1.0)), 0);
    QCOMPARE(bars.barIndexAt(axes.pixel(2.5, 1.0)), -1);
    QCOMPARE(bars.barIndexAt(axes.pixel(2.5, 3.0)), 1);
    QCOMPARE(bars.barIndexAt(axes.pixel(0.5, 1.0)), -1);
}

void BarHoverTest::narrowBarsAreHitAtMinimumWidth()
{
    auto axes = AxisPair{};
    auto bars = QAccelPlot::BarSeries{};
    setBars(bars, axes);
    bars.setBarWidth(0.0001);
    bars.setMinimumWidth(10.0);

    const auto center = axes.pixel(1.0, 1.5);
    QCOMPARE(bars.barIndexAt(center + QPointF{4.0, 0.0}), 0);
    QCOMPARE(bars.barIndexAt(center - QPointF{4.0, 0.0}), 0);
    QCOMPARE(bars.barIndexAt(center + QPointF{8.0, 0.0}), -1);
    // The minimum applies across the bar only.
    QCOMPARE(bars.barIndexAt(axes.pixel(1.0, 2.0) - QPointF{0.0, 4.0}), -1);
}

void BarHoverTest::geometryChangesMoveHitTests()
{
    auto axes = AxisPair{};
    auto bars = QAccelPlot::BarSeries{};
    setBars(bars, axes);
    QCOMPARE(bars.barIndexAt(axes.pixel(1.0, 1.5)), 0);

    bars.setBarOffset(1.0);
    QCOMPARE(bars.barIndexAt(axes.pixel(1.0, 1.5)), -1);
    QCOMPARE(bars.barIndexAt(axes.pixel(2.0, 1.5)), 0);

    bars.setBaselineValue(0.0);
    QCOMPARE(bars.barIndexAt(axes.pixel(2.0, 0.5)), 0);

    bars.setData(std::vector<double>{0.0, 3.5}, 1);
    QCOMPARE(bars.barIndexAt(axes.pixel(1.0, 3.0)), 0);
    QCOMPARE(bars.barIndexAt(axes.pixel(3.0, 2.5)), -1);
}

void BarHoverTest::unboundedBaselineAndInvalidBars()
{
    constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();
    constexpr auto kInf = std::numeric_limits<double>::infinity();
    auto axes = AxisPair{};
    auto bars = QAccelPlot::BarSeries{};
    setBars(bars, axes);
    bars.setBaselineValue(-kInf);
    bars.setData(std::vector<double>{1.0, 2.0, kNaN, 3.0, 3.0, kNaN, kInf, 1.0}, 4);

    QCOMPARE(bars.barIndexAt(axes.pixel(1.0, 0.1)), 0);
    QCOMPARE(bars.barIndexAt(axes.pixel(3.0, 0.5)), -1);
    QCOMPARE(bars.barIndexAt(axes.pixel(1.0, 2.5)), -1);
}

void BarHoverTest::tallBarsKeepLookupsLocal()
{
    // Each bar spans at least half the value range. If the grid subdivided the value axis, most
    // bars would cover more than its per-item cell limit and every lookup would scan all of them.
    constexpr auto kBarCount = 1'000'000;
    constexpr auto kLookupCount = 2000;
    auto data = std::vector<double>(static_cast<size_t>(kBarCount) * 2);
    for (auto i = 0; i < kBarCount; ++i) {
        data[static_cast<size_t>(i) * 2] = i;
        data[static_cast<size_t>(i) * 2 + 1] = 500.0 + i % 500;
    }
    auto xAxis = QAccelPlot::Axis{};
    auto yAxis = QAccelPlot::Axis{};
    xAxis.setSide(QAccelPlot::Axis::Bottom);
    yAxis.setSide(QAccelPlot::Axis::Left);
    xAxis.setViewportMin(0.0);
    xAxis.setViewportMax(100.0);
    yAxis.setViewportMin(0.0);
    yAxis.setViewportMax(1000.0);
    auto bars = QAccelPlot::BarSeries{};
    bars.setXAxis(&xAxis);
    bars.setYAxis(&yAxis);
    bars.setPlotRect({0.0, 0.0, 400.0, 400.0});
    bars.setData(std::move(data), kBarCount);

    const auto pixelOf = [&](const int bar) { return QPointF{xAxis.coordToPixel(bar, 400.0), yAxis.coordToPixel(250.0, 400.0)}; };
    // The first lookup builds the grid, which is O(N) and not what this test measures.
    QCOMPARE(bars.barIndexAt(pixelOf(0)), 0);

    auto timer = QElapsedTimer{};
    timer.start();
    for (auto lookup = 0; lookup < kLookupCount; ++lookup) {
        const auto bar = lookup % 100;
        QCOMPARE(bars.barIndexAt(pixelOf(bar)), bar);
    }
    // A linear scan of a million bars takes about a millisecond per lookup, so this bound fails
    // by a wide margin, while grid lookups finish in a few milliseconds in total.
    const auto elapsed = timer.elapsed();
    QVERIFY2(elapsed < 500, qPrintable(QStringLiteral("%1 lookups took %2 ms").arg(kLookupCount).arg(elapsed)));
}

void BarHoverTest::hoverEventsTrackBarUnderCursor()
{
    auto axes = AxisPair{};
    auto bars = HoverableBars{};
    setBars(bars, axes);
    auto spy = QSignalSpy{&bars, &QAccelPlot::BarSeries::hoveredIndexChanged};

    const auto hover = [&bars](const QEvent::Type type, const QPointF& position) {
        auto event = QAccelPlotTest::hoverEvent(type, position, position);
        if (type == QEvent::HoverEnter) {
            bars.hoverEnterEvent(&event);
        } else if (type == QEvent::HoverMove) {
            bars.hoverMoveEvent(&event);
        } else {
            bars.hoverLeaveEvent(&event);
        }
    };
    hover(QEvent::HoverEnter, axes.pixel(1.0, 1.5));
    QCOMPARE(bars.hoveredIndex(), 0);
    hover(QEvent::HoverMove, axes.pixel(3.0, 2.5));
    QCOMPARE(bars.hoveredIndex(), 1);
    hover(QEvent::HoverMove, axes.pixel(2.0, 3.5));
    QCOMPARE(bars.hoveredIndex(), -1);
    hover(QEvent::HoverMove, axes.pixel(3.0, 2.5));
    hover(QEvent::HoverLeave, axes.pixel(3.0, 2.5));
    QCOMPARE(bars.hoveredIndex(), -1);
    QCOMPARE(spy.count(), 5);

    hover(QEvent::HoverMove, axes.pixel(3.0, 2.5));
    bars.setData(std::vector<double>{1.0, 2.0}, 1);
    QCOMPARE(bars.hoveredIndex(), -1);
}

QTEST_MAIN(BarHoverTest)
#include "tst_bar_hover.moc"
