//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/export/PlotExporter.hpp"

#include <QClipboard>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickPaintedItem>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrl>
#include <QtTest/QtTest>

#include <cstdlib>
#include <memory>

namespace QAccelPlot {

namespace {

constexpr auto kPlotWidth = 200;
constexpr auto kPlotHeight = 150;

constexpr auto kScene = R"(
import QtQuick
import QAccelPlot

PlotView {
    id: plot
    width: 200
    height: 150
    plotAreaColor: "red"
    axesAreaColor: "blue"
    grid.gridVisible: false
    grid.subGridVisible: false
    border.width: 0
    xAxis: Axis { label: "X"; viewportMin: 0; viewportMax: 10 }
    yAxis: Axis { label: "Y"; viewportMin: 0; viewportMax: 10 }

    RectangleSeries {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "lime"
        Component.onCompleted: setData([{ x1: 6, y1: 6, x2: 9, y2: 9 }])
    }
}
)";

bool isColor(const QColor& actual, const QColor& expected, const int tolerance = 8)
{
    return std::abs(actual.red() - expected.red()) <= tolerance && std::abs(actual.green() - expected.green()) <= tolerance
        && std::abs(actual.blue() - expected.blue()) <= tolerance && std::abs(actual.alpha() - expected.alpha()) <= tolerance;
}

} // namespace

class PlotExporterTest : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void defaultsAndValidation();
    void supportedFormatsIncludeCommonRasterFormats();
    void failsWithoutPlot();
    void failsForUnsupportedFormat();
    void failsForHiddenPlot();
    void failsWhenWindowHidesBeforeCapture();
    void failsWhenWindowIsDestroyedBeforeCapture();
    void grabsAtScaleWithResolution();
    void leavesOutBackgroundsOnlyForTheCapture();
    void rasterizesPaintedItemsAtScaleAndRestoresThem();
    void rejectsRequestWhileBusy();
    void savesPng();
    void savesJpegOverWhite();
    void savesToFileUrl();
    void copiesToClipboard();

private:
    QImage grab(PlotExporter& exporter);
    // Size of an image at the default scale, which follows the window's device pixel ratio.
    QSize screenImageSize() const;
    QPoint screenImageCenter() const;
    // Image pixel inside the lime rectangle at the default scale.
    QPoint screenImageRectangle() const;

    std::unique_ptr<QQuickWindow> window_;
    std::unique_ptr<QQmlEngine> engine_;
    std::unique_ptr<QObject> root_;
    QAccelPlot* plot_{nullptr};
    qreal devicePixelRatio_{1.0};
};

void PlotExporterTest::init()
{
    window_ = std::make_unique<QQuickWindow>();
    if (window_->rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Exporting requires a hardware scene graph backend");
    }
    window_->setColor(Qt::black);
    window_->resize(kPlotWidth, kPlotHeight);
    engine_ = std::make_unique<QQmlEngine>();
    auto component = QQmlComponent{engine_.get()};
    component.setData(kScene, QUrl{});
    root_.reset(component.create());
    QVERIFY2(root_, qPrintable(component.errorString()));
    plot_ = qobject_cast<QAccelPlot*>(root_.get());
    QVERIFY(plot_);
    plot_->setParentItem(window_->contentItem());
    window_->show();
    QVERIFY(QTest::qWaitForWindowExposed(window_.get()));
    devicePixelRatio_ = window_->effectiveDevicePixelRatio();
}

void PlotExporterTest::cleanup()
{
    root_.reset();
    engine_.reset();
    window_.reset();
    plot_ = nullptr;
}

QImage PlotExporterTest::grab(PlotExporter& exporter)
{
    auto images = QSignalSpy{&exporter, &PlotExporter::imageReady};
    auto finished = QSignalSpy{&exporter, &PlotExporter::finished};
    if (!exporter.grabImage()) {
        return {};
    }
    if (!finished.wait() || images.count() != 1 || !finished.constFirst().at(0).toBool()) {
        return {};
    }
    return images.constFirst().at(0).value<QImage>();
}

QSize PlotExporterTest::screenImageSize() const
{
    return QSize(qRound(kPlotWidth * devicePixelRatio_), qRound(kPlotHeight * devicePixelRatio_));
}

QPoint PlotExporterTest::screenImageCenter() const
{
    return (plot_->plotRect().center() * devicePixelRatio_).toPoint();
}

QPoint PlotExporterTest::screenImageRectangle() const
{
    return (QPointF(plot_->dataToPixelX(7.5), plot_->dataToPixelY(7.5)) * devicePixelRatio_).toPoint();
}

void PlotExporterTest::defaultsAndValidation()
{
    auto exporter = PlotExporter{};
    QVERIFY(!exporter.plot());
    QCOMPARE(exporter.scale(), 0.0);
    QCOMPARE(exporter.quality(), -1);
    QCOMPARE(exporter.dpi(), 0.0);
    QVERIFY(!exporter.transparentBackground());
    QVERIFY(!exporter.busy());

    exporter.setScale(3.0);
    exporter.setQuality(90);
    exporter.setDpi(300.0);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("ignoring scale"));
    exporter.setScale(-1.0);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("ignoring quality"));
    exporter.setQuality(101);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("ignoring quality"));
    exporter.setQuality(-2);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("ignoring dpi"));
    exporter.setDpi(qQNaN());
    QCOMPARE(exporter.scale(), 3.0);
    QCOMPARE(exporter.quality(), 90);
    QCOMPARE(exporter.dpi(), 300.0);
}

void PlotExporterTest::supportedFormatsIncludeCommonRasterFormats()
{
    const auto formats = PlotExporter::supportedFormats();
    QVERIFY(formats.contains(QStringLiteral("png")));
    QVERIFY(formats.contains(QStringLiteral("jpg")));
    QVERIFY(formats.contains(QStringLiteral("bmp")));
}

void PlotExporterTest::failsWithoutPlot()
{
    auto exporter = PlotExporter{};
    auto finished = QSignalSpy{&exporter, &PlotExporter::finished};
    QTest::ignoreMessage(QtWarningMsg, "PlotExporter: No plot is set.");
    QVERIFY(!exporter.grabImage());
    QCOMPARE(finished.count(), 1);
    QCOMPARE(finished.constFirst().at(0).toBool(), false);
    QCOMPARE(finished.constFirst().at(2).toString(), QStringLiteral("No plot is set."));
    QVERIFY(!exporter.busy());
}

void PlotExporterTest::failsForUnsupportedFormat()
{
    auto exporter = PlotExporter{};
    exporter.setPlot(plot_);
    auto finished = QSignalSpy{&exporter, &PlotExporter::finished};
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("unsupported image format"));
    QVERIFY(!exporter.save(QStringLiteral("plot.unknown")));
    QCOMPARE(finished.count(), 1);
    QCOMPARE(finished.constFirst().at(0).toBool(), false);
}

void PlotExporterTest::failsForHiddenPlot()
{
    auto exporter = PlotExporter{};
    exporter.setPlot(plot_);
    plot_->setVisible(false);
    QTest::ignoreMessage(QtWarningMsg, "PlotExporter: The plot is not visible.");
    QVERIFY(!exporter.grabImage());
}

void PlotExporterTest::failsWhenWindowHidesBeforeCapture()
{
    auto exporter = PlotExporter{};
    exporter.setPlot(plot_);
    exporter.setTransparentBackground(true);
    auto finished = QSignalSpy{&exporter, &PlotExporter::finished};
    QVERIFY(exporter.grabImage());
    QTest::ignoreMessage(QtWarningMsg, "PlotExporter: The plot's window was hidden before the capture.");
    window_->hide();
    QCOMPARE(finished.count(), 1);
    QCOMPARE(finished.constFirst().at(0).toBool(), false);
    QVERIFY(!exporter.busy());
    QTest::ignoreMessage(QtWarningMsg, "PlotExporter: The plot is not visible.");
    QVERIFY(!exporter.grabImage());

    // Qt 6.2 under xvfb never exposes a window shown again after hide(), so a new window shows the plot.
    auto shownWindow = QQuickWindow{};
    shownWindow.setColor(Qt::black);
    shownWindow.resize(kPlotWidth, kPlotHeight);
    plot_->setParentItem(shownWindow.contentItem());
    shownWindow.show();
    QVERIFY(QTest::qWaitForWindowExposed(&shownWindow));
    QVERIFY(isColor(shownWindow.grabWindow().pixelColor(2, 2), Qt::blue));
    QVERIFY(!grab(exporter).isNull());
    plot_->setParentItem(nullptr);
}

void PlotExporterTest::failsWhenWindowIsDestroyedBeforeCapture()
{
    auto exporter = PlotExporter{};
    exporter.setPlot(plot_);
    auto finished = QSignalSpy{&exporter, &PlotExporter::finished};
    QVERIFY(exporter.grabImage());
    // Destroying the window hides it first on some platforms.
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("PlotExporter: The plot's window was (hidden|destroyed) before the capture."));
    window_.reset();
    QCOMPARE(finished.count(), 1);
    QCOMPARE(finished.constFirst().at(0).toBool(), false);
    QVERIFY(!exporter.busy());
}

void PlotExporterTest::grabsAtScaleWithResolution()
{
    auto exporter = PlotExporter{};
    exporter.setPlot(plot_);
    exporter.setScale(2.0);
    const auto image = grab(exporter);
    QVERIFY(!image.isNull());
    QCOMPARE(image.size(), QSize(2 * kPlotWidth, 2 * kPlotHeight));
    QCOMPARE(image.devicePixelRatio(), 1.0);
    QCOMPARE(image.dotsPerMeterX(), qRound(2.0 * 96.0 / 0.0254));

    const auto center = (plot_->plotRect().center() * 2.0).toPoint();
    QVERIFY(isColor(image.pixelColor(center), Qt::red));
    QVERIFY(isColor(image.pixelColor(2, 2), Qt::blue));

    exporter.setDpi(300.0);
    QCOMPARE(grab(exporter).dotsPerMeterX(), qRound(300.0 / 0.0254));
}

void PlotExporterTest::leavesOutBackgroundsOnlyForTheCapture()
{
    auto exporter = PlotExporter{};
    exporter.setPlot(plot_);
    exporter.setTransparentBackground(true);
    auto axesColorChanges = QSignalSpy{plot_, &QAccelPlot::axesAreaColorChanged};
    auto plotColorChanges = QSignalSpy{plot_, &QAccelPlot::plotAreaColorChanged};
    const auto image = grab(exporter);
    QVERIFY(!image.isNull());
    QCOMPARE(image.pixelColor(2, 2).alpha(), 0);
    QCOMPARE(image.pixelColor(screenImageCenter()).alpha(), 0);
    QVERIFY(isColor(image.pixelColor(screenImageRectangle()), Qt::green));

    QCOMPARE(axesColorChanges.count(), 0);
    QCOMPARE(plotColorChanges.count(), 0);
    const auto screen = window_->grabWindow();
    QVERIFY(isColor(screen.pixelColor(2, 2), Qt::blue));
    QVERIFY(isColor(screen.pixelColor(screenImageCenter()), Qt::red));
}

void PlotExporterTest::rasterizesPaintedItemsAtScaleAndRestoresThem()
{
    auto* axis = plot_->xAxis();
    QVERIFY(axis);
    const auto originalTextureSize = axis->textureSize();

    auto exporter = PlotExporter{};
    exporter.setPlot(plot_);
    exporter.setScale(3.0);
    auto finished = QSignalSpy{&exporter, &PlotExporter::finished};
    QVERIFY(exporter.grabImage());
    // QQuickPaintedItem multiplies textureSize by the device pixel ratio.
    const auto textureScale = 3.0 / devicePixelRatio_;
    QCOMPARE(axis->textureSize(), QSize(qCeil(axis->width() * textureScale), qCeil(axis->height() * textureScale)));
    QVERIFY(finished.wait());
    QCOMPARE(axis->textureSize(), originalTextureSize);
}

void PlotExporterTest::rejectsRequestWhileBusy()
{
    auto exporter = PlotExporter{};
    exporter.setPlot(plot_);
    auto finished = QSignalSpy{&exporter, &PlotExporter::finished};
    QVERIFY(exporter.grabImage());
    QVERIFY(exporter.busy());
    QTest::ignoreMessage(QtWarningMsg, "PlotExporter: Another export is in progress.");
    QVERIFY(!exporter.grabImage());
    QCOMPARE(finished.count(), 1);
    QTRY_COMPARE(finished.count(), 2);
    QCOMPARE(finished.at(1).at(0).toBool(), true);
    QVERIFY(!exporter.busy());
}

void PlotExporterTest::savesPng()
{
    auto directory = QTemporaryDir{};
    QVERIFY(directory.isValid());
    const auto path = directory.filePath(QStringLiteral("plot.png"));

    auto exporter = PlotExporter{};
    exporter.setPlot(plot_);
    exporter.setScale(2.0);
    exporter.setTransparentBackground(true);
    auto finished = QSignalSpy{&exporter, &PlotExporter::finished};
    QVERIFY(exporter.save(path));
    QVERIFY(finished.wait());
    QCOMPARE(finished.constFirst().at(0).toBool(), true);
    QCOMPARE(finished.constFirst().at(1).toString(), path);

    const auto image = QImage(path);
    QCOMPARE(image.size(), QSize(2 * kPlotWidth, 2 * kPlotHeight));
    QCOMPARE(image.pixelColor(2, 2).alpha(), 0);
    QCOMPARE(image.dotsPerMeterX(), qRound(2.0 * 96.0 / 0.0254));
}

void PlotExporterTest::savesJpegOverWhite()
{
    auto directory = QTemporaryDir{};
    QVERIFY(directory.isValid());
    const auto path = directory.filePath(QStringLiteral("plot.jpg"));

    auto exporter = PlotExporter{};
    exporter.setPlot(plot_);
    exporter.setQuality(100);
    exporter.setTransparentBackground(true);
    auto finished = QSignalSpy{&exporter, &PlotExporter::finished};
    QVERIFY(exporter.save(path));
    QVERIFY(finished.wait());
    QCOMPARE(finished.constFirst().at(0).toBool(), true);

    const auto image = QImage(path);
    QCOMPARE(image.size(), screenImageSize());
    QVERIFY(isColor(image.pixelColor(2, 2), Qt::white));
    QVERIFY(isColor(image.pixelColor(screenImageCenter()), Qt::white));
    QVERIFY(isColor(image.pixelColor(screenImageRectangle()), Qt::green));
}

void PlotExporterTest::savesToFileUrl()
{
    auto directory = QTemporaryDir{};
    QVERIFY(directory.isValid());
    const auto path = directory.filePath(QStringLiteral("plot.png"));

    auto exporter = PlotExporter{};
    exporter.setPlot(plot_);
    auto finished = QSignalSpy{&exporter, &PlotExporter::finished};
    QVERIFY(exporter.save(QUrl::fromLocalFile(path).toString()));
    QVERIFY(finished.wait());
    QCOMPARE(finished.constFirst().at(0).toBool(), true);
    QCOMPARE(finished.constFirst().at(1).toString(), path);
    QVERIFY(QFileInfo::exists(path));
}

void PlotExporterTest::copiesToClipboard()
{
    auto exporter = PlotExporter{};
    exporter.setPlot(plot_);
    auto finished = QSignalSpy{&exporter, &PlotExporter::finished};
    QVERIFY(exporter.copyToClipboard());
    QVERIFY(finished.wait());
    QCOMPARE(finished.constFirst().at(0).toBool(), true);
    QCOMPARE(QGuiApplication::clipboard()->image().size(), screenImageSize());
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Render one device pixel per logical pixel, unless a run sets QT_SCALE_FACTOR to test another ratio.
    if (!qEnvironmentVariableIsSet("QT_SCALE_FACTOR")) {
        qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    }
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::PlotExporterTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_plot_exporter.moc"
