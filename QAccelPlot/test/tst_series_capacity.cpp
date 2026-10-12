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
#include "QAccelPlot/series/RectangleSeries.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQuickWindow>
#include <QRegularExpression>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <memory>
#include <vector>

namespace QAccelPlot {

namespace {

constexpr auto kSize = 100;
// With QACCELPLOT_MAX_TEXTURE_SIZE=1 the data texture holds 8192 floats.
constexpr auto kTextureFloats = 8192;

// A 100 px window whose series map the data range [0, 1] onto it.
struct Scene {
    QQuickWindow window;
    Axis xAxis;
    Axis yAxis;

    Scene()
    {
        window.setColor(Qt::black);
        window.resize(kSize, kSize);
        xAxis.setOrientation(Qt::Horizontal);
        yAxis.setOrientation(Qt::Vertical);
    }

    bool isSoftware() const
    {
        return window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
    }

    void add(PlotSeries& series)
    {
        series.setParentItem(window.contentItem());
        series.setXAxis(&xAxis);
        series.setYAxis(&yAxis);
        series.setPlotRect({0, 0, kSize, kSize});
    }
};

bool isLit(const QImage& image, const int x, const int y)
{
    return qGray(image.pixel(x, y)) > 200;
}

bool isDark(const QImage& image, const int x, const int y)
{
    return qGray(image.pixel(x, y)) < 50;
}

} // namespace

// A series draws the records its GPU buffers hold and warns about the rest.
class SeriesCapacityTest : public QObject {
    Q_OBJECT

private slots:
    void linePointsBeyondCapacityAreNotDrawn();
    void rectanglesBeyondCapacityAreNotDrawn();
    void barsBeyondCapacityAreNotDrawn();
};

void SeriesCapacityTest::linePointsBeyondCapacityAreNotDrawn()
{
    auto scene = Scene{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    constexpr auto kCapacity = kTextureFloats / 2;
    constexpr auto kPointCount = 6000;

    // Points within the capacity run along the upper line, the ones beyond it along the lower line.
    auto data = std::vector<float>{};
    for (auto index = 0; index < kPointCount; ++index) {
        const auto upper = index < kCapacity;
        const auto first = upper ? 0 : kCapacity;
        const auto last = upper ? kCapacity - 1 : kPointCount - 1;
        data.push_back(static_cast<float>(index - first) / static_cast<float>(last - first));
        data.push_back(upper ? 0.75f : 0.25f);
    }
    auto curve = LineCurve{};
    scene.add(curve);
    curve.setColor(Qt::white);
    curve.setLineWidth(6.0);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("LineCurve has 6000 points but draws at most 4096.*"));
    curve.setDataF(std::move(data), kPointCount);

    scene.window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window));
    const auto image = scene.window.grabWindow();
    QVERIFY(isLit(image, 50, 25));
    QVERIFY(isDark(image, 50, 75));
}

void SeriesCapacityTest::rectanglesBeyondCapacityAreNotDrawn()
{
    auto scene = Scene{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    constexpr auto kCapacity = kTextureFloats / 4;
    constexpr auto kRectangleCount = 3000;

    // Rectangles within the capacity cover the left, the ones beyond it the right.
    auto data = std::vector<float>{};
    for (auto index = 0; index < kRectangleCount; ++index) {
        const auto left = index < kCapacity ? 0.1f : 0.6f;
        data.insert(data.end(), {left, 0.1f, left + 0.3f, 0.9f});
    }
    auto rectangles = RectangleSeries{};
    scene.add(rectangles);
    rectangles.setColor(Qt::white);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("RectangleSeries has 3000 rectangles but draws at most 2048.*"));
    rectangles.setDataF(std::move(data), kRectangleCount);

    scene.window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window));
    const auto image = scene.window.grabWindow();
    QVERIFY(isLit(image, 25, 50));
    QVERIFY(isDark(image, 75, 50));
}

void SeriesCapacityTest::barsBeyondCapacityAreNotDrawn()
{
    auto scene = Scene{};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    constexpr auto kCapacity = kTextureFloats / 2;
    constexpr auto kBarCount = 6000;

    // Bars within the capacity stand at the left, the ones beyond it at the right.
    auto data = std::vector<float>{};
    for (auto index = 0; index < kBarCount; ++index) {
        data.insert(data.end(), {index < kCapacity ? 0.25f : 0.75f, 0.8f});
    }
    auto bars = BarSeries{};
    scene.add(bars);
    bars.setColor(Qt::white);
    bars.setBarWidth(0.3);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BarSeries has 6000 bars but draws at most 4096.*"));
    bars.setDataF(std::move(data), kBarCount);

    scene.window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window));
    const auto image = scene.window.grabWindow();
    QVERIFY(isLit(image, 25, 50));
    QVERIFY(isDark(image, 75, 50));
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    qputenv("QACCELPLOT_MAX_TEXTURE_SIZE", "1");
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::SeriesCapacityTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_series_capacity.moc"
