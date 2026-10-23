//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/QAccelPlot.hpp"
#include "QAccelPlot/linestyles/DashLine.hpp"
#include "QAccelPlot/renderers/LineCurveLineRenderer.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/series/internal/StepPath.hpp"
#include "WindowPlacement.hpp"

#include <QImage>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <vector>

namespace QAccelPlot {

namespace {

constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();

class RenderableCurve final : public LineCurve {
public:
    using LineCurve::updatePaintNode;
};

// Data X in [0, 10] maps to [0, 1000] px and data Y in [0, 10] maps to [100, 0] px.
struct AxesFixture {
    Axis xAxis;
    Axis yAxis;

    AxesFixture()
    {
        xAxis.setOrientation(Qt::Horizontal);
        yAxis.setOrientation(Qt::Vertical);
        xAxis.setViewportMin(0.0);
        xAxis.setViewportMax(10.0);
        yAxis.setViewportMin(0.0);
        yAxis.setViewportMax(10.0);
    }
};

// Renderer parameters for \a data drawn with \a mode on \a axes in a 1000 × 100 px viewport. They refer to
// \a gradient and \a fill, which must outlive them.
LineCurveRenderParams renderParams(const std::vector<float>& data, AxesFixture& axes, const StepMode mode, const GradientColorPayload& gradient,
    const GradientFillPayload& fill, const LineStyle* lineStyle)
{
    return LineCurveRenderParams{nullptr, data, {data.data(), nullptr}, static_cast<int>(data.size() / 2), true, Qt::red, false, 1, {0, 0}, {10, 10},
        {1000, 100}, &axes.xAxis, &axes.yAxis, false, false, true, 1, gradient, fill, nullptr, lineStyle, mode};
}

QPointF corner(const std::vector<double>& data, const int from, const int corner, const StepMode mode, const bool logScaleX = false)
{
    const auto point = Internal::stepCorner(CurveDataView{nullptr, data.data()}, from, from + 1, corner, mode, logScaleX, false);
    return point.value_or(QPointF{kNaN, kNaN});
}

bool drawn(const std::vector<double>& data, const int from, const int corner, const StepMode mode)
{
    return Internal::stepCorner(CurveDataView{nullptr, data.data()}, from, from + 1, corner, mode, false, false).has_value();
}

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
)";

// A red step line through (1, 2), (5, 8), and (9, 4).
constexpr auto kStepScene = R"(
    xAxis: Axis { viewportMin: 0; viewportMax: 10 }
    yAxis: Axis { viewportMin: 0; viewportMax: 10 }

    LineCurve {
        objectName: "curve"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        lineWidth: 3
        hoverRadius: 0
        stepMode: StepMode.After
        Component.onCompleted: setData([Qt.point(1, 2), Qt.point(5, 8), Qt.point(9, 4)])
    }
}
)";

// A step line with a blue fill to y = 0 under it.
constexpr auto kFillScene = R"(
    xAxis: Axis { viewportMin: 0; viewportMax: 10 }
    yAxis: Axis { viewportMin: 0; viewportMax: 10 }

    LineCurve {
        objectName: "curve"
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        lineWidth: 1
        hoverRadius: 0
        stepMode: StepMode.After
        effects: GradientFill {
            baseline: GradientFillBaseline.Value
            baselineValue: 0
            opacity: 1
            gradient: Gradient {
                GradientStop { position: 0; color: "blue" }
                GradientStop { position: 1; color: "blue" }
            }
        }
        Component.onCompleted: setData([Qt.point(1, 2), Qt.point(5, 8), Qt.point(9, 4)])
    }
}
)";

// A centered step line from (2, 2) to (50, 8) on a log X axis.
constexpr auto kLogScene = R"(
    xAxis: Axis { viewportMin: 1; viewportMax: 100; logScale: true }
    yAxis: Axis { viewportMin: 0; viewportMax: 10 }

    LineCurve {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        lineWidth: 3
        hoverRadius: 0
        stepMode: StepMode.Center
        Component.onCompleted: setData([Qt.point(2, 2), Qt.point(50, 8)])
    }
}
)";

// A step line whose second data point has no Y.
constexpr auto kGapScene = R"(
    xAxis: Axis { viewportMin: 0; viewportMax: 10 }
    yAxis: Axis { viewportMin: 0; viewportMax: 10 }

    LineCurve {
        xAxis: plot.xAxis
        yAxis: plot.yAxis
        color: "red"
        lineWidth: 3
        hoverRadius: 0
        stepMode: StepMode.After
        Component.onCompleted: setData([Qt.point(1, 2), Qt.point(5, NaN), Qt.point(8, 4), Qt.point(9, 6)])
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
            QAccelPlotTest::moveAwayFromCursor(window_);
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

    LineCurve* curve() const
    {
        return plot_->findChild<LineCurve*>(QStringLiteral("curve"));
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

class LineCurveStepsTest : public QObject {
    Q_OBJECT

private slots:
    void cornersHoldTheOwningValue();
    void cornersHoldUpToAnInvalidNeighbor();
    void centerCornersUseTheScreenMidpoint();
    void lastDataPointHasNoStep();
    void verticesEncodeCorners();
    void dashLengthsFollowSteps();
    void fillFollowsSteps();
    void hoverFollowsSteps();
    void stepModeNotifiesAndKeepsData();
    void vertexCacheMatchesCornerCount();
    void linesDrawSteps();
    void fillDrawsSteps();
    void centerStepsAreHalfwayOnLogAxes();
    void stepsHoldUpToGaps();
};

void LineCurveStepsTest::cornersHoldTheOwningValue()
{
    const auto data = std::vector<double>{1, 2, 5, 8, 9, 4};
    QCOMPARE(Internal::stepCornerCount(StepMode::None), 1);
    QCOMPARE(Internal::stepCornerCount(StepMode::Before), 2);
    QCOMPARE(Internal::stepCornerCount(StepMode::After), 2);
    QCOMPARE(Internal::stepCornerCount(StepMode::Center), 3);
    QCOMPARE(corner(data, 0, 0, StepMode::After), QPointF(1, 2));
    QCOMPARE(corner(data, 0, 1, StepMode::After), QPointF(5, 2));
    QCOMPARE(corner(data, 0, 1, StepMode::Before), QPointF(1, 8));
    QCOMPARE(corner(data, 1, 1, StepMode::Center), QPointF(7, 8));
    QCOMPARE(corner(data, 1, 2, StepMode::Center), QPointF(7, 4));
}

void LineCurveStepsTest::cornersHoldUpToAnInvalidNeighbor()
{
    // The second data point has a finite X but no Y; the fourth has neither.
    const auto data = std::vector<double>{1, 2, 5, kNaN, 9, 4, kNaN, kNaN, 12, 6};
    QCOMPARE(corner(data, 0, 1, StepMode::After), QPointF(5, 2));
    QVERIFY(!drawn(data, 1, 1, StepMode::After));
    QVERIFY(!drawn(data, 0, 1, StepMode::Before));
    QCOMPARE(corner(data, 1, 1, StepMode::Before), QPointF(5, 4));
    QCOMPARE(corner(data, 0, 1, StepMode::Center), QPointF(3, 2));
    QVERIFY(!drawn(data, 0, 2, StepMode::Center));
    QCOMPARE(corner(data, 1, 2, StepMode::Center), QPointF(7, 4));

    // Without an X there is no X to hold up to.
    QVERIFY(!drawn(data, 2, 1, StepMode::After));
    QVERIFY(!drawn(data, 3, 1, StepMode::Before));
    QVERIFY(!drawn(data, 2, 1, StepMode::Center));
}

void LineCurveStepsTest::centerCornersUseTheScreenMidpoint()
{
    const auto data = std::vector<double>{1, 2, 100, 8};
    QCOMPARE(corner(data, 0, 1, StepMode::Center), QPointF(50.5, 2));
    QCOMPARE(corner(data, 0, 1, StepMode::Center, true), QPointF(10, 2));
    QCOMPARE(corner(data, 0, 2, StepMode::Center, true), QPointF(10, 8));
}

void LineCurveStepsTest::lastDataPointHasNoStep()
{
    const auto data = std::vector<double>{1, 2, 5, 8};
    const auto view = CurveDataView{nullptr, data.data()};
    QCOMPARE(Internal::linePathPoint(view, 2, 0, 1, StepMode::After, false, false), std::optional<QPointF>(QPointF(5, 2)));
    QCOMPARE(Internal::linePathPoint(view, 2, 1, 1, StepMode::After, false, false), std::optional<QPointF>(QPointF(5, 8)));
    QCOMPARE(Internal::linePathPoint(view, 2, 1, 2, StepMode::Center, false, false), std::optional<QPointF>(QPointF(5, 8)));
}

void LineCurveStepsTest::verticesEncodeCorners()
{
    const auto data = std::vector<float>{1, 2, 5, 8, 9, 4};
    auto cache = std::vector<char>{};
    LineCurveLineRenderer{}.buildVertexCache(data, 3, cache, StepMode::Center);
    QCOMPARE(cache.size(), 3 * 3 * 2 * sizeof(LineVertex));
    const auto* vertices = reinterpret_cast<const LineVertex*>(cache.data());
    for (auto vertex = 0; vertex < 18; ++vertex) {
        const auto path = vertex / 2;
        QCOMPARE(vertices[vertex].id, static_cast<float>(path / 3));
        QCOMPARE(vertices[vertex].side, static_cast<float>((1 + path % 3) * (vertex % 2 == 0 ? 1 : -1)));
    }
}

void LineCurveStepsTest::dashLengthsFollowSteps()
{
    auto axes = AxesFixture{};
    auto dash = DashLine{};
    dash.setPattern({8, 4});
    const auto data = std::vector<float>{1, 2, 5, 8, 9, 4};
    const auto gradient = GradientColorPayload{};
    const auto fill = GradientFillPayload{};
    const auto params = renderParams(data, axes, StepMode::After, gradient, fill, &dash);
    const auto root = std::unique_ptr<QSGNode>{LineCurveLineRenderer{}.paint(nullptr, params)};
    const auto* geometry = static_cast<const QSGGeometryNode*>(root->lastChild())->geometry();
    QCOMPARE(geometry->vertexCount(), 12);

    // Pixel path: (100, 80) → (500, 80) → (500, 20) → (900, 20) → (900, 60), and the last point repeated.
    const auto* vertices = static_cast<const LineVertex*>(geometry->vertexData());
    const auto expected = std::vector<float>{0, 400, 460, 860, 900, 900};
    for (auto path = 0; path < 6; ++path) {
        QCOMPARE(vertices[path * 2].arcLength, expected[static_cast<std::size_t>(path)]);
        QCOMPARE(vertices[path * 2 + 1].arcLength, expected[static_cast<std::size_t>(path)]);
    }
}

void LineCurveStepsTest::fillFollowsSteps()
{
    auto axes = AxesFixture{};
    auto fill = GradientFillPayload{};
    fill.enabled = true;
    fill.stops = {{0, Qt::red}, {1, Qt::blue}};
    fill.gradientValueMin = 0;
    fill.gradientValueMax = 10;
    fill.baseline = GradientFillBaseline::Value;
    fill.baselineValue = 0;
    // A lone data point before the gap still fills its step up to the X of the invalid one.
    const auto nan = std::numeric_limits<float>::quiet_NaN();
    const auto data = std::vector<float>{1, 2, 5, nan, 8, 4, 9, 6};
    const auto gradient = GradientColorPayload{};
    const auto params = renderParams(data, axes, StepMode::After, gradient, fill, nullptr);
    const auto root = std::unique_ptr<QSGNode>{LineCurveLineRenderer{}.paint(nullptr, params)};
    const auto* geometry = static_cast<const QSGGeometryNode*>(root->firstChild())->geometry();

    // {x, y, gradientCoordinate} per vertex, a baseline and a curve vertex per outline point, two restart vertices between runs.
    const auto* vertices = static_cast<const float*>(geometry->vertexData());
    const auto curveVertex = [&](const int vertex) { return QPointF{vertices[vertex * 3], vertices[vertex * 3 + 1]}; };
    QCOMPARE(geometry->vertexCount(), 2 * 2 + 2 + 3 * 2);
    QCOMPARE(curveVertex(1), QPointF(100, 80));
    QCOMPARE(curveVertex(3), QPointF(500, 80));
    QCOMPARE(curveVertex(7), QPointF(800, 60));
    QCOMPARE(curveVertex(9), QPointF(900, 60));
    QCOMPARE(curveVertex(11), QPointF(900, 40));
}

void LineCurveStepsTest::hoverFollowsSteps()
{
    auto axes = AxesFixture{};
    auto curve = LineCurve{};
    curve.setXAxis(&axes.xAxis);
    curve.setYAxis(&axes.yAxis);
    curve.setSize(QSizeF{1000.0, 100.0});
    curve.setData(QList<QPointF>{{1, 2}, {5, 8}, {9, 4}});
    const auto hits = [&](const QPointF& point) { return static_cast<const QQuickItem&>(curve).contains(point); };

    // The straight line passes (3, 5); the steps pass the flat at y = 2 and the riser at x = 5.
    QVERIFY(hits({300, 50}));
    QVERIFY(!hits({300, 80}));
    curve.setStepMode(StepMode::After);
    QVERIFY(!hits({300, 50}));
    QVERIFY(hits({300, 80}));
    QVERIFY(hits({500, 50}));
    curve.setStepMode(StepMode::Center);
    QVERIFY(hits({200, 80}));
    QVERIFY(hits({300, 50}));
    QVERIFY(!hits({400, 80}));
}

void LineCurveStepsTest::stepModeNotifiesAndKeepsData()
{
    auto curve = LineCurve{};
    QCOMPARE(curve.stepMode(), StepMode::None);
    auto spy = QSignalSpy{&curve, &LineCurve::stepModeChanged};
    curve.setData(QList<QPointF>{{1, 2}, {5, 8}});
    const auto revision = curve.dataRevision();
    curve.setStepMode(StepMode::Before);
    curve.setStepMode(StepMode::Before);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(curve.stepMode(), StepMode::Before);
    QCOMPARE(curve.property("stepMode").value<StepMode>(), StepMode::Before);
    QCOMPARE(curve.dataRevision(), revision);
}

void LineCurveStepsTest::vertexCacheMatchesCornerCount()
{
    auto window = QQuickWindow{};
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom curve materials require a hardware scene graph backend");
    }
    auto axes = AxesFixture{};
    auto curve = RenderableCurve{};
    curve.setParentItem(window.contentItem());
    curve.setXAxis(&axes.xAxis);
    curve.setYAxis(&axes.yAxis);
    curve.setPlotRect({0, 0, 100, 100});
    curve.setStepMode(StepMode::After);
    const auto data = std::vector<float>{1, 2, 5, 8, 9, 4};
    // Distinguish uploaded caches from ones the curve builds itself by reversing their bytes.
    const auto reversedCache = [](const std::vector<float>& points, const StepMode mode) {
        auto cache = std::vector<char>{};
        LineCurveLineRenderer{}.buildVertexCache(points, static_cast<int>(points.size() / 2), cache, mode);
        std::reverse(cache.begin(), cache.end());
        return cache;
    };
    const auto lineVerticesMatch = [](const QSGNode* root, const std::vector<char>& cache) {
        const auto* geometry = static_cast<const QSGGeometryNode*>(root->lastChild())->geometry();
        return static_cast<std::size_t>(geometry->vertexCount() * geometry->sizeOfVertex()) == cache.size()
            && std::memcmp(geometry->vertexData(), cache.data(), cache.size()) == 0;
    };

    // Before and After share their vertices.
    const auto beforeCache = reversedCache(data, StepMode::Before);
    curve.setDataFWithCache(std::vector<float>(data), 3, std::vector<char>(beforeCache));
    auto node = std::unique_ptr<QSGNode>{curve.updatePaintNode(nullptr, nullptr)};
    QVERIFY(node);
    QVERIFY(lineVerticesMatch(node.get(), beforeCache));

    // A cache without steps has half the vertices, so the curve builds its own.
    const auto fourPoints = std::vector<float>{1, 2, 5, 8, 7, 3, 9, 4};
    QTest::ignoreMessage(QtWarningMsg, "Ignoring vertex cache with 128 bytes; expected 256");
    curve.setDataFWithCache(std::vector<float>(fourPoints), 4, reversedCache(fourPoints, StepMode::None));
    node.reset(curve.updatePaintNode(node.release(), nullptr));
    const auto* geometry = static_cast<const QSGGeometryNode*>(node->lastChild())->geometry();
    QCOMPARE(geometry->vertexCount(), 16);
    QCOMPARE(static_cast<const LineVertex*>(geometry->vertexData())[2].side, 2.0f);
}

void LineCurveStepsTest::linesDrawSteps()
{
    auto scene = SceneWindow{kStepScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));
    auto* curve = scene.curve();
    QVERIFY(curve);
    const auto colorAt = [&](const qreal x, const qreal y) { return scene.grab().pixelColor(scene.pixel(x, y)); };

    // After: flat first, then the riser at the next data point; nothing past the last one.
    QTRY_VERIFY(isColor(colorAt(3, 2), Qt::red));
    QVERIFY(isColor(colorAt(3, 5), Qt::black));
    QVERIFY(isColor(colorAt(5, 5), Qt::red));
    QVERIFY(isColor(colorAt(7, 8), Qt::red));
    QVERIFY(isColor(colorAt(9, 6), Qt::red));
    QVERIFY(isColor(colorAt(9.5, 4), Qt::black));

    curve->setStepMode(StepMode::Before);
    QTRY_VERIFY(isColor(colorAt(3, 8), Qt::red));
    QVERIFY(isColor(colorAt(1, 5), Qt::red));
    QVERIFY(isColor(colorAt(3, 2), Qt::black));
    QVERIFY(isColor(colorAt(7, 4), Qt::red));

    curve->setStepMode(StepMode::Center);
    QTRY_VERIFY(isColor(colorAt(2, 2), Qt::red));
    QVERIFY(isColor(colorAt(3, 5), Qt::red));
    QVERIFY(isColor(colorAt(4, 2), Qt::black));
    QVERIFY(isColor(colorAt(5, 8), Qt::red));
    QVERIFY(isColor(colorAt(7, 6), Qt::red));
    QVERIFY(isColor(colorAt(8, 4), Qt::red));

    curve->setStepMode(StepMode::None);
    QTRY_VERIFY(isColor(colorAt(3, 5), Qt::red));
    QVERIFY(isColor(colorAt(3, 2), Qt::black));
}

void LineCurveStepsTest::fillDrawsSteps()
{
    auto scene = SceneWindow{kFillScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));
    const auto colorAt = [&](const qreal x, const qreal y) { return scene.grab().pixelColor(scene.pixel(x, y)); };

    // Below the straight line from (1, 2) to (5, 8) but above the step at y = 2.
    QTRY_VERIFY(isColor(colorAt(3, 1), Qt::blue));
    QVERIFY(isColor(colorAt(4.5, 6), Qt::black));
    QVERIFY(isColor(colorAt(6, 7), Qt::blue));
    QVERIFY(isColor(colorAt(9.5, 1), Qt::black));
    scene.curve()->setStepMode(StepMode::None);
    QTRY_VERIFY(isColor(colorAt(4.5, 6), Qt::blue));
}

void LineCurveStepsTest::centerStepsAreHalfwayOnLogAxes()
{
    auto scene = SceneWindow{kLogScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));
    const auto colorAt = [&](const qreal x, const qreal y) { return scene.grab().pixelColor(scene.pixel(x, y)); };

    // The riser sits at the geometric mean 10, not at the arithmetic mean 26.
    QTRY_VERIFY(isColor(colorAt(10, 5), Qt::red));
    QVERIFY(isColor(colorAt(26, 5), Qt::black));
    QVERIFY(isColor(colorAt(5, 2), Qt::red));
    QVERIFY(isColor(colorAt(20, 8), Qt::red));
}

void LineCurveStepsTest::stepsHoldUpToGaps()
{
    auto scene = SceneWindow{kGapScene};
    if (scene.isSoftware()) {
        QSKIP("Custom materials require a hardware scene graph backend");
    }
    QVERIFY2(scene.plot(), qPrintable(scene.error()));
    QVERIFY(QTest::qWaitForWindowExposed(&scene.window()));
    const auto colorAt = [&](const qreal x, const qreal y) { return scene.grab().pixelColor(scene.pixel(x, y)); };

    // The first value holds up to the X of the data point without a Y, and the line resumes after it.
    QTRY_VERIFY(isColor(colorAt(4, 2), Qt::red));
    QVERIFY(isColor(colorAt(6, 2), Qt::black));
    QVERIFY(isColor(colorAt(5, 5), Qt::black));
    QVERIFY(isColor(colorAt(8.5, 4), Qt::red));
    QVERIFY(isColor(colorAt(9, 5), Qt::red));
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::LineCurveStepsTest)
#include "tst_line_curve_steps.moc"
