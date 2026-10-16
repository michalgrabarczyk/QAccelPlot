//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/series/HeatmapSeries.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlExpression>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <memory>

namespace QAccelPlot {

namespace {

// A 2 × 2 grid over x 2..8 and y 2..8: row 0 (y 2..5) holds 0 and 2, row 1 (y 5..8) holds NaN
// and 2. The colormap runs from red at 0 to blue at 2.
constexpr auto kScene = R"(
import QtQuick
import QAccelPlot

PlotView {
    id: plot
    width: 400
    height: 300
    plotAreaColor: "black"
    axesAreaColor: "#202020"
    grid.gridVisible: false
    grid.subGridVisible: false

    xAxis: Axis { viewportMin: 0; viewportMax: 10 }
    yAxis: Axis { viewportMin: 0; viewportMax: 10 }

    HeatmapSeries {
        objectName: "heatmap"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        xMin: 2
        xMax: 8
        yMin: 2
        yMax: 8
        colormap: Colormap {
            min: 0
            max: 2
            stops: [
                GradientStop { position: 0; color: "red" },
                GradientStop { position: 1; color: "blue" }
            ]
        }
        Component.onCompleted: setValues([0, 2, NaN, 2], 2)
    }
}
)";

// Shows the PlotView of kScene in a 400 × 300 window.
class SceneWindow {
public:
    SceneWindow()
    {
        window_.setColor(Qt::black);
        window_.resize(400, 300);
        component_.setData(kScene, QUrl{});
        root_.reset(component_.create());
        plot_ = qobject_cast<QAccelPlot*>(root_.get());
        if (plot_) {
            plot_->setParentItem(window_.contentItem());
            window_.show();
        }
    }

    bool isSoftware() const
    {
        return window_.rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
    }

    QString error() const
    {
        return component_.errorString();
    }

    QQuickWindow& window()
    {
        return window_;
    }

    QAccelPlot* plot() const
    {
        return plot_;
    }

    HeatmapSeries* heatmap() const
    {
        return plot_->findChild<HeatmapSeries*>(QStringLiteral("heatmap"));
    }

    QImage grab()
    {
        return window_.grabWindow();
    }

    QPoint pixel(const qreal x, const qreal y) const
    {
        return {qRound(plot_->dataToPixelX(x)), qRound(plot_->dataToPixelY(y))};
    }

private:
    QQuickWindow window_;
    QQmlEngine engine_;
    QQmlComponent component_{&engine_};
    std::unique_ptr<QObject> root_;
    QAccelPlot* plot_{nullptr};
};

bool isColor(const QColor& pixel, const QColor& expected)
{
    constexpr auto kTolerance = 60;
    return std::abs(pixel.red() - expected.red()) < kTolerance && std::abs(pixel.green() - expected.green()) < kTolerance
        && std::abs(pixel.blue() - expected.blue()) < kTolerance;
}

} // namespace

class HeatmapSeriesRenderingTest : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cellsTakeTheirColormapColor();
    void cellsFollowTheViewport();
    void colormapChangesRecolorTheCells();
    void logNormLeavesNonPositiveCellsTransparent();
    void newValuesAndExtentRedrawTheGrid();
    void logAxisDrawsNothing();
    void qmlReadsCellRecords();

private:
    std::unique_ptr<SceneWindow> scene_;
};

void HeatmapSeriesRenderingTest::init()
{
    scene_ = std::make_unique<SceneWindow>();
    if (scene_->isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene_->plot(), qPrintable(scene_->error()));
    QVERIFY(scene_->heatmap());
    QVERIFY(QTest::qWaitForWindowExposed(&scene_->window()));
}

void HeatmapSeriesRenderingTest::cellsTakeTheirColormapColor()
{
    const auto image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(3.5, 3.5)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(6.5, 3.5)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(6.5, 6.5)), Qt::blue));
    // The NaN cell is transparent, and nothing is drawn outside the grid.
    QVERIFY(isColor(image.pixelColor(scene_->pixel(3.5, 6.5)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(1.5, 3.5)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(8.5, 6.5)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(3.5, 1.5)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(6.5, 8.5)), Qt::black));
    // Cell edges fall where the extent puts them.
    QVERIFY(isColor(image.pixelColor(scene_->pixel(4.9, 2.1)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(5.1, 2.1)), Qt::blue));
}

void HeatmapSeriesRenderingTest::cellsFollowTheViewport()
{
    auto* plot = scene_->plot();
    plot->xAxis()->setViewportMin(3.0);
    plot->xAxis()->setViewportMax(7.0);
    plot->yAxis()->setViewportMin(-2.0);
    plot->yAxis()->setViewportMax(4.0);
    const auto image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(4.5, 3.0)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(5.5, 3.0)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(4.5, 1.0)), Qt::black));
}

void HeatmapSeriesRenderingTest::colormapChangesRecolorTheCells()
{
    scene_->heatmap()->colormap()->setReversed(true);
    auto image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(3.5, 3.5)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(6.5, 3.5)), Qt::red));

    // A wider range moves the value 2 to the middle of the ramp.
    scene_->heatmap()->colormap()->setReversed(false);
    scene_->heatmap()->colormap()->setMax(4.0);
    image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(6.5, 3.5)), QColor{128, 0, 128}));
}

void HeatmapSeriesRenderingTest::logNormLeavesNonPositiveCellsTransparent()
{
    auto* colormap = scene_->heatmap()->colormap();
    colormap->setRange(1.0, 2.0);
    colormap->setNorm(Colormap::Normalization::Log);
    const auto image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(3.5, 3.5)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(6.5, 3.5)), Qt::blue));
}

void HeatmapSeriesRenderingTest::newValuesAndExtentRedrawTheGrid()
{
    auto* heatmap = scene_->heatmap();
    // One row of three cells over x 1..4 and y 0..1.
    heatmap->setValues({2.0f, 0.0f, 2.0f}, 3, 1);
    heatmap->setXMin(1.0);
    heatmap->setXMax(4.0);
    heatmap->setYMin(0.0);
    heatmap->setYMax(1.0);
    const auto image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(1.5, 0.5)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(2.5, 0.5)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(3.5, 0.5)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(3.5, 3.5)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(4.5, 0.5)), Qt::black));
}

void HeatmapSeriesRenderingTest::logAxisDrawsNothing()
{
    auto* plot = scene_->plot();
    plot->xAxis()->setViewportMin(1.0);
    QTest::ignoreMessage(QtWarningMsg, "HeatmapSeries does not support logarithmic axes and draws nothing on them.");
    plot->xAxis()->setLogScale(true);
    auto image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(6.5, 3.5)), Qt::black));

    plot->xAxis()->setLogScale(false);
    plot->xAxis()->setViewportMin(0.0);
    plot->xAxis()->setViewportMax(10.0);
    image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(6.5, 3.5)), Qt::blue));
}

void HeatmapSeriesRenderingTest::qmlReadsCellRecords()
{
    auto* heatmap = scene_->heatmap();
    // The series fills the plot area, so its local pixels are the plot's minus the plot area origin.
    const auto origin = scene_->plot()->plotRect().topLeft();
    const auto position = QPointF{scene_->pixel(6.5, 3.5)} - origin;
    auto expression = QQmlExpression{
        qmlContext(heatmap), heatmap, QStringLiteral("inspection.recordAtPosition(Qt.point(%1, %2)).fields.value").arg(position.x()).arg(position.y())};
    QCOMPARE(expression.evaluate().toDouble(), 2.0);
    QVERIFY(!expression.hasError());
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Pixel lookups use logical coordinates, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::HeatmapSeriesRenderingTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_heatmap_series_rendering.moc"
