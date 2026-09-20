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
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <vector>

namespace QAccelPlot {

namespace {

using Shape = LineCurve::PointShape;

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
    curve.setMarkerShape(style.shape);
    curve.setMarkerSize(style.size);
    curve.setMarkerFilled(style.filled);
    curve.setMarkerStrokeWidth(style.strokeWidth);
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
    cloud.setMarkerShape(static_cast<PointCloud::MarkerShape>(style.shape));
    cloud.setMarkerSize(style.size);
    cloud.setMarkerFilled(style.filled);
    cloud.setMarkerStrokeWidth(style.strokeWidth);
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

} // namespace

class MarkerShapesTest : public QObject {
    Q_OBJECT

private slots:
    void shapeIndicesMatchShaders();
    void shapesCoverTheirOutline_data();
    void shapesCoverTheirOutline();
    void hollowMarkersDrawOnlyTheOutline_data();
    void hollowMarkersDrawOnlyTheOutline();
    void pointCloudMarkersMatchLineCurve_data();
    void pointCloudMarkersMatchLineCurve();
    void pixelMarkerCoversOnePixel();
    void pixelMarkerHitTestIgnoresMarkerSize();
};

void MarkerShapesTest::shapeIndicesMatchShaders()
{
    // PointShape values are public API, and point.frag and point.vert select shapes by value - 1.
    // Append new shapes at the end; never reorder or insert.
    const auto expected = std::vector<std::pair<Shape, int>>{{Shape::None, 0}, {Shape::Circle, 1}, {Shape::Square, 2}, {Shape::Diamond, 3},
        {Shape::TriangleUp, 4}, {Shape::TriangleDown, 5}, {Shape::TriangleLeft, 6}, {Shape::TriangleRight, 7}, {Shape::Cross, 8}, {Shape::XCross, 9},
        {Shape::HLine, 10}, {Shape::VLine, 11}, {Shape::Star, 12}, {Shape::Asterisk, 13}, {Shape::Pixel, 14}, {Shape::Hexagon, 15}, {Shape::Pentagon, 16}};
    QCOMPARE(QMetaEnum::fromType<Shape>().keyCount(), static_cast<int>(expected.size()));
    for (const auto& [shape, value] : expected) {
        QCOMPARE(static_cast<int>(shape), value);
    }
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
    curve.setMarkerSize(20.0);
    curve.setData(std::vector<double>{0.5}, std::vector<double>{0.5});
    // QQuickItem::contains() is public; LineCurve narrows its override to protected.
    const auto hits = [&curve](const QPointF& point) { return static_cast<const QQuickItem&>(curve).contains(point); };

    curve.setMarkerShape(Shape::Circle);
    QVERIFY(hits({60.0, 50.0}));

    curve.setMarkerShape(Shape::Pixel);
    QVERIFY(hits({52.0, 50.0}));
    QVERIFY(!hits({60.0, 50.0}));
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
