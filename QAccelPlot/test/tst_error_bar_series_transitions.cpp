//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "OpacityAnimations.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/inspection/InspectionResult.hpp"
#include "QAccelPlot/inspection/SeriesInspection.hpp"
#include "QAccelPlot/series/ErrorBarSeries.hpp"
#include "QAccelPlot/transitions/DrawTransition.hpp"
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

// Vertical error bars at \a xs from 4 to 6, with values at 5.
std::vector<double> errorBarsAt(std::initializer_list<double> xs)
{
    auto data = std::vector<double>{};
    for (const auto x : xs) {
        data.insert(data.end(), {x, 5.0, 4.0, 6.0});
    }
    return data;
}

} // namespace

class ErrorBarSeriesTransitionsTest : public QObject {
    Q_OBJECT

public:
    enum class Interruption { Detach, Replace, Cancel, Destroy };
    Q_ENUM(Interruption)

private slots:
    void init();
    void cleanup();
    void morphMovesErrorBarsToTheirNewPlace();
    void morphMovesBounds();
    void newErrorBarsAppearAtTheirPlace();
    void removedErrorBarsDisappear();
    void drawShowsErrorBarsInOrder();
    void fadedEnterStartsTransparent();
    void floatDataAnimates();
    void dataIsTheNewOneFromTheStart();
    void hoverPicksTheDrawnErrorBars();
    void transitionReachesTargetData();
    void interruptedTransitionShowsTargetData_data();
    void interruptedTransitionShowsTargetData();
    void appendingSkipsTheAnimation();
    void clearDataSkipsTheAnimation();
    void transitionPropertyNotifies();

private:
    // The item position of the data point (\a x, \a y).
    QPointF pixelAt(qreal x, qreal y) const;
    // True when the window shows every probe's color at its pixel.
    bool shows(std::initializer_list<Probe> probes) const;

    std::unique_ptr<QQuickWindow> window_;
    std::unique_ptr<Axis> xAxis_;
    std::unique_ptr<Axis> yAxis_;
    std::unique_ptr<ErrorBarSeries> bars_;
};

void ErrorBarSeriesTransitionsTest::init()
{
    window_ = std::make_unique<QQuickWindow>();
    if (window_->rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    window_->setColor(Qt::black);
    window_->resize(kSize, kSize);
    xAxis_ = makeAxis(Qt::Horizontal);
    yAxis_ = makeAxis(Qt::Vertical);
    bars_ = std::make_unique<ErrorBarSeries>();
    bars_->setParentItem(window_->contentItem());
    bars_->setSize({kSize, kSize});
    bars_->setXAxis(xAxis_.get());
    bars_->setYAxis(yAxis_.get());
    bars_->setPlotRect({0, 0, kSize, kSize});
    bars_->setColor(Qt::red);
    bars_->setLineWidth(4.0);
    bars_->setCapLength(0.0);
    // A real cursor over the window would hover an error bar and draw it in the hover color.
    bars_->setHoverEnabled(false);
    QAccelPlotTest::moveAwayFromCursor(*window_);
    window_->show();
    QVERIFY(QTest::qWaitForWindowExposed(window_.get()));
}

void ErrorBarSeriesTransitionsTest::cleanup()
{
    bars_.reset();
    window_.reset();
    xAxis_.reset();
    yAxis_.reset();
}

void ErrorBarSeriesTransitionsTest::morphMovesErrorBarsToTheirNewPlace()
{
    auto morph = HeldHalfway<MorphTransition>{};
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setUpdateTransition(&morph);

    bars_->setData(errorBarsAt({8.0}), 1);

    // The data is the new one at once, while the error bar is drawn half way from x 2 to x 8.
    QCOMPARE(bars_->errorBarAt(0).value(QStringLiteral("x")).toDouble(), 8.0);
    QVERIFY(morph.running());
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {2.0, 5.0, Qt::black}, {8.0, 5.0, Qt::black}}));
    bars_->setUpdateTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::morphMovesBounds()
{
    auto morph = HeldHalfway<MorphTransition>{};
    bars_->setData(std::vector<double>{5.0, 5.0, 4.0, 6.0}, 1);
    bars_->setUpdateTransition(&morph);

    bars_->setErrorData(std::vector<double>{5.0}, std::vector<double>{5.0}, std::vector<double>{3.0});

    // Half way from 4..6 to 2..8 is 3..7.
    QTRY_VERIFY(shows({{5.0, 3.5, Qt::red}, {5.0, 6.5, Qt::red}, {5.0, 2.5, Qt::black}, {5.0, 7.5, Qt::black}}));
    bars_->setUpdateTransition(nullptr);
    QTRY_VERIFY(shows({{5.0, 2.5, Qt::red}, {5.0, 7.5, Qt::red}}));
}

void ErrorBarSeriesTransitionsTest::newErrorBarsAppearAtTheirPlace()
{
    auto morph = HeldHalfway<MorphTransition>{};
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setUpdateTransition(&morph);

    bars_->setData(errorBarsAt({8.0, 3.0}), 2);

    QCOMPARE(bars_->count(), 2);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {3.0, 5.0, Qt::red}, {8.0, 5.0, Qt::black}}));
    bars_->setUpdateTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::removedErrorBarsDisappear()
{
    auto morph = HeldHalfway<MorphTransition>{};
    bars_->setData(errorBarsAt({2.0, 7.0}), 2);
    QTRY_VERIFY(shows({{7.0, 5.0, Qt::red}}));
    bars_->setUpdateTransition(&morph);

    bars_->setData(errorBarsAt({8.0}), 1);

    QCOMPARE(bars_->count(), 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {7.0, 5.0, Qt::black}}));
    bars_->setUpdateTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::drawShowsErrorBarsInOrder()
{
    auto draw = HeldHalfway<DrawTransition>{};
    bars_->setEnterTransition(&draw);

    bars_->setData(errorBarsAt({2.0, 4.0, 6.0, 8.0}), 4);

    QCOMPARE(bars_->count(), 4);
    QTRY_VERIFY(shows({{2.0, 5.0, Qt::red}, {4.0, 5.0, Qt::red}, {6.0, 5.0, Qt::black}, {8.0, 5.0, Qt::black}}));

    draw.cancel();
    QTRY_VERIFY(shows({{6.0, 5.0, Qt::red}, {8.0, 5.0, Qt::red}}));
    bars_->setEnterTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::floatDataAnimates()
{
    auto morph = HeldHalfway<MorphTransition>{};
    bars_->setDataF(std::vector<float>{2.0f, 5.0f, 4.0f, 6.0f}, 1);
    bars_->setUpdateTransition(&morph);

    const auto next = std::vector<float>{8.0f, 5.0f, 4.0f, 6.0f};
    bars_->setDataF(next.data(), 1);

    QCOMPARE(bars_->errorBarAt(0).value(QStringLiteral("x")).toDouble(), 8.0);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {2.0, 5.0, Qt::black}, {8.0, 5.0, Qt::black}}));
    bars_->setUpdateTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::dataIsTheNewOneFromTheStart()
{
    auto morph = HeldHalfway<MorphTransition>{};
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setUpdateTransition(&morph);
    auto counts = QSignalSpy{bars_.get(), &ErrorBarSeries::countChanged};

    bars_->setData(std::vector<double>{8.0, 5.0, 4.0, 6.0, 3.0, 2.0, 1.0, 3.0}, 2);

    QCOMPARE(counts.count(), 1);
    QCOMPARE(bars_->count(), 2);
    QCOMPARE(bars_->xDataRange()->min, 3.0);
    QCOMPARE(bars_->xDataRange()->max, 8.0);
    QCOMPARE(bars_->yDataRange()->min, 1.0);
    QCOMPARE(bars_->yDataRange()->max, 6.0);

    // Still so on a frame of the animation.
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}}));
    const auto record = bars_->inspection()->recordAt(0);
    QCOMPARE(record.status, InspectionStatus::Ready);
    QCOMPARE(record.fields.value(QStringLiteral("x")).toDouble(), 8.0);
    bars_->setUpdateTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::hoverPicksTheDrawnErrorBars()
{
    auto morph = HeldHalfway<MorphTransition>{};
    bars_->setHoverRadius(0.0);
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setUpdateTransition(&morph);
    bars_->setData(errorBarsAt({8.0}), 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}}));

    QCOMPARE(bars_->errorBarIndexAt(pixelAt(5.0, 5.0)), 0);
    QCOMPARE(bars_->errorBarIndexAt(pixelAt(8.0, 5.0)), -1);

    bars_->setUpdateTransition(nullptr);
    QCOMPARE(bars_->errorBarIndexAt(pixelAt(5.0, 5.0)), -1);
    QCOMPARE(bars_->errorBarIndexAt(pixelAt(8.0, 5.0)), 0);
}

void ErrorBarSeriesTransitionsTest::transitionReachesTargetData()
{
    auto morph = MorphTransition{};
    morph.setDuration(50);
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setUpdateTransition(&morph);

    bars_->setData(errorBarsAt({8.0}), 1);
    QTRY_VERIFY_WITH_TIMEOUT(!morph.running(), 2000);

    QTRY_VERIFY(shows({{8.0, 5.0, Qt::red}, {5.0, 5.0, Qt::black}, {2.0, 5.0, Qt::black}}));
    bars_->setUpdateTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::interruptedTransitionShowsTargetData_data()
{
    QTest::addColumn<Interruption>("interruption");
    QTest::newRow("detach") << Interruption::Detach;
    QTest::newRow("replace") << Interruption::Replace;
    QTest::newRow("cancel") << Interruption::Cancel;
    QTest::newRow("destroy") << Interruption::Destroy;
}

void ErrorBarSeriesTransitionsTest::interruptedTransitionShowsTargetData()
{
    QFETCH(Interruption, interruption);
    auto transition = std::make_unique<HeldHalfway<MorphTransition>>();
    auto replacement = MorphTransition{};
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setUpdateTransition(transition.get());
    bars_->setData(errorBarsAt({8.0}), 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {8.0, 5.0, Qt::black}}));

    switch (interruption) {
    case Interruption::Detach:
        bars_->setUpdateTransition(nullptr);
        break;
    case Interruption::Replace:
        bars_->setUpdateTransition(&replacement);
        break;
    case Interruption::Cancel:
        transition->cancel();
        break;
    case Interruption::Destroy:
        transition.reset();
        break;
    }

    QTRY_VERIFY(shows({{8.0, 5.0, Qt::red}, {5.0, 5.0, Qt::black}}));
    QVERIFY(!transition || !transition->running());
    bars_->setUpdateTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::appendingSkipsTheAnimation()
{
    auto morph = HeldHalfway<MorphTransition>{};
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setUpdateTransition(&morph);
    bars_->setData(errorBarsAt({8.0}), 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}}));

    bars_->appendData(3.0, 5.0, 4.0, 6.0);

    QCOMPARE(bars_->count(), 2);
    QVERIFY(!morph.running());
    QTRY_VERIFY(shows({{8.0, 5.0, Qt::red}, {3.0, 5.0, Qt::red}, {5.0, 5.0, Qt::black}}));
    bars_->setUpdateTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::clearDataSkipsTheAnimation()
{
    auto morph = HeldHalfway<MorphTransition>{};
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setUpdateTransition(&morph);
    bars_->setData(errorBarsAt({8.0}), 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}}));

    bars_->clearData();

    QCOMPARE(bars_->count(), 0);
    QVERIFY(!morph.running());
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::black}, {8.0, 5.0, Qt::black}}));
    bars_->setUpdateTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::transitionPropertyNotifies()
{
    auto changes = QSignalSpy{bars_.get(), &ErrorBarSeries::updateTransitionChanged};
    auto morph = std::make_unique<MorphTransition>();

    bars_->setUpdateTransition(morph.get());
    bars_->setUpdateTransition(morph.get());
    QCOMPARE(changes.count(), 1);
    QCOMPARE(bars_->updateTransition(), static_cast<DataTransition*>(morph.get()));

    morph.reset();
    QCOMPARE(changes.count(), 2);
    QVERIFY(!bars_->updateTransition());
}

QPointF ErrorBarSeriesTransitionsTest::pixelAt(const qreal x, const qreal y) const
{
    return {(x - xAxis_->viewportMin()) / kDataSpan * kSize, (1.0 - (y - yAxis_->viewportMin()) / kDataSpan) * kSize};
}

void ErrorBarSeriesTransitionsTest::fadedEnterStartsTransparent()
{
    auto draw = DrawTransition{};
    draw.setDuration(kLongDuration);
    draw.setStagger(0.0);
    Test::addAnimation(draw, R"(NumberAnimation { property: "opacity"; from: 0; duration: 10000 })");
    bars_->setEnterTransition(&draw);

    bars_->setData(errorBarsAt({5.0}), 1);

    // Shown from the start, but still transparent.
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::black}}));
    draw.setDuration(0);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}}));
    bars_->setEnterTransition(nullptr);
}

bool ErrorBarSeriesTransitionsTest::shows(const std::initializer_list<Probe> probes) const
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
    auto test = QAccelPlot::ErrorBarSeriesTransitionsTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_error_bar_series_transitions.moc"
