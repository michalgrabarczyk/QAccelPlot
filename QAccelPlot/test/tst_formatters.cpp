//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/AxisTicker.hpp"
#include "QAccelPlot/formatters/DateTimeTickLabelFormatter.hpp"
#include "QAccelPlot/formatters/LogTickLabelFormatter.hpp"
#include "QAccelPlot/formatters/NumericTickLabelFormatter.hpp"
#include "QAccelPlot/formatters/TextTickLabelFormatter.hpp"

#include <QDateTime>
#include <QJSEngine>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include <cmath>
#include <memory>

class TestFormatters : public QObject {
    Q_OBJECT

private slots:
    void numericFormat_data();
    void numericFormat();
    void numericLogTick_data();
    void numericLogTick();
    void logFormatter_logTickKeepsSuperscript();
    void tickLabelCallback_logTickReceivesValueAsStep();

    void datetime_defaultFormat();
    void datetime_customFormat();
    void datetime_formatChanged_emitsSignal();

    void logFormat_data();
    void logFormat();

    void textFormat_data();
    void textFormat();
    void text_labelsChanged_emitsSignal();

    void ticker_formatterPropertyChange_emitsSignal();
    void ticker_formatterReplacement_emitsSignal();
    void ticker_nullFormatter_usesDefault();
    void ticker_destroyedFormatter_usesDefault();

    void tickLabelCallback_receivesValueAndTickStep();
    void tickLabelCallback_errorFallsBackToFormatter();
    void tickLabel_nonCallableIsIgnored();
    void tickLabel_changeEmitsFormatChanged();
};

void TestFormatters::numericFormat_data()
{
    QTest::addColumn<qreal>("value");
    QTest::addColumn<qreal>("tickStep");
    QTest::addColumn<QString>("expected");

    QTest::newRow("fractional-step") << 1.23456 << 0.1 << QStringLiteral("1.2");
    QTest::newRow("zero-step") << 1.5 << 0.0 << QStringLiteral("1.5");
    QTest::newRow("infinite-step") << 1.5 << qInf() << QStringLiteral("1.5");
    QTest::newRow("large-step") << 1000.0 << 10.0 << QStringLiteral("1000");
    QTest::newRow("negative-value") << -0.5 << 0.1 << QStringLiteral("-0.5");
    QTest::newRow("zero-value") << 0.0 << 10.0 << QStringLiteral("0");
    QTest::newRow("very-small-step") << 0.123456789 << 1e-4 << QStringLiteral("0.1235");
    QTest::newRow("tiny-negative-rounds-to-positive-zero") << -1e-10 << 0.1 << QStringLiteral("0.0");
    QTest::newRow("five-step") << 15.0 << 5.0 << QStringLiteral("15");
    QTest::newRow("quarter-step") << 0.75 << 0.25 << QStringLiteral("0.75");
    QTest::newRow("negative-step") << 2.0 << -1.0 << QStringLiteral("2.0");
    QTest::newRow("nan-step") << 3.0 << qQNaN() << QStringLiteral("3.0");

    // Nice steps produced by AxisTickPainter::computeNiceStep (1, 2, 5 x 10^n) and
    // other terminating steps get exactly the decimals needed to write the step.
    QTest::newRow("step-100") << 300.0 << 100.0 << QStringLiteral("300");
    QTest::newRow("step-10") << 20.0 << 10.0 << QStringLiteral("20");
    QTest::newRow("step-5") << 10.0 << 5.0 << QStringLiteral("10");
    QTest::newRow("step-2") << 4.0 << 2.0 << QStringLiteral("4");
    QTest::newRow("step-1") << 2.0 << 1.0 << QStringLiteral("2");
    QTest::newRow("step-0.5") << 1.0 << 0.5 << QStringLiteral("1.0");
    QTest::newRow("step-0.2") << 0.4 << 0.2 << QStringLiteral("0.4");
    QTest::newRow("step-0.1") << 0.2 << 0.1 << QStringLiteral("0.2");
    QTest::newRow("step-0.25") << 0.75 << 0.25 << QStringLiteral("0.75");
    QTest::newRow("step-0.05") << 0.15 << 0.05 << QStringLiteral("0.15");
    QTest::newRow("step-0.01") << 0.03 << 0.01 << QStringLiteral("0.03");
    QTest::newRow("step-2.5") << 7.5 << 2.5 << QStringLiteral("7.5");

    // Steps carrying floating-point noise must not gain an extra decimal place.
    QTest::newRow("noisy-step-0.1-add") << 0.3 << (0.1 + 0.2 - 0.2) << QStringLiteral("0.3");
    QTest::newRow("noisy-step-0.1-mul") << 0.3 << (3.0 * 0.1 / 3.0) << QStringLiteral("0.3");
    QTest::newRow("noisy-step-0.3") << 0.6 << (0.1 + 0.2) << QStringLiteral("0.6");
    QTest::newRow("noisy-step-0.001") << 0.002 << (0.1 * 0.01) << QStringLiteral("0.002");

    // A non-terminating step is cut off two places past its leading digit.
    QTest::newRow("non-terminating-step") << (2.0 / 3.0) << (1.0 / 3.0) << QStringLiteral("0.667");
}

void TestFormatters::numericFormat()
{
    QFETCH(qreal, value);
    QFETCH(qreal, tickStep);
    QFETCH(QString, expected);

    const auto formatter = QAccelPlot::NumericTickLabelFormatter{};
    QCOMPARE(formatter.format(value, tickStep), expected);
}

void TestFormatters::numericLogTick_data()
{
    QTest::addColumn<qreal>("value");
    QTest::addColumn<QString>("expected");

    QTest::newRow("1e-3") << 0.001 << QStringLiteral("10\u207B\u00B3");
    QTest::newRow("1e-2") << 0.01 << QStringLiteral("10\u207B\u00B2");
    QTest::newRow("1e-1") << 0.1 << QStringLiteral("10\u207B\u00B9");
    QTest::newRow("1e0") << 1.0 << QStringLiteral("10\u2070");
    QTest::newRow("1e1") << 10.0 << QStringLiteral("10\u00B9");
    QTest::newRow("1e2") << 100.0 << QStringLiteral("10\u00B2");
    QTest::newRow("1e12") << 1e12 << QStringLiteral("10\u00B9\u00B2");
    QTest::newRow("pow-negative-exponent") << std::pow(10.0, -5) << QStringLiteral("10\u207B\u2075");
    QTest::newRow("non-decade") << 50.0 << QStringLiteral("50");
}

void TestFormatters::numericLogTick()
{
    QFETCH(qreal, value);
    QFETCH(QString, expected);

    const auto formatter = QAccelPlot::NumericTickLabelFormatter{};
    QCOMPARE(formatter.formatLogTick(value), expected);
}

void TestFormatters::logFormatter_logTickKeepsSuperscript()
{
    const auto formatter = QAccelPlot::LogTickLabelFormatter{};
    QCOMPARE(formatter.formatLogTick(100.0), QStringLiteral("10\u00B2"));
    QCOMPARE(formatter.formatLogTick(50.0), QStringLiteral("50"));
}

void TestFormatters::tickLabelCallback_logTickReceivesValueAsStep()
{
    auto engine = QJSEngine{};
    auto formatter = QAccelPlot::NumericTickLabelFormatter{};
    formatter.setTickLabel(engine.evaluate(QStringLiteral("(function(value, tickStep) { return value + '/' + tickStep; })")));

    QCOMPARE(formatter.formatLogTick(100.0), QStringLiteral("100/100"));
}

void TestFormatters::datetime_defaultFormat()
{
    const auto formatter = QAccelPlot::DateTimeTickLabelFormatter{};
    const auto milliseconds = qint64{1000000000000};
    const auto expected = QDateTime::fromMSecsSinceEpoch(milliseconds).toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    QCOMPARE(formatter.format(static_cast<qreal>(milliseconds), 1.0), expected);
}

void TestFormatters::datetime_customFormat()
{
    auto formatter = QAccelPlot::DateTimeTickLabelFormatter{};
    formatter.setDateTimeFormat(QStringLiteral("yyyy"));
    const auto expected = QDateTime::fromMSecsSinceEpoch(0).toString(QStringLiteral("yyyy"));
    QCOMPARE(formatter.format(0.0, 1.0), expected);
}

void TestFormatters::datetime_formatChanged_emitsSignal()
{
    auto formatter = QAccelPlot::DateTimeTickLabelFormatter{};
    auto spy = QSignalSpy{&formatter, &QAccelPlot::DateTimeTickLabelFormatter::dateTimeFormatChanged};
    formatter.setDateTimeFormat(QStringLiteral("dd/MM/yyyy"));
    QCOMPARE(spy.count(), 1);

    formatter.setDateTimeFormat(QStringLiteral("dd/MM/yyyy"));
    QCOMPARE(spy.count(), 1);
}

void TestFormatters::logFormat_data()
{
    QTest::addColumn<qreal>("value");
    QTest::addColumn<QString>("expected");

    QTest::newRow("power-zero") << 1.0 << QStringLiteral("10\u2070");
    QTest::newRow("power-two") << 100.0 << QStringLiteral("10\u00B2");
    QTest::newRow("power-three") << 1000.0 << QStringLiteral("10\u00B3");
    QTest::newRow("non-power") << 50.0 << QStringLiteral("50");
    QTest::newRow("negative-power") << 0.001 << QStringLiteral("10\u207B\u00B3");
    QTest::newRow("multi-digit-power") << 1e12 << QStringLiteral("10\u00B9\u00B2");
    QTest::newRow("negative-multi-digit-power") << 1e-12 << QStringLiteral("10\u207B\u00B9\u00B2");

    const auto belowThreshold = std::pow(10.0, 1.005);
    const auto aboveThreshold = std::pow(10.0, 1.02);
    QTest::newRow("near-power-below-threshold") << belowThreshold << QStringLiteral("10\u00B9");
    QTest::newRow("near-power-above-threshold") << aboveThreshold << QString::number(aboveThreshold, 'g', 3);
    QTest::newRow("zero") << 0.0 << QStringLiteral("0");
    QTest::newRow("negative") << -5.0 << QStringLiteral("-5");
}

void TestFormatters::logFormat()
{
    QFETCH(qreal, value);
    QFETCH(QString, expected);

    const auto formatter = QAccelPlot::LogTickLabelFormatter{};
    QCOMPARE(formatter.format(value, 1.0), expected);
}

void TestFormatters::textFormat_data()
{
    QTest::addColumn<QStringList>("labels");
    QTest::addColumn<qreal>("value");
    QTest::addColumn<QString>("expected");

    const auto alphabet = QStringList{QStringLiteral("alpha"), QStringLiteral("beta"), QStringLiteral("gamma")};
    const auto numbers = QStringList{QStringLiteral("zero"), QStringLiteral("one"), QStringLiteral("two")};
    QTest::newRow("first-label") << alphabet << 0.0 << QStringLiteral("alpha");
    QTest::newRow("middle-label") << alphabet << 1.0 << QStringLiteral("beta");
    QTest::newRow("last-label") << alphabet << 2.0 << QStringLiteral("gamma");
    QTest::newRow("out-of-bounds") << QStringList{QStringLiteral("only")} << 5.0 << QString{};
    QTest::newRow("negative-index") << QStringList{QStringLiteral("a"), QStringLiteral("b")} << -1.0 << QString{};
    QTest::newRow("empty-list") << QStringList{} << 0.0 << QString{};
    QTest::newRow("fraction-rounds-up") << numbers << 1.6 << QStringLiteral("two");
    QTest::newRow("fraction-rounds-down") << numbers << 0.4 << QStringLiteral("zero");
    QTest::newRow("half-rounds-up") << numbers << 1.5 << QStringLiteral("two");
    QTest::newRow("small-negative-rounds-to-first") << numbers << -0.4 << QStringLiteral("zero");
    QTest::newRow("negative-half-is-out-of-range") << numbers << -0.5 << QString{};
    QTest::newRow("past-last-rounds-out-of-range") << numbers << 2.5 << QString{};
}

void TestFormatters::textFormat()
{
    QFETCH(QStringList, labels);
    QFETCH(qreal, value);
    QFETCH(QString, expected);

    auto formatter = QAccelPlot::TextTickLabelFormatter{};
    formatter.setLabels(labels);
    QCOMPARE(formatter.format(value, 1.0), expected);
}

void TestFormatters::text_labelsChanged_emitsSignal()
{
    auto formatter = QAccelPlot::TextTickLabelFormatter{};
    auto spy = QSignalSpy{&formatter, &QAccelPlot::TextTickLabelFormatter::labelsChanged};
    formatter.setLabels({QStringLiteral("x")});
    QCOMPARE(spy.count(), 1);

    formatter.setLabels({QStringLiteral("x")});
    QCOMPARE(spy.count(), 1);
}

void TestFormatters::ticker_formatterPropertyChange_emitsSignal()
{
    auto ticker = QAccelPlot::AxisTicker{};
    auto formatter = QAccelPlot::DateTimeTickLabelFormatter{};
    ticker.setTickLabelFormatter(&formatter);
    auto spy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickLabelFormatChanged};

    formatter.setDateTimeFormat(QStringLiteral("hh:mm:ss"));

    QCOMPARE(spy.count(), 1);
}

void TestFormatters::ticker_formatterReplacement_emitsSignal()
{
    auto ticker = QAccelPlot::AxisTicker{};
    auto formatter = QAccelPlot::DateTimeTickLabelFormatter{};
    auto spy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickLabelFormatterChanged};

    ticker.setTickLabelFormatter(&formatter);

    QCOMPARE(spy.count(), 1);
}

void TestFormatters::ticker_nullFormatter_usesDefault()
{
    auto ticker = QAccelPlot::AxisTicker{};

    ticker.setTickLabelFormatter(nullptr);

    QVERIFY(ticker.tickLabelFormatter() != nullptr);
    QCOMPARE(ticker.tickLabelFormatter()->format(1.25, 0.25), QStringLiteral("1.25"));
}

void TestFormatters::ticker_destroyedFormatter_usesDefault()
{
    auto ticker = QAccelPlot::AxisTicker{};
    auto formatterChangedSpy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickLabelFormatterChanged};
    auto* formatterAddress = static_cast<QAccelPlot::TickLabelFormatter*>(nullptr);

    {
        auto formatter = std::make_unique<QAccelPlot::DateTimeTickLabelFormatter>();
        formatterAddress = formatter.get();
        ticker.setTickLabelFormatter(formatter.get());
        QCOMPARE(ticker.tickLabelFormatter(), formatterAddress);
    }

    QVERIFY(ticker.tickLabelFormatter() != nullptr);
    QVERIFY(ticker.tickLabelFormatter() != formatterAddress);
    QCOMPARE(formatterChangedSpy.count(), 2);
}

void TestFormatters::tickLabelCallback_receivesValueAndTickStep()
{
    auto engine = QJSEngine{};
    auto formatter = QAccelPlot::NumericTickLabelFormatter{};
    formatter.setTickLabel(engine.evaluate(QStringLiteral("(function(value, tickStep) { return value + '|' + tickStep; })")));

    QCOMPARE(formatter.format(1.5, 0.1), QStringLiteral("1.5|0.1"));
}

void TestFormatters::tickLabelCallback_errorFallsBackToFormatter()
{
    auto engine = QJSEngine{};
    auto formatter = QAccelPlot::NumericTickLabelFormatter{};
    formatter.setTickLabel(engine.evaluate(QStringLiteral("(function() { throw new Error('bad label'); })")));

    QCOMPARE(formatter.format(1.25, 0.25), QStringLiteral("1.25"));
}

void TestFormatters::tickLabel_nonCallableIsIgnored()
{
    auto formatter = QAccelPlot::NumericTickLabelFormatter{};
    formatter.setTickLabel(QJSValue{QStringLiteral("not a function")});

    QCOMPARE(formatter.format(1.25, 0.25), QStringLiteral("1.25"));
}

void TestFormatters::tickLabel_changeEmitsFormatChanged()
{
    auto engine = QJSEngine{};
    auto formatter = QAccelPlot::LogTickLabelFormatter{};
    auto spy = QSignalSpy{&formatter, &QAccelPlot::TickLabelFormatter::formatChanged};

    formatter.setTickLabel(engine.evaluate(QStringLiteral("(function(value) { return 'v' + value; })")));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(formatter.format(100.0, 1.0), QStringLiteral("v100"));

    formatter.setTickLabel(QJSValue{});
    QCOMPARE(spy.count(), 2);
    QCOMPARE(formatter.format(100.0, 1.0), QStringLiteral("10²"));
}

QTEST_GUILESS_MAIN(TestFormatters)
#include "tst_formatters.moc"
