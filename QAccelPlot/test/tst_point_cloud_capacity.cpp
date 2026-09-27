//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/series/PointCloud.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <vector>

namespace QAccelPlot {

namespace {

// With QACCELPLOT_MAX_TEXTURE_SIZE=1 the 8192-float-wide data texture holds 4096 XY points.
constexpr auto kCapacity = 4096;
constexpr auto kPointCount = 6000;
constexpr auto kSize = 100;

} // namespace

class PointCloudCapacityTest : public QObject {
    Q_OBJECT

private slots:
    void pointsBeyondCapacityAreNotHovered();
};

void PointCloudCapacityTest::pointsBeyondCapacityAreNotHovered()
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

    // Points within the capacity sit at the left, the ones beyond it at the right.
    auto data = std::vector<float>{};
    for (auto index = 0; index < kPointCount; ++index) {
        data.push_back(index < kCapacity ? 0.25f : 0.75f);
        data.push_back(0.5f);
    }
    auto cloud = PointCloud{};
    cloud.setParentItem(window.contentItem());
    cloud.setXAxis(&xAxis);
    cloud.setYAxis(&yAxis);
    cloud.setPlotRect({0, 0, kSize, kSize});
    cloud.setColor(Qt::white);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("PointCloud has 6000 points but the GPU data texture holds at most 4096.*"));
    cloud.setDataF(std::move(data), kPointCount);

    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const auto image = window.grabWindow();
    QVERIFY(qGray(image.pixel(25, 50)) > 200);
    QVERIFY(qGray(image.pixel(75, 50)) < 50);

    const auto drawn = cloud.pointIndexAt({25, 50});
    QVERIFY(drawn >= 0 && drawn < kCapacity);
    QCOMPARE(cloud.pointIndexAt({75, 50}), -1);
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    qputenv("QACCELPLOT_MAX_TEXTURE_SIZE", "1");
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::PointCloudCapacityTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_point_cloud_capacity.moc"
