//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/transitions/DrawTransition.hpp"
#include "QAccelPlot/transitions/MorphTransition.hpp"

#include <QGuiApplication>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QThread>
#include <QtTest/QtTest>

#include <memory>

namespace QAccelPlot {

namespace {

// 100 x 100 px plot showing data in [0, 1] x [0, 1].
constexpr auto kSize = 100;

struct Scene {
    std::unique_ptr<QQuickWindow> window;
    std::unique_ptr<Axis> xAxis;
    std::unique_ptr<Axis> yAxis;
    std::unique_ptr<LineCurve> curve;
};

std::unique_ptr<Axis> makeAxis(const Axis::Orientation orientation)
{
    auto axis = std::make_unique<Axis>();
    axis->setOrientation(orientation);
    axis->setViewportMin(0.0);
    axis->setViewportMax(1.0);
    return axis;
}

QList<QPointF> horizontalLine(const qreal y)
{
    return {{0.1, y}, {0.9, y}};
}

// 101 samples across the plot, so a partially drawn line misses the right edge.
QList<QPointF> denseHorizontalLine(const qreal y)
{
    auto points = QList<QPointF>{};
    for (auto index = 0; index <= 100; ++index) {
        points.append({index / 100.0, y});
    }
    return points;
}

bool hits(const LineCurve* curve, const QPointF& point)
{
    return static_cast<const QQuickItem*>(curve)->contains(point);
}

// Long enough that the tests always interrupt the animation part way.
constexpr auto kLongDuration = 10'000;

} // namespace

class LineCurveTransitionsTest : public QObject {
    Q_OBJECT

public:
    enum class Interruption { Detach, Replace, Cancel, Destroy };
    Q_ENUM(Interruption)

private slots:
    void init();
    void cleanup();
    void runningChangedIsEmittedOnGuiThread();
    void transitionReachesTargetData();
    void transitionAdvancesWithParentAtConstruction();
    void sharedTransitionAnimatesCurvesIndependently();
    void interruptedTransitionShowsTargetData_data();
    void interruptedTransitionShowsTargetData();
    void appendDataDuringTransitionKeepsTargetData();
    void reassignedTransitionKeepsCurrentData();

private:
    std::unique_ptr<LineCurve> addCurve(qreal y) const;

    Scene scene_;
};

void LineCurveTransitionsTest::init()
{
    scene_.window = std::make_unique<QQuickWindow>();
    if (scene_.window->rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom curve materials require a hardware scene graph backend");
    }
    scene_.window->resize(kSize, kSize);
    scene_.xAxis = makeAxis(Axis::Horizontal);
    scene_.yAxis = makeAxis(Axis::Vertical);
    scene_.curve = std::make_unique<LineCurve>();
    scene_.curve->setParentItem(scene_.window->contentItem());
    scene_.curve->setXAxis(scene_.xAxis.get());
    scene_.curve->setYAxis(scene_.yAxis.get());
    scene_.curve->setPlotRect({0, 0, kSize, kSize});
    scene_.curve->setData(horizontalLine(0.2));
    scene_.window->show();
    QVERIFY(QTest::qWaitForWindowExposed(scene_.window.get()));
}

void LineCurveTransitionsTest::cleanup()
{
    scene_.curve.reset();
    scene_.window.reset();
    scene_.xAxis.reset();
    scene_.yAxis.reset();
}

void LineCurveTransitionsTest::runningChangedIsEmittedOnGuiThread()
{
    auto transition = MorphTransition{};
    transition.setDuration(50);
    scene_.curve->setTransition(&transition);

    // QML handlers and bindings run directly on the emitting thread, like a direct connection.
    auto emittingThreads = QList<QThread*>{};
    connect(&transition, &DataTransition::runningChanged, this, [&]() { emittingThreads.append(QThread::currentThread()); }, Qt::DirectConnection);

    scene_.curve->setData(horizontalLine(0.8));
    QTRY_VERIFY_WITH_TIMEOUT(!transition.running(), 2000);

    QCOMPARE(emittingThreads.size(), 2);
    for (const auto* thread : std::as_const(emittingThreads)) {
        QCOMPARE(thread, qApp->thread());
    }
    scene_.curve->setTransition(nullptr);
}

void LineCurveTransitionsTest::transitionReachesTargetData()
{
    auto transition = MorphTransition{};
    transition.setDuration(50);
    scene_.curve->setTransition(&transition);

    scene_.curve->setData(horizontalLine(0.8));
    QTRY_VERIFY_WITH_TIMEOUT(!transition.running(), 2000);

    // Data y = 0.8 maps to pixel row 20, the start data y = 0.2 to row 80.
    const auto* item = static_cast<const QQuickItem*>(scene_.curve.get());
    QVERIFY(item->contains({50, 20}));
    QVERIFY(!item->contains({50, 80}));
    scene_.curve->setTransition(nullptr);
}

void LineCurveTransitionsTest::transitionAdvancesWithParentAtConstruction()
{
    // The parent is already in the window, so the curve joins it from the QQuickItem constructor.
    auto curve = std::make_unique<LineCurve>(scene_.window->contentItem());
    curve->setXAxis(scene_.xAxis.get());
    curve->setYAxis(scene_.yAxis.get());
    curve->setPlotRect({0, 0, kSize, kSize});
    curve->setData(horizontalLine(0.2));
    auto transition = MorphTransition{};
    transition.setDuration(50);
    curve->setTransition(&transition);

    curve->setData(horizontalLine(0.8));
    QTRY_VERIFY_WITH_TIMEOUT(!transition.running(), 2000);

    QVERIFY(static_cast<const QQuickItem*>(curve.get())->contains({50, 20}));
    curve->setTransition(nullptr);
}

void LineCurveTransitionsTest::sharedTransitionAnimatesCurvesIndependently()
{
    const auto other = addCurve(0.95);
    auto transition = MorphTransition{};
    transition.setDuration(100);
    scene_.curve->setTransition(&transition);
    other->setTransition(&transition);

    scene_.curve->setData(horizontalLine(0.8));
    other->setData(horizontalLine(0.05));
    QTRY_VERIFY_WITH_TIMEOUT(!transition.running(), 2000);

    // Data y = 0.8 maps to pixel row 20, y = 0.05 to row 95.
    QVERIFY(hits(scene_.curve.get(), {50, 20}));
    QVERIFY(!hits(scene_.curve.get(), {50, 95}));
    QVERIFY(hits(other.get(), {50, 95}));
    scene_.curve->setTransition(nullptr);
    other->setTransition(nullptr);
}

void LineCurveTransitionsTest::interruptedTransitionShowsTargetData_data()
{
    QTest::addColumn<Interruption>("interruption");
    QTest::newRow("detach") << Interruption::Detach;
    QTest::newRow("replace") << Interruption::Replace;
    QTest::newRow("cancel") << Interruption::Cancel;
    QTest::newRow("destroy") << Interruption::Destroy;
}

void LineCurveTransitionsTest::interruptedTransitionShowsTargetData()
{
    QFETCH(Interruption, interruption);
    auto transition = std::make_unique<DrawTransition>();
    transition->setDuration(kLongDuration);
    auto replacement = MorphTransition{};
    scene_.curve->setTransition(transition.get());
    scene_.curve->setData(denseHorizontalLine(0.8));
    // The first frame draws the start of the target.
    QTRY_VERIFY(hits(scene_.curve.get(), {5, 20}));

    switch (interruption) {
    case Interruption::Detach:
        scene_.curve->setTransition(nullptr);
        break;
    case Interruption::Replace:
        scene_.curve->setTransition(&replacement);
        break;
    case Interruption::Cancel:
        transition->cancel();
        break;
    case Interruption::Destroy:
        transition.reset();
        break;
    }
    QTest::qWait(50);

    QVERIFY(hits(scene_.curve.get(), {95, 20}));
    QVERIFY(!transition || !transition->running());
    scene_.curve->setTransition(nullptr);
}

void LineCurveTransitionsTest::appendDataDuringTransitionKeepsTargetData()
{
    auto transition = DrawTransition{};
    transition.setDuration(kLongDuration);
    scene_.curve->setTransition(&transition);

    // A step from y = 0.2 (row 80) on the left half to y = 0.8 (row 20) on the right half.
    auto target = QList<QPointF>{};
    for (auto index = 0; index < 100; ++index) {
        const auto x = index / 100.0;
        target.append({x, x < 0.5 ? 0.2 : 0.8});
    }
    scene_.curve->setData(target);
    // The first frames draw only the start of the target, which ends the old line across the plot.
    QTRY_VERIFY(!hits(scene_.curve.get(), {50, 80}));

    scene_.curve->appendData(1.0, 0.8);
    QTest::qWait(50);

    QVERIFY(!transition.running());
    QVERIFY(hits(scene_.curve.get(), {25, 80}));
    QVERIFY(hits(scene_.curve.get(), {75, 20}));
    scene_.curve->setTransition(nullptr);
}

void LineCurveTransitionsTest::reassignedTransitionKeepsCurrentData()
{
    auto transition = MorphTransition{};
    transition.setDuration(kLongDuration);
    scene_.curve->setTransition(&transition);
    scene_.curve->setData(horizontalLine(0.8));
    QTest::qWait(50);

    scene_.curve->setTransition(nullptr);
    scene_.curve->setData(horizontalLine(0.5));
    scene_.curve->setTransition(&transition);
    // Any repaint lets a transition that still runs overwrite the data.
    scene_.curve->setColor(Qt::red);
    QTest::qWait(100);

    QVERIFY(!transition.running());
    QVERIFY(hits(scene_.curve.get(), {50, 50}));
    QVERIFY(!hits(scene_.curve.get(), {50, 20}));
    scene_.curve->setTransition(nullptr);
}

std::unique_ptr<LineCurve> LineCurveTransitionsTest::addCurve(const qreal y) const
{
    auto curve = std::make_unique<LineCurve>();
    curve->setParentItem(scene_.window->contentItem());
    curve->setXAxis(scene_.xAxis.get());
    curve->setYAxis(scene_.yAxis.get());
    curve->setPlotRect({0, 0, kSize, kSize});
    curve->setData(horizontalLine(y));
    return curve;
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // The threaded render loop runs updatePaintNode() off the GUI thread, which these tests check against.
    qputenv("QSG_RENDER_LOOP", "threaded");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::LineCurveTransitionsTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_line_curve_transitions.moc"
