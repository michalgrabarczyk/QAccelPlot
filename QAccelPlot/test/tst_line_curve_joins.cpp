//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/series/LineCurve.hpp"

#include <QGuiApplication>
#include <QQuickWindow>
#include <QtTest/QtTest>

#include <algorithm>
#include <cmath>
#include <memory>

namespace QAccelPlot {

namespace {

constexpr auto kWidth = 400;
constexpr auto kHeight = 100;
constexpr auto kSamples = 100;
constexpr auto kYSpan = 10.0;

std::unique_ptr<Axis> makeAxis(const Qt::Orientation orientation, const qreal max)
{
    auto axis = std::make_unique<Axis>();
    axis->setOrientation(orientation);
    axis->setViewportMin(0.0);
    axis->setViewportMax(max);
    return axis;
}

// Y values at x = 0, 1, ...: a zig-zag between 1 and 9, or uniform noise in [1, 9] from a fixed seed.
std::vector<double> samples(const bool noise)
{
    auto values = std::vector<double>{};
    auto state = 12345U;
    for (auto i = 0; i < kSamples; ++i) {
        state = state * 1664525U + 1013904223U;
        const auto random = static_cast<double>(state >> 8U) / static_cast<double>(1U << 24U);
        values.push_back(noise ? 1.0 + 8.0 * random : (i % 2 == 0 ? 1.0 : 9.0));
    }
    return values;
}

// Gives curve the samples (i, values[i]).
void setSamples(LineCurve& curve, const std::vector<double>& values)
{
    auto data = std::vector<double>{};
    for (auto i = std::size_t{0}; i < values.size(); ++i) {
        data.push_back(static_cast<double>(i));
        data.push_back(values[i]);
    }
    curve.setData(std::move(data), static_cast<int>(values.size()));
}

} // namespace

class LineCurveJoinsTest : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void steepSegmentsKeepTheirWidth_data();
    void steepSegmentsKeepTheirWidth();
    void sharpJoinsDoNotSpike();

private:
    // Returns the pixel of data position (x, y).
    static QPointF pixelOf(qreal x, qreal y);
    // Returns the brightest red within 2 pixels of (x, y) along row y of image.
    static int redNear(const QImage& image, QPointF pixel);
    void place(LineCurve& curve) const;

    std::unique_ptr<QQuickWindow> window_;
    std::unique_ptr<Axis> xAxis_;
    std::unique_ptr<Axis> yAxis_;
};

void LineCurveJoinsTest::init()
{
    window_ = std::make_unique<QQuickWindow>();
    if (window_->rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    window_->setColor(Qt::black);
    window_->resize(kWidth, kHeight);
    xAxis_ = makeAxis(Qt::Horizontal, kSamples - 1);
    yAxis_ = makeAxis(Qt::Vertical, kYSpan);
    window_->show();
    QVERIFY(QTest::qWaitForWindowExposed(window_.get()));
}

void LineCurveJoinsTest::cleanup()
{
    window_.reset();
}

void LineCurveJoinsTest::steepSegmentsKeepTheirWidth_data()
{
    QTest::addColumn<bool>("noise");
    QTest::addColumn<qreal>("lineWidth");
    QTest::addColumn<bool>("antialiasing");

    QTest::newRow("zig-zag") << false << 1.0 << true;
    QTest::newRow("noise") << true << 1.0 << true;
    QTest::newRow("zig-zag aliased") << false << 1.0 << false;
    QTest::newRow("zig-zag wide") << false << 3.0 << true;
}

void LineCurveJoinsTest::steepSegmentsKeepTheirWidth()
{
    QFETCH(bool, noise);
    QFETCH(qreal, lineWidth);
    QFETCH(bool, antialiasing);

    // Samples 4 px apart that swing tens of pixels turn sharply at almost every join.
    auto curve = LineCurve{};
    curve.setColor(Qt::red);
    curve.setLineWidth(lineWidth);
    curve.setAntialiasing(antialiasing);
    const auto values = samples(noise);
    setSamples(curve, values);
    place(curve);
    QTRY_VERIFY(redNear(window_->grabWindow(), pixelOf(0.5, (values[0] + values[1]) / 2.0)) > 100);

    const auto image = window_->grabWindow();
    for (auto i = 0; i + 1 < kSamples; ++i) {
        const auto from = pixelOf(i, values[i]);
        const auto to = pixelOf(i + 1, values[i + 1]);
        if (std::abs(to.y() - from.y()) < 20.0) {
            continue;
        }
        // The middle of every steep segment is drawn, not only the antialiased ends.
        for (const auto t : {0.3, 0.4, 0.5, 0.6, 0.7}) {
            const auto pixel = from + (to - from) * t;
            const auto red = redNear(image, pixel);
            QVERIFY2(red > 100, qPrintable(QStringLiteral("segment %1 is missing at (%2, %3): red %4").arg(i).arg(pixel.x()).arg(pixel.y()).arg(red)));
        }
    }
}

void LineCurveJoinsTest::sharpJoinsDoNotSpike()
{
    auto curve = LineCurve{};
    curve.setColor(Qt::red);
    curve.setLineWidth(3.0);
    const auto values = samples(false);
    setSamples(curve, values);
    place(curve);
    QTRY_VERIFY(redNear(window_->grabWindow(), pixelOf(0.5, 5.0)) > 100);

    // The tips stay within the line width and antialiasing of the samples at y = 1 and y = 9.
    const auto image = window_->grabWindow();
    for (const auto y : {qRound(pixelOf(0.0, 9.0).y()) - 4, qRound(pixelOf(0.0, 1.0).y()) + 4}) {
        for (auto x = 0; x < kWidth; ++x) {
            QVERIFY2(image.pixelColor(x, y).red() < 30, qPrintable(QStringLiteral("spike at (%1, %2)").arg(x).arg(y)));
        }
    }
}

QPointF LineCurveJoinsTest::pixelOf(const qreal x, const qreal y)
{
    return {x / (kSamples - 1) * kWidth, (1.0 - y / kYSpan) * kHeight};
}

int LineCurveJoinsTest::redNear(const QImage& image, const QPointF pixel)
{
    const auto row = std::clamp(static_cast<int>(std::floor(pixel.y())), 0, image.height() - 1);
    const auto column = static_cast<int>(std::floor(pixel.x()));
    auto red = 0;
    for (auto x = std::max(column - 2, 0); x <= std::min(column + 2, image.width() - 1); ++x) {
        red = std::max(red, image.pixelColor(x, row).red());
    }
    return red;
}

void LineCurveJoinsTest::place(LineCurve& curve) const
{
    curve.setParentItem(window_->contentItem());
    curve.setXAxis(xAxis_.get());
    curve.setYAxis(yAxis_.get());
    curve.setPlotRect({0, 0, kWidth, kHeight});
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Pixel lookups use logical coordinates, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::LineCurveJoinsTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_line_curve_joins.moc"
