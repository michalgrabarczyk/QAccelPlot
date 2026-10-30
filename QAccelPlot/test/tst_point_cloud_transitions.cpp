//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/inspection/SeriesInspection.hpp"
#include "QAccelPlot/series/PointCloud.hpp"
#include "QAccelPlot/transitions/DrawTransition.hpp"
#include "QAccelPlot/transitions/MorphTransition.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <algorithm>
#include <cmath>
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

// The color of a value half way through the grayscale colormap.
const auto kGray = QColor{128, 128, 128};

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

struct Scene {
    std::unique_ptr<QQuickWindow> window;
    std::unique_ptr<Axis> xAxis;
    std::unique_ptr<Axis> yAxis;
    std::unique_ptr<Colormap> grayscale;
    std::unique_ptr<PointCloud> cloud;
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

} // namespace

class PointCloudTransitionsTest : public QObject {
    Q_OBJECT

public:
    enum class Interruption { Detach, Replace, Cancel, Destroy };
    Q_ENUM(Interruption)

private slots:
    void init();
    void cleanup();
    void morphMovesPointsToTheirNewPositions();
    void morphBlendsValues();
    void newPointsAppearAtTheirPosition();
    void removedPointsDisappear();
    void drawShowsPointsInOrder();
    void floatDataAnimates();
    void largeCoordinatesAnimate();
    void addedValuesApplyAtOnce();
    void removedValuesApplyAtOnce();
    void setValuesAnimates();
    void valuesSetAfterTheDataKeepItsAnimation();
    void valuesSetWithoutATransitionEndTheAnimation();
    void newDataContinuesFromTheDrawnPoints();
    void dataIsTheNewOneFromTheStart();
    void hoverPicksTheDrawnPoints();
    void transitionReachesTargetData();
    void interruptedTransitionShowsTargetData_data();
    void interruptedTransitionShowsTargetData();
    void clearDataSkipsTheAnimation();
    void sharedTransitionAnimatesSeriesIndependently();
    void transitionPropertyNotifies();

private:
    std::unique_ptr<PointCloud> addCloud(const QColor& color) const;
    // The item position of the data point (\a x, \a y).
    QPointF pixelAt(qreal x, qreal y) const;
    // True when the window shows every probe's color at its pixel.
    bool shows(std::initializer_list<Probe> probes) const;

    Scene scene_;
};

void PointCloudTransitionsTest::init()
{
    scene_.window = std::make_unique<QQuickWindow>();
    if (scene_.window->rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    scene_.window->setColor(Qt::black);
    scene_.window->resize(kSize, kSize);
    scene_.xAxis = makeAxis(Qt::Horizontal);
    scene_.yAxis = makeAxis(Qt::Vertical);
    scene_.grayscale = std::make_unique<Colormap>();
    scene_.grayscale->setPreset(Colormap::Preset::Grayscale);
    scene_.grayscale->setRange(0.0, 1.0);
    scene_.cloud = addCloud(Qt::red);
    scene_.window->show();
    QVERIFY(QTest::qWaitForWindowExposed(scene_.window.get()));
}

void PointCloudTransitionsTest::cleanup()
{
    scene_.cloud.reset();
    scene_.window.reset();
    scene_.grayscale.reset();
    scene_.xAxis.reset();
    scene_.yAxis.reset();
}

void PointCloudTransitionsTest::morphMovesPointsToTheirNewPositions()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.cloud->setData(std::vector<double>{2.0, 2.0}, 1);
    scene_.cloud->setTransition(&morph);

    scene_.cloud->setData(std::vector<double>{8.0, 8.0}, 1);

    // The data is the new one at once, while the point is drawn half way from (2, 2) to (8, 8).
    QCOMPARE(scene_.cloud->pointAt(0), QPointF(8.0, 8.0));
    QVERIFY(morph.running());
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {2.0, 2.0, Qt::black}, {8.0, 8.0, Qt::black}}));
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::morphBlendsValues()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.cloud->setColormap(scene_.grayscale.get());
    scene_.cloud->setData(std::vector<double>{5.0, 5.0}, std::vector<float>{0.0f}, 1);
    scene_.cloud->setTransition(&morph);

    scene_.cloud->setData(std::vector<double>{5.0, 5.0}, std::vector<float>{1.0f}, 1);

    QCOMPARE(scene_.cloud->valueAt(0), 1.0);
    QTRY_VERIFY(shows({{5.0, 5.0, kGray}}));
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::newPointsAppearAtTheirPosition()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.cloud->setData(std::vector<double>{2.0, 2.0}, 1);
    scene_.cloud->setTransition(&morph);

    scene_.cloud->setData(std::vector<double>{8.0, 8.0, 5.0, 2.0}, 2);

    QCOMPARE(scene_.cloud->count(), 2);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {5.0, 2.0, Qt::red}, {8.0, 8.0, Qt::black}}));
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::removedPointsDisappear()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.cloud->setData(std::vector<double>{2.0, 2.0, 5.0, 8.0}, 2);
    QTRY_VERIFY(shows({{5.0, 8.0, Qt::red}}));
    scene_.cloud->setTransition(&morph);

    scene_.cloud->setData(std::vector<double>{8.0, 8.0}, 1);

    QCOMPARE(scene_.cloud->count(), 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {5.0, 8.0, Qt::black}}));
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::drawShowsPointsInOrder()
{
    auto draw = HeldAt<DrawTransition>{0.5};
    scene_.cloud->setTransition(&draw);

    scene_.cloud->setData(std::vector<double>{2.0, 5.0, 4.0, 5.0, 6.0, 5.0, 8.0, 5.0}, 4);

    QCOMPARE(scene_.cloud->count(), 4);
    QTRY_VERIFY(shows({{2.0, 5.0, Qt::red}, {4.0, 5.0, Qt::red}, {6.0, 5.0, Qt::black}, {8.0, 5.0, Qt::black}}));

    draw.cancel();
    QTRY_VERIFY(shows({{6.0, 5.0, Qt::red}, {8.0, 5.0, Qt::red}}));
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::floatDataAnimates()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.cloud->setDataF(std::vector<float>{2.0f, 2.0f}, 1);
    scene_.cloud->setTransition(&morph);

    const auto next = std::vector<float>{8.0f, 8.0f};
    scene_.cloud->setDataF(next.data(), 1);

    QCOMPARE(scene_.cloud->pointAt(0), QPointF(8.0, 8.0));
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {2.0, 2.0, Qt::black}, {8.0, 8.0, Qt::black}}));
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::largeCoordinatesAnimate()
{
    // Single precision resolves 64 units here, so the frames must be uploaded relative to an origin.
    constexpr auto kOffset = 1.0e9;
    scene_.xAxis->setViewportMin(kOffset);
    scene_.xAxis->setViewportMax(kOffset + kDataSpan);
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.cloud->setData(std::vector<double>{kOffset + 2.0, 2.0}, 1);
    scene_.cloud->setTransition(&morph);

    scene_.cloud->setData(std::vector<double>{kOffset + 8.0, 8.0}, 1);

    QTRY_VERIFY(shows({{kOffset + 5.0, 5.0, Qt::red}, {kOffset + 2.0, 2.0, Qt::black}, {kOffset + 8.0, 8.0, Qt::black}}));
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::addedValuesApplyAtOnce()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.cloud->setColormap(scene_.grayscale.get());
    scene_.cloud->setData(std::vector<double>{2.0, 2.0}, 1);
    scene_.cloud->setTransition(&morph);

    scene_.cloud->setData(std::vector<double>{8.0, 8.0}, std::vector<float>{1.0f}, 1);

    // The point still moves, in the color of its new value.
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::white}, {8.0, 8.0, Qt::black}}));
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::removedValuesApplyAtOnce()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.cloud->setColormap(scene_.grayscale.get());
    scene_.cloud->setData(std::vector<double>{2.0, 2.0}, std::vector<float>{1.0f}, 1);
    scene_.cloud->setTransition(&morph);

    scene_.cloud->setData(std::vector<double>{8.0, 8.0}, 1);

    QVERIFY(std::isnan(scene_.cloud->valueAt(0)));
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {8.0, 8.0, Qt::black}}));
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::setValuesAnimates()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.cloud->setColormap(scene_.grayscale.get());
    scene_.cloud->setDataF(std::vector<float>{5.0f, 5.0f}, std::vector<float>{0.0f}, 1);
    scene_.cloud->setTransition(&morph);

    scene_.cloud->setValues({1.0});

    QCOMPARE(scene_.cloud->valueAt(0), 1.0);
    QCOMPARE(scene_.cloud->pointAt(0), QPointF(5.0, 5.0));
    QVERIFY(morph.running());
    QTRY_VERIFY(shows({{5.0, 5.0, kGray}}));

    scene_.cloud->setValues({});
    QVERIFY(std::isnan(scene_.cloud->valueAt(0)));
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}}));
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::valuesSetAfterTheDataKeepItsAnimation()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.cloud->setColormap(scene_.grayscale.get());
    scene_.cloud->setData(QList<QPointF>{{2.0, 2.0}});
    scene_.cloud->setTransition(&morph);

    // How QML supplies values: after the points.
    scene_.cloud->setData(QList<QPointF>{{8.0, 8.0}});
    scene_.cloud->setValues({1.0});

    QTRY_VERIFY(shows({{5.0, 5.0, Qt::white}, {2.0, 2.0, Qt::black}, {8.0, 8.0, Qt::black}}));
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::valuesSetWithoutATransitionEndTheAnimation()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.cloud->setColormap(scene_.grayscale.get());
    scene_.cloud->setData(std::vector<double>{2.0, 2.0}, 1);
    scene_.cloud->setTransition(&morph);
    scene_.cloud->setData(std::vector<double>{8.0, 8.0}, 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}}));

    morph.setEnabled(false);
    scene_.cloud->setValues({1.0});

    QVERIFY(!morph.running());
    QCOMPARE(scene_.cloud->valueAt(0), 1.0);
    QTRY_VERIFY(shows({{8.0, 8.0, Qt::white}, {5.0, 5.0, Qt::black}}));
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::newDataContinuesFromTheDrawnPoints()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.cloud->setData(std::vector<double>{0.0, 5.0}, 1);
    scene_.cloud->setTransition(&morph);
    scene_.cloud->setData(std::vector<double>{8.0, 5.0}, 1);
    QTRY_VERIFY(shows({{4.0, 5.0, Qt::red}}));

    // Half way from the drawn 4, not from the data position 8, back to 0.
    scene_.cloud->setData(std::vector<double>{0.0, 5.0}, 1);

    QTRY_VERIFY(shows({{2.0, 5.0, Qt::red}, {4.0, 5.0, Qt::black}}));
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::dataIsTheNewOneFromTheStart()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.cloud->setData(std::vector<double>{2.0, 2.0}, std::vector<float>{0.0f}, 1);
    scene_.cloud->setTransition(&morph);
    auto counts = QSignalSpy{scene_.cloud.get(), &PointCloud::countChanged};

    scene_.cloud->setData(std::vector<double>{8.0, 6.0, 4.0, 3.0}, std::vector<float>{5.0f, 7.0f}, 2);

    QCOMPARE(counts.count(), 1);
    QCOMPARE(scene_.cloud->count(), 2);
    QCOMPARE(scene_.cloud->pointAt(1), QPointF(4.0, 3.0));
    QCOMPARE(scene_.cloud->valueAt(0), 5.0);
    QCOMPARE(scene_.cloud->xDataRange()->min, 4.0);
    QCOMPARE(scene_.cloud->xDataRange()->max, 8.0);
    QCOMPARE(scene_.cloud->yDataRange()->min, 3.0);
    QCOMPARE(scene_.cloud->yDataRange()->max, 6.0);
    QCOMPARE(scene_.cloud->dataValueMin(), 5.0);
    QCOMPARE(scene_.cloud->dataValueMax(), 7.0);

    // Still so on a frame of the animation.
    QTRY_VERIFY(shows({{5.0, 4.0, Qt::red}}));
    QCOMPARE(scene_.cloud->pointAt(0), QPointF(8.0, 6.0));
    QCOMPARE(scene_.cloud->valueAt(0), 5.0);
    const auto dataPoint = scene_.cloud->inspection()->dataPointAt(0);
    QCOMPARE(dataPoint.x, 8.0);
    QCOMPARE(dataPoint.y, 6.0);
    QCOMPARE(dataPoint.value, 5.0);
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::hoverPicksTheDrawnPoints()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.cloud->setData(std::vector<double>{2.0, 2.0}, 1);
    scene_.cloud->setTransition(&morph);
    scene_.cloud->setData(std::vector<double>{8.0, 8.0}, 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}}));

    QCOMPARE(scene_.cloud->pointIndexAt(pixelAt(5.0, 5.0)), 0);
    QCOMPARE(scene_.cloud->pointIndexAt(pixelAt(8.0, 8.0)), -1);

    scene_.cloud->setTransition(nullptr);
    QCOMPARE(scene_.cloud->pointIndexAt(pixelAt(5.0, 5.0)), -1);
    QCOMPARE(scene_.cloud->pointIndexAt(pixelAt(8.0, 8.0)), 0);
}

void PointCloudTransitionsTest::transitionReachesTargetData()
{
    auto morph = MorphTransition{};
    morph.setDuration(50);
    scene_.cloud->setData(std::vector<double>{2.0, 2.0}, 1);
    scene_.cloud->setTransition(&morph);

    scene_.cloud->setData(std::vector<double>{8.0, 8.0}, 1);
    QTRY_VERIFY_WITH_TIMEOUT(!morph.running(), 2000);

    QTRY_VERIFY(shows({{8.0, 8.0, Qt::red}, {5.0, 5.0, Qt::black}, {2.0, 2.0, Qt::black}}));
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::interruptedTransitionShowsTargetData_data()
{
    QTest::addColumn<Interruption>("interruption");
    QTest::newRow("detach") << Interruption::Detach;
    QTest::newRow("replace") << Interruption::Replace;
    QTest::newRow("cancel") << Interruption::Cancel;
    QTest::newRow("destroy") << Interruption::Destroy;
}

void PointCloudTransitionsTest::interruptedTransitionShowsTargetData()
{
    QFETCH(Interruption, interruption);
    auto transition = std::make_unique<HeldAt<MorphTransition>>(0.5);
    auto replacement = MorphTransition{};
    scene_.cloud->setData(std::vector<double>{2.0, 2.0}, 1);
    scene_.cloud->setTransition(transition.get());
    scene_.cloud->setData(std::vector<double>{8.0, 8.0}, 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}, {8.0, 8.0, Qt::black}}));

    switch (interruption) {
    case Interruption::Detach:
        scene_.cloud->setTransition(nullptr);
        break;
    case Interruption::Replace:
        scene_.cloud->setTransition(&replacement);
        break;
    case Interruption::Cancel:
        transition->cancel();
        break;
    case Interruption::Destroy:
        transition.reset();
        break;
    }

    QTRY_VERIFY(shows({{8.0, 8.0, Qt::red}, {5.0, 5.0, Qt::black}}));
    QVERIFY(!transition || !transition->running());
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::clearDataSkipsTheAnimation()
{
    auto morph = HeldAt<MorphTransition>{0.5};
    scene_.cloud->setData(std::vector<double>{2.0, 2.0}, 1);
    scene_.cloud->setTransition(&morph);
    scene_.cloud->setData(std::vector<double>{8.0, 8.0}, 1);
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::red}}));

    scene_.cloud->clearData();

    QCOMPARE(scene_.cloud->count(), 0);
    QVERIFY(!morph.running());
    QTRY_VERIFY(shows({{5.0, 5.0, Qt::black}, {8.0, 8.0, Qt::black}}));
    scene_.cloud->setTransition(nullptr);
}

void PointCloudTransitionsTest::sharedTransitionAnimatesSeriesIndependently()
{
    const auto other = addCloud(Qt::blue);
    auto morph = MorphTransition{};
    morph.setDuration(100);
    scene_.cloud->setData(std::vector<double>{2.0, 2.0}, 1);
    other->setData(std::vector<double>{2.0, 8.0}, 1);
    scene_.cloud->setTransition(&morph);
    other->setTransition(&morph);

    scene_.cloud->setData(std::vector<double>{8.0, 2.0}, 1);
    other->setData(std::vector<double>{8.0, 8.0}, 1);
    QTRY_VERIFY_WITH_TIMEOUT(!morph.running(), 2000);

    QTRY_VERIFY(shows({{8.0, 2.0, Qt::red}, {8.0, 8.0, Qt::blue}, {2.0, 2.0, Qt::black}, {2.0, 8.0, Qt::black}}));
    scene_.cloud->setTransition(nullptr);
    other->setTransition(nullptr);
}

void PointCloudTransitionsTest::transitionPropertyNotifies()
{
    auto changes = QSignalSpy{scene_.cloud.get(), &PointCloud::transitionChanged};
    auto morph = std::make_unique<MorphTransition>();

    scene_.cloud->setTransition(morph.get());
    scene_.cloud->setTransition(morph.get());
    QCOMPARE(changes.count(), 1);
    QCOMPARE(scene_.cloud->transition(), static_cast<DataTransition*>(morph.get()));

    morph.reset();
    QCOMPARE(changes.count(), 2);
    QVERIFY(!scene_.cloud->transition());
}

std::unique_ptr<PointCloud> PointCloudTransitionsTest::addCloud(const QColor& color) const
{
    auto cloud = std::make_unique<PointCloud>();
    cloud->setParentItem(scene_.window->contentItem());
    cloud->setSize({kSize, kSize});
    cloud->setXAxis(scene_.xAxis.get());
    cloud->setYAxis(scene_.yAxis.get());
    cloud->setPlotRect({0, 0, kSize, kSize});
    cloud->setColor(color);
    cloud->marker()->setShape(MarkerShape::Square);
    cloud->marker()->setSize(6.0);
    return cloud;
}

QPointF PointCloudTransitionsTest::pixelAt(const qreal x, const qreal y) const
{
    return {(x - scene_.xAxis->viewportMin()) / kDataSpan * kSize, (1.0 - (y - scene_.yAxis->viewportMin()) / kDataSpan) * kSize};
}

bool PointCloudTransitionsTest::shows(const std::initializer_list<Probe> probes) const
{
    const auto image = scene_.window->grabWindow();
    return std::all_of(
        probes.begin(), probes.end(), [&](const Probe& probe) { return isColor(image.pixelColor(pixelAt(probe.x, probe.y).toPoint()), probe.color); });
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Pixel lookups use logical coordinates, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::PointCloudTransitionsTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_point_cloud_transitions.moc"
