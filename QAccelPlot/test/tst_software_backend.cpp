//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/PlotView.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/export/PlotExporter.hpp"
#include "QAccelPlot/series/BandSeries.hpp"
#include "QAccelPlot/series/BarSeries.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/series/PointCloud.hpp"
#include "QAccelPlot/series/RectangleSeries.hpp"

#include <QGuiApplication>
#include <QQuickWindow>
#include <QRegularExpression>
#include <QSGRendererInterface>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include <vector>

namespace QAccelPlot {

namespace {

constexpr auto kSize = 100;
constexpr auto kDataUpdates = 3;

} // namespace

// The software scene graph backend cannot run the series' shaders. Every series must skip drawing
// there, once with a warning, and keep working while its data changes. Exporting a plot fails with an error.
class SoftwareBackendTest : public QObject {
    Q_OBJECT

private slots:
    void lineCurveIsSkipped();
    void pointCloudIsSkipped();
    void bandSeriesIsSkipped();
    void rectangleSeriesIsSkipped();
    void barSeriesIsSkipped();
    void exportFails();

private:
    // Renders \a Series after each of several data updates and expects its one warning.
    template <typename Series> void renderAfterDataUpdates(const char* typeName, std::vector<double> data, int recordCount);
};

void SoftwareBackendTest::lineCurveIsSkipped()
{
    renderAfterDataUpdates<LineCurve>("LineCurve", {0.0, 0.0, 1.0, 1.0}, 2);
}

void SoftwareBackendTest::pointCloudIsSkipped()
{
    renderAfterDataUpdates<PointCloud>("PointCloud", {0.5, 0.5}, 1);
}

void SoftwareBackendTest::bandSeriesIsSkipped()
{
    renderAfterDataUpdates<BandSeries>("BandSeries", {0.0, 0.2, 0.8, 1.0, 0.2, 0.8}, 2);
}

void SoftwareBackendTest::rectangleSeriesIsSkipped()
{
    renderAfterDataUpdates<RectangleSeries>("RectangleSeries", {0.1, 0.1, 0.5, 0.5}, 1);
}

void SoftwareBackendTest::barSeriesIsSkipped()
{
    renderAfterDataUpdates<BarSeries>("BarSeries", {0.5, 0.5}, 1);
}

void SoftwareBackendTest::exportFails()
{
    auto window = QQuickWindow{};
    window.resize(kSize, kSize);
    auto plot = PlotView{};
    plot.setSize({kSize, kSize});
    plot.setParentItem(window.contentItem());
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto exporter = PlotExporter{};
    exporter.setPlot(&plot);
    auto finished = QSignalSpy{&exporter, &PlotExporter::finished};
    QTest::ignoreMessage(QtWarningMsg, "PlotExporter: Exporting requires a hardware scene graph backend.");
    QVERIFY(!exporter.grabImage());
    QCOMPARE(finished.count(), 1);
    QCOMPARE(finished.constFirst().at(0).toBool(), false);
}

template <typename Series> void SoftwareBackendTest::renderAfterDataUpdates(const char* typeName, std::vector<double> data, const int recordCount)
{
    auto window = QQuickWindow{};
    QCOMPARE(window.rendererInterface()->graphicsApi(), QSGRendererInterface::Software);
    window.resize(kSize, kSize);
    auto xAxis = Axis{};
    xAxis.setOrientation(Qt::Horizontal);
    auto yAxis = Axis{};
    yAxis.setOrientation(Qt::Vertical);
    auto series = Series{};
    series.setParentItem(window.contentItem());
    series.setXAxis(&xAxis);
    series.setYAxis(&yAxis);
    series.setPlotRect({0, 0, kSize, kSize});

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("%1 custom rendering requires a hardware scene graph backend.*").arg(typeName)));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    for (auto update = 0; update < kDataUpdates; ++update) {
        // Every update after the first reuses the scene graph resources of the previous one.
        static_cast<PlotSeries&>(series).setData(std::vector<double>{data}, recordCount);
        QVERIFY(!window.grabWindow().isNull());
    }
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    qputenv("QT_QUICK_BACKEND", "software");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::SoftwareBackendTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_software_backend.moc"
