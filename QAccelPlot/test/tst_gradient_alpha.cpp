//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/PlotView.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <cstdlib>
#include <memory>

namespace QAccelPlot {

namespace {

constexpr auto kScene = R"(
import QtQuick
import QAccelPlot

PlotView {
    id: plot
    width: 200
    height: 200
    plotAreaColor: "white"
    axesAreaColor: "white"
    grid.gridVisible: false
    grid.subGridVisible: false
    xAxis: Axis { viewportMin: 0; viewportMax: 10 }
    yAxis: Axis { viewportMin: 0; viewportMax: 10 }
    %1
}
)";

constexpr auto kTranslucentRamp = R"(Colormap {
    min: 0
    max: 2
    stops: [
        GradientStop { position: 0; color: "#80ff0000" },
        GradientStop { position: 1; color: "#80ff0000" }
    ]
})";

constexpr auto kFadeRamp = R"(Colormap {
    stops: [
        GradientStop { position: 0; color: "#ffff0000" },
        GradientStop { position: 1; color: "#00ff0000" }
    ]
})";

constexpr auto kFill = R"(LineCurve {
    xAxis: plot.xAxis
    yAxis: plot.yAxis
    effects: [GradientFill { opacity: 1; colormap: %1 }]
    Component.onCompleted: setData([Qt.point(0, 8), Qt.point(10, 8)])
})";

constexpr auto kHorizontalFill = R"(LineCurve {
    xAxis: plot.xAxis
    yAxis: plot.yAxis
    effects: [GradientFill { opacity: 1; direction: Qt.Horizontal; valueMin: 0; valueMax: 10; colormap: %1 }]
    Component.onCompleted: setData([Qt.point(0, 8), Qt.point(10, 8)])
})";

constexpr auto kStroke = R"(LineCurve {
    xAxis: plot.xAxis
    yAxis: plot.yAxis
    lineWidth: 16
    effects: [GradientStroke { colormap: %1 }]
    Component.onCompleted: setData([Qt.point(0, 5), Qt.point(10, 5)])
})";

constexpr auto kMarkers = R"(LineCurve {
    xAxis: plot.xAxis
    yAxis: plot.yAxis
    lineStyle: NoLine {}
    marker.shape: MarkerShape.Square
    marker.size: 24
    effects: [GradientStroke { colormap: %1 }]
    Component.onCompleted: setData([Qt.point(5, 5)])
})";

constexpr auto kPointCloud = R"(PointCloud {
    xAxis: plot.xAxis
    yAxis: plot.yAxis
    marker.shape: MarkerShape.Square
    marker.size: 24
    colormap: %1
    Component.onCompleted: {
        setData([Qt.point(5, 5)]);
        setValues([1]);
    }
})";

constexpr auto kHeatmap = R"(HeatmapSeries {
    xAxis: plot.xAxis
    yAxis: plot.yAxis
    xMin: 0
    xMax: 10
    yMin: 0
    yMax: 10
    colormap: %1
    Component.onCompleted: setValues([1], 1)
})";

constexpr auto kBars = R"(BarSeries {
    xAxis: plot.xAxis
    yAxis: plot.yAxis
    barWidth: 4
    colormap: %1
    Component.onCompleted: setData([{ position: 5, value: 8 }])
})";

QString seriesWithRamp(const char* series, const char* ramp)
{
    return QString::fromUtf8(series).arg(QString::fromUtf8(ramp));
}

bool isNear(const QColor& actual, const QColor& expected)
{
    constexpr auto kTolerance = 3;
    return std::abs(actual.red() - expected.red()) <= kTolerance && std::abs(actual.green() - expected.green()) <= kTolerance
        && std::abs(actual.blue() - expected.blue()) <= kTolerance;
}

} // namespace

class GradientAlphaTest : public QObject {
    Q_OBJECT

private slots:
    void translucentColorKeepsItsHue_data();
    void translucentColorKeepsItsHue();
};

void GradientAlphaTest::translucentColorKeepsItsHue_data()
{
    QTest::addColumn<QString>("series");
    QTest::addColumn<QPointF>("sample");

    QTest::newRow("GradientFill") << seriesWithRamp(kFill, kTranslucentRamp) << QPointF{5.0, 4.0};
    QTest::newRow("GradientFill fade") << seriesWithRamp(kHorizontalFill, kFadeRamp) << QPointF{5.0, 4.0};
    QTest::newRow("GradientStroke") << seriesWithRamp(kStroke, kTranslucentRamp) << QPointF{5.0, 5.0};
    QTest::newRow("GradientStroke markers") << seriesWithRamp(kMarkers, kTranslucentRamp) << QPointF{5.0, 5.0};
    QTest::newRow("PointCloud colormap") << seriesWithRamp(kPointCloud, kTranslucentRamp) << QPointF{5.0, 5.0};
    QTest::newRow("HeatmapSeries colormap") << seriesWithRamp(kHeatmap, kTranslucentRamp) << QPointF{5.0, 5.0};
    QTest::newRow("BarSeries colormap") << seriesWithRamp(kBars, kTranslucentRamp) << QPointF{5.0, 4.0};
}

void GradientAlphaTest::translucentColorKeepsItsHue()
{
    QFETCH(const QString, series);
    QFETCH(const QPointF, sample);

    auto window = QQuickWindow{};
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    window.setColor(Qt::white);
    window.resize(200, 200);

    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(QString::fromUtf8(kScene).arg(series).toUtf8(), QUrl{});
    const auto root = std::unique_ptr<QObject>{component.create()};
    auto* plot = qobject_cast<PlotView*>(root.get());
    QVERIFY2(plot, qPrintable(component.errorString()));
    plot->setParentItem(window.contentItem());

    const auto image = window.grabWindow();
    QVERIFY(!image.isNull());
    const auto pixel = QPoint{qRound(plot->dataToPixelX(sample.x())), qRound(plot->dataToPixelY(sample.y()))};
    const auto actual = image.pixelColor(pixel);
    const auto expected = QColor{255, 127, 127};
    QVERIFY2(isNear(actual, expected),
        qPrintable(QStringLiteral("pixel (%1, %2) is rgb(%3, %4, %5), expected rgb(255, 127, 127)")
                .arg(pixel.x())
                .arg(pixel.y())
                .arg(actual.red())
                .arg(actual.green())
                .arg(actual.blue())));
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Sample positions are logical pixels, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::GradientAlphaTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_gradient_alpha.moc"
