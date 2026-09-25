//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/linestyles/NoLine.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/series/PointCloud.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QRegularExpression>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <memory>
#include <vector>

namespace QAccelPlot {

namespace {

using Shape = PlotSeries::MarkerShape;

// A single white marker is drawn on black at the centre of a kCurveSize px curve, i.e. at pixel
// corner (kCentre, kCentre). Offsets below address the pixel (kCentre + dx, kCentre + dy) in the
// y-down item space, and are chosen at least 1 px away from every shape edge.
constexpr auto kCurveSize = 64;
constexpr auto kCentre = 32;
constexpr auto kMarkerSize = 20.0;

struct MarkerStyle {
    Shape shape;
    qreal size{kMarkerSize};
    bool filled{true};
    qreal strokeWidth{1.0};
};

struct RenderResult {
    QImage image;
    bool softwareBackend{false};
};

RenderResult renderMarker(const MarkerStyle& style)
{
    auto window = QQuickWindow{};
    window.setColor(Qt::black);
    window.resize(kCurveSize, kCurveSize);

    auto xAxis = Axis{};
    auto yAxis = Axis{};
    xAxis.setOrientation(Axis::Horizontal);
    yAxis.setOrientation(Axis::Vertical);
    for (auto* axis : {&xAxis, &yAxis}) {
        axis->setViewportMin(0.0);
        axis->setViewportMax(1.0);
    }

    auto curve = LineCurve{window.contentItem()};
    curve.setSize(QSizeF{kCurveSize, kCurveSize});
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);
    curve.setLineStyle(new NoLine{&curve});
    curve.setColor(Qt::white);
    curve.marker()->setShape(style.shape);
    curve.marker()->setSize(style.size);
    curve.marker()->setFilled(style.filled);
    curve.marker()->setStrokeWidth(style.strokeWidth);
    // Hard edges make every sampled pixel either fully inside or fully outside.
    curve.setAntialiasingEnabled(false);
    curve.setData(std::vector<double>{0.5}, std::vector<double>{0.5});

    window.show();
    if (!QTest::qWaitForWindowExposed(&window)) {
        return {};
    }
    auto result = RenderResult{};
    result.image = window.grabWindow();
    result.softwareBackend = window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
    return result;
}

RenderResult renderPointCloudMarker(const MarkerStyle& style)
{
    auto window = QQuickWindow{};
    window.setColor(Qt::black);
    window.resize(kCurveSize, kCurveSize);

    auto xAxis = Axis{};
    auto yAxis = Axis{};
    xAxis.setOrientation(Axis::Horizontal);
    yAxis.setOrientation(Axis::Vertical);
    for (auto* axis : {&xAxis, &yAxis}) {
        axis->setViewportMin(0.0);
        axis->setViewportMax(1.0);
    }

    auto cloud = PointCloud{window.contentItem()};
    cloud.setSize(QSizeF{kCurveSize, kCurveSize});
    cloud.setXAxis(&xAxis);
    cloud.setYAxis(&yAxis);
    cloud.setColor(Qt::white);
    cloud.marker()->setShape(style.shape);
    cloud.marker()->setSize(style.size);
    cloud.marker()->setFilled(style.filled);
    cloud.marker()->setStrokeWidth(style.strokeWidth);
    cloud.setAntialiasingEnabled(false);
    cloud.setDataF(std::vector<float>{0.5f, 0.5f}, 1);

    window.show();
    if (!QTest::qWaitForWindowExposed(&window)) {
        return {};
    }
    auto result = RenderResult{};
    result.image = window.grabWindow();
    result.softwareBackend = window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
    return result;
}

bool isLit(const QImage& image, const QPoint& offset)
{
    return qGray(image.pixel(kCentre + offset.x(), kCentre + offset.y())) > 127;
}

// Returns the legend symbol item drawn for \a series, or nullptr.
QQuickItem* findLegendSymbol(QQuickItem* item, const QObject* series)
{
    if (item->property("sourceSeries").value<QObject*>() == series) {
        return item;
    }
    for (auto* child : item->childItems()) {
        if (auto* symbol = findLegendSymbol(child, series)) {
            return symbol;
        }
    }
    return nullptr;
}

} // namespace

class MarkerShapesTest : public QObject {
    Q_OBJECT

private slots:
    void shapeIndicesMatchShaders();
    void lineCurveMarkerDefaultsAndNotifies();
    void lineCurveMarkerGroupBindsFromQml();
    void legendSymbolFollowsMarkerSettings();
    void pointCloudRejectsNoneShape();
    void shapesCoverTheirOutline_data();
    void shapesCoverTheirOutline();
    void hollowMarkersDrawOnlyTheOutline_data();
    void hollowMarkersDrawOnlyTheOutline();
    void pointCloudMarkersMatchLineCurve_data();
    void pointCloudMarkersMatchLineCurve();
    void pixelMarkerCoversOnePixel();
    void pixelMarkerHitTestIgnoresMarkerSize();
    void gradientStrokeColorsMarkersBetweenStops();
};

void MarkerShapesTest::shapeIndicesMatchShaders()
{
    // MarkerShape values are public API, and point.frag and point.vert select shapes by value - 1.
    // Append new shapes at the end; never reorder or insert.
    const auto expected = std::vector<std::pair<Shape, int>>{{Shape::None, 0}, {Shape::Circle, 1}, {Shape::Square, 2}, {Shape::Diamond, 3},
        {Shape::TriangleUp, 4}, {Shape::TriangleDown, 5}, {Shape::TriangleLeft, 6}, {Shape::TriangleRight, 7}, {Shape::Cross, 8}, {Shape::XCross, 9},
        {Shape::HLine, 10}, {Shape::VLine, 11}, {Shape::Star, 12}, {Shape::Asterisk, 13}, {Shape::Pixel, 14}, {Shape::Hexagon, 15}, {Shape::Pentagon, 16}};
    QCOMPARE(QMetaEnum::fromType<Shape>().keyCount(), static_cast<int>(expected.size()));
    for (const auto& [shape, value] : expected) {
        QCOMPARE(static_cast<int>(shape), value);
    }

    // The enum lives on PlotSeries, but both series must still expose it through their own
    // meta-object: that is how QML resolves LineCurve.Circle and PointCloud.Circle.
    for (const auto* meta : {&LineCurve::staticMetaObject, &PointCloud::staticMetaObject}) {
        const auto index = meta->indexOfEnumerator("MarkerShape");
        QVERIFY(index >= 0);
        QCOMPARE(meta->enumerator(index).keyToValue("Pentagon"), static_cast<int>(Shape::Pentagon));
    }
}

void MarkerShapesTest::lineCurveMarkerDefaultsAndNotifies()
{
    auto curve = LineCurve{};
    auto* marker = curve.marker();
    QVERIFY(marker);
    QCOMPARE(marker->parent(), &curve);
    QCOMPARE(marker->shape(), Shape::None);
    QCOMPARE(marker->size(), 4.0);
    QVERIFY(marker->filled());
    QCOMPARE(marker->strokeWidth(), 1.0);

    auto shapeSpy = QSignalSpy{marker, &SeriesMarker::shapeChanged};
    auto sizeSpy = QSignalSpy{marker, &SeriesMarker::sizeChanged};
    auto filledSpy = QSignalSpy{marker, &SeriesMarker::filledChanged};
    auto strokeSpy = QSignalSpy{marker, &SeriesMarker::strokeWidthChanged};
    for (auto i = 0; i < 2; ++i) {
        marker->setShape(Shape::Star);
        marker->setSize(9.0);
        marker->setFilled(false);
        marker->setStrokeWidth(2.5);
    }

    QCOMPARE(marker->shape(), Shape::Star);
    QCOMPARE(marker->size(), 9.0);
    QVERIFY(!marker->filled());
    QCOMPARE(marker->strokeWidth(), 2.5);
    QCOMPARE(shapeSpy.count(), 1);
    QCOMPARE(sizeSpy.count(), 1);
    QCOMPARE(filledSpy.count(), 1);
    QCOMPARE(strokeSpy.count(), 1);

    marker->setShape(Shape::None);
    QCOMPARE(marker->shape(), Shape::None);
    marker->setSize(-3.0);
    QCOMPARE(marker->size(), 0.0);
    marker->setStrokeWidth(-2.0);
    QCOMPARE(marker->strokeWidth(), 0.0);
}

void MarkerShapesTest::lineCurveMarkerGroupBindsFromQml()
{
    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    component.setData("import QtQuick\n"
                      "import QAccelPlot\n"
                      "LineCurve {\n"
                      "    marker.shape: LineCurve.Diamond\n"
                      "    marker.size: 7\n"
                      "    marker.filled: false\n"
                      "    marker.strokeWidth: 2\n"
                      "}\n",
        QUrl{});
    const auto root = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(root, qPrintable(component.errorString()));

    const auto* curve = qobject_cast<LineCurve*>(root.get());
    QVERIFY(curve);
    QCOMPARE(curve->marker()->shape(), Shape::Diamond);
    QCOMPARE(curve->marker()->size(), 7.0);
    QVERIFY(!curve->marker()->filled());
    QCOMPARE(curve->marker()->strokeWidth(), 2.0);
}

void MarkerShapesTest::legendSymbolFollowsMarkerSettings()
{
    auto engine = QQmlEngine{};
    auto warnings = QList<QQmlError>{};
    connect(&engine, &QQmlEngine::warnings, this, [&warnings](const QList<QQmlError>& errors) { warnings.append(errors); });
    auto component = QQmlComponent{&engine};
    component.setData("import QtQuick\n"
                      "import QAccelPlot\n"
                      "Item {\n"
                      "    property alias curve: curve\n"
                      "    property alias cloud: cloud\n"
                      "    LineCurve {\n"
                      "        id: curve\n"
                      "        marker.shape: LineCurve.Hexagon\n"
                      "        marker.size: 6\n"
                      "        marker.filled: false\n"
                      "        marker.strokeWidth: 2\n"
                      "    }\n"
                      "    PointCloud {\n"
                      "        id: cloud\n"
                      "        marker.shape: PointCloud.Square\n"
                      "        marker.size: 5\n"
                      "        marker.filled: false\n"
                      "        marker.strokeWidth: 1.5\n"
                      "    }\n"
                      "    Legend { series: [curve, cloud] }\n"
                      "}\n",
        QUrl{});
    const auto root = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(root, qPrintable(component.errorString()));
    for (const auto& warning : std::as_const(warnings)) {
        QFAIL(qPrintable(warning.toString()));
    }

    auto* rootItem = qobject_cast<QQuickItem*>(root.get());
    auto* curve = root->property("curve").value<LineCurve*>();
    const auto* cloud = root->property("cloud").value<PointCloud*>();
    QVERIFY(rootItem && curve && cloud);

    const auto* curveSymbol = findLegendSymbol(rootItem, curve);
    QVERIFY(curveSymbol);
    QCOMPARE(curveSymbol->property("curveMarker").toInt(), static_cast<int>(Shape::Hexagon));
    QCOMPARE(curveSymbol->property("curveMarkerSize").toReal(), 6.0);
    QVERIFY(!curveSymbol->property("curveMarkerFilled").toBool());
    QCOMPARE(curveSymbol->property("curveMarkerStrokeWidth").toReal(), 2.0);

    curve->marker()->setSize(8.0);
    QCOMPARE(curveSymbol->property("curveMarkerSize").toReal(), 8.0);

    const auto* cloudSymbol = findLegendSymbol(rootItem, cloud);
    QVERIFY(cloudSymbol);
    QCOMPARE(cloudSymbol->property("curveMarker").toInt(), static_cast<int>(Shape::Square));
    QCOMPARE(cloudSymbol->property("curveMarkerSize").toReal(), 5.0);
    QVERIFY(!cloudSymbol->property("curveMarkerFilled").toBool());
    QCOMPARE(cloudSymbol->property("curveMarkerStrokeWidth").toReal(), 1.5);
}

void MarkerShapesTest::pointCloudRejectsNoneShape()
{
    auto cloud = PointCloud{};
    cloud.marker()->setShape(Shape::Square);
    auto spy = QSignalSpy{cloud.marker(), &SeriesMarker::shapeChanged};

    // A cloud always draws its points, so None must not silently blank it.
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("PointCloud marker\\.shape does not accept None.*"));
    cloud.marker()->setShape(Shape::None);

    QCOMPARE(cloud.marker()->shape(), Shape::Square);
    QCOMPARE(spy.count(), 0);
}

void MarkerShapesTest::shapesCoverTheirOutline_data()
{
    QTest::addColumn<Shape>("shape");
    QTest::addColumn<QList<QPoint>>("inside");
    QTest::addColumn<QList<QPoint>>("outside");

    // Offsets for a marker of radius 20 px.
    QTest::newRow("Circle") << Shape::Circle << QList<QPoint>{{0, 0}, {14, 0}} << QList<QPoint>{{15, 15}, {-19, -19}};
    QTest::newRow("Square") << Shape::Square << QList<QPoint>{{18, 18}, {-19, -19}} << QList<QPoint>{{20, 0}, {0, 21}};
    QTest::newRow("Diamond") << Shape::Diamond << QList<QPoint>{{0, -17}, {14, 0}} << QList<QPoint>{{10, -10}, {-18, 0}};
    QTest::newRow("TriangleUp") << Shape::TriangleUp << QList<QPoint>{{0, -16}, {-17, 12}} << QList<QPoint>{{-12, -12}, {0, 17}};
    QTest::newRow("TriangleDown") << Shape::TriangleDown << QList<QPoint>{{0, 15}, {-17, -13}} << QList<QPoint>{{-12, 11}, {0, -18}};
    QTest::newRow("TriangleLeft") << Shape::TriangleLeft << QList<QPoint>{{-16, 0}, {12, -17}} << QList<QPoint>{{-12, -12}, {17, 0}};
    QTest::newRow("TriangleRight") << Shape::TriangleRight << QList<QPoint>{{15, 0}, {-13, -17}} << QList<QPoint>{{11, -12}, {-18, 0}};
    QTest::newRow("Cross") << Shape::Cross << QList<QPoint>{{18, 0}, {0, -18}} << QList<QPoint>{{12, 12}, {14, -14}};
    QTest::newRow("XCross") << Shape::XCross << QList<QPoint>{{13, 13}, {-13, -14}} << QList<QPoint>{{18, 0}, {0, -18}};
    QTest::newRow("Star") << Shape::Star << QList<QPoint>{{0, -15}, {0, 0}} << QList<QPoint>{{0, 16}, {-8, -12}};
    QTest::newRow("Asterisk") << Shape::Asterisk << QList<QPoint>{{18, 0}, {12, 12}} << QList<QPoint>{{16, 8}, {-8, 16}};
    QTest::newRow("Pentagon") << Shape::Pentagon << QList<QPoint>{{0, -17}, {0, 15}} << QList<QPoint>{{0, 17}, {-15, -15}};
    QTest::newRow("Hexagon") << Shape::Hexagon << QList<QPoint>{{0, 18}, {16, 0}} << QList<QPoint>{{18, 0}, {14, -14}};
    QTest::newRow("HLine") << Shape::HLine << QList<QPoint>{{18, 0}, {-18, -3}} << QList<QPoint>{{0, 6}, {0, -6}};
    QTest::newRow("VLine") << Shape::VLine << QList<QPoint>{{0, 18}, {-3, -18}} << QList<QPoint>{{6, 0}, {-6, 0}};
}

void MarkerShapesTest::shapesCoverTheirOutline()
{
    QFETCH(Shape, shape);
    QFETCH(QList<QPoint>, inside);
    QFETCH(QList<QPoint>, outside);

    const auto result = renderMarker({shape});
    QVERIFY(!result.image.isNull());
    if (result.softwareBackend) {
        QSKIP("Custom materials do not render with the software scene graph backend");
    }

    for (const auto& offset : inside) {
        QVERIFY2(isLit(result.image, offset), qPrintable(QStringLiteral("(%1, %2) should be inside").arg(offset.x()).arg(offset.y())));
    }
    for (const auto& offset : outside) {
        QVERIFY2(!isLit(result.image, offset), qPrintable(QStringLiteral("(%1, %2) should be outside").arg(offset.x()).arg(offset.y())));
    }
}

void MarkerShapesTest::hollowMarkersDrawOnlyTheOutline_data()
{
    QTest::addColumn<Shape>("shape");
    QTest::addColumn<QList<QPoint>>("inside");
    QTest::addColumn<QList<QPoint>>("outside");

    // Offsets for a marker of radius 20 px with a 3 px outline.
    QTest::newRow("Circle") << Shape::Circle << QList<QPoint>{{18, 0}, {0, -19}} << QList<QPoint>{{0, 0}, {15, 0}, {20, 0}};
    QTest::newRow("Square") << Shape::Square << QList<QPoint>{{18, 5}, {-19, -19}} << QList<QPoint>{{0, 0}, {15, 5}};
    QTest::newRow("Hexagon") << Shape::Hexagon << QList<QPoint>{{16, 0}} << QList<QPoint>{{0, 0}, {12, 0}};
    // Line-like shapes have no interior to hollow out.
    QTest::newRow("Cross") << Shape::Cross << QList<QPoint>{{0, 0}, {18, 0}} << QList<QPoint>{{12, 12}};
}

void MarkerShapesTest::hollowMarkersDrawOnlyTheOutline()
{
    QFETCH(Shape, shape);
    QFETCH(QList<QPoint>, inside);
    QFETCH(QList<QPoint>, outside);

    auto style = MarkerStyle{shape};
    style.filled = false;
    style.strokeWidth = 3.0;
    const auto result = renderMarker(style);
    QVERIFY(!result.image.isNull());
    if (result.softwareBackend) {
        QSKIP("Custom materials do not render with the software scene graph backend");
    }

    for (const auto& offset : inside) {
        QVERIFY2(isLit(result.image, offset), qPrintable(QStringLiteral("(%1, %2) should be drawn").arg(offset.x()).arg(offset.y())));
    }
    for (const auto& offset : outside) {
        QVERIFY2(!isLit(result.image, offset), qPrintable(QStringLiteral("(%1, %2) should be empty").arg(offset.x()).arg(offset.y())));
    }
}

void MarkerShapesTest::pointCloudMarkersMatchLineCurve_data()
{
    QTest::addColumn<Shape>("shape");
    QTest::addColumn<bool>("filled");

    // PointCloud and LineCurve markers share point_shapes.glsl, so the same style must
    // produce the same pixels. Covers a closed shape, a line shape, and hollow outlines.
    QTest::newRow("Circle filled") << Shape::Circle << true;
    QTest::newRow("Circle hollow") << Shape::Circle << false;
    QTest::newRow("Square hollow") << Shape::Square << false;
    QTest::newRow("Hexagon hollow") << Shape::Hexagon << false;
    // Line shapes have no interior, so markerFilled must not change them.
    QTest::newRow("Cross hollow") << Shape::Cross << false;
}

void MarkerShapesTest::pointCloudMarkersMatchLineCurve()
{
    QFETCH(Shape, shape);
    QFETCH(bool, filled);

    auto style = MarkerStyle{shape};
    style.filled = filled;
    style.strokeWidth = 3.0;

    const auto curveResult = renderMarker(style);
    const auto cloudResult = renderPointCloudMarker(style);
    QVERIFY(!curveResult.image.isNull());
    QVERIFY(!cloudResult.image.isNull());
    if (curveResult.softwareBackend || cloudResult.softwareBackend) {
        QSKIP("Custom materials do not render with the software scene graph backend");
    }

    QCOMPARE(cloudResult.image.size(), curveResult.image.size());
    for (auto y = 0; y < curveResult.image.height(); ++y) {
        for (auto x = 0; x < curveResult.image.width(); ++x) {
            const auto offset = QPoint{x - kCentre, y - kCentre};
            QVERIFY2(isLit(cloudResult.image, offset) == isLit(curveResult.image, offset),
                qPrintable(QStringLiteral("(%1, %2) differs between PointCloud and LineCurve").arg(offset.x()).arg(offset.y())));
        }
    }
}

void MarkerShapesTest::pixelMarkerCoversOnePixel()
{
    const auto result = renderMarker({Shape::Pixel});
    QVERIFY(!result.image.isNull());
    if (result.softwareBackend) {
        QSKIP("Custom materials do not render with the software scene graph backend");
    }

    auto litPixels = QList<QPoint>{};
    for (auto y = 0; y < kCurveSize; ++y) {
        for (auto x = 0; x < kCurveSize; ++x) {
            if (qGray(result.image.pixel(x, y)) > 0) {
                litPixels.append({x, y});
            }
        }
    }
    QCOMPARE(litPixels.size(), 1);
    QCOMPARE(qGray(result.image.pixel(litPixels.front())), 255);
    QVERIFY((litPixels.front() - QPoint{kCentre, kCentre}).manhattanLength() <= 2);
}

void MarkerShapesTest::pixelMarkerHitTestIgnoresMarkerSize()
{
    auto xAxis = Axis{};
    auto yAxis = Axis{};
    xAxis.setOrientation(Axis::Horizontal);
    yAxis.setOrientation(Axis::Vertical);
    for (auto* axis : {&xAxis, &yAxis}) {
        axis->setViewportMin(0.0);
        axis->setViewportMax(1.0);
    }
    auto curve = LineCurve{};
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);
    curve.setSize(QSizeF{100.0, 100.0});
    curve.marker()->setSize(20.0);
    curve.setData(std::vector<double>{0.5}, std::vector<double>{0.5});
    // QQuickItem::contains() is public; LineCurve narrows its override to protected.
    const auto hits = [&curve](const QPointF& point) { return static_cast<const QQuickItem&>(curve).contains(point); };

    curve.marker()->setShape(Shape::Circle);
    QVERIFY(hits({60.0, 50.0}));

    curve.marker()->setShape(Shape::Pixel);
    QVERIFY(hits({52.0, 50.0}));
    QVERIFY(!hits({60.0, 50.0}));
}

void MarkerShapesTest::gradientStrokeColorsMarkersBetweenStops()
{
    auto window = QQuickWindow{};
    window.setColor(Qt::black);
    window.resize(kCurveSize, kCurveSize);

    auto engine = QQmlEngine{};
    auto component = QQmlComponent{&engine};
    // Markers at x = 0.25 and 0.75 fall midway into the first and second of three stops.
    component.setData("import QtQuick\n"
                      "import QAccelPlot\n"
                      "LineCurve {\n"
                      "    width: 64; height: 64\n"
                      "    xAxis: Axis { orientation: Axis.Horizontal; viewportMin: 0; viewportMax: 1 }\n"
                      "    yAxis: Axis { orientation: Axis.Vertical; viewportMin: 0; viewportMax: 1 }\n"
                      "    lineStyle: NoLine {}\n"
                      "    marker.shape: LineCurve.Square\n"
                      "    marker.size: 8\n"
                      "    antialiasingEnabled: false\n"
                      "    effects: GradientStroke {\n"
                      "        direction: GradientDirection.Horizontal\n"
                      "        gradientValueMinSource: GradientValueSource.Fixed; gradientValueMin: 0\n"
                      "        gradientValueMaxSource: GradientValueSource.Fixed; gradientValueMax: 1\n"
                      "        gradient: Gradient {\n"
                      "            GradientStop { position: 0.0; color: \"#ff0000\" }\n"
                      "            GradientStop { position: 0.5; color: \"#00ff00\" }\n"
                      "            GradientStop { position: 1.0; color: \"#0000ff\" }\n"
                      "        }\n"
                      "    }\n"
                      "    Component.onCompleted: setData([Qt.point(0.25, 0.5), Qt.point(0.75, 0.5)])\n"
                      "}\n",
        QUrl{});
    const auto root = std::unique_ptr<QObject>{component.create()};
    QVERIFY2(root, qPrintable(component.errorString()));
    qobject_cast<QQuickItem*>(root.get())->setParentItem(window.contentItem());

    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const auto image = window.grabWindow();
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom materials do not render with the software scene graph backend");
    }

    const auto near = [](const QColor& pixel, const QColor& expected) {
        constexpr auto kTolerance = 8;
        return std::abs(pixel.red() - expected.red()) <= kTolerance && std::abs(pixel.green() - expected.green()) <= kTolerance
            && std::abs(pixel.blue() - expected.blue()) <= kTolerance;
    };
    const auto left = image.pixelColor(kCurveSize / 4, kCentre);
    const auto right = image.pixelColor(kCurveSize * 3 / 4, kCentre);
    QVERIFY2(near(left, QColor{128, 128, 0}), qPrintable(left.name()));
    QVERIFY2(near(right, QColor{0, 128, 128}), qPrintable(right.name()));
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Offsets are logical pixels, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::MarkerShapesTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_marker_shapes.moc"
