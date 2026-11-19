//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/inspection/InspectionResult.hpp"
#include "QAccelPlot/inspection/SeriesInspection.hpp"
#include "QAccelPlot/series/CandlestickSeries.hpp"
#include "QAccelPlot/transitions/GrowTransition.hpp"
#include "QAccelPlot/transitions/MorphTransition.hpp"
#include "WindowPlacement.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <algorithm>
#include <cstdlib>
#include <initializer_list>
#include <limits>
#include <memory>
#include <vector>

namespace QAccelPlot {

namespace {

// 200 x 200 px plot showing data in [0, 10] x [0, 10].
constexpr auto kSize = 200;
constexpr auto kDataSpan = 10.0;

// Long enough that the tests always interrupt the animation part way.
constexpr auto kLongDuration = 10'000;

// Eases every progress after the start to halfway, so a run starts from the data drawn.
qreal halfway(const qreal progress)
{
    return progress > 0.0 ? 0.5 : 0.0;
}

// A transition held halfway, so the tests can check a frame of the animation.
template <typename Transition> class HeldHalfway : public Transition {
public:
    HeldHalfway()
    {
        this->setDuration(kLongDuration);
        auto easing = QEasingCurve{};
        easing.setCustomType(halfway);
        this->setEasing(easing);
    }
};

// A pixel of the plot, given in data coordinates, and the color expected there.
struct Probe {
    qreal x;
    qreal y;
    QColor color;
};

std::unique_ptr<Axis> makeAxis(const Qt::Orientation orientation)
{
    auto axis = std::make_unique<Axis>();
    axis->setOrientation(orientation);
    axis->setViewportMin(0.0);
    axis->setViewportMax(kDataSpan);
    return axis;
}

bool isColor(const QColor& pixel, const QColor& expected)
{
    constexpr auto kTolerance = 60;
    return std::abs(pixel.red() - expected.red()) < kTolerance && std::abs(pixel.green() - expected.green()) < kTolerance
        && std::abs(pixel.blue() - expected.blue()) < kTolerance;
}

// Up candles at \a xs with a body from 4 to 6 over a wick from 3 to 7.
std::vector<double> candlesAt(std::initializer_list<double> xs)
{
    auto data = std::vector<double>{};
    for (const auto x : xs) {
        data.insert(data.end(), {x, 4.0, 7.0, 3.0, 6.0});
    }
    return data;
}

} // namespace

class CandlestickSeriesTransitionsTest : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void morphMovesCandlesToTheirNewPlace();
    void morphMovesEveryValue();
    void growGrowsCandlesFromTheirOpen();
    void growFromABaselineMovesTheOpenToo();
    void growFromAnInfiniteOpenStartsAtTheViewportEdge();
    void dataIsTheNewOneFromTheStart();
    void hoverPicksTheDrawnCandles();
    void appendingAndReplacingSkipTheAnimation();
    void clearDataRunsTheExitTransition();
    void cancellingShowsTargetData();

private:
    // The item position of the data point (\a x, \a y).
    QPointF pixelAt(qreal x, qreal y) const;
    // True when the window shows every probe's color at its pixel.
    bool shows(std::initializer_list<Probe> probes) const;

    std::unique_ptr<QQuickWindow> window_;
    std::unique_ptr<Axis> xAxis_;
    std::unique_ptr<Axis> yAxis_;
    std::unique_ptr<CandlestickSeries> candles_;
};

void CandlestickSeriesTransitionsTest::init()
{
    window_ = std::make_unique<QQuickWindow>();
    if (window_->rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    window_->setColor(Qt::black);
    window_->resize(kSize, kSize);
    xAxis_ = makeAxis(Qt::Horizontal);
    yAxis_ = makeAxis(Qt::Vertical);
    candles_ = std::make_unique<CandlestickSeries>();
    candles_->setParentItem(window_->contentItem());
    candles_->setSize({kSize, kSize});
    candles_->setXAxis(xAxis_.get());
    candles_->setYAxis(yAxis_.get());
    candles_->setPlotRect({0, 0, kSize, kSize});
    candles_->setUpColor(Qt::red);
    candles_->setDownColor(Qt::blue);
    candles_->setLineWidth(4.0);
    // Bodies 1.6 data units wide.
    candles_->setSpacing(2.0);
    // A real cursor over the window would hover a candle.
    candles_->setHoverEnabled(false);
    QAccelPlotTest::moveAwayFromCursor(*window_);
    window_->show();
    QVERIFY(QTest::qWaitForWindowExposed(window_.get()));
}

void CandlestickSeriesTransitionsTest::cleanup()
{
    candles_.reset();
    window_.reset();
    xAxis_.reset();
    yAxis_.reset();
}

void CandlestickSeriesTransitionsTest::morphMovesCandlesToTheirNewPlace()
{
    auto morph = HeldHalfway<MorphTransition>{};
    candles_->setData(candlesAt({2.0}), 1);
    candles_->setUpdateTransition(&morph);

    candles_->setData(candlesAt({8.0}), 1);

    // The data is the new one at once, while the candle is drawn half way from x 2 to x 8.
    QCOMPARE(candles_->candleAt(0).value(QStringLiteral("x")).toDouble(), 8.0);
    QVERIFY(morph.running());
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {2.0, 5.0, Qt::black}, {8.0, 5.0, Qt::black}}));
    candles_->setUpdateTransition(nullptr);
    QTRY_VERIFY(shows({{8.0, 5.0, Qt::red}, {5.0, 5.0, Qt::black}}));
}

void CandlestickSeriesTransitionsTest::morphMovesEveryValue()
{
    auto morph = HeldHalfway<MorphTransition>{};
    candles_->setData(std::vector<double>{5.0, 4.0, 7.0, 3.0, 6.0}, 1);
    candles_->setUpdateTransition(&morph);

    // From a body 4..6 over a wick 3..7 to a down candle with a body 8..2 over a wick 1..9.
    candles_->setData(std::vector<double>{5.0, 8.0, 9.0, 1.0, 2.0}, 1);

    // Half way, a down candle: a body from open 6 to close 4 over a wick from 2 to 8.
    QTRY_VERIFY(shows({{5.6, 5.0, Qt::blue}, {5.6, 6.5, Qt::black}, {5.0, 7.5, Qt::blue}, {5.0, 8.5, Qt::black}, {5.0, 2.5, Qt::blue}}));
    candles_->setUpdateTransition(nullptr);
    QTRY_VERIFY(shows({{5.6, 7.5, Qt::blue}, {5.0, 8.5, Qt::blue}}));
}

void CandlestickSeriesTransitionsTest::growGrowsCandlesFromTheirOpen()
{
    auto grow = HeldHalfway<GrowTransition>{};
    candles_->setEnterTransition(&grow);

    candles_->setData(std::vector<double>{5.0, 4.0, 8.0, 2.0, 8.0}, 1);

    // Half way from the open 4: a body from 4 to 6 over a wick from 3 to 6.
    QTRY_VERIFY(shows({{5.6, 5.0, Qt::red}, {5.6, 7.0, Qt::black}, {5.0, 7.0, Qt::black}, {5.0, 3.5, Qt::red}, {5.0, 2.5, Qt::black}}));
    candles_->setEnterTransition(nullptr);
    QTRY_VERIFY(shows({{5.6, 7.0, Qt::red}, {5.0, 2.5, Qt::red}}));
}

void CandlestickSeriesTransitionsTest::growFromABaselineMovesTheOpenToo()
{
    auto grow = HeldHalfway<GrowTransition>{};
    grow.setBaseline(0.0);
    candles_->setEnterTransition(&grow);

    candles_->setData(std::vector<double>{5.0, 4.0, 8.0, 2.0, 8.0}, 1);

    // Half way from 0: a body from 2 to 4.
    QTRY_VERIFY(shows({{5.6, 3.0, Qt::red}, {5.6, 5.0, Qt::black}}));
    candles_->setEnterTransition(nullptr);
}

void CandlestickSeriesTransitionsTest::growFromAnInfiniteOpenStartsAtTheViewportEdge()
{
    auto grow = HeldHalfway<GrowTransition>{};
    candles_->setEnterTransition(&grow);

    candles_->setData(std::vector<double>{5.0, -std::numeric_limits<double>::infinity(), 8.0, 2.0, 8.0}, 1);

    // Half way from the bottom edge 0: a body from below the plot to 4.
    QTRY_VERIFY(shows({{5.6, 2.0, Qt::red}, {5.6, 6.0, Qt::black}}));
    candles_->setEnterTransition(nullptr);
    QTRY_VERIFY(shows({{5.6, 6.0, Qt::red}}));
}

void CandlestickSeriesTransitionsTest::dataIsTheNewOneFromTheStart()
{
    auto morph = HeldHalfway<MorphTransition>{};
    candles_->resetSpacing();
    candles_->setData(candlesAt({2.0}), 1);
    candles_->setUpdateTransition(&morph);
    auto counts = QSignalSpy{candles_.get(), &CandlestickSeries::countChanged};

    candles_->setData(std::vector<double>{8.0, 5.0, 6.0, 1.0, 5.5, 6.0, 4.0, 9.0, 3.0, 4.5}, 2);

    QCOMPARE(counts.count(), 1);
    QCOMPARE(candles_->count(), 2);
    QCOMPARE(candles_->spacing(), 2.0);
    QCOMPARE(candles_->xDataRange()->min, 5.2);
    QCOMPARE(candles_->xDataRange()->max, 8.8);
    QCOMPARE(candles_->yDataRange()->min, 1.0);
    QCOMPARE(candles_->yDataRange()->max, 9.0);

    QVERIFY(morph.running());
    const auto record = candles_->inspection()->recordAt(1);
    QCOMPARE(record.status, InspectionStatus::Ready);
    QCOMPARE(record.fields.value(QStringLiteral("close")).toDouble(), 4.5);
    candles_->setUpdateTransition(nullptr);
}

void CandlestickSeriesTransitionsTest::hoverPicksTheDrawnCandles()
{
    auto morph = HeldHalfway<MorphTransition>{};
    candles_->setHoverRadius(0.0);
    candles_->setData(candlesAt({2.0}), 1);
    candles_->setUpdateTransition(&morph);
    candles_->setData(candlesAt({8.0}), 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}}));

    QCOMPARE(candles_->candleIndexAt(pixelAt(5.0, 5.0)), 0);
    QCOMPARE(candles_->candleIndexAt(pixelAt(8.0, 5.0)), -1);

    candles_->setUpdateTransition(nullptr);
    QCOMPARE(candles_->candleIndexAt(pixelAt(5.0, 5.0)), -1);
    QCOMPARE(candles_->candleIndexAt(pixelAt(8.0, 5.0)), 0);
}

void CandlestickSeriesTransitionsTest::appendingAndReplacingSkipTheAnimation()
{
    auto morph = HeldHalfway<MorphTransition>{};
    candles_->setData(candlesAt({2.0}), 1);
    candles_->setUpdateTransition(&morph);
    candles_->setData(candlesAt({8.0}), 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}}));

    candles_->appendData(3.0, 4.0, 7.0, 3.0, 6.0);
    QCOMPARE(candles_->count(), 2);
    QVERIFY(!morph.running());
    QTRY_VERIFY(shows({{8.0, 5.0, Qt::red}, {3.0, 5.0, Qt::red}, {5.0, 5.0, Qt::black}}));

    candles_->setData(candlesAt({2.0}), 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}}));
    candles_->replaceData(0, 3.0, 4.0, 7.0, 3.0, 6.0);
    QVERIFY(!morph.running());
    QTRY_VERIFY(shows({{3.0, 5.0, Qt::red}, {5.0, 5.0, Qt::black}}));
    candles_->setUpdateTransition(nullptr);
}

void CandlestickSeriesTransitionsTest::clearDataRunsTheExitTransition()
{
    auto grow = HeldHalfway<GrowTransition>{};
    candles_->setData(std::vector<double>{5.0, 4.0, 8.0, 2.0, 8.0}, 1);
    candles_->setExitTransition(&grow);

    candles_->clearData();

    // Half way back to the open 4: a body from 4 to 6.
    QCOMPARE(candles_->count(), 0);
    QVERIFY(grow.running());
    QTRY_VERIFY(shows({{5.6, 5.0, Qt::red}, {5.6, 7.0, Qt::black}}));
    QVERIFY(candles_->animating());
    candles_->setExitTransition(nullptr);
    QVERIFY(!candles_->animating());
    QTRY_VERIFY(shows({{5.6, 5.0, Qt::black}}));
}

void CandlestickSeriesTransitionsTest::cancellingShowsTargetData()
{
    auto morph = HeldHalfway<MorphTransition>{};
    candles_->setData(candlesAt({2.0}), 1);
    candles_->setUpdateTransition(&morph);
    candles_->setData(candlesAt({8.0}), 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {8.0, 5.0, Qt::black}}));

    morph.cancel();

    QTRY_VERIFY(shows({{8.0, 5.0, Qt::red}, {5.0, 5.0, Qt::black}}));
    candles_->setUpdateTransition(nullptr);
}

QPointF CandlestickSeriesTransitionsTest::pixelAt(const qreal x, const qreal y) const
{
    return {(x - xAxis_->viewportMin()) / kDataSpan * kSize, (1.0 - (y - yAxis_->viewportMin()) / kDataSpan) * kSize};
}

bool CandlestickSeriesTransitionsTest::shows(const std::initializer_list<Probe> probes) const
{
    const auto image = window_->grabWindow();
    return std::all_of(
        probes.begin(), probes.end(), [&](const Probe& probe) { return isColor(image.pixelColor(pixelAt(probe.x, probe.y).toPoint()), probe.color); });
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Pixel lookups use logical coordinates, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::CandlestickSeriesTransitionsTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_candlestick_series_transitions.moc"
