//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "HoverEvents.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/Glow.hpp"
#include "QAccelPlot/inspection/InspectionResult.hpp"
#include "QAccelPlot/inspection/SeriesInspection.hpp"
#include "QAccelPlot/legend/LegendEntry.hpp"
#include "QAccelPlot/materials/CandlestickMaterial.hpp"
#include "QAccelPlot/series/CandlestickSeries.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"

#include <QMetaProperty>
#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QtTest/QtTest>

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

namespace QAccelPlot {

namespace {

constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();
constexpr auto kInf = std::numeric_limits<double>::infinity();

class TestableCandles final : public CandlestickSeries {
public:
    using CandlestickSeries::hoverEnterEvent;
    using CandlestickSeries::hoverLeaveEvent;
    using CandlestickSeries::hoverMoveEvent;
    using CandlestickSeries::updatePaintNode;

    mutable int scanCount{0};

protected:
    DataRanges computeDataRanges() const override
    {
        ++scanCount;
        return CandlestickSeries::computeDataRanges();
    }
};

// Candles bound to axes spanning 0..10 in a 100 × 100 px item, so one data unit is 10 px.
struct CandleFixture {
    CandleFixture()
    {
        for (auto* axis : {&xAxis, &yAxis}) {
            axis->setViewportMin(0.0);
            axis->setViewportMax(10.0);
        }
        yAxis.setOrientation(Qt::Vertical);
        candles.setParentItem(window.contentItem());
        candles.setSize({100.0, 100.0});
        candles.setPlotRect({0.0, 0.0, 100.0, 100.0});
        candles.setXAxis(&xAxis);
        candles.setYAxis(&yAxis);
    }

    QPointF pixel(const double x, const double y) const
    {
        return {xAxis.coordToPixel(x, 100.0), yAxis.coordToPixel(y, 100.0)};
    }

    void hover(const QEvent::Type type, const QPointF& position)
    {
        auto event = QAccelPlotTest::hoverEvent(type, position, position);
        if (type == QEvent::HoverEnter) {
            candles.hoverEnterEvent(&event);
        } else if (type == QEvent::HoverMove) {
            candles.hoverMoveEvent(&event);
        } else {
            candles.hoverLeaveEvent(&event);
        }
    }

    QSGGeometryNode* paint()
    {
        node.reset(candles.updatePaintNode(node.release(), nullptr));
        return static_cast<QSGGeometryNode*>(node.get());
    }

    QQuickWindow window;
    Axis xAxis;
    Axis yAxis;
    TestableCandles candles;
    std::unique_ptr<QSGNode> node;
};

bool hasCandle(const QVariantMap& candle, const double x, const double open, const double high, const double low, const double close)
{
    const auto same = [&candle](const char* key, const double expected) {
        const auto actual = candle.value(QString::fromLatin1(key)).toDouble();
        return std::isnan(expected) ? std::isnan(actual) : qFuzzyCompare(actual, expected);
    };
    return !candle.isEmpty() && same("x", x) && same("open", open) && same("high", high) && same("low", low) && same("close", close);
}

void compareRange(const std::optional<PlotSeries::DataExtent>& range, const double min, const double max)
{
    QVERIFY(range);
    QCOMPARE(range->min, min);
    QCOMPARE(range->max, max);
}

} // namespace

class CandlestickSeriesDataTest : public QObject {
    Q_OBJECT

private slots:
    void defaultsUseThePalette();
    void setDataStoresQuintuplesUpToTheShortestList();
    void appendingAndReplacingChangeOneDataPoint();
    void spacingDefaultsToTheSmallestGap();
    void spacingCanBePinnedAndReset();
    void dataRangesWidenXByHalfACandle();
    void appendingKeepsTheWidenedRanges();
    void autoRescaleAxisWidensByTheNewSpacing();
    void autoRescaleAxisFollowsAppendedAndReplacedSpacing();
    void replacingKeepsTheWidenedRanges();
    void unchangedSpacingKeepsTheRangesWithoutARescan();
    void hitTestFindsBodyAndWick();
    void hitTestFindsOhlcLineAndTicks();
    void minimumWidthAndDojiKeepCandlesVisible();
    void nearestCandleWinsAndTiesGoToHigherIndex();
    void invalidAndUnboundedValues();
    void logAxisSkipsNonPositiveX();
    void logAxisClampsNonPositiveCandleEdgesToX();
    void hoverEventsTrackCandleUnderCursor();
    void inspectionReturnsRecords();
    void legendEntryShowsBothColors();
    void paintReservesVerticesAndSetsUniforms();
};

void CandlestickSeriesDataTest::defaultsUseThePalette()
{
    const auto candles = CandlestickSeries{};
    QCOMPARE(candles.upColor(), ColorPalette::dark().candleUp);
    QCOMPARE(candles.downColor(), ColorPalette::dark().candleDown);
    QVERIFY(!candles.hoverColor().isValid());
    QCOMPARE(candles.style(), CandlestickSeries::Style::Candlestick);
    QCOMPARE(candles.candleWidth(), 0.8);
    QCOMPARE(candles.spacing(), 1.0);
    QCOMPARE(candles.lineWidth(), 1.0);
    QCOMPARE(candles.minimumWidth(), 1.0);
    QCOMPARE(candles.legendSymbol(), PlotSeries::LegendSymbol::Candle);
}

void CandlestickSeriesDataTest::setDataStoresQuintuplesUpToTheShortestList()
{
    auto candles = CandlestickSeries{};
    auto spy = QSignalSpy{&candles, &CandlestickSeries::countChanged};
    candles.setData(QList<qreal>{1, 2, 3}, QList<qreal>{4, 5, 6}, QList<qreal>{7, 8}, QList<qreal>{1, 2, 3}, QList<qreal>{5, 4, 3});

    QCOMPARE(candles.count(), 2);
    QCOMPARE(spy.count(), 1);
    QVERIFY(hasCandle(candles.candleAt(1), 2, 5, 8, 2, 4));
    QVERIFY(candles.candleAt(2).isEmpty());
    QVERIFY(candles.candleAt(-1).isEmpty());

    candles.setData(std::vector<double>{1}, std::vector<double>{2}, std::vector<double>{3}, std::vector<double>{0}, std::vector<double>{1});
    QVERIFY(hasCandle(candles.candleAt(0), 1, 2, 3, 0, 1));

    candles.setData(std::vector<double>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 2);
    QVERIFY(hasCandle(candles.candleAt(1), 6, 7, 8, 9, 10));
    candles.setDataF(std::vector<float>{1, 2, 3, 4, 5}, 1);
    QVERIFY(hasCandle(candles.candleAt(0), 1, 2, 3, 4, 5));

    candles.clearData();
    QCOMPARE(candles.count(), 0);
}

void CandlestickSeriesDataTest::appendingAndReplacingChangeOneDataPoint()
{
    auto candles = CandlestickSeries{};
    auto countSpy = QSignalSpy{&candles, &CandlestickSeries::countChanged};
    candles.appendData(1, 2, 3, 1, 2.5);
    candles.appendData(2, 2.5, 4, 2, 3);
    QCOMPARE(candles.count(), 2);
    QCOMPARE(countSpy.count(), 2);

    auto revisionSpy = QSignalSpy{&candles, &CandlestickSeries::dataRevisionChanged};
    candles.replaceData(1, 2, 2.5, 5, 2, 4.5);
    QCOMPARE(countSpy.count(), 2);
    QCOMPARE(revisionSpy.count(), 1);
    QVERIFY(hasCandle(candles.candleAt(0), 1, 2, 3, 1, 2.5));
    QVERIFY(hasCandle(candles.candleAt(1), 2, 2.5, 5, 2, 4.5));

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("CandlestickSeries data point index 2 is out of range for 2 candles"));
    candles.replaceData(2, 3, 1, 1, 1, 1);
    QCOMPARE(revisionSpy.count(), 1);
}

void CandlestickSeriesDataTest::spacingDefaultsToTheSmallestGap()
{
    auto candles = CandlestickSeries{};
    auto spy = QSignalSpy{&candles, &CandlestickSeries::spacingChanged};
    const auto ones = QList<qreal>{1, 1, 1, 1};
    // Nothing read the spacing, so the new data has no spacing to announce.
    candles.setData(QList<qreal>{0, 2, 3, 7}, ones, ones, ones, ones);
    QCOMPARE(spy.count(), 0);
    QCOMPARE(candles.spacing(), 1.0);

    // Gaps are taken in stored order, between finite x values, and zero gaps are ignored.
    candles.setData(QList<qreal>{5, kNaN, 4.5, 4.5, kInf, 7}, ones + ones, ones + ones, ones + ones, ones + ones);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(candles.spacing(), 0.5);

    // Appending lowers it in place.
    candles.appendData(7.25, 1, 1, 1, 1);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(candles.spacing(), 0.25);
    // A candle that keeps the smallest gap leaves it unannounced.
    candles.appendData(7.5, 1, 1, 1, 1);
    candles.replaceData(6, 7.25, 2, 2, 2, 2);
    QCOMPARE(spy.count(), 2);

    // Without a positive gap it falls back to 1.
    candles.setData(QList<qreal>{3, 3}, QList<qreal>{1, 1}, QList<qreal>{1, 1}, QList<qreal>{1, 1}, QList<qreal>{1, 1});
    QCOMPARE(candles.spacing(), 1.0);
    candles.clearData();
    QCOMPARE(candles.spacing(), 1.0);
    candles.appendData(10, 1, 1, 1, 1);
    QCOMPARE(candles.spacing(), 1.0);
    candles.appendData(10.1, 1, 1, 1, 1);
    QVERIFY(qFuzzyCompare(candles.spacing(), 0.1));

    // Replacing an x rescans.
    candles.replaceData(1, 14, 1, 1, 1, 1);
    QCOMPARE(candles.spacing(), 4.0);
    // Replacing only the values keeps it.
    candles.replaceData(1, 14, 2, 3, 1, 2);
    QCOMPARE(candles.spacing(), 4.0);
}

void CandlestickSeriesDataTest::spacingCanBePinnedAndReset()
{
    auto candles = CandlestickSeries{};
    const auto ones = QList<qreal>{1, 1, 1};
    candles.setData(QList<qreal>{0, 2, 3}, ones, ones, ones, ones);
    auto spy = QSignalSpy{&candles, &CandlestickSeries::spacingChanged};

    candles.setSpacing(5);
    QCOMPARE(candles.spacing(), 5.0);
    QCOMPARE(spy.count(), 1);
    candles.setSpacing(5);
    QCOMPARE(spy.count(), 1);

    // A pinned spacing ignores the data.
    candles.appendData(3.1, 1, 1, 1, 1);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(candles.spacing(), 5.0);

    // QML assigns undefined through the RESET function.
    const auto property = candles.metaObject()->property(candles.metaObject()->indexOfProperty("spacing"));
    QVERIFY(property.isResettable());
    QVERIFY(property.reset(&candles));
    QCOMPARE(spy.count(), 2);
    QVERIFY(qFuzzyCompare(candles.spacing(), 0.1));

    // Values that are not finite and positive return to the default too.
    for (const auto value : {0.0, -1.0, kNaN, kInf}) {
        candles.setSpacing(2);
        candles.setSpacing(value);
        QVERIFY2(qFuzzyCompare(candles.spacing(), 0.1), qPrintable(QString::number(value)));
    }
    candles.resetSpacing();
    QCOMPARE(spy.count(), 10);
}

void CandlestickSeriesDataTest::dataRangesWidenXByHalfACandle()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    candles.setData(QList<qreal>{1, 2, 3}, QList<qreal>{5, 6, kNaN}, QList<qreal>{8, kNaN, 9}, QList<qreal>{2, 3, 4}, QList<qreal>{6, 5, 7});
    compareRange(candles.xDataRange(), 0.6, 3.4);
    compareRange(candles.yDataRange(), 2.0, 9.0);

    candles.setCandleWidth(1);
    compareRange(candles.xDataRange(), 0.5, 3.5);
    candles.setSpacing(2);
    compareRange(candles.xDataRange(), 0.0, 4.0);
    candles.setCandleWidth(0);
    compareRange(candles.xDataRange(), 1.0, 3.0);

    // On a log axis a widened minimum at or below 0 keeps the x value.
    candles.setCandleWidth(1);
    fixture.xAxis.setViewportMin(0.1);
    fixture.xAxis.setLogScale(true);
    compareRange(candles.xDataRange(), 1.0, 4.0);
}

void CandlestickSeriesDataTest::appendingKeepsTheWidenedRanges()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    const auto ones = QList<qreal>{1, 1, 1};
    candles.setData(QList<qreal>{1, 2, 3}, ones, ones, ones, ones);
    compareRange(candles.xDataRange(), 0.6, 3.4);

    candles.appendData(4, 1, 5, 0, 2);
    compareRange(candles.xDataRange(), 0.6, 4.4);
    compareRange(candles.yDataRange(), 0.0, 5.0);

    // A smaller gap narrows every candle.
    candles.appendData(4.5, 1, 1, 1, 1);
    compareRange(candles.xDataRange(), 0.8, 4.7);

    // The same with ranges not read since the last data change.
    candles.appendData(4.75, 1, 1, 1, 1);
    candles.appendData(4.8, 1, 1, 1, 1);
    QVERIFY(qFuzzyCompare(candles.xDataRange()->min, 0.98));
    QVERIFY(qFuzzyCompare(candles.xDataRange()->max, 4.82));
}

void CandlestickSeriesDataTest::autoRescaleAxisWidensByTheNewSpacing()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    fixture.xAxis.setAutoRescale(true);
    const auto ones = QList<qreal>{1, 1, 1};
    candles.setData(QList<qreal>{0, 1, 2}, ones, ones, ones, ones);
    // Drawing reads the spacing.
    QCOMPARE(candles.spacing(), 1.0);

    // The axis reads the ranges while the new data is published.
    constexpr auto day = 86400.0;
    candles.setData(QList<qreal>{0, day, 2 * day}, ones, ones, ones, ones);
    compareRange(candles.xDataRange(), -0.4 * day, 2.4 * day);
    QCOMPARE(fixture.xAxis.dataMin(), -0.4 * day);
    QCOMPARE(fixture.xAxis.dataMax(), 2.4 * day);
}

void CandlestickSeriesDataTest::autoRescaleAxisFollowsAppendedAndReplacedSpacing()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    auto& xAxis = fixture.xAxis;
    xAxis.setAutoRescale(true);
    const auto ones = QList<qreal>{1, 1, 1};
    candles.setData(QList<qreal>{0, 1, 2}, ones, ones, ones, ones);
    QCOMPARE(xAxis.dataMin(), -0.4);
    QCOMPARE(xAxis.dataMax(), 2.4);

    // An append closer than the spacing narrows every candle.
    candles.appendData(2.5, 1, 1, 1, 1);
    QCOMPARE(xAxis.dataMin(), -0.2);
    QCOMPARE(xAxis.dataMax(), 2.7);

    // Moving the candle that made the shortest gap widens every candle.
    candles.replaceData(3, 3, 1, 1, 1, 1);
    QCOMPARE(xAxis.dataMin(), -0.4);
    QCOMPARE(xAxis.dataMax(), 3.4);

    // Moving a candle inside the range to shorten a gap narrows every candle.
    candles.replaceData(1, 1.5, 1, 1, 1, 1);
    QCOMPARE(xAxis.dataMin(), -0.2);
    QCOMPARE(xAxis.dataMax(), 3.2);
}

void CandlestickSeriesDataTest::replacingKeepsTheWidenedRanges()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    candles.setData(QList<qreal>{1, 2, 3}, QList<qreal>{2, 2, 2}, QList<qreal>{4, 5, 4}, QList<qreal>{1, 1, 0}, QList<qreal>{3, 3, 3});
    compareRange(candles.xDataRange(), 0.6, 3.4);
    compareRange(candles.yDataRange(), 0.0, 5.0);

    // The candle at the x edge keeps its x: its widened extent still reaches the edge.
    candles.replaceData(2, 3, 2, 4, 0.5, 3);
    compareRange(candles.xDataRange(), 0.6, 3.4);
    compareRange(candles.yDataRange(), 0.5, 5.0);

    // Moving it past the edge widens the range by half a candle.
    candles.replaceData(2, 5, 2, 4, 1, 3);
    compareRange(candles.xDataRange(), 0.6, 5.4);

    // Moving it inward narrows the range and the spacing.
    candles.replaceData(2, 2.5, 2, 4, 1, 3);
    compareRange(candles.xDataRange(), 0.8, 2.7);

    // Lowering the highest high shrinks the y range.
    candles.replaceData(1, 2, 2, 4, 1, 3);
    compareRange(candles.yDataRange(), 1.0, 4.0);
}

void CandlestickSeriesDataTest::unchangedSpacingKeepsTheRangesWithoutARescan()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    const auto ones = QList<qreal>{1, 1, 1};
    candles.setData(QList<qreal>{1, 2, 3}, ones, ones, ones, ones);
    compareRange(candles.xDataRange(), 0.6, 3.4);
    const auto scans = candles.scanCount;

    candles.appendData(4, 1, 2, 0, 1);
    compareRange(candles.xDataRange(), 0.6, 4.4);
    compareRange(candles.yDataRange(), 0.0, 2.0);
    candles.replaceData(3, 4, 1, 2, 0, 1.5);
    compareRange(candles.xDataRange(), 0.6, 4.4);
    // Moving the last candle outward keeps the shortest gap.
    candles.replaceData(3, 6, 1, 2, 0, 1);
    compareRange(candles.xDataRange(), 0.6, 6.4);
    compareRange(candles.yDataRange(), 0.0, 2.0);
    QCOMPARE(candles.scanCount, scans);
}

void CandlestickSeriesDataTest::hitTestFindsBodyAndWick()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    candles.setSpacing(2);
    candles.setLineWidth(2);
    candles.setHoverRadius(0);
    // A body from 4 to 6, 1.6 data units (16 px) wide, over a wick from 2 to 8.
    candles.setData(QList<qreal>{5}, QList<qreal>{4}, QList<qreal>{8}, QList<qreal>{2}, QList<qreal>{6});

    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 5)), 0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5.75, 5)), 0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5.85, 5)), -1);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 7.5)), 0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 7.5) + QPointF{0.9, 0}), 0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 7.5) + QPointF{1.5, 0}), -1);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 8.2)), -1);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 2.5)), 0);
    QVERIFY(candles.contains(fixture.pixel(5, 5)));
    QVERIFY(!candles.contains(fixture.pixel(7, 5)));

    candles.setHoverRadius(4);
    for (const auto radius : {kNaN, kInf}) {
        candles.setHoverRadius(radius);
        QCOMPARE(candles.hoverRadius(), 4.0);
    }
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5.8, 5) + QPointF{3.5, 0}), 0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5.8, 5) + QPointF{4.5, 0}), -1);

    // Without a line width only the body is hit.
    candles.setHoverRadius(0);
    candles.setLineWidth(0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 7.5)), -1);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 5)), 0);
}

void CandlestickSeriesDataTest::hitTestFindsOhlcLineAndTicks()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    candles.setStyle(CandlestickSeries::Style::Ohlc);
    candles.setSpacing(2);
    candles.setLineWidth(2);
    candles.setHoverRadius(0);
    candles.setData(QList<qreal>{5}, QList<qreal>{4}, QList<qreal>{8}, QList<qreal>{2}, QList<qreal>{6});

    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 5)), 0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5.5, 5)), -1);
    // The open tick reaches left and the close tick right, each half the candle width.
    QCOMPARE(candles.candleIndexAt(fixture.pixel(4.3, 4)), 0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5.5, 4)), -1);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5.7, 6)), 0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(4.5, 6)), -1);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5.9, 6)), -1);

    candles.setLineWidth(0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 5)), -1);
}

void CandlestickSeriesDataTest::minimumWidthAndDojiKeepCandlesVisible()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    candles.setHoverRadius(0);
    candles.setSpacing(0.01);
    candles.setMinimumWidth(6);
    candles.setLineWidth(4);
    // A doji without a wick.
    candles.setData(QList<qreal>{5}, QList<qreal>{5}, QList<qreal>{kNaN}, QList<qreal>{kNaN}, QList<qreal>{5});

    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 5) + QPointF{2.5, 0}), 0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 5) + QPointF{3.5, 0}), -1);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 5) + QPointF{2.5, 1.5}), 0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 5) + QPointF{2.5, 2.5}), -1);
}

void CandlestickSeriesDataTest::nearestCandleWinsAndTiesGoToHigherIndex()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    candles.setHoverRadius(20);
    candles.setData(QList<qreal>{4, 6}, QList<qreal>{4, 4}, QList<qreal>{6, 6}, QList<qreal>{4, 4}, QList<qreal>{6, 6});

    QCOMPARE(candles.candleIndexAt(fixture.pixel(4.9, 5)), 0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5.1, 5)), 1);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 5)), 1);
}

void CandlestickSeriesDataTest::invalidAndUnboundedValues()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    candles.setHoverRadius(0);
    candles.setSpacing(2);
    // A NaN high leaves out the upper wick, an infinite low reaches past the plot edge, and the
    // last three are not drawn: a NaN x, a NaN open, and a NaN close.
    candles.setData(
        QList<qreal>{2, kNaN, 5, 8}, QList<qreal>{4, 4, kNaN, 4}, QList<qreal>{kNaN, 8, 8, 8}, QList<qreal>{-kInf, 2, 2, 2}, QList<qreal>{6, 6, 6, kNaN});

    QCOMPARE(candles.candleIndexAt(fixture.pixel(2, 7)), -1);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(2, 0.1)), 0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 5)), -1);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(8, 5)), -1);
    QCOMPARE(candles.inspection()->recordAt(0).status, InspectionStatus::Ready);
    for (const auto index : {1, 2, 3}) {
        QCOMPARE(candles.inspection()->recordAt(index).status, InspectionStatus::NoMatch);
    }
    // Inconsistent values are drawn as given: a high below the body.
    candles.setData(QList<qreal>{5}, QList<qreal>{4}, QList<qreal>{3}, QList<qreal>{2}, QList<qreal>{6});
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 2.5)), 0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(5, 5)), 0);
}

void CandlestickSeriesDataTest::logAxisSkipsNonPositiveX()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    fixture.xAxis.setViewportMin(1);
    fixture.xAxis.setViewportMax(100);
    fixture.xAxis.setLogScale(true);
    candles.setHoverRadius(0);
    candles.setSpacing(1);
    candles.setData(QList<qreal>{-1, 10}, QList<qreal>{4, 4}, QList<qreal>{8, 8}, QList<qreal>{2, 2}, QList<qreal>{6, 6});

    QCOMPARE(candles.candleIndexAt(fixture.pixel(10, 5)), 1);
    QCOMPARE(candles.inspection()->recordAt(0).status, InspectionStatus::NoMatch);
    compareRange(candles.xDataRange(), 9.6, 10.4);
}

void CandlestickSeriesDataTest::logAxisClampsNonPositiveCandleEdgesToX()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    fixture.xAxis.setViewportMin(0.1);
    fixture.xAxis.setViewportMax(1000);
    fixture.xAxis.setLogScale(true);
    candles.setHoverRadius(0);
    // A spacing of 9 makes the first candle span -2.6 to 4.6.
    candles.setData(QList<qreal>{1, 10, 100}, QList<qreal>{4, 4, 4}, QList<qreal>{8, 8, 8}, QList<qreal>{2, 2, 2}, QList<qreal>{6, 6, 6});

    compareRange(candles.xDataRange(), 1.0, 103.6);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(2, 5)), 0);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(0.2, 5)), -1);

    candles.setStyle(CandlestickSeries::Style::Ohlc);
    candles.setLineWidth(2);
    QCOMPARE(candles.candleIndexAt(fixture.pixel(0.2, 4)), -1);
}

void CandlestickSeriesDataTest::hoverEventsTrackCandleUnderCursor()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    candles.setData(QList<qreal>{2, 8}, QList<qreal>{4, 6}, QList<qreal>{7, 7}, QList<qreal>{3, 3}, QList<qreal>{6, 4});
    auto spy = QSignalSpy{&candles, &CandlestickSeries::hoveredIndexChanged};

    fixture.hover(QEvent::HoverEnter, fixture.pixel(2, 5));
    QCOMPARE(candles.hoveredIndex(), 0);
    QVERIFY(candles.hovered());
    fixture.hover(QEvent::HoverMove, fixture.pixel(8, 5));
    QCOMPARE(candles.hoveredIndex(), 1);
    fixture.hover(QEvent::HoverMove, fixture.pixel(5, 5));
    QCOMPARE(candles.hoveredIndex(), -1);
    QVERIFY(!candles.hovered());
    fixture.hover(QEvent::HoverMove, fixture.pixel(8, 5));
    fixture.hover(QEvent::HoverLeave, fixture.pixel(8, 5));
    QCOMPARE(candles.hoveredIndex(), -1);
    QCOMPARE(spy.count(), 5);

    fixture.hover(QEvent::HoverEnter, fixture.pixel(8, 5));
    candles.setData(QList<qreal>{2}, QList<qreal>{4}, QList<qreal>{7}, QList<qreal>{3}, QList<qreal>{6});
    QCOMPARE(candles.hoveredIndex(), -1);
}

void CandlestickSeriesDataTest::inspectionReturnsRecords()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    candles.setData(QList<qreal>{2, 8}, QList<qreal>{4, 6}, QList<qreal>{7, 7}, QList<qreal>{3, 3}, QList<qreal>{6, 4});

    const auto record = candles.inspection()->recordAt(1);
    QCOMPARE(record.status, InspectionStatus::Ready);
    QCOMPARE(record.index, 1);
    QVERIFY(hasCandle(record.fields, 8, 6, 7, 3, 4));
    QCOMPARE(candles.inspection()->recordAt(2).status, InspectionStatus::InvalidArgument);

    const auto hit = candles.inspection()->recordAtPosition(fixture.pixel(2, 5));
    QCOMPARE(hit.status, InspectionStatus::Ready);
    QCOMPARE(hit.index, 0);
    QVERIFY(!candles.inspection()->recordAtPosition(fixture.pixel(5, 5)).valid());
}

void CandlestickSeriesDataTest::legendEntryShowsBothColors()
{
    auto candles = CandlestickSeries{};
    auto spy = QSignalSpy{&candles, &PlotSeries::legendEntryChanged};
    candles.setUpColor(Qt::green);
    candles.setDownColor(Qt::red);
    candles.setLineWidth(2);
    QCOMPARE(spy.count(), 3);

    const auto entry = candles.legendEntry();
    QVERIFY(entry);
    QCOMPARE(entry->symbol, PlotSeries::LegendSymbol::Candle);
    QCOMPARE(entry->color, QColor{Qt::green});
    QCOMPARE(entry->secondaryColor, QColor{Qt::red});
    QCOMPARE(entry->lineWidth, 2.0);
}

void CandlestickSeriesDataTest::paintReservesVerticesAndSetsUniforms()
{
    auto fixture = CandleFixture{};
    auto& candles = fixture.candles;
    candles.setData(QList<qreal>{2, 4}, QList<qreal>{4, 6}, QList<qreal>{7, 7}, QList<qreal>{3, 3}, QList<qreal>{6, 4});
    candles.setStyle(CandlestickSeries::Style::Ohlc);
    candles.setLineWidth(3);
    candles.setMinimumWidth(5);
    candles.setCandleWidth(0.5);
    candles.setUpColor(Qt::green);
    candles.setDownColor(Qt::red);
    auto glow = Glow{};
    auto effects = candles.effects();
    effects.append(&effects, &glow);
    fixture.hover(QEvent::HoverEnter, fixture.pixel(2, 5));
    QCOMPARE(candles.hoveredIndex(), 0);
    auto* node = fixture.paint();
    if (!node) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }

    const auto* material = static_cast<CandlestickMaterial*>(node->material());
    QCOMPARE(node->geometry()->vertexCount(), 256 * CandlestickMaterial::kVerticesPerCandle);
    QCOMPARE(material->candleCount, 2.0f);
    QCOMPARE(material->lineWidth, 3.0f);
    QCOMPARE(material->minimumWidth, 5.0f);
    QCOMPARE(material->halfWidth, 0.5f);
    QCOMPARE(material->ohlc, 1.0f);
    QCOMPARE(material->color, QColor{Qt::green});
    QCOMPARE(material->downColor, QColor{Qt::red});
    QCOMPARE(material->glow.color, QColor{Qt::green});
    QCOMPARE(material->glowDownColor, QColor{Qt::red});
    // An invalid hover color highlights no candle.
    QCOMPARE(material->hoveredIndex, -1.0f);
    QVERIFY(material->dataTexture);

    candles.setHoverColor(Qt::yellow);
    node = fixture.paint();
    material = static_cast<CandlestickMaterial*>(node->material());
    QCOMPARE(material->hoveredIndex, 0.0f);
    QCOMPARE(material->hoverColor, QColor{Qt::yellow});

    for (auto i = 0; i < 300; ++i) {
        candles.appendData(5 + i, 1, 2, 0, 1);
    }
    node = fixture.paint();
    QCOMPARE(node->geometry()->vertexCount(), 512 * CandlestickMaterial::kVerticesPerCandle);
    QCOMPARE(static_cast<CandlestickMaterial*>(node->material())->candleCount, 302.0f);

    candles.clearData();
    QVERIFY(!fixture.paint());
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::CandlestickSeriesDataTest)
#include "tst_candlestick_series_data.moc"
