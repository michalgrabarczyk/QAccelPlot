//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/effects/GradientFill.hpp"
#include "QAccelPlot/effects/GradientStroke.hpp"
#include "QAccelPlot/effects/internal/GradientCoordinates.hpp"

#include <QtTest/QtTest>

using namespace QAccelPlot;

namespace {

constexpr auto kDataMin = qreal{-4.0};
constexpr auto kDataMax = qreal{12.0};
constexpr auto kFixedValue = qreal{3.5};

// Minimal stand-in for a QML GradientStop, read through its meta-object properties.
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

QObject* createGradient(QObject* parent)
{
    auto* gradient = new QObject{parent};
    new TestGradientStop{0.0, Qt::blue, gradient};
    new TestGradientStop{1.0, Qt::red, gradient};
    return gradient;
}

enum class Bound { Min, Max };

// Sets the bound named by fixed to kFixedValue and leaves the other unset.
template <typename Effect> void configureFixedBound(Effect& effect, const Bound fixed)
{
    effect.setGradient(createGradient(&effect));
    if (fixed == Bound::Min) {
        effect.setValueMin(kFixedValue);
    } else {
        effect.setValueMax(kFixedValue);
    }
}

template <typename Effect> void verifyBoundsReset()
{
    auto effect = Effect{};
    effect.setGradient(createGradient(&effect));
    QVERIFY(std::isnan(effect.valueMin()));
    QVERIFY(std::isnan(effect.valueMax()));
    QVERIFY(!effect.payload().gradientValueMin.has_value());

    auto minSpy = QSignalSpy{&effect, &Effect::valueMinChanged};
    effect.setValueMin(kFixedValue);
    QCOMPARE(effect.payload().gradientValueMin, std::optional<qreal>{kFixedValue});
    effect.resetValueMin();
    QVERIFY(std::isnan(effect.valueMin()));
    QVERIFY(!effect.payload().gradientValueMin.has_value());
    effect.setValueMin(std::numeric_limits<qreal>::quiet_NaN());
    QCOMPARE(minSpy.count(), 2);

    effect.setValueMax(kFixedValue);
    effect.setValueMax(std::numeric_limits<qreal>::quiet_NaN());
    QVERIFY(std::isnan(effect.valueMax()));
    QVERIFY(!effect.payload().gradientValueMax.has_value());
}

} // namespace

class TestGradientValueRange : public QObject {
    Q_OBJECT

private slots:
    void strokeFixedMinDataRangeMax_keepsFixedMin();
    void strokeDataRangeMinFixedMax_keepsFixedMax();
    void fillFixedMinDataRangeMax_keepsFixedMin();
    void fillDataRangeMinFixedMax_keepsFixedMax();
    void strokeBoundsAreUnsetByDefaultAndReset();
    void fillBoundsAreUnsetByDefaultAndReset();
    void fillBaselineIsUnsetByDefaultAndReset();
    void fixedMinAboveData_doesNotReverseRange();
    void fixedMaxBelowData_doesNotReverseRange();
    void fixedReversedBounds_areKept();
};

void TestGradientValueRange::strokeFixedMinDataRangeMax_keepsFixedMin()
{
    auto stroke = GradientStroke{};
    configureFixedBound(stroke, Bound::Min);

    auto payload = stroke.payload();
    QVERIFY(payload.isValid());
    Internal::resolveGradientValueRange(payload, kDataMin, kDataMax);

    QCOMPARE(payload.gradientValueMin, std::optional<qreal>{kFixedValue});
    QCOMPARE(payload.gradientValueMax, std::optional<qreal>{kDataMax});
}

void TestGradientValueRange::strokeDataRangeMinFixedMax_keepsFixedMax()
{
    auto stroke = GradientStroke{};
    configureFixedBound(stroke, Bound::Max);

    auto payload = stroke.payload();
    QVERIFY(payload.isValid());
    Internal::resolveGradientValueRange(payload, kDataMin, kDataMax);

    QCOMPARE(payload.gradientValueMin, std::optional<qreal>{kDataMin});
    QCOMPARE(payload.gradientValueMax, std::optional<qreal>{kFixedValue});
}

void TestGradientValueRange::fillFixedMinDataRangeMax_keepsFixedMin()
{
    auto fill = GradientFill{};
    configureFixedBound(fill, Bound::Min);

    auto payload = fill.payload();
    QVERIFY(payload.isValid());
    Internal::resolveGradientValueRange(payload, kDataMin, kDataMax);

    QCOMPARE(payload.gradientValueMin, std::optional<qreal>{kFixedValue});
    QCOMPARE(payload.gradientValueMax, std::optional<qreal>{kDataMax});
}

void TestGradientValueRange::fillDataRangeMinFixedMax_keepsFixedMax()
{
    auto fill = GradientFill{};
    configureFixedBound(fill, Bound::Max);

    auto payload = fill.payload();
    QVERIFY(payload.isValid());
    Internal::resolveGradientValueRange(payload, kDataMin, kDataMax);

    QCOMPARE(payload.gradientValueMin, std::optional<qreal>{kDataMin});
    QCOMPARE(payload.gradientValueMax, std::optional<qreal>{kFixedValue});
}

void TestGradientValueRange::strokeBoundsAreUnsetByDefaultAndReset()
{
    verifyBoundsReset<GradientStroke>();
}

void TestGradientValueRange::fillBoundsAreUnsetByDefaultAndReset()
{
    verifyBoundsReset<GradientFill>();
}

void TestGradientValueRange::fillBaselineIsUnsetByDefaultAndReset()
{
    auto fill = GradientFill{};
    QVERIFY(std::isnan(fill.baselineValue()));
    QVERIFY(!fill.payload().baselineValue.has_value());

    auto spy = QSignalSpy{&fill, &GradientFill::baselineValueChanged};
    fill.setBaselineValue(0.0);
    QCOMPARE(fill.payload().baselineValue, std::optional<qreal>{0.0});
    fill.resetBaselineValue();
    QVERIFY(std::isnan(fill.baselineValue()));
    QVERIFY(!fill.payload().baselineValue.has_value());
    fill.resetBaselineValue();
    QCOMPARE(spy.count(), 2);
}

void TestGradientValueRange::fixedMinAboveData_doesNotReverseRange()
{
    auto payload = GradientColorPayload{};
    payload.gradientValueMin = kDataMax + 1.0;
    Internal::resolveGradientValueRange(payload, kDataMin, kDataMax);

    QCOMPARE(payload.gradientValueMin, std::optional<qreal>{kDataMax + 1.0});
    QCOMPARE(payload.gradientValueMax, std::optional<qreal>{kDataMax + 1.0});
}

void TestGradientValueRange::fixedMaxBelowData_doesNotReverseRange()
{
    auto payload = GradientColorPayload{};
    payload.gradientValueMax = kDataMin - 1.0;
    Internal::resolveGradientValueRange(payload, kDataMin, kDataMax);

    QCOMPARE(payload.gradientValueMin, std::optional<qreal>{kDataMin - 1.0});
    QCOMPARE(payload.gradientValueMax, std::optional<qreal>{kDataMin - 1.0});
}

void TestGradientValueRange::fixedReversedBounds_areKept()
{
    auto payload = GradientColorPayload{};
    payload.gradientValueMin = kDataMax;
    payload.gradientValueMax = kDataMin;
    Internal::resolveGradientValueRange(payload, kDataMin, kDataMax);

    QCOMPARE(payload.gradientValueMin, std::optional<qreal>{kDataMax});
    QCOMPARE(payload.gradientValueMax, std::optional<qreal>{kDataMin});
}

QTEST_GUILESS_MAIN(TestGradientValueRange)
#include "tst_gradient_value_range.moc"
