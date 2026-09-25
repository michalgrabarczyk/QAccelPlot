//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/series/PointCloud.hpp"

#include <QGuiApplication>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QThread>
#include <QtTest/QtTest>

#include <memory>

using PlotView = QAccelPlot::QAccelPlot;

class PlotSeriesRegistrationTest : public QObject {
    Q_OBJECT

private slots:
    void seriesConstructedWithPlotParentAreRegistered();
    void seriesConstructedWithPlotParentFollowPlotResize();
    void seriesRemovedBeforeRegistrationAreIgnored();
    void plotRectIsNeverAssignedOnRenderThread();
};

void PlotSeriesRegistrationTest::seriesConstructedWithPlotParentAreRegistered()
{
    auto plot = PlotView{};
    plot.setSize({300, 200});
    auto changed = QSignalSpy{&plot, &PlotView::seriesChanged};

    auto* curve = new QAccelPlot::LineCurve(&plot);
    auto* cloud = new QAccelPlot::PointCloud(&plot);

    QTRY_COMPARE(plot.series().size(), 2);
    QVERIFY(plot.series().contains(curve));
    QVERIFY(plot.series().contains(cloud));
    QCOMPARE(changed.count(), 2);
    QCOMPARE(curve->plotRect(), plot.plotRect());
    QCOMPARE(cloud->plotRect(), plot.plotRect());
}

void PlotSeriesRegistrationTest::seriesConstructedWithPlotParentFollowPlotResize()
{
    auto plot = PlotView{};
    plot.setSize({300, 200});
    auto* curve = new QAccelPlot::LineCurve(&plot);
    QTRY_VERIFY(plot.series().contains(curve));

    plot.setSize({150, 100});

    QCOMPARE(curve->plotRect(), plot.plotRect());
    QCOMPARE(curve->position(), plot.plotRect().topLeft());
    QCOMPARE(curve->size(), plot.plotRect().size());
}

void PlotSeriesRegistrationTest::seriesRemovedBeforeRegistrationAreIgnored()
{
    auto plot = PlotView{};
    auto other = QQuickItem{};
    auto* moved = new QAccelPlot::LineCurve(&plot);
    moved->setParentItem(&other);
    auto* destroyed = new QAccelPlot::LineCurve(&plot);
    delete destroyed;

    QCoreApplication::processEvents();

    QVERIFY(plot.series().isEmpty());
    delete moved;
}

void PlotSeriesRegistrationTest::plotRectIsNeverAssignedOnRenderThread()
{
    auto window = QQuickWindow{};
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom curve materials require a hardware scene graph backend");
    }
    window.resize(300, 200);
    auto xAxis = QAccelPlot::Axis{};
    xAxis.setOrientation(QAccelPlot::Axis::Horizontal);
    auto yAxis = QAccelPlot::Axis{};
    yAxis.setOrientation(QAccelPlot::Axis::Vertical);
    auto plot = std::make_unique<PlotView>(window.contentItem());
    plot->setSize({300, 200});

    auto* curve = new QAccelPlot::LineCurve(plot.get());
    curve->setXAxis(&xAxis);
    curve->setYAxis(&yAxis);
    curve->setData(QList<QPointF>{{0.1, 0.1}, {0.9, 0.9}});
    auto assigningThreads = QList<QThread*>{};
    connect(curve, &QAccelPlot::PlotSeries::plotRectChanged, this, [&]() { assigningThreads.append(QThread::currentThread()); }, Qt::DirectConnection);

    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QTRY_COMPARE(curve->plotRect(), plot->plotRect());

    for (const auto* thread : std::as_const(assigningThreads)) {
        QCOMPARE(thread, qApp->thread());
    }
}

int main(int argc, char* argv[])
{
    // The threaded render loop runs updatePaintNode() off the GUI thread, which this test checks against.
    qputenv("QSG_RENDER_LOOP", "threaded");
    auto app = QGuiApplication{argc, argv};
    auto test = PlotSeriesRegistrationTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_plot_series_registration.moc"
