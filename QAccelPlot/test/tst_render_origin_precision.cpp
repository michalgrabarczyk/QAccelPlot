//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/series/BarSeries.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/series/PointCloud.hpp"
#include "QAccelPlot/series/RectangleSeries.hpp"

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

// Width of the rectangles and bars that take the place of the markers: 4 px.
constexpr auto kMarkWidthMs = 0.4;

// One rectangle at the start of the year and one over each of the ten samples at its end.
std::vector<double> yearOfRectangles()
{
    auto data = std::vector<double>{kYearStart, 0.0, kYearStart + kMarkWidthMs, 1.0};
    for (auto i = 0; i < kMarkerCount; ++i) {
        const auto center = kViewportStart + i + 0.5;
        data.insert(data.end(), {center - kMarkWidthMs / 2, 0.0, center + kMarkWidthMs / 2, 1.0});
    }
    return data;
}

// The samples as bars that reach the top of the plot.
std::vector<double> yearOfBars()
{
    auto data = yearOfSamples();
    for (auto i = std::size_t{1}; i < data.size(); i += 2) {
        data[i] = 1.0;
    }
    return data;
}

std::unique_ptr<Axis> makeAxis(const Qt::Orientation orientation, const qreal min, const qreal max)
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
    void rectangleSeriesKeepsMillisecondPrecision();
    void rectangleSeriesKeepsPrecisionAfterPanning();
    void barSeriesKeepsMillisecondPrecision();
    void barSeriesKeepsPrecisionAfterPanning();

private:
    void verifyMarkersAtEveryMillisecond();
    // Shows the start of the year until \a setData has run, then its end.
    template <typename SetData> void setDataBeforePanning(const SetData& setData);
    void addRectangles(RectangleSeries& rectangles);
    void addBars(BarSeries& bars);

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
    xAxis_ = makeAxis(Qt::Horizontal, kViewportStart, kViewportStart + kMarkerCount);
    yAxis_ = makeAxis(Qt::Vertical, 0.0, 1.0);
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
    curve.setAntialiasing(false);
    curve.setLineStyle(nullptr);
    curve.marker()->setShape(MarkerShape::Square);
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
    curve.setAntialiasing(false);
    curve.setLineStyle(nullptr);
    curve.marker()->setShape(MarkerShape::Square);
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
    cloud.setAntialiasing(false);
    cloud.marker()->setShape(MarkerShape::Square);
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
    cloud.setAntialiasing(false);
    cloud.marker()->setShape(MarkerShape::Square);
    cloud.marker()->setSize(2);
    xAxis_->setViewportMin(kYearStart);
    xAxis_->setViewportMax(kYearStart + kMarkerCount);
    cloud.setData(yearOfSamples(), kMarkerCount + 1);
    xAxis_->setViewportMin(kViewportStart);
    xAxis_->setViewportMax(kViewportStart + kMarkerCount);

    verifyMarkersAtEveryMillisecond();
}

template <typename SetData> void RenderOriginPrecisionTest::setDataBeforePanning(const SetData& setData)
{
    xAxis_->setViewportMin(kYearStart);
    xAxis_->setViewportMax(kYearStart + kMarkerCount);
    setData();
    // The series draws once with the start of the year in view.
    window_->show();
    QVERIFY(QTest::qWaitForWindowExposed(window_.get()));
    QVERIFY(!window_->grabWindow().isNull());
    xAxis_->setViewportMin(kViewportStart);
    xAxis_->setViewportMax(kViewportStart + kMarkerCount);
}

void RenderOriginPrecisionTest::addRectangles(RectangleSeries& rectangles)
{
    rectangles.setParentItem(window_->contentItem());
    rectangles.setXAxis(xAxis_.get());
    rectangles.setYAxis(yAxis_.get());
    rectangles.setPlotRect({0, 0, kWidth, kHeight});
    rectangles.setColor(Qt::white);
}

void RenderOriginPrecisionTest::addBars(BarSeries& bars)
{
    bars.setParentItem(window_->contentItem());
    bars.setXAxis(xAxis_.get());
    bars.setYAxis(yAxis_.get());
    bars.setPlotRect({0, 0, kWidth, kHeight});
    bars.setColor(Qt::white);
    bars.setBarWidth(kMarkWidthMs);
}

void RenderOriginPrecisionTest::rectangleSeriesKeepsMillisecondPrecision()
{
    auto rectangles = RectangleSeries{};
    addRectangles(rectangles);
    rectangles.setData(yearOfRectangles(), kMarkerCount + 1);

    verifyMarkersAtEveryMillisecond();
}

void RenderOriginPrecisionTest::rectangleSeriesKeepsPrecisionAfterPanning()
{
    auto rectangles = RectangleSeries{};
    addRectangles(rectangles);
    setDataBeforePanning([&rectangles] { rectangles.setData(yearOfRectangles(), kMarkerCount + 1); });

    verifyMarkersAtEveryMillisecond();
}

void RenderOriginPrecisionTest::barSeriesKeepsMillisecondPrecision()
{
    auto bars = BarSeries{};
    addBars(bars);
    bars.setData(yearOfBars(), kMarkerCount + 1);

    verifyMarkersAtEveryMillisecond();
}

void RenderOriginPrecisionTest::barSeriesKeepsPrecisionAfterPanning()
{
    auto bars = BarSeries{};
    addBars(bars);
    setDataBeforePanning([&bars] { bars.setData(yearOfBars(), kMarkerCount + 1); });

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
