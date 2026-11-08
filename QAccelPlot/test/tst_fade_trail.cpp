//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/FadeTrail.hpp"
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

void appendEffect(PlotSeries& series, SeriesEffect* effect)
{
    auto list = series.effects();
    list.append(&list, effect);
}

} // namespace

class FadeTrailTest : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void oldestPointsFade();
    void lineFadesTowardItsStart();
    void shortTrailLeavesNewerPoints();
    void propertiesAreClamped();

private:
    // Returns the red value at the pixel of data position (x, y).
    int redAt(qreal x, qreal y) const;
    void place(PlotSeries& series) const;

    std::unique_ptr<QQuickWindow> window_;
    std::unique_ptr<Axis> xAxis_;
    std::unique_ptr<Axis> yAxis_;
};

void FadeTrailTest::init()
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

void FadeTrailTest::cleanup()
{
    window_.reset();
}

void FadeTrailTest::oldestPointsFade()
{
    auto cloud = PointCloud{};
    cloud.setColor(Qt::red);
    cloud.marker()->setSize(5.0);
    cloud.setData(std::vector<double>{1.0, 5.0, 5.0, 5.0, 9.0, 5.0}, 3);
    place(cloud);
    QTRY_COMPARE(redAt(1.0, 5.0), 255);
    auto trail = FadeTrail{};
    trail.setLength(1.0);

    appendEffect(cloud, &trail);

    QTRY_COMPARE(redAt(1.0, 5.0), 0);
    QVERIFY(qAbs(redAt(5.0, 5.0) - 128) < 20);
    QCOMPARE(redAt(9.0, 5.0), 255);
}

void FadeTrailTest::lineFadesTowardItsStart()
{
    auto curve = LineCurve{};
    curve.setColor(Qt::red);
    curve.setLineWidth(4.0);
    curve.setData(std::vector<double>{0.0, 5.0, 10.0, 5.0}, 2);
    place(curve);
    auto trail = FadeTrail{};
    trail.setLength(1.0);
    trail.setMinOpacity(0.2);

    appendEffect(curve, &trail);

    QTRY_VERIFY(redAt(1.0, 5.0) < 100);
    QVERIFY(redAt(9.0, 5.0) > redAt(5.0, 5.0));
    QVERIFY(redAt(5.0, 5.0) > redAt(1.0, 5.0));
}

void FadeTrailTest::shortTrailLeavesNewerPoints()
{
    auto cloud = PointCloud{};
    cloud.setColor(Qt::red);
    cloud.marker()->setSize(5.0);
    cloud.setData(std::vector<double>{1.0, 5.0, 5.0, 5.0, 9.0, 5.0}, 3);
    place(cloud);
    auto trail = FadeTrail{};
    trail.setLength(0.5);

    appendEffect(cloud, &trail);

    // Only the first of three points fades: the fading part ends at the second.
    QTRY_COMPARE(redAt(1.0, 5.0), 0);
    QCOMPARE(redAt(5.0, 5.0), 255);
}

void FadeTrailTest::propertiesAreClamped()
{
    auto trail = FadeTrail{};
    QCOMPARE(trail.length(), 0.5);
    QCOMPARE(trail.minOpacity(), 0.0);

    trail.setLength(2.0);
    trail.setMinOpacity(-1.0);

    QCOMPARE(trail.length(), 1.0);
    QCOMPARE(trail.minOpacity(), 0.0);
}

int FadeTrailTest::redAt(const qreal x, const qreal y) const
{
    const auto image = window_->grabWindow();
    return image.pixelColor(QPoint{qRound(x / kDataSpan * kSize), qRound((1.0 - y / kDataSpan) * kSize)}).red();
}

void FadeTrailTest::place(PlotSeries& series) const
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
    auto test = QAccelPlot::FadeTrailTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_fade_trail.moc"
