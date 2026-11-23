//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include <QColor>
#include <QGuiApplication>
#include <QImage>
#include <QPoint>
#include <QPointF>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <cmath>
#include <cstdlib>
#include <memory>

namespace {

constexpr auto kSize = 300;
constexpr auto kValueMin = 1.0;
constexpr auto kValueMax = 1000.0;
constexpr auto kNarrowMin = 1000000.0;
constexpr auto kNarrowMax = 1000020.0;
constexpr auto kColorTolerance = 6;

struct Scene {
    bool fill{true};
    bool logScale{true};
    bool vertical{true};
    QPointF from;
    QPointF to;
    qreal baseline{kValueMin};
    qreal valueMin{kValueMin};
    qreal valueMax{kValueMax};
};

QString qmlNumber(const qreal value)
{
    return QString::number(value, 'g', 17);
}

QByteArray sceneQml(const Scene& scene)
{
    const auto gradientAxis = QStringLiteral("logScale: %1; viewportMin: %2; viewportMax: %3")
                                  .arg(scene.logScale ? QStringLiteral("true") : QStringLiteral("false"))
                                  .arg(qmlNumber(scene.valueMin))
                                  .arg(qmlNumber(scene.valueMax));
    const auto otherAxis = QStringLiteral("viewportMin: 0; viewportMax: 1");
    const auto effect
        = scene.fill ? QStringLiteral("GradientFill { opacity: 1; baselineValue: %1; ").arg(qmlNumber(scene.baseline)) : QStringLiteral("GradientStroke { ");
    return QStringLiteral(R"(
import QtQuick
import QAccelPlot
LineCurve {
    width: %1; height: %1
    antialiasing: false
    lineWidth: %2
    xAxis: Axis { orientation: Qt.Horizontal; %3 }
    yAxis: Axis { orientation: Qt.Vertical; %4 }
    effects: %5
        direction: %6
        valueMin: %7
        valueMax: %8
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#ff0000" }
            GradientStop { position: 1.0; color: "#0000ff" }
        }
    }
    Component.onCompleted: setData([Qt.point(%9, %10), Qt.point(%11, %12)])
}
)")
        .arg(kSize)
        .arg(scene.fill ? 1 : 20)
        .arg(scene.vertical ? otherAxis : gradientAxis)
        .arg(scene.vertical ? gradientAxis : otherAxis)
        .arg(effect)
        .arg(scene.vertical ? QStringLiteral("Qt.Vertical") : QStringLiteral("Qt.Horizontal"))
        .arg(qmlNumber(scene.valueMin))
        .arg(qmlNumber(scene.valueMax))
        .arg(qmlNumber(scene.from.x()))
        .arg(qmlNumber(scene.from.y()))
        .arg(qmlNumber(scene.to.x()))
        .arg(qmlNumber(scene.to.y()))
        .toUtf8();
}

// Returns the palette position the gradient assigns to the data value at the center of \a pixel.
qreal expectedPosition(const Scene& scene, const QPoint& pixel)
{
    const auto fraction = scene.vertical ? 1.0 - (pixel.y() + 0.5) / kSize : (pixel.x() + 0.5) / kSize;
    const auto range = scene.valueMax - scene.valueMin;
    const auto value = scene.logScale ? scene.valueMin * std::pow(scene.valueMax / scene.valueMin, fraction) : scene.valueMin + range * fraction;
    const auto normalized = (value - scene.valueMin) / range;
    return scene.vertical ? 1.0 - normalized : normalized;
}

bool colorMatches(const QColor& actual, const qreal position)
{
    const auto red = static_cast<int>(std::lround((1.0 - position) * 255.0));
    const auto blue = static_cast<int>(std::lround(position * 255.0));
    return std::abs(actual.red() - red) <= kColorTolerance && std::abs(actual.blue() - blue) <= kColorTolerance;
}

} // namespace

Q_DECLARE_METATYPE(Scene)

class GradientLogInterpolationTest : public QObject {
    Q_OBJECT

private slots:
    void colorFollowsDataValueBetweenVertices_data();
    void colorFollowsDataValueBetweenVertices();
};

void GradientLogInterpolationTest::colorFollowsDataValueBetweenVertices_data()
{
    QTest::addColumn<Scene>("scene");
    QTest::addColumn<QList<QPoint>>("probes");

    const auto verticalFill = QList<QPoint>{{295, 200}, {295, 100}, {295, 30}};
    const auto verticalStroke = QList<QPoint>{{150, 250}, {150, 200}, {150, 100}, {150, 30}};
    const auto horizontal = QList<QPoint>{{30, 150}, {100, 150}, {200, 150}, {270, 150}};

    // Row 200 is the height of y = 10, so both columns must show the same color.
    QTest::newRow("fill, log Y") << Scene{true, true, true, {0.0, 10.0}, {1.0, 1000.0}} << (QList<QPoint>{{5, 200}} + verticalFill);
    QTest::newRow("stroke, log Y") << Scene{false, true, true, {0.5, 1.0}, {0.5, 1000.0}} << verticalStroke;
    QTest::newRow("fill, log X") << Scene{true, true, false, {1.0, 1.0}, {1000.0, 1.0}, 0.0} << horizontal;
    QTest::newRow("stroke, log X") << Scene{false, true, false, {1.0, 0.5}, {1000.0, 0.5}} << horizontal;
    // A non-positive baseline has no log value; the fill takes its color from the height it is drawn at.
    QTest::newRow("fill, log Y, non-positive baseline") << Scene{true, true, true, {0.0, 10.0}, {1.0, 10.0}, 0.0} << QList<QPoint>{{150, 100}, {150, 30}};
    // Values this close together differ by less than the float resolution of their log10.
    const auto narrowProbes = QList<QPoint>{{150, 250}, {150, 150}, {150, 50}};
    QTest::newRow("fill, log Y, narrow range") << Scene{true, true, true, {0.0, kNarrowMax}, {1.0, kNarrowMax}, kNarrowMin, kNarrowMin, kNarrowMax}
                                               << narrowProbes;
    QTest::newRow("stroke, log Y, narrow range") << Scene{false, true, true, {0.5, kNarrowMin}, {0.5, kNarrowMax}, kNarrowMin, kNarrowMin, kNarrowMax}
                                                 << narrowProbes;
    QTest::newRow("fill, linear Y") << Scene{true, false, true, {0.0, 10.0}, {1.0, 1000.0}} << verticalFill;
    QTest::newRow("stroke, linear Y") << Scene{false, false, true, {0.5, 1.0}, {0.5, 1000.0}} << verticalStroke;
    QTest::newRow("fill, linear X") << Scene{true, false, false, {1.0, 1.0}, {1000.0, 1.0}, 0.0} << horizontal;
    QTest::newRow("stroke, linear X") << Scene{false, false, false, {1.0, 0.5}, {1000.0, 0.5}} << horizontal;
}

void GradientLogInterpolationTest::colorFollowsDataValueBetweenVertices()
{
    QFETCH(Scene, scene);
    QFETCH(QList<QPoint>, probes);

    auto window = QQuickWindow{};
    window.setColor(Qt::black);
    window.resize(kSize, kSize);

    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData(sceneQml(scene), QUrl{});
    const auto root = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(root, qPrintable(component.errorString()));
    qobject_cast<QQuickItem*>(root.get())->setParentItem(window.contentItem());

    const auto image = window.grabWindow();
    QVERIFY(!image.isNull());
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom materials do not render with the software scene graph backend");
    }

    for (const auto& probe : probes) {
        const auto actual = image.pixelColor(probe);
        const auto position = expectedPosition(scene, probe);
        QVERIFY2(colorMatches(actual, position),
            qPrintable(QStringLiteral("pixel (%1, %2) is rgb(%3, %4, %5), expected palette position %6 = rgb(%7, 0, %8)")
                    .arg(probe.x())
                    .arg(probe.y())
                    .arg(actual.red())
                    .arg(actual.green())
                    .arg(actual.blue())
                    .arg(position, 0, 'f', 3)
                    .arg(std::lround((1.0 - position) * 255.0))
                    .arg(std::lround(position * 255.0))));
    }
}

int main(int argc, char* argv[])
{
    // Probes are logical pixels, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = GradientLogInterpolationTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_gradient_log_interpolation.moc"
