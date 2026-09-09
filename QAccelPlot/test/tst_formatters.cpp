//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "AxisTicker.hpp"
#include "DateTimeTickLabelFormatter.hpp"
#include "LogTickLabelFormatter.hpp"
#include "NumericTickLabelFormatter.hpp"
#include "TextTickLabelFormatter.hpp"

#include <cmath>
#include <memory>

#include <QDateTime>
#include <QSignalSpy>
#include <QtTest/QtTest>

class TestFormatters : public QObject {
    Q_OBJECT

private slots:
    // NumericTickLabelFormatter
    void numeric_precision_fromTickStep();
    void numeric_zeroTickStep_usesPrecisionOne();
    void numeric_largeTickStep_zeroPrecision();
    void numeric_negativeValue();

    // DateTimeTickLabelFormatter
    void datetime_defaultFormat();
    void datetime_customFormat();
    void datetime_formatChanged_emitsSignal();

    // LogTickLabelFormatter
    void log_exactPowerOfTen_exponentialNotation();
    void log_nonPowerOfTen_generalNotation();
    void log_negativePower();

    // TextTickLabelFormatter
    void text_validIndex_returnsLabel();
    void text_outOfBounds_returnsEmpty();
    void text_negativeIndex_returnsEmpty();
    void text_emptyList_returnsEmpty();
    void text_labelsChanged_emitsSignal();

    // Formatter change propagation
    void ticker_formatterPropertyChange_emitsSignal();
    void ticker_formatterReplacement_emitsSignal();
    void ticker_nullFormatter_usesDefault();
    void ticker_destroyedFormatter_usesDefault();

    // Corner cases
    void numeric_zeroValue_precisionZero();
    void numeric_verySmallTickStep_highPrecision();
    void log_nearPowerBelowThreshold_usesExponential();
    void log_nearPowerAboveThreshold_usesGeneral();
    void text_fractionalValueRoundsToIndex();
};

// ---------------------------------------------------------------------------
// NumericTickLabelFormatter
// ---------------------------------------------------------------------------

void TestFormatters::numeric_precision_fromTickStep()
{
    auto fmt = QAccelPlot::NumericTickLabelFormatter{};
    // tickStep=0.1 → precision=2; 1.23456 rounded to 2dp = "1.23"
    QCOMPARE(fmt.format(1.23456, 0.1), QStringLiteral("1.23"));
}

void TestFormatters::numeric_zeroTickStep_usesPrecisionOne()
{
    auto fmt = QAccelPlot::NumericTickLabelFormatter{};
    // tickStep <= 0 → precision=1
    QCOMPARE(fmt.format(1.5, 0.0), QStringLiteral("1.5"));
}

void TestFormatters::numeric_largeTickStep_zeroPrecision()
{
    auto fmt = QAccelPlot::NumericTickLabelFormatter{};
    // tickStep=10.0 → -log10(10)+0.5 = -0.5, ceil(-0.5)=0 → precision=0
    QCOMPARE(fmt.format(1000.0, 10.0), QStringLiteral("1000"));
}

void TestFormatters::numeric_negativeValue()
{
    auto fmt = QAccelPlot::NumericTickLabelFormatter{};
    QCOMPARE(fmt.format(-0.5, 0.1), QStringLiteral("-0.50"));
}

// ---------------------------------------------------------------------------
// DateTimeTickLabelFormatter
// ---------------------------------------------------------------------------

void TestFormatters::datetime_defaultFormat()
{
    auto fmt = QAccelPlot::DateTimeTickLabelFormatter{};
    // Use the same conversion the implementation uses, so the test is TZ-agnostic.
    const auto ms = qint64{1000000000000};
    const auto expected = QDateTime::fromMSecsSinceEpoch(ms).toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    QCOMPARE(fmt.format(static_cast<qreal>(ms), 1.0), expected);
}

void TestFormatters::datetime_customFormat()
{
    auto fmt = QAccelPlot::DateTimeTickLabelFormatter{};
    fmt.setDateTimeFormat(QStringLiteral("yyyy"));
    const auto ms = qint64{0};
    const auto expected = QDateTime::fromMSecsSinceEpoch(ms).toString(QStringLiteral("yyyy"));
    QCOMPARE(fmt.format(0.0, 1.0), expected);
}

void TestFormatters::datetime_formatChanged_emitsSignal()
{
    auto fmt = QAccelPlot::DateTimeTickLabelFormatter{};
    auto spy = QSignalSpy{&fmt, &QAccelPlot::DateTimeTickLabelFormatter::dateTimeFormatChanged};
    fmt.setDateTimeFormat(QStringLiteral("dd/MM/yyyy"));
    QCOMPARE(spy.count(), 1);
    // Setting the same value should not emit again.
    fmt.setDateTimeFormat(QStringLiteral("dd/MM/yyyy"));
    QCOMPARE(spy.count(), 1);
}

// ---------------------------------------------------------------------------
// LogTickLabelFormatter
// ---------------------------------------------------------------------------

void TestFormatters::log_exactPowerOfTen_exponentialNotation()
{
    auto fmt = QAccelPlot::LogTickLabelFormatter{};
    QCOMPARE(fmt.format(1.0, 1.0), QStringLiteral("1e0"));
    QCOMPARE(fmt.format(100.0, 1.0), QStringLiteral("1e2"));
    QCOMPARE(fmt.format(1000.0, 1.0), QStringLiteral("1e3"));
}

void TestFormatters::log_nonPowerOfTen_generalNotation()
{
    auto fmt = QAccelPlot::LogTickLabelFormatter{};
    // 50 → log10(50)≈1.699, |1.699-2|=0.301 > 0.01 → 'g' notation
    QCOMPARE(fmt.format(50.0, 1.0), QStringLiteral("50"));
}

void TestFormatters::log_negativePower()
{
    auto fmt = QAccelPlot::LogTickLabelFormatter{};
    QCOMPARE(fmt.format(0.001, 1.0), QStringLiteral("1e-3"));
}

// ---------------------------------------------------------------------------
// TextTickLabelFormatter
// ---------------------------------------------------------------------------

void TestFormatters::text_validIndex_returnsLabel()
{
    auto fmt = QAccelPlot::TextTickLabelFormatter{};
    fmt.setLabels({QStringLiteral("alpha"), QStringLiteral("beta"), QStringLiteral("gamma")});
    QCOMPARE(fmt.format(0.0, 1.0), QStringLiteral("alpha"));
    QCOMPARE(fmt.format(1.0, 1.0), QStringLiteral("beta"));
    QCOMPARE(fmt.format(2.0, 1.0), QStringLiteral("gamma"));
}

void TestFormatters::text_outOfBounds_returnsEmpty()
{
    auto fmt = QAccelPlot::TextTickLabelFormatter{};
    fmt.setLabels({QStringLiteral("only")});
    QCOMPARE(fmt.format(5.0, 1.0), QString{});
}

void TestFormatters::text_negativeIndex_returnsEmpty()
{
    auto fmt = QAccelPlot::TextTickLabelFormatter{};
    fmt.setLabels({QStringLiteral("a"), QStringLiteral("b")});
    QCOMPARE(fmt.format(-1.0, 1.0), QString{});
}

void TestFormatters::text_emptyList_returnsEmpty()
{
    auto fmt = QAccelPlot::TextTickLabelFormatter{};
    QCOMPARE(fmt.format(0.0, 1.0), QString{});
}

void TestFormatters::text_labelsChanged_emitsSignal()
{
    auto fmt = QAccelPlot::TextTickLabelFormatter{};
    auto spy = QSignalSpy{&fmt, &QAccelPlot::TextTickLabelFormatter::labelsChanged};
    fmt.setLabels({QStringLiteral("x")});
    QCOMPARE(spy.count(), 1);
    // Same value: no signal.
    fmt.setLabels({QStringLiteral("x")});
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
    QCOMPARE(ticker.tickLabelFormatter()->format(1.25, 0.1), QStringLiteral("1.25"));
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

void TestFormatters::numeric_zeroValue_precisionZero()
{
    auto fmt = QAccelPlot::NumericTickLabelFormatter{};
    // tickStep=10 → precision = ceil(-log10(10)+0.5) = ceil(-0.5) = 0 → "0"
    QCOMPARE(fmt.format(0.0, 10.0), QStringLiteral("0"));
}

void TestFormatters::numeric_verySmallTickStep_highPrecision()
{
    auto fmt = QAccelPlot::NumericTickLabelFormatter{};
    // tickStep=1e-4 → precision = ceil(-log10(1e-4)+0.5) = ceil(4.5) = 5
    QCOMPARE(fmt.format(0.123456789, 1e-4), QStringLiteral("0.12346"));
}

void TestFormatters::log_nearPowerBelowThreshold_usesExponential()
{
    auto fmt = QAccelPlot::LogTickLabelFormatter{};
    // value = 10^1.005 → logValue≈1.005, |1.005-1|=0.005 < 0.01 → "1e1"
    const auto value = std::pow(10.0, 1.005);
    QCOMPARE(fmt.format(value, 1.0), QStringLiteral("1e1"));
}

void TestFormatters::log_nearPowerAboveThreshold_usesGeneral()
{
    auto fmt = QAccelPlot::LogTickLabelFormatter{};
    // value = 10^1.02 → logValue≈1.02, |1.02-1|=0.02 > 0.01 → general 'g' notation
    const auto value = std::pow(10.0, 1.02);
    QCOMPARE(fmt.format(value, 1.0), QString::number(value, 'g', 3));
}

void TestFormatters::text_fractionalValueRoundsToIndex()
{
    auto fmt = QAccelPlot::TextTickLabelFormatter{};
    fmt.setLabels({QStringLiteral("zero"), QStringLiteral("one"), QStringLiteral("two")});
    // qRound(1.6)==2 → "two";  qRound(0.4)==0 → "zero"
    QCOMPARE(fmt.format(1.6, 1.0), QStringLiteral("two"));
    QCOMPARE(fmt.format(0.4, 1.0), QStringLiteral("zero"));
}

QTEST_GUILESS_MAIN(TestFormatters)
#include "tst_formatters.moc"
