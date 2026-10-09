//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/series/BarSeries.hpp"
#include "QAccelPlot/transitions/DrawTransition.hpp"
#include "QAccelPlot/transitions/MorphTransition.hpp"

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
    void interpolate(double /*easedProgress*/, const DataTransition::Dataset& from, const DataTransition::Dataset& to, DataTransition::Dataset& out) override
    {
        Transition::interpolate(progress_, from, to, out);
    }

private:
    double progress_;
};

struct Scene {
    std::unique_ptr<QQuickWindow> window;
    std::unique_ptr<Axis> xAxis;
    std::unique_ptr<Axis> yAxis;
    std::unique_ptr<BarSeries> bars;
};

// A pixel of the plot, given in data coordinates, and the color expected there.
struct Probe {
    qreal x;
    qreal y;
    QColor color;
};

std::unique_ptr<Axis> makeAxis(const Axis::Orientation orientation)
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

} // namespace

class BarSeriesTransitionsTest : public QObject {
    Q_OBJECT

public:
    enum class Interruption { Detach, Replace, Cancel, Destroy };
    Q_ENUM(Interruption)

private slots:
    void init();
    void cleanup();
    void morphMovesBarsToTheirNewValues();
    void newBarsGrowFromTheBaseline();
    void removedBarsShrinkToTheBaseline();
    void removedBarsKeepTheirCategoryColor();
    void categoriesSetDuringAnAnimationApply();
    void drawShowsBarsInOrder();
    void rangedBarsMorphTheirExtent();
    void changedLayoutGrowsTheNewBarsIn();
    void floatDataAnimates();
    void newDataContinuesFromTheDrawnBars();
    void transitionReachesTargetData();
    void interruptedTransitionShowsTargetData_data();
    void interruptedTransitionShowsTargetData();
    void clearDataSkipsTheAnimation();
    void sharedTransitionAnimatesSeriesIndependently();
    void transitionPropertyNotifies();

private:
    std::unique_ptr<BarSeries> addBars(const QColor& color) const;
    // True when the window shows every probe's color at its pixel.
    bool shows(std::initializer_list<Probe> probes) const;

    Scene scene_;
};

void BarSeriesTransitionsTest::init()
{
    scene_.window = std::make_unique<QQuickWindow>();
    if (scene_.window->rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    scene_.window->setColor(Qt::black);
    scene_.window->resize(kSize, kSize);
    scene_.xAxis = makeAxis(Axis::Horizontal);
    scene_.yAxis = makeAxis(Axis::Vertical);
    scene_.bars = addBars(Qt::red);
    scene_.window->show();
    QVERIFY(QTest::qWaitForWindowExposed(scene_.window.get()));
}

void BarSeriesTransitionsTest::cleanup()
{
    scene_.bars.reset();
    scene_.window.reset();
    scene_.xAxis.reset();
    scene_.yAxis.reset();
}

void BarSeriesTransitionsTest::morphMovesBarsToTheirNewValues()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.bars->setData(std::vector<double>{5.0, 2.0}, 1);
    scene_.bars->setTransition(&morph);

    scene_.bars->setData(std::vector<double>{5.0, 8.0}, 1);

    // The data is the new one at once, while the bar is drawn half way from 2 to 8.
    QCOMPARE(scene_.bars->barAt(0).value(QStringLiteral("value")).toDouble(), 8.0);
    QVERIFY(morph.running());
    QTRY_VERIFY(shows({{5.0, 4.5, Qt::red}, {5.0, 5.5, Qt::black}}));
    scene_.bars->setTransition(nullptr);
}

void BarSeriesTransitionsTest::newBarsGrowFromTheBaseline()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.bars->setTransition(&morph);

    scene_.bars->setData(std::vector<double>{2.0, 8.0, 7.0, 4.0}, 2);

    QCOMPARE(scene_.bars->count(), 2);
    QTRY_VERIFY(shows({{2.0, 3.5, Qt::red}, {2.0, 4.5, Qt::black}, {7.0, 1.5, Qt::red}, {7.0, 2.5, Qt::black}}));
    scene_.bars->setTransition(nullptr);
}

void BarSeriesTransitionsTest::removedBarsShrinkToTheBaseline()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.bars->setData(std::vector<double>{2.0, 8.0, 7.0, 4.0}, 2);
    scene_.bars->setTransition(&morph);

    scene_.bars->setData(std::vector<double>{2.0, 8.0}, 1);

    QCOMPARE(scene_.bars->count(), 1);
    QTRY_VERIFY(shows({{2.0, 7.5, Qt::red}, {7.0, 1.5, Qt::red}, {7.0, 2.5, Qt::black}}));

    morph.cancel();
    QTRY_VERIFY(shows({{2.0, 7.5, Qt::red}, {7.0, 1.5, Qt::black}}));
    scene_.bars->setTransition(nullptr);
}

void BarSeriesTransitionsTest::removedBarsKeepTheirCategoryColor()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.bars->setCategoryColors({Qt::blue});
    scene_.bars->setData(std::vector<double>{2.0, 8.0, 7.0, 8.0}, std::vector<int>{-1, 0}, 2);
    scene_.bars->setTransition(&morph);

    scene_.bars->setData(std::vector<double>{2.0, 8.0}, 1);

    QTRY_VERIFY(shows({{2.0, 7.5, Qt::red}, {7.0, 3.5, Qt::blue}, {7.0, 4.5, Qt::black}}));
    scene_.bars->setTransition(nullptr);
}

void BarSeriesTransitionsTest::categoriesSetDuringAnAnimationApply()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.bars->setCategoryColors({Qt::blue});
    scene_.bars->setTransition(&morph);
    scene_.bars->setData(std::vector<double>{2.0, 8.0, 7.0, 8.0}, 2);
    QTRY_VERIFY(shows({{2.0, 3.5, Qt::red}, {7.0, 3.5, Qt::red}}));

    scene_.bars->setCategories({-1, 0});

    QTRY_VERIFY(shows({{2.0, 3.5, Qt::red}, {7.0, 3.5, Qt::blue}, {7.0, 4.5, Qt::black}}));
    scene_.bars->setTransition(nullptr);
}

void BarSeriesTransitionsTest::drawShowsBarsInOrder()
{
    auto draw = HeldAt<DrawTransition>{0.5};
    scene_.bars->setTransition(&draw);

    scene_.bars->setData(std::vector<double>{1.0, 8.0, 3.0, 8.0, 5.0, 8.0, 7.0, 8.0}, 4);

    QTRY_VERIFY(shows({{1.0, 5.0, Qt::red}, {3.0, 5.0, Qt::red}, {5.0, 5.0, Qt::black}, {7.0, 5.0, Qt::black}}));

    draw.cancel();
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {7.0, 5.0, Qt::red}}));
    scene_.bars->setTransition(nullptr);
}

void BarSeriesTransitionsTest::rangedBarsMorphTheirExtent()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.bars->setRangedData(std::vector<double>{1.0, 2.0, 4.0}, 1);
    scene_.bars->setTransition(&morph);

    scene_.bars->setRangedData(std::vector<double>{3.0, 5.0, 8.0}, 1);

    // Half way, the bar spans 2..3.5 and reaches 6.
    QTRY_VERIFY(shows({{2.75, 5.5, Qt::red}, {2.75, 6.5, Qt::black}, {1.5, 2.0, Qt::black}, {4.25, 2.0, Qt::black}}));
    scene_.bars->setTransition(nullptr);
}

void BarSeriesTransitionsTest::changedLayoutGrowsTheNewBarsIn()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.bars->setData(std::vector<double>{2.0, 8.0}, 1);
    scene_.bars->setTransition(&morph);

    scene_.bars->setRangedData(std::vector<double>{6.0, 8.0, 8.0}, 1);

    QTRY_VERIFY(shows({{2.0, 2.0, Qt::black}, {7.0, 3.5, Qt::red}, {7.0, 4.5, Qt::black}}));
    scene_.bars->setTransition(nullptr);
}

void BarSeriesTransitionsTest::floatDataAnimates()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.bars->setDataF(std::vector<float>{5.0f, 2.0f}, 1);
    scene_.bars->setTransition(&morph);

    const auto next = std::vector<float>{5.0f, 8.0f};
    scene_.bars->setDataF(next.data(), 1);

    QCOMPARE(scene_.bars->barAt(0).value(QStringLiteral("value")).toDouble(), 8.0);
    QTRY_VERIFY(shows({{5.0, 4.5, Qt::red}, {5.0, 5.5, Qt::black}}));
    scene_.bars->setTransition(nullptr);
}

void BarSeriesTransitionsTest::newDataContinuesFromTheDrawnBars()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.bars->setData(std::vector<double>{5.0, 0.0}, 1);
    scene_.bars->setTransition(&morph);
    scene_.bars->setData(std::vector<double>{5.0, 8.0}, 1);
    QTRY_VERIFY(shows({{5.0, 3.5, Qt::red}, {5.0, 4.5, Qt::black}}));

    // Half way from the drawn 4, not from the data value 8, back to 0.
    scene_.bars->setData(std::vector<double>{5.0, 0.0}, 1);

    QTRY_VERIFY(shows({{5.0, 1.5, Qt::red}, {5.0, 2.5, Qt::black}}));
    scene_.bars->setTransition(nullptr);
}

void BarSeriesTransitionsTest::transitionReachesTargetData()
{
    auto morph = MorphTransition{};
    morph.setDuration(50);
    scene_.bars->setData(std::vector<double>{5.0, 2.0}, 1);
    scene_.bars->setTransition(&morph);

    scene_.bars->setData(std::vector<double>{5.0, 8.0}, 1);
    QTRY_VERIFY_WITH_TIMEOUT(!morph.running(), 2000);

    QTRY_VERIFY(shows({{5.0, 7.5, Qt::red}, {5.0, 8.5, Qt::black}}));
    scene_.bars->setTransition(nullptr);
}

void BarSeriesTransitionsTest::interruptedTransitionShowsTargetData_data()
{
    QTest::addColumn<Interruption>("interruption");
    QTest::newRow("detach") << Interruption::Detach;
    QTest::newRow("replace") << Interruption::Replace;
    QTest::newRow("cancel") << Interruption::Cancel;
    QTest::newRow("destroy") << Interruption::Destroy;
}

void BarSeriesTransitionsTest::interruptedTransitionShowsTargetData()
{
    QFETCH(Interruption, interruption);
    auto transition = std::make_unique<HeldAt<MorphTransition>>(0.5);
    auto replacement = MorphTransition{};
    scene_.bars->setData(std::vector<double>{5.0, 2.0}, 1);
    scene_.bars->setTransition(transition.get());
    scene_.bars->setData(std::vector<double>{5.0, 8.0}, 1);
    QTRY_VERIFY(shows({{5.0, 4.5, Qt::red}, {5.0, 5.5, Qt::black}}));

    switch (interruption) {
    case Interruption::Detach:
        scene_.bars->setTransition(nullptr);
        break;
    case Interruption::Replace:
        scene_.bars->setTransition(&replacement);
        break;
    case Interruption::Cancel:
        transition->cancel();
        break;
    case Interruption::Destroy:
        transition.reset();
        break;
    }

    QTRY_VERIFY(shows({{5.0, 7.5, Qt::red}}));
    QVERIFY(!transition || !transition->running());
    scene_.bars->setTransition(nullptr);
}

void BarSeriesTransitionsTest::clearDataSkipsTheAnimation()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.bars->setData(std::vector<double>{5.0, 8.0}, 1);
    scene_.bars->setTransition(&morph);
    scene_.bars->setData(std::vector<double>{5.0, 4.0}, 1);
    QVERIFY(morph.running());

    scene_.bars->clearData();

    QCOMPARE(scene_.bars->count(), 0);
    QVERIFY(!morph.running());
    QTRY_VERIFY(shows({{5.0, 1.0, Qt::black}}));
    scene_.bars->setTransition(nullptr);
}

void BarSeriesTransitionsTest::sharedTransitionAnimatesSeriesIndependently()
{
    const auto other = addBars(Qt::blue);
    auto morph = MorphTransition{};
    morph.setDuration(100);
    scene_.bars->setTransition(&morph);
    other->setTransition(&morph);

    scene_.bars->setData(std::vector<double>{2.0, 8.0}, 1);
    other->setData(std::vector<double>{7.0, 4.0}, 1);
    QTRY_VERIFY_WITH_TIMEOUT(!morph.running(), 2000);

    QTRY_VERIFY(shows({{2.0, 7.5, Qt::red}, {7.0, 3.5, Qt::blue}, {7.0, 4.5, Qt::black}}));
    scene_.bars->setTransition(nullptr);
    other->setTransition(nullptr);
}

void BarSeriesTransitionsTest::transitionPropertyNotifies()
{
    auto changes = QSignalSpy{scene_.bars.get(), &BarSeries::transitionChanged};
    auto morph = std::make_unique<MorphTransition>();

    scene_.bars->setTransition(morph.get());
    scene_.bars->setTransition(morph.get());
    QCOMPARE(changes.count(), 1);
    QCOMPARE(scene_.bars->transition(), static_cast<DataTransition*>(morph.get()));

    morph.reset();
    QCOMPARE(changes.count(), 2);
    QVERIFY(!scene_.bars->transition());
}

std::unique_ptr<BarSeries> BarSeriesTransitionsTest::addBars(const QColor& color) const
{
    auto bars = std::make_unique<BarSeries>();
    bars->setParentItem(scene_.window->contentItem());
    bars->setXAxis(scene_.xAxis.get());
    bars->setYAxis(scene_.yAxis.get());
    bars->setPlotRect({0, 0, kSize, kSize});
    bars->setColor(color);
    bars->setBarWidth(1.0);
    return bars;
}

bool BarSeriesTransitionsTest::shows(const std::initializer_list<Probe> probes) const
{
    const auto image = scene_.window->grabWindow();
    return std::all_of(probes.begin(), probes.end(), [&image](const Probe& probe) {
        const auto pixel = QPoint{qRound(probe.x / kDataSpan * kSize), qRound((1.0 - probe.y / kDataSpan) * kSize)};
        return isColor(image.pixelColor(pixel), probe.color);
    });
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Pixel lookups use logical coordinates, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::BarSeriesTransitionsTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_bar_series_transitions.moc"
