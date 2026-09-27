//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/axis/Axis.hpp"

#include <QGuiApplication>
#include <QImage>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <memory>

namespace QAccelPlot {

namespace {

constexpr auto kScenePrefix = R"(
import QtQuick
import QAccelPlot

PlotView {
    id: plot
    width: 400
    height: 300
    plotAreaColor: "black"
    axesAreaColor: "#202020"
    grid.gridVisible: false
    grid.subGridVisible: false

    xAxis: Axis { viewportMin: 0; viewportMax: 10 }
    yAxis: Axis { viewportMin: 0; viewportMax: 10 }
)";

// A red full-height span at x 1..2, a red rectangle with a NaN edge at x 6..8, and a blue
// full-width band at y 4..5 drawn on top.
constexpr auto kUnboundedScene = R"(
    RectangleList {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        Component.onCompleted: setData([{ x1: 1, x2: 2 }, { x1: 6, y1: 6, x2: 8, y2: NaN }])
    }

    RectangleList {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "blue"
        Component.onCompleted: setData([{ y1: 4, y2: 5 }])
    }
}
)";

// A red span far narrower than a pixel at x = 5, and a blue rectangle of zero height at y = 2.
constexpr auto kNarrowScene = R"(
    RectangleList {
        objectName: "narrow"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        minimumWidth: 8
        Component.onCompleted: setData([{ x1: 5, x2: 5.0001 }])
    }

    RectangleList {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "blue"
        minimumHeight: 8
        Component.onCompleted: setData([{ x1: 1, y1: 2, x2: 3, y2: 2 }])
    }
}
)";

// Full-height spans in categories 0 and 1, and one without a category.
constexpr auto kCategoryScene = R"(
    RectangleList {
        objectName: "categorized"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "blue"
        categoryColors: ["red", "lime"]
        Component.onCompleted: setData([
            { x1: 1, x2: 2, category: 0 },
            { x1: 4, x2: 5, category: 1 },
            { x1: 7, x2: 8 }
        ])
    }
}
)";

// Two touching red rectangles with a 3 px white outline.
constexpr auto kBorderScene = R"(
    RectangleList {
        objectName: "outlined"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        border.width: 3
        border.color: "white"
        Component.onCompleted: setData([{ x1: 2, y1: 2, x2: 5, y2: 8 }, { x1: 5, y1: 2, x2: 8, y2: 8 }])
    }
}
)";

// Two red spans that turn yellow while hovered.
constexpr auto kHoverScene = R"(
    RectangleList {
        objectName: "hoverable"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        hoverColor: "yellow"
        Component.onCompleted: setData([{ x1: 1, x2: 3 }, { x1: 6, x2: 8 }])
    }
}
)";

// Shows a PlotView built from kScenePrefix and \a sceneBody in a 400 × 300 window.
class SceneWindow {
public:
    explicit SceneWindow(const char* sceneBody)
    {
        window_.setColor(Qt::black);
        window_.resize(400, 300);
        component_.setData(QByteArray{kScenePrefix} + sceneBody, QUrl{});
        root_.reset(component_.create());
        plot_ = qobject_cast<QAccelPlot*>(root_.get());
        if (plot_) {
            plot_->setParentItem(window_.contentItem());
            window_.show();
        }
    }

    bool isSoftware() const
    {
        return window_.rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
    }

    QString error() const
    {
        return component_.errorString();
    }

    QQuickWindow& window()
    {
        return window_;
    }

    QAccelPlot* plot() const
    {
        return plot_;
    }

    QImage grab()
    {
        return window_.grabWindow();
    }

    QPoint pixel(const qreal x, const qreal y) const
    {
        return {qRound(plot_->dataToPixelX(x)), qRound(plot_->dataToPixelY(y))};
    }

private:
    QQuickWindow window_;
    QQmlEngine engine_;
    QQmlComponent component_{&engine_};
    std::unique_ptr<QObject> root_;
    QAccelPlot* plot_{nullptr};
};

bool isColor(const QColor& pixel, const QColor& expected)
{
    constexpr auto kTolerance = 60;
    return std::abs(pixel.red() - expected.red()) < kTolerance && std::abs(pixel.green() - expected.green()) < kTolerance
        && std::abs(pixel.blue() - expected.blue()) < kTolerance;
}

} // namespace

class RectangleListRenderingTest : public QObject {
    Q_OBJECT

private slots:
    void unboundedEdgesReachThePlotEdges();
    void narrowRectanglesKeepMinimumSize();
    void categoriesSelectFillColors();
    void borderOutlinesEachRectangle();
    void hoverColorHighlightsHoveredRectangle();
};

void RectangleListRenderingTest::unboundedEdgesReachThePlotEdges()
{
    auto scene = SceneWindow{kUnboundedScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto area = plot->plotRect().toAlignedRect();
    const auto top = area.top() + 2;
    const auto bottom = area.bottom() - 2;
    const auto left = area.left() + 2;
    const auto right = area.right() - 2;
    const auto pixelX = [&scene](const qreal x) { return scene.pixel(x, 0.0).x(); };
    const auto pixelY = [&scene](const qreal y) { return scene.pixel(0.0, y).y(); };

    auto image = scene.grab();
    QVERIFY(isColor(image.pixelColor(pixelX(1.5), top), Qt::red));
    QVERIFY(isColor(image.pixelColor(pixelX(1.5), bottom), Qt::red));
    QVERIFY(isColor(image.pixelColor(left, pixelY(4.5)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(right, pixelY(4.5)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(scene.pixel(7.0, 7.0)), Qt::black));
    QVERIFY(isColor(image.pixelColor(scene.pixel(5.0, 8.0)), Qt::black));

    // Far from the data, the span still fills the plot height and the band leaves the view.
    plot->yAxis()->setViewportMin(1.0e6);
    plot->yAxis()->setViewportMax(1.0e6 + 10.0);
    image = scene.grab();
    QVERIFY(isColor(image.pixelColor(pixelX(1.5), top), Qt::red));
    QVERIFY(isColor(image.pixelColor(pixelX(1.5), bottom), Qt::red));
    QVERIFY(isColor(image.pixelColor(pixelX(5.0), (top + bottom) / 2), Qt::black));

    plot->yAxis()->setViewportMin(1.0);
    plot->yAxis()->setViewportMax(1000.0);
    plot->yAxis()->setLogScale(true);
    image = scene.grab();
    QVERIFY(isColor(image.pixelColor(pixelX(1.5), top), Qt::red));
    QVERIFY(isColor(image.pixelColor(pixelX(1.5), bottom), Qt::red));
    QVERIFY(isColor(image.pixelColor(left, pixelY(4.5)), Qt::blue));

    plot->xAxis()->setViewportMin(1.0e9);
    plot->xAxis()->setViewportMax(1.0e9 + 10.0);
    image = scene.grab();
    QVERIFY(isColor(image.pixelColor(left, pixelY(4.5)), Qt::blue));
    QVERIFY(isColor(image.pixelColor(right, pixelY(4.5)), Qt::blue));
    QVERIFY(isColor(image.pixelColor((left + right) / 2, top), Qt::black));
}

void RectangleListRenderingTest::narrowRectanglesKeepMinimumSize()
{
    auto scene = SceneWindow{kNarrowScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto span = scene.pixel(5.0, 5.0);
    const auto band = scene.pixel(2.0, 2.0);
    auto image = scene.grab();
    for (const auto offset : {-3, 0, 3}) {
        QVERIFY(isColor(image.pixelColor(span.x() + offset, span.y()), Qt::red));
        QVERIFY(isColor(image.pixelColor(band.x(), band.y() + offset), Qt::blue));
    }
    for (const auto offset : {-7, 7}) {
        QVERIFY(isColor(image.pixelColor(span.x() + offset, span.y()), Qt::black));
        QVERIFY(isColor(image.pixelColor(band.x(), band.y() + offset), Qt::black));
    }

    // Without a minimum, the 0.004 px wide span covers at most one pixel center.
    auto* narrow = plot->findChild<QQuickItem*>(QStringLiteral("narrow"));
    QVERIFY(narrow);
    narrow->setProperty("minimumWidth", 0.0);
    image = scene.grab();
    auto redPixels = 0;
    for (auto offset = -3; offset <= 3; ++offset) {
        redPixels += isColor(image.pixelColor(span.x() + offset, span.y()), Qt::red) ? 1 : 0;
    }
    QVERIFY(redPixels <= 1);
}

void RectangleListRenderingTest::categoriesSelectFillColors()
{
    auto scene = SceneWindow{kCategoryScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));
    auto* rectangles = plot->findChild<QQuickItem*>(QStringLiteral("categorized"));
    QVERIFY(rectangles);

    const auto first = scene.pixel(1.5, 5.0);
    const auto second = scene.pixel(4.5, 5.0);
    const auto uncategorized = scene.pixel(7.5, 5.0);
    auto image = scene.grab();
    QVERIFY(isColor(image.pixelColor(first), Qt::red));
    QVERIFY(isColor(image.pixelColor(second), Qt::green));
    QVERIFY(isColor(image.pixelColor(uncategorized), Qt::blue));

    rectangles->setProperty("categoryColors", QVariant::fromValue(QList<QColor>{Qt::yellow, Qt::green}));
    rectangles->setProperty("color", QColor{Qt::white});
    image = scene.grab();
    QVERIFY(isColor(image.pixelColor(first), Qt::yellow));
    QVERIFY(isColor(image.pixelColor(uncategorized), Qt::white));

    QVERIFY(QMetaObject::invokeMethod(rectangles, "setCategories", Q_ARG(QList<int>, (QList<int>{1, 5, 0}))));
    image = scene.grab();
    QVERIFY(isColor(image.pixelColor(first), Qt::green));
    QVERIFY(isColor(image.pixelColor(second), Qt::white));
    QVERIFY(isColor(image.pixelColor(uncategorized), Qt::yellow));
}

void RectangleListRenderingTest::borderOutlinesEachRectangle()
{
    auto scene = SceneWindow{kBorderScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));

    const auto center = scene.pixel(3.5, 5.0);
    const auto leftEdge = scene.pixel(2.0, 5.0);
    const auto sharedEdge = scene.pixel(5.0, 5.0);
    const auto topEdge = scene.pixel(3.5, 8.0);
    auto image = scene.grab();
    QVERIFY(isColor(image.pixelColor(center), Qt::red));
    QVERIFY(isColor(image.pixelColor(leftEdge.x() + 1, leftEdge.y()), Qt::white));
    QVERIFY(isColor(image.pixelColor(leftEdge.x() + 6, leftEdge.y()), Qt::red));
    QVERIFY(isColor(image.pixelColor(sharedEdge.x() - 2, sharedEdge.y()), Qt::white));
    QVERIFY(isColor(image.pixelColor(sharedEdge.x() + 1, sharedEdge.y()), Qt::white));
    QVERIFY(isColor(image.pixelColor(topEdge.x(), topEdge.y() + 1), Qt::white));
    QVERIFY(isColor(image.pixelColor(leftEdge.x() - 2, leftEdge.y()), Qt::black));

    auto* rectangles = plot->findChild<QQuickItem*>(QStringLiteral("outlined"));
    QVERIFY(rectangles);
    rectangles->property("border").value<QObject*>()->setProperty("width", 0.0);
    image = scene.grab();
    QVERIFY(isColor(image.pixelColor(leftEdge.x() + 1, leftEdge.y()), Qt::red));
}

void RectangleListRenderingTest::hoverColorHighlightsHoveredRectangle()
{
    auto scene = SceneWindow{kHoverScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    auto* plot = scene.plot();
    QVERIFY2(plot, qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));
    auto* rectangles = plot->findChild<QQuickItem*>(QStringLiteral("hoverable"));
    QVERIFY(rectangles);

    const auto first = scene.pixel(2.0, 5.0);
    const auto second = scene.pixel(7.0, 5.0);
    // Deliver hover straight to the item, so the real mouse cannot interfere.
    const auto hover = [rectangles](const QEvent::Type type, const QPoint& windowPosition) {
        const auto position = rectangles->mapFromScene(windowPosition);
        auto event = QHoverEvent{type, position, position, position};
        QCoreApplication::sendEvent(rectangles, &event);
    };

    hover(QEvent::HoverEnter, first);
    QCOMPARE(rectangles->property("hoveredIndex").toInt(), 0);
    auto image = scene.grab();
    QVERIFY(isColor(image.pixelColor(first), Qt::yellow));
    QVERIFY(isColor(image.pixelColor(second), Qt::red));

    hover(QEvent::HoverMove, second);
    image = scene.grab();
    QVERIFY(isColor(image.pixelColor(first), Qt::red));
    QVERIFY(isColor(image.pixelColor(second), Qt::yellow));

    rectangles->setProperty("hoverColor", QColor{});
    image = scene.grab();
    QVERIFY(isColor(image.pixelColor(second), Qt::red));
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Pixel lookups use logical coordinates, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::RectangleListRenderingTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_rectangle_list_rendering.moc"
