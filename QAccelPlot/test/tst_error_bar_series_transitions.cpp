//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
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

// A transition held at one progress, so the tests can check a frame of the animation.
template <typename Transition> class HeldAt : public Transition {
public:
    explicit HeldAt(const double progress)
        : progress_(progress)
    {
        this->setDuration(kLongDuration);
    }

protected:
    void interpolate(
        [[maybe_unused]] double easedProgress, const DataTransition::Dataset& from, const DataTransition::Dataset& to, DataTransition::Dataset& out) override
    {
        Transition::interpolate(progress_, from, to, out);
    }

private:
    double progress_;
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
    auto morph = HeldAt<MorphTransition>{0.5};
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setTransition(&morph);

    bars_->setData(errorBarsAt({8.0}), 1);

    // The data is the new one at once, while the error bar is drawn half way from x 2 to x 8.
    QCOMPARE(bars_->errorBarAt(0).value(QStringLiteral("x")).toDouble(), 8.0);
    QVERIFY(morph.running());
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {2.0, 5.0, Qt::black}, {8.0, 5.0, Qt::black}}));
    bars_->setTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::morphMovesBounds()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    bars_->setData(std::vector<double>{5.0, 5.0, 4.0, 6.0}, 1);
    bars_->setTransition(&morph);

    bars_->setErrorData(std::vector<double>{5.0}, std::vector<double>{5.0}, std::vector<double>{3.0});

    // Half way from 4..6 to 2..8 is 3..7.
    QTRY_VERIFY(shows({{5.0, 3.5, Qt::red}, {5.0, 6.5, Qt::red}, {5.0, 2.5, Qt::black}, {5.0, 7.5, Qt::black}}));
    bars_->setTransition(nullptr);
    QTRY_VERIFY(shows({{5.0, 2.5, Qt::red}, {5.0, 7.5, Qt::red}}));
}

void ErrorBarSeriesTransitionsTest::newErrorBarsAppearAtTheirPlace()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setTransition(&morph);

    bars_->setData(errorBarsAt({8.0, 3.0}), 2);

    QCOMPARE(bars_->count(), 2);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {3.0, 5.0, Qt::red}, {8.0, 5.0, Qt::black}}));
    bars_->setTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::removedErrorBarsDisappear()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    bars_->setData(errorBarsAt({2.0, 7.0}), 2);
    QTRY_VERIFY(shows({{7.0, 5.0, Qt::red}}));
    bars_->setTransition(&morph);

    bars_->setData(errorBarsAt({8.0}), 1);

    QCOMPARE(bars_->count(), 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {7.0, 5.0, Qt::black}}));
    bars_->setTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::drawShowsErrorBarsInOrder()
{
    auto draw = HeldAt<DrawTransition>{0.5};
    bars_->setTransition(&draw);

    bars_->setData(errorBarsAt({2.0, 4.0, 6.0, 8.0}), 4);

    QCOMPARE(bars_->count(), 4);
    QTRY_VERIFY(shows({{2.0, 5.0, Qt::red}, {4.0, 5.0, Qt::red}, {6.0, 5.0, Qt::black}, {8.0, 5.0, Qt::black}}));

    draw.cancel();
    QTRY_VERIFY(shows({{6.0, 5.0, Qt::red}, {8.0, 5.0, Qt::red}}));
    bars_->setTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::floatDataAnimates()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    bars_->setDataF(std::vector<float>{2.0f, 5.0f, 4.0f, 6.0f}, 1);
    bars_->setTransition(&morph);

    const auto next = std::vector<float>{8.0f, 5.0f, 4.0f, 6.0f};
    bars_->setDataF(next.data(), 1);

    QCOMPARE(bars_->errorBarAt(0).value(QStringLiteral("x")).toDouble(), 8.0);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {2.0, 5.0, Qt::black}, {8.0, 5.0, Qt::black}}));
    bars_->setTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::dataIsTheNewOneFromTheStart()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setTransition(&morph);
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
    bars_->setTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::hoverPicksTheDrawnErrorBars()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    bars_->setHoverRadius(0.0);
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setTransition(&morph);
    bars_->setData(errorBarsAt({8.0}), 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}}));

    QCOMPARE(bars_->errorBarIndexAt(pixelAt(5.0, 5.0)), 0);
    QCOMPARE(bars_->errorBarIndexAt(pixelAt(8.0, 5.0)), -1);

    bars_->setTransition(nullptr);
    QCOMPARE(bars_->errorBarIndexAt(pixelAt(5.0, 5.0)), -1);
    QCOMPARE(bars_->errorBarIndexAt(pixelAt(8.0, 5.0)), 0);
}

void ErrorBarSeriesTransitionsTest::transitionReachesTargetData()
{
    auto morph = MorphTransition{};
    morph.setDuration(50);
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setTransition(&morph);

    bars_->setData(errorBarsAt({8.0}), 1);
    QTRY_VERIFY_WITH_TIMEOUT(!morph.running(), 2000);

    QTRY_VERIFY(shows({{8.0, 5.0, Qt::red}, {5.0, 5.0, Qt::black}, {2.0, 5.0, Qt::black}}));
    bars_->setTransition(nullptr);
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
    auto transition = std::make_unique<HeldAt<MorphTransition>>(0.5);
    auto replacement = MorphTransition{};
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setTransition(transition.get());
    bars_->setData(errorBarsAt({8.0}), 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {8.0, 5.0, Qt::black}}));

    switch (interruption) {
    case Interruption::Detach:
        bars_->setTransition(nullptr);
        break;
    case Interruption::Replace:
        bars_->setTransition(&replacement);
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
    bars_->setTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::appendingSkipsTheAnimation()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setTransition(&morph);
    bars_->setData(errorBarsAt({8.0}), 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}}));

    bars_->appendData(3.0, 5.0, 4.0, 6.0);

    QCOMPARE(bars_->count(), 2);
    QVERIFY(!morph.running());
    QTRY_VERIFY(shows({{8.0, 5.0, Qt::red}, {3.0, 5.0, Qt::red}, {5.0, 5.0, Qt::black}}));
    bars_->setTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::clearDataSkipsTheAnimation()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    bars_->setData(errorBarsAt({2.0}), 1);
    bars_->setTransition(&morph);
    bars_->setData(errorBarsAt({8.0}), 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}}));

    bars_->clearData();

    QCOMPARE(bars_->count(), 0);
    QVERIFY(!morph.running());
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::black}, {8.0, 5.0, Qt::black}}));
    bars_->setTransition(nullptr);
}

void ErrorBarSeriesTransitionsTest::transitionPropertyNotifies()
{
    auto changes = QSignalSpy{bars_.get(), &ErrorBarSeries::transitionChanged};
    auto morph = std::make_unique<MorphTransition>();

    bars_->setTransition(morph.get());
    bars_->setTransition(morph.get());
    QCOMPARE(changes.count(), 1);
    QCOMPARE(bars_->transition(), static_cast<DataTransition*>(morph.get()));

    morph.reset();
    QCOMPARE(changes.count(), 2);
    QVERIFY(!bars_->transition());
}

QPointF ErrorBarSeriesTransitionsTest::pixelAt(const qreal x, const qreal y) const
{
    return {(x - xAxis_->viewportMin()) / kDataSpan * kSize, (1.0 - (y - yAxis_->viewportMin()) / kDataSpan) * kSize};
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
