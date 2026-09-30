//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/series/BandSeries.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <vector>

namespace QAccelPlot {

namespace {

// With QACCELPLOT_MAX_TEXTURE_SIZE=1 the 8192-float-wide data texture holds 2730 samples.
constexpr auto kCapacity = 2730;
constexpr auto kSampleCount = 4000;
constexpr auto kSize = 100;

} // namespace

class BandSeriesCapacityTest : public QObject {
    Q_OBJECT

private slots:
    void samplesBeyondCapacityAreNotDrawn();
};

void BandSeriesCapacityTest::samplesBeyondCapacityAreNotDrawn()
{
    auto window = QQuickWindow{};
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    window.setColor(Qt::black);
    window.resize(kSize, kSize);
    auto xAxis = Axis{};
    xAxis.setOrientation(Axis::Horizontal);
    auto yAxis = Axis{};
    yAxis.setOrientation(Axis::Vertical);

    // Samples within the capacity cover the left half, the ones beyond it the right half.
    auto data = std::vector<float>{};
    for (auto index = 0; index < kSampleCount; ++index) {
        const auto x = index < kCapacity ? 0.5f * static_cast<float>(index) / (kCapacity - 1)
                                         : 0.5f + 0.5f * static_cast<float>(index - kCapacity) / (kSampleCount - kCapacity - 1);
        data.insert(data.end(), {x, 0.25f, 0.75f});
    }
    auto band = BandSeries{};
    band.setParentItem(window.contentItem());
    band.setXAxis(&xAxis);
    band.setYAxis(&yAxis);
    band.setPlotRect({0, 0, kSize, kSize});
    band.setColor(Qt::white);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("BandSeries has 4000 samples but draws at most 2730.*"));
    band.setDataF(std::move(data), kSampleCount);

    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const auto image = window.grabWindow();
    QVERIFY(qGray(image.pixel(25, 50)) > 200);
    QVERIFY(qGray(image.pixel(75, 50)) < 50);
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    qputenv("QACCELPLOT_MAX_TEXTURE_SIZE", "1");
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::BandSeriesCapacityTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_band_series_capacity.moc"
