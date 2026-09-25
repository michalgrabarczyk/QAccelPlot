//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
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

} // namespace

class LineCurveTransitionsTest : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void runningChangedIsEmittedOnGuiThread();
    void transitionReachesTargetData();

private:
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
