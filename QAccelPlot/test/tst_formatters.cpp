//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "axis/AxisTicker.hpp"
#include "formatters/DateTimeTickLabelFormatter.hpp"
#include "formatters/LogTickLabelFormatter.hpp"
#include "formatters/NumericTickLabelFormatter.hpp"
#include "formatters/TextTickLabelFormatter.hpp"

#include <QDateTime>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include <cmath>
#include <memory>

class TestFormatters : public QObject {
    Q_OBJECT

private slots:
    void numericFormat_data();
    void numericFormat();

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
};

void TestFormatters::numericFormat_data()
{
    QTest::addColumn<qreal>("value");
    QTest::addColumn<qreal>("tickStep");
    QTest::addColumn<QString>("expected");

    QTest::newRow("fractional-step") << 1.23456 << 0.1 << QStringLiteral("1.23");
    QTest::newRow("zero-step") << 1.5 << 0.0 << QStringLiteral("1.5");
    QTest::newRow("large-step") << 1000.0 << 10.0 << QStringLiteral("1000");
    QTest::newRow("negative-value") << -0.5 << 0.1 << QStringLiteral("-0.50");
    QTest::newRow("zero-value") << 0.0 << 10.0 << QStringLiteral("0");
    QTest::newRow("very-small-step") << 0.123456789 << 1e-4 << QStringLiteral("0.12346");
    QTest::newRow("tiny-negative-rounds-to-positive-zero") << -1e-10 << 0.1 << QStringLiteral("0.00");
}

void TestFormatters::numericFormat()
{
    QFETCH(qreal, value);
    QFETCH(qreal, tickStep);
    QFETCH(QString, expected);

    const auto formatter = QAccelPlot::NumericTickLabelFormatter{};
    QCOMPARE(formatter.format(value, tickStep), expected);
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

QTEST_GUILESS_MAIN(TestFormatters)
#include "tst_formatters.moc"
