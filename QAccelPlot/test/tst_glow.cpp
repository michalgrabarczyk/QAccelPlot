//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/Glow.hpp"
#include "QAccelPlot/series/BarSeries.hpp"
#include "QAccelPlot/series/ErrorBarSeries.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/series/PointCloud.hpp"

#include <QGuiApplication>
#include <QQuickWindow>
#include <QtTest/QtTest>

#include <memory>

namespace QAccelPlot {

namespace {

constexpr auto kSize = 200;
constexpr auto kDataSpan = 10.0;

std::unique_ptr<Axis> makeAxis(const Qt::Orientation orientation)
{
    auto axis = std::make_unique<Axis>();
    axis->setOrientation(orientation);
    axis->setViewportMin(0.0);
    axis->setViewportMax(kDataSpan);
    return axis;
}

} // namespace

class GlowTest : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void haloSurroundsTheSeries_data();
    void haloSurroundsTheSeries();
    void haloTakesItsOwnColor();
    void disabledGlowDrawsNoHalo();
    void propertiesAreClamped();

private:
    // Returns the color at the pixel of data position (x, y), moved by offset pixels.
    QColor pixelAt(qreal x, qreal y, QPoint offset = {}) const;
    void place(PlotSeries& series) const;

    std::unique_ptr<QQuickWindow> window_;
    std::unique_ptr<Axis> xAxis_;
    std::unique_ptr<Axis> yAxis_;
};

void GlowTest::init()
{
    window_ = std::make_unique<QQuickWindow>();
    if (window_->rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    window_->setColor(Qt::black);
    window_->resize(kSize, kSize);
    xAxis_ = makeAxis(Qt::Horizontal);
    yAxis_ = makeAxis(Qt::Vertical);
    window_->show();
    QVERIFY(QTest::qWaitForWindowExposed(window_.get()));
}

void GlowTest::cleanup()
{
    window_.reset();
}

void GlowTest::haloSurroundsTheSeries_data()
{
    QTest::addColumn<int>("kind");
    QTest::addColumn<QPoint>("offset");

    // A bar at x = 5, 20 pixels wide; a marker of radius 4; a vertical error bar; a horizontal line.
    QTest::newRow("bar") << 0 << QPoint{14, 0};
    QTest::newRow("point") << 1 << QPoint{8, 0};
    QTest::newRow("error bar") << 2 << QPoint{5, 0};
    QTest::newRow("line") << 3 << QPoint{0, 5};
}

void GlowTest::haloSurroundsTheSeries()
{
    QFETCH(int, kind);
    QFETCH(QPoint, offset);
    auto series = std::unique_ptr<PlotSeries>{};
    if (kind == 0) {
        auto bars = std::make_unique<BarSeries>();
        bars->setColor(Qt::red);
        bars->setBarWidth(1.0);
        bars->setData(std::vector<double>{5.0, 8.0}, 1);
        series = std::move(bars);
    } else if (kind == 1) {
        auto cloud = std::make_unique<PointCloud>();
        cloud->setColor(Qt::red);
        cloud->marker()->setSize(4.0);
        cloud->setData(std::vector<double>{5.0, 5.0}, 1);
        series = std::move(cloud);
    } else if (kind == 2) {
        auto errorBars = std::make_unique<ErrorBarSeries>();
        errorBars->setColor(Qt::red);
        errorBars->setCapLength(0.0);
        errorBars->setData(std::vector<double>{5.0, 5.0, 2.0, 8.0}, 1);
        series = std::move(errorBars);
    } else {
        auto curve = std::make_unique<LineCurve>();
        curve->setColor(Qt::red);
        curve->setLineWidth(2.0);
        curve->setData(std::vector<double>{0.0, 5.0, 10.0, 5.0}, 2);
        series = std::move(curve);
    }
    place(*series);
    QTRY_VERIFY(pixelAt(5.0, 5.0).red() > 150);
    QCOMPARE(pixelAt(5.0, 5.0, offset), QColor{Qt::black});

    auto glow = Glow{};
    glow.setIntensity(1.0);
    auto effects = series->effects();
    effects.append(&effects, &glow);

    QTRY_VERIFY(pixelAt(5.0, 5.0, offset).red() > 20);
    QCOMPARE(pixelAt(5.0, 5.0, offset).green(), 0);
    QCOMPARE(pixelAt(5.0, 5.0, offset + offset + offset).red(), 0);
}

void GlowTest::haloTakesItsOwnColor()
{
    auto cloud = PointCloud{};
    cloud.setColor(Qt::red);
    cloud.marker()->setSize(4.0);
    cloud.setData(std::vector<double>{5.0, 5.0}, 1);
    place(cloud);
    auto glow = Glow{};
    glow.setColor(Qt::green);
    glow.setIntensity(1.0);
    auto effects = cloud.effects();
    effects.append(&effects, &glow);

    QTRY_VERIFY(pixelAt(5.0, 5.0, {8, 0}).green() > 20);
    QCOMPARE(pixelAt(5.0, 5.0, {8, 0}).red(), 0);
}

void GlowTest::disabledGlowDrawsNoHalo()
{
    auto bars = BarSeries{};
    bars.setColor(Qt::red);
    bars.setBarWidth(1.0);
    bars.setData(std::vector<double>{5.0, 8.0}, 1);
    place(bars);
    auto glow = Glow{};
    glow.setIntensity(1.0);
    auto effects = bars.effects();
    effects.append(&effects, &glow);
    QTRY_VERIFY(pixelAt(5.0, 5.0, {14, 0}).red() > 20);

    glow.setEnabled(false);

    QTRY_COMPARE(pixelAt(5.0, 5.0, {14, 0}), QColor{Qt::black});
}

void GlowTest::propertiesAreClamped()
{
    auto glow = Glow{};
    QVERIFY(!glow.color().isValid());
    QCOMPARE(glow.radius(), 8.0);
    QCOMPARE(glow.intensity(), 0.5);

    glow.setRadius(-2.0);
    glow.setIntensity(3.0);

    QCOMPARE(glow.radius(), 0.0);
    QCOMPARE(glow.intensity(), 1.0);
}

QColor GlowTest::pixelAt(const qreal x, const qreal y, const QPoint offset) const
{
    const auto image = window_->grabWindow();
    const auto pixel = QPoint{qRound(x / kDataSpan * kSize), qRound((1.0 - y / kDataSpan) * kSize)} + offset;
    return image.pixelColor(pixel);
}

void GlowTest::place(PlotSeries& series) const
{
    series.setParentItem(window_->contentItem());
    series.setXAxis(xAxis_.get());
    series.setYAxis(yAxis_.get());
    series.setPlotRect({0, 0, kSize, kSize});
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Pixel lookups use logical coordinates, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::GlowTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_glow.moc"
