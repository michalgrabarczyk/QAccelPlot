//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/GradientFill.hpp"
#include "QAccelPlot/effects/GradientStroke.hpp"
#include "QAccelPlot/linestyles/NoLine.hpp"
#include "QAccelPlot/series/LineCurve.hpp"

#include <QColor>
#include <QGuiApplication>
#include <QImage>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtTest/QtTest>

#include <array>
#include <cmath>
#include <limits>

namespace QAccelPlot {

enum class Style { Stroke, Markers, Fill };

// Positions are on a 0-100 grid along the gradient direction; the data value is origin + grid * scale.
struct RampCase {
    Style style{Style::Stroke};
    Qt::Orientation direction{Qt::Horizontal};
    qreal valueMin{std::numeric_limits<qreal>::quiet_NaN()};
    qreal valueMax{std::numeric_limits<qreal>::quiet_NaN()};
    qreal origin{0.0};
    qreal scale{1.0};
    // Gradient position expected at each of kSamples. NaN skips a sample.
    std::array<qreal, 3> expected{};
};

} // namespace QAccelPlot

Q_DECLARE_METATYPE(QAccelPlot::RampCase)

namespace QAccelPlot {

namespace {

constexpr auto kNaN = std::numeric_limits<qreal>::quiet_NaN();
constexpr auto kWindowSize = 200;
constexpr auto kGridMax = 100.0;
constexpr auto kPixelsPerGridUnit = 2.0;
constexpr auto kCrossPosition = 50.0;
// Inside the fill under a diagonal curve at every sampled position.
constexpr auto kDiagonalFillCrossPosition = 90.0;
constexpr auto kPositionTolerance = 0.05;
constexpr auto kSamples = std::array<qreal, 3>{25.0, 50.0, 75.0};

class TestGradientStop : public QObject {
    Q_OBJECT
    Q_PROPERTY(qreal position MEMBER position_ CONSTANT)
    Q_PROPERTY(QColor color MEMBER color_ CONSTANT)

public:
    TestGradientStop(const qreal position, const QColor& color, QObject* parent)
        : QObject(parent)
        , position_(position)
        , color_(color)
    {
    }

private:
    qreal position_{0.0};
    QColor color_;
};

QObject* createBlueToRedGradient(QObject* parent)
{
    auto* gradient = new QObject{parent};
    new TestGradientStop{0.0, QColor{0, 0, 255}, gradient};
    new TestGradientStop{1.0, QColor{255, 0, 0}, gradient};
    return gradient;
}

template <typename Effect> Effect* addEffect(LineCurve& curve, const RampCase& rampCase)
{
    auto* effect = new Effect{&curve};
    effect->setDirection(rampCase.direction);
    effect->setGradient(createBlueToRedGradient(effect));
    effect->setValueMin(rampCase.valueMin);
    effect->setValueMax(rampCase.valueMax);
    auto effects = curve.effects();
    effects.append(&effects, effect);
    return effect;
}

QPointF dataPoint(const RampCase& rampCase, const qreal along, const qreal across)
{
    const auto value = rampCase.origin + along * rampCase.scale;
    return rampCase.direction == Qt::Horizontal ? QPointF{value, across} : QPointF{across, value};
}

QList<QPointF> curvePoints(const RampCase& rampCase)
{
    switch (rampCase.style) {
    case Style::Stroke:
        return {dataPoint(rampCase, 0.0, kCrossPosition), dataPoint(rampCase, kGridMax, kCrossPosition)};
    case Style::Markers: {
        auto points = QList<QPointF>{dataPoint(rampCase, 0.0, kCrossPosition), dataPoint(rampCase, kGridMax, kCrossPosition)};
        for (const auto sample : kSamples) {
            points.append(dataPoint(rampCase, sample, kCrossPosition));
        }
        return points;
    }
    case Style::Fill:
        if (rampCase.direction == Qt::Horizontal) {
            return {dataPoint(rampCase, 0.0, kGridMax), dataPoint(rampCase, kGridMax, kGridMax)};
        }
        return {dataPoint(rampCase, 0.0, 0.0), dataPoint(rampCase, kGridMax, kGridMax)};
    }
    return {};
}

void applyStyle(LineCurve& curve, const RampCase& rampCase)
{
    switch (rampCase.style) {
    case Style::Stroke:
        curve.setLineWidth(8.0);
        addEffect<GradientStroke>(curve, rampCase);
        break;
    case Style::Markers:
        curve.setLineStyle(new NoLine{&curve});
        curve.marker()->setShape(MarkerShape::Square);
        curve.marker()->setSize(12.0);
        addEffect<GradientStroke>(curve, rampCase);
        break;
    case Style::Fill: {
        auto* fill = addEffect<GradientFill>(curve, rampCase);
        fill->setOpacity(1.0);
        fill->setBaselineValue(rampCase.direction == Qt::Vertical ? rampCase.origin : 0.0);
        break;
    }
    }
}

QImage renderRamp(const RampCase& rampCase, bool& softwareBackend)
{
    auto window = QQuickWindow{};
    window.setColor(Qt::black);
    window.resize(kWindowSize, kWindowSize);

    auto xAxis = Axis{};
    auto yAxis = Axis{};
    xAxis.setOrientation(Qt::Horizontal);
    yAxis.setOrientation(Qt::Vertical);
    auto& gradientAxis = rampCase.direction == Qt::Horizontal ? xAxis : yAxis;
    auto& crossAxis = rampCase.direction == Qt::Horizontal ? yAxis : xAxis;
    gradientAxis.setViewportRange(rampCase.origin, rampCase.origin + kGridMax * rampCase.scale);
    crossAxis.setViewportRange(0.0, kGridMax);

    auto curve = LineCurve{window.contentItem()};
    curve.setSize(QSizeF{kWindowSize, kWindowSize});
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);
    curve.setAntialiasing(false);
    applyStyle(curve, rampCase);
    curve.setData(curvePoints(rampCase));

    // The window stays hidden; grabWindow() renders it offscreen.
    const auto image = window.grabWindow();
    softwareBackend = window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software;
    return image;
}

QPoint samplePixel(const RampCase& rampCase, const qreal along)
{
    const auto diagonalFill = rampCase.style == Style::Fill && rampCase.direction == Qt::Vertical;
    const auto across = diagonalFill ? kDiagonalFillCrossPosition : kCrossPosition;
    const auto alongPixel = static_cast<int>(along * kPixelsPerGridUnit);
    const auto acrossPixel = static_cast<int>(across * kPixelsPerGridUnit);
    return rampCase.direction == Qt::Horizontal ? QPoint{alongPixel, kWindowSize - acrossPixel} : QPoint{acrossPixel, kWindowSize - alongPixel};
}

const char* styleName(const Style style)
{
    switch (style) {
    case Style::Stroke:
        return "stroke";
    case Style::Markers:
        return "markers";
    case Style::Fill:
        return "fill";
    }
    return "";
}

// Adds a row per style and direction. \a horizontal holds the positions of a horizontal gradient;
// a vertical gradient runs the other way, because its position 0 is the largest value.
void addRampRows(const char* name, const qreal valueMin, const qreal valueMax, const qreal origin, const qreal scale, const std::array<qreal, 3>& horizontal)
{
    for (const auto style : {Style::Stroke, Style::Markers, Style::Fill}) {
        for (const auto direction : {Qt::Horizontal, Qt::Vertical}) {
            auto rampCase = RampCase{style, direction, valueMin, valueMax, origin, scale, horizontal};
            if (direction == Qt::Vertical) {
                for (auto& position : rampCase.expected) {
                    position = 1.0 - position;
                }
            }
            QTest::addRow("%s/%s/%s", name, styleName(style), direction == Qt::Horizontal ? "horizontal" : "vertical") << rampCase;
        }
    }
}

} // namespace

class GradientRangeRenderingTest : public QObject {
    Q_OBJECT

private slots:
    void mapsValuesAcrossRamp_data();
    void mapsValuesAcrossRamp();
};

void GradientRangeRenderingTest::mapsValuesAcrossRamp_data()
{
    QTest::addColumn<RampCase>("rampCase");

    addRampRows("forward", 0.0, 100.0, 0.0, 1.0, {0.25, 0.5, 0.75});
    addRampRows("reversed", 100.0, 0.0, 0.0, 1.0, {0.75, 0.5, 0.25});
    addRampRows("smallDataRange", kNaN, kNaN, 1e-8, 1e-10, {0.25, 0.5, 0.75});
    addRampRows("zeroRange", 50.0, 50.0, 0.0, 1.0, {0.0, kNaN, 1.0});
    addRampRows("minAboveData", 200.0, kNaN, 0.0, 1.0, {0.0, 0.0, 0.0});
    addRampRows("maxBelowData", kNaN, -50.0, 0.0, 1.0, {1.0, 1.0, 1.0});
}

void GradientRangeRenderingTest::mapsValuesAcrossRamp()
{
    QFETCH(RampCase, rampCase);

    auto softwareBackend = false;
    const auto image = renderRamp(rampCase, softwareBackend);
    if (softwareBackend) {
        QSKIP("Custom materials do not render with the software scene graph backend");
    }
    QVERIFY(!image.isNull());

    for (auto i = std::size_t{0}; i < kSamples.size(); ++i) {
        if (std::isnan(rampCase.expected[i])) {
            continue;
        }
        const auto pixel = samplePixel(rampCase, kSamples[i]);
        const auto color = image.pixelColor(pixel);
        const auto coverage = color.red() + color.blue();
        QVERIFY2(coverage > 200, qPrintable(QStringLiteral("nothing drawn at grid %1: %2").arg(kSamples[i]).arg(color.name())));
        const auto position = static_cast<qreal>(color.red()) / coverage;
        QVERIFY2(std::abs(position - rampCase.expected[i]) <= kPositionTolerance,
            qPrintable(QStringLiteral("grid %1: position %2, expected %3 (%4)").arg(kSamples[i]).arg(position).arg(rampCase.expected[i]).arg(color.name())));
    }
}

} // namespace QAccelPlot

int main(int argc, char* argv[])
{
    // Sample positions are logical pixels, so render one device pixel per logical pixel.
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    auto app = QGuiApplication{argc, argv};
    auto test = QAccelPlot::GradientRangeRenderingTest{};
    return QTest::qExec(&test, argc, argv);
}

#include "tst_gradient_range_rendering.moc"
