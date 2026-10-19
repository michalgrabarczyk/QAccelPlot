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

#include <atomic>
#include <limits>
#include <memory>
#include <vector>

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

// Columns and visible cells of highCellIndicesStayExactInMixedMappings().
constexpr auto kPrecisionColumns = 1 << 20;
constexpr auto kPrecisionVisibleCells = 32;

bool isColor(const QColor& pixel, const QColor& expected)
{
    constexpr auto kTolerance = 60;
    return std::abs(pixel.red() - expected.red()) < kTolerance && std::abs(pixel.green() - expected.green()) < kTolerance
        && std::abs(pixel.blue() - expected.blue()) < kTolerance;
}

// Counts the warnings about too many columns or rows while it is alive and passes other messages on.
class PrecisionWarningCounter {
public:
    PrecisionWarningCounter()
    {
        count_ = 0;
        previousHandler_ = qInstallMessageHandler(handle);
    }

    ~PrecisionWarningCounter()
    {
        qInstallMessageHandler(previousHandler_);
    }

    PrecisionWarningCounter(const PrecisionWarningCounter&) = delete;
    PrecisionWarningCounter& operator=(const PrecisionWarningCounter&) = delete;

    int count() const
    {
        return count_;
    }

private:
    static void handle(const QtMsgType type, const QMessageLogContext& context, const QString& message)
    {
        if (type == QtWarningMsg && message.contains(QStringLiteral("columns or rows are not drawn"))) {
            ++count_;
            return;
        }
        previousHandler_(type, context, message);
    }

    static inline std::atomic<int> count_{0};
    static inline QtMessageHandler previousHandler_{nullptr};
};

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
    void linearCellsOnLogAxes();
    void linearCellsClipAtZeroOnLogAxis();
    void logCellsOnLinearAndLogAxes();
    void logSpacingWithNonPositiveEdgeDrawsNothing();
    void highCellIndicesStayExactInMixedMappings();
    void infiniteEdgeDrawsNothing();
    void tooManyColumnsDrawNothing();
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

void HeatmapSeriesRenderingTest::linearCellsOnLogAxes()
{
    auto* plot = scene_->plot();
    for (auto* axis : {plot->xAxis(), plot->yAxis()}) {
        axis->setViewportMin(1.0);
        axis->setLogScale(true);
    }
    // Column and row edges 2, 5, 8.
    const auto image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(4.5, 3.5)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(4.9, 4.9)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(5.2, 3.5)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(4.5, 5.2)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(7.5, 7.5)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(1.5, 3.5)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(9.0, 3.5)), Qt::black));
}

void HeatmapSeriesRenderingTest::linearCellsClipAtZeroOnLogAxis()
{
    // Column edges -4, 2, 8: column 0 shows from the axis start up to 2.
    scene_->heatmap()->setXMin(-4.0);
    auto* axis = scene_->plot()->xAxis();
    axis->setViewportMin(0.1);
    axis->setLogScale(true);
    const auto image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(0.12, 3.5)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(1.8, 3.5)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(2.5, 3.5)), Qt::blue));
}

void HeatmapSeriesRenderingTest::logCellsOnLinearAndLogAxes()
{
    auto* heatmap = scene_->heatmap();
    heatmap->setXSpacing(HeatmapSeries::Spacing::Log);
    heatmap->setYSpacing(HeatmapSeries::Spacing::Log);
    // Column and row edges 1, 10, 100.
    heatmap->setXMin(1.0);
    heatmap->setXMax(100.0);
    heatmap->setYMin(1.0);
    heatmap->setYMax(100.0);
    auto* plot = scene_->plot();
    plot->xAxis()->setViewportMax(200.0);
    plot->yAxis()->setViewportMax(200.0);
    auto image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(5.0, 5.0)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(13.0, 5.0)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(5.0, 13.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(50.0, 50.0)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(170.0, 5.0)), Qt::black));

    for (auto* axis : {plot->xAxis(), plot->yAxis()}) {
        axis->setViewportMin(0.5);
        axis->setLogScale(true);
    }
    image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(9.0, 5.0)), Qt::red));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(11.0, 5.0)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(5.0, 11.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(50.0, 50.0)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(0.8, 5.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(170.0, 5.0)), Qt::black));
}

void HeatmapSeriesRenderingTest::logSpacingWithNonPositiveEdgeDrawsNothing()
{
    auto* heatmap = scene_->heatmap();
    heatmap->setXSpacing(HeatmapSeries::Spacing::Log);
    auto image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(3.5, 3.5)), Qt::red));

    QTest::ignoreMessage(QtWarningMsg, "HeatmapSeries with Log spacing needs positive grid edges in that dimension; the heatmap is not drawn.");
    heatmap->setXMin(0.0);
    image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(3.5, 3.5)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(6.5, 3.5)), Qt::black));

    heatmap->setXMin(2.0);
    image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(3.5, 3.5)), Qt::red));
}

void HeatmapSeriesRenderingTest::highCellIndicesStayExactInMixedMappings()
{
    auto values = std::vector<float>(kPrecisionColumns);
    for (auto i = 0; i < kPrecisionColumns; ++i) {
        values[static_cast<std::size_t>(i)] = i % 2 == 0 ? 0.0f : 2.0f;
    }
    auto* heatmap = scene_->heatmap();
    heatmap->setValues(std::move(values), kPrecisionColumns, 1);
    heatmap->setYMin(0.0);
    heatmap->setYMax(10.0);
    auto* axis = scene_->plot()->xAxis();
    const auto checkCells = [this](const int first, const auto& center) {
        const auto image = scene_->grab();
        for (auto cell = first; cell < first + kPrecisionVisibleCells; ++cell) {
            const auto expected = cell % 2 == 0 ? QColor{Qt::red} : QColor{Qt::blue};
            QVERIFY2(isColor(image.pixelColor(scene_->pixel(center(cell), 5.0)), expected), qPrintable(QStringLiteral("cell %1").arg(cell)));
        }
    };

    // Linear cells [i, i + 1] on a log axis.
    constexpr auto kLinearFirst = 1'000'000;
    heatmap->setXMin(0.0);
    heatmap->setXMax(kPrecisionColumns);
    axis->setViewportMin(kLinearFirst);
    axis->setViewportMax(kLinearFirst + kPrecisionVisibleCells);
    axis->setLogScale(true);
    checkCells(kLinearFirst, [](const int cell) { return cell + 0.5; });
    if (QTest::currentTestFailed()) {
        return;
    }

    // Log cells over six decades on a linear axis, near 900000.
    const auto edge = [](const double cell) { return std::pow(10.0, 6.0 * cell / kPrecisionColumns); };
    const auto logFirst = static_cast<int>(std::ceil(std::log10(900'000.0) / 6.0 * kPrecisionColumns));
    heatmap->setXMin(1.0);
    heatmap->setXMax(1'000'000.0);
    heatmap->setXSpacing(HeatmapSeries::Spacing::Log);
    axis->setLogScale(false);
    axis->setViewportMin(edge(logFirst));
    axis->setViewportMax(edge(logFirst + kPrecisionVisibleCells));
    checkCells(logFirst, [&edge](const int cell) { return edge(cell + 0.5); });
}

void HeatmapSeriesRenderingTest::infiniteEdgeDrawsNothing()
{
    auto* heatmap = scene_->heatmap();
    heatmap->setXMax(std::numeric_limits<qreal>::infinity());
    auto image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(3.5, 3.5)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(9.5, 3.5)), Qt::black));

    heatmap->setXMax(8.0);
    heatmap->setXMin(-std::numeric_limits<qreal>::infinity());
    image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(3.5, 3.5)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene_->pixel(6.5, 3.5)), Qt::black));

    heatmap->setXMin(2.0);
    image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(3.5, 3.5)), Qt::red));
}

void HeatmapSeriesRenderingTest::tooManyColumnsDrawNothing()
{
    constexpr auto kColumns = (1 << 24) + 1;
    auto* heatmap = scene_->heatmap();
    const auto warnings = PrecisionWarningCounter{};

    heatmap->setValues(std::vector<float>(kColumns, 2.0f), kColumns, 1);
    auto image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(6.5, 3.5)), Qt::black));
    QCOMPARE(warnings.count(), 1);
    // Further frames do not warn again.
    heatmap->setXMax(9.0);
    image = scene_->grab();
    QCOMPARE(warnings.count(), 1);

    heatmap->setValues({0.0f, 2.0f}, 2, 1);
    image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(3.5, 3.5)), Qt::red));

    heatmap->setValues(std::vector<float>(kColumns, 2.0f), kColumns, 1);
    image = scene_->grab();
    QVERIFY(isColor(image.pixelColor(scene_->pixel(3.5, 3.5)), Qt::black));
    QCOMPARE(warnings.count(), 2);
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
