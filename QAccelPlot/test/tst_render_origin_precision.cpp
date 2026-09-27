//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/series/PointCloud.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <memory>
#include <vector>

namespace QAccelPlot {

namespace {

// A year of epoch-millisecond timestamps: the first sample starts the year, and ten samples
// 1 ms apart sit at its end. The viewport shows only those ten, 10 px apart.
constexpr auto kYearStart = 1'700'000'000'000.0;
constexpr auto kYearMs = 365.0 * 24 * 3600 * 1000;
constexpr auto kViewportStart = kYearStart + kYearMs;
constexpr auto kMarkerCount = 10;
constexpr auto kWidth = 100;
constexpr auto kHeight = 20;

std::vector<double> yearOfSamples()
{
    auto data = std::vector<double>{kYearStart, 0.5};
    for (auto i = 0; i < kMarkerCount; ++i) {
        data.push_back(kViewportStart + i + 0.5);
        data.push_back(0.5);
    }
    return data;
}

std::unique_ptr<Axis> makeAxis(const Axis::Orientation orientation, const qreal min, const qreal max)
{
    auto axis = std::make_unique<Axis>();
    axis->setOrientation(orientation);
    axis->setViewportMin(min);
    axis->setViewportMax(max);
    return axis;
}

} // namespace

class RenderOriginPrecisionTest : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void lineCurveMarkersKeepMillisecondPrecision();
    void lineCurveKeepsPrecisionAfterPanning();
    void pointCloudKeepsMillisecondPrecision();
    void pointCloudKeepsPrecisionAfterPanning();

private:
    void verifyMarkersAtEveryMillisecond();

    std::unique_ptr<QQuickWindow> window_;
    std::unique_ptr<Axis> xAxis_;
    std::unique_ptr<Axis> yAxis_;
};

void RenderOriginPrecisionTest::init()
{
    window_ = std::make_unique<QQuickWindow>();
    if (window_->rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    window_->setColor(Qt::black);
    window_->resize(kWidth, kHeight);
    xAxis_ = makeAxis(Axis::Horizontal, kViewportStart, kViewportStart + kMarkerCount);
    yAxis_ = makeAxis(Axis::Vertical, 0.0, 1.0);
}

void RenderOriginPrecisionTest::cleanup()
{
    window_.reset();
    xAxis_.reset();
    yAxis_.reset();
}

void RenderOriginPrecisionTest::verifyMarkersAtEveryMillisecond()
{
    window_->show();
    QVERIFY(QTest::qWaitForWindowExposed(window_.get()));
    const auto image = window_->grabWindow();
    for (auto i = 0; i < kMarkerCount; ++i) {
        const auto marker = QPoint{i * 10 + 5, kHeight / 2};
        const auto gap = QPoint{i * 10, kHeight / 2};
        QVERIFY2(qGray(image.pixel(marker)) > 200, qPrintable(QStringLiteral("no marker at x = %1").arg(marker.x())));
        QVERIFY2(qGray(image.pixel(gap)) < 50, qPrintable(QStringLiteral("unexpected marker at x = %1").arg(gap.x())));
    }
}

void RenderOriginPrecisionTest::lineCurveMarkersKeepMillisecondPrecision()
{
    auto curve = LineCurve{};
    curve.setParentItem(window_->contentItem());
    curve.setXAxis(xAxis_.get());
    curve.setYAxis(yAxis_.get());
    curve.setPlotRect({0, 0, kWidth, kHeight});
    curve.setColor(Qt::white);
    curve.setAntialiasingEnabled(false);
    curve.setLineStyle(nullptr);
    curve.marker()->setShape(LineCurve::MarkerShape::Square);
    curve.marker()->setSize(2);
    curve.setData(yearOfSamples(), kMarkerCount + 1);

    verifyMarkersAtEveryMillisecond();
}

void RenderOriginPrecisionTest::lineCurveKeepsPrecisionAfterPanning()
{
    auto curve = LineCurve{};
    curve.setParentItem(window_->contentItem());
    curve.setXAxis(xAxis_.get());
    curve.setYAxis(yAxis_.get());
    curve.setPlotRect({0, 0, kWidth, kHeight});
    curve.setColor(Qt::white);
    curve.setAntialiasingEnabled(false);
    curve.setLineStyle(nullptr);
    curve.marker()->setShape(LineCurve::MarkerShape::Square);
    curve.marker()->setSize(2);
    // Set while the start of the year is in view, then pan to its end.
    xAxis_->setViewportMin(kYearStart);
    xAxis_->setViewportMax(kYearStart + kMarkerCount);
    curve.setData(yearOfSamples(), kMarkerCount + 1);
    xAxis_->setViewportMin(kViewportStart);
    xAxis_->setViewportMax(kViewportStart + kMarkerCount);

    verifyMarkersAtEveryMillisecond();
}

void RenderOriginPrecisionTest::pointCloudKeepsMillisecondPrecision()
{
    auto cloud = PointCloud{};
    cloud.setParentItem(window_->contentItem());
    cloud.setXAxis(xAxis_.get());
    cloud.setYAxis(yAxis_.get());
    cloud.setPlotRect({0, 0, kWidth, kHeight});
    cloud.setColor(Qt::white);
    cloud.setAntialiasingEnabled(false);
    cloud.marker()->setShape(PointCloud::MarkerShape::Square);
    cloud.marker()->setSize(2);
    cloud.setData(yearOfSamples(), kMarkerCount + 1);

    verifyMarkersAtEveryMillisecond();
}

void RenderOriginPrecisionTest::pointCloudKeepsPrecisionAfterPanning()
{
    auto cloud = PointCloud{};
    cloud.setParentItem(window_->contentItem());
    cloud.setXAxis(xAxis_.get());
    cloud.setYAxis(yAxis_.get());
    cloud.setPlotRect({0, 0, kWidth, kHeight});
    cloud.setColor(Qt::white);
    cloud.setAntialiasingEnabled(false);
    cloud.marker()->setShape(PointCloud::MarkerShape::Square);
    cloud.marker()->setSize(2);
    xAxis_->setViewportMin(kYearStart);
    xAxis_->setViewportMax(kYearStart + kMarkerCount);
    cloud.setData(yearOfSamples(), kMarkerCount + 1);
    xAxis_->setViewportMin(kViewportStart);
    xAxis_->setViewportMax(kViewportStart + kMarkerCount);

    verifyMarkersAtEveryMillisecond();
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Pixel lookups use logical coordinates, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::RenderOriginPrecisionTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_render_origin_precision.moc"
