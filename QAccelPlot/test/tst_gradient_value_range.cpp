//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/effects/GradientCoordinateUtils.hpp"
#include "QAccelPlot/effects/GradientFill.hpp"
#include "QAccelPlot/effects/GradientStroke.hpp"

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

template <typename Effect> void configureRangeSources(Effect& effect, const GradientValueSource minSource, const GradientValueSource maxSource)
{
    effect.setGradient(createGradient(&effect));
    effect.setGradientValueMinSource(minSource);
    effect.setGradientValueMin(kFixedValue);
    effect.setGradientValueMaxSource(maxSource);
    effect.setGradientValueMax(kFixedValue);
}

} // namespace

class TestGradientValueRange : public QObject {
    Q_OBJECT

private slots:
    void strokeFixedMinDataRangeMax_keepsFixedMin();
    void strokeDataRangeMinFixedMax_keepsFixedMax();
    void fillFixedMinDataRangeMax_keepsFixedMin();
    void fillDataRangeMinFixedMax_keepsFixedMax();
};

void TestGradientValueRange::strokeFixedMinDataRangeMax_keepsFixedMin()
{
    auto stroke = GradientStroke{};
    configureRangeSources(stroke, GradientValueSource::Fixed, GradientValueSource::DataRange);

    auto payload = stroke.payload();
    QVERIFY(payload.isValid());
    resolveGradientValueRange(payload, kDataMin, kDataMax);

    QCOMPARE(payload.gradientValueMin, std::optional<qreal>{kFixedValue});
    QCOMPARE(payload.gradientValueMax, std::optional<qreal>{kDataMax});
}

void TestGradientValueRange::strokeDataRangeMinFixedMax_keepsFixedMax()
{
    auto stroke = GradientStroke{};
    configureRangeSources(stroke, GradientValueSource::DataRange, GradientValueSource::Fixed);

    auto payload = stroke.payload();
    QVERIFY(payload.isValid());
    resolveGradientValueRange(payload, kDataMin, kDataMax);

    QCOMPARE(payload.gradientValueMin, std::optional<qreal>{kDataMin});
    QCOMPARE(payload.gradientValueMax, std::optional<qreal>{kFixedValue});
}

void TestGradientValueRange::fillFixedMinDataRangeMax_keepsFixedMin()
{
    auto fill = GradientFill{};
    configureRangeSources(fill, GradientValueSource::Fixed, GradientValueSource::DataRange);

    auto payload = fill.payload();
    QVERIFY(payload.isValid());
    resolveGradientValueRange(payload, kDataMin, kDataMax);

    QCOMPARE(payload.gradientValueMin, std::optional<qreal>{kFixedValue});
    QCOMPARE(payload.gradientValueMax, std::optional<qreal>{kDataMax});
}

void TestGradientValueRange::fillDataRangeMinFixedMax_keepsFixedMax()
{
    auto fill = GradientFill{};
    configureRangeSources(fill, GradientValueSource::DataRange, GradientValueSource::Fixed);

    auto payload = fill.payload();
    QVERIFY(payload.isValid());
    resolveGradientValueRange(payload, kDataMin, kDataMax);

    QCOMPARE(payload.gradientValueMin, std::optional<qreal>{kDataMin});
    QCOMPARE(payload.gradientValueMax, std::optional<qreal>{kFixedValue});
}

QTEST_GUILESS_MAIN(TestGradientValueRange)
#include "tst_gradient_value_range.moc"
