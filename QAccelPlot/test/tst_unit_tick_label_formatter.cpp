//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/axis/AxisTickPainter.hpp"
#include "QAccelPlot/axis/AxisTicker.hpp"
#include "QAccelPlot/formatters/UnitTickLabelFormatter.hpp"

#include <QJSEngine>
#include <QSignalSpy>
#include <QtTest/QtTest>

namespace QAccelPlot {

namespace {

QStringList tickLabels(const AxisTicks& ticks)
{
    auto labels = QStringList{};
    for (const auto& tick : ticks.majorTicks) {
        labels.append(tick.label);
    }
    return labels;
}

} // namespace

class UnitTickLabelFormatterTest : public QObject {
    Q_OBJECT

private slots:
    void sharedPrefixFollowsVisibleRange_data();
    void sharedPrefixFollowsVisibleRange();
    void labelsShareDecimalsOfPrefixedStep_data();
    void labelsShareDecimalsOfPrefixedStep();
    void tickLabelCallbackReceivesPrefixedValues();
    void unitChangeEmitsFormatChanged();
    void prefixModeDefaultsToLargestMagnitude();
    void allowedPrefixesRestrictTheSharedPrefix_data();
    void allowedPrefixesRestrictTheSharedPrefix();
    void formatValueFollowsTheAllowedPrefixes_data();
    void formatValueFollowsTheAllowedPrefixes();
    void allowedPrefixesReadBackSortedAndNormalized();
    void allowedPrefixesChangeEmitsFormatChanged();
    void allowedPrefixesResetRestoresEveryPowerOf1000();
};

void UnitTickLabelFormatterTest::sharedPrefixFollowsVisibleRange_data()
{
    QTest::addColumn<qreal>("viewportMin");
    QTest::addColumn<qreal>("viewportMax");
    QTest::addColumn<QStringList>("labels");
    QTest::addColumn<QString>("unitText");
    QTest::newRow("millimeters") << 0.0 << 0.004 << QStringList{"0", "1", "2", "3", "4"} << QStringLiteral("mm");
    QTest::newRow("micrometers") << 0.0 << 4e-6 << QStringList{"0", "1", "2", "3", "4"} << QStringLiteral("µm");
    QTest::newRow("nanometers") << 0.0 << 4e-9 << QStringList{"0", "1", "2", "3", "4"} << QStringLiteral("nm");
    QTest::newRow("meters") << 0.0 << 4.0 << QStringList{"0", "1", "2", "3", "4"} << QStringLiteral("m");
    QTest::newRow("kilometers") << 0.0 << 4000.0 << QStringList{"0", "1", "2", "3", "4"} << QStringLiteral("km");
    QTest::newRow("hundreds of meters") << 0.0 << 400.0 << QStringList{"0", "100", "200", "300", "400"} << QStringLiteral("m");
    QTest::newRow("negative range") << -4000.0 << 0.0 << QStringList{"-4", "-3", "-2", "-1", "0"} << QStringLiteral("km");
}

void UnitTickLabelFormatterTest::sharedPrefixFollowsVisibleRange()
{
    QFETCH(qreal, viewportMin);
    QFETCH(qreal, viewportMax);
    QFETCH(QStringList, labels);
    QFETCH(QString, unitText);
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    auto ticker = AxisTicker{};
    ticker.setTickCount(5);
    ticker.setTickLabelFormatter(&formatter);

    const auto ticks = AxisTickPainter::computeTicks(viewportMin, viewportMax, false, &ticker);

    QCOMPARE(tickLabels(ticks), labels);
    QCOMPARE(ticks.unitText, unitText);
}

void UnitTickLabelFormatterTest::labelsShareDecimalsOfPrefixedStep_data()
{
    QTest::addColumn<qreal>("viewportMin");
    QTest::addColumn<qreal>("viewportMax");
    QTest::addColumn<int>("tickCount");
    QTest::addColumn<QStringList>("labels");
    QTest::addColumn<QString>("unitText");
    QTest::newRow("step 0.5 mm") << 0.0 << 0.002 << 4 << QStringList{"0.0", "0.5", "1.0", "1.5", "2.0"} << QStringLiteral("mm");
    QTest::newRow("step 0.05 mm") << 0.001 << 0.0012 << 4 << QStringList{"1.00", "1.05", "1.10", "1.15", "1.20"} << QStringLiteral("mm");
    QTest::newRow("step 20 km") << 0.0 << 100000.0 << 5 << QStringList{"0", "20", "40", "60", "80", "100"} << QStringLiteral("km");
    QTest::newRow("step 0.2 km") << 0.0 << 1000.0 << 5 << QStringList{"0.0", "0.2", "0.4", "0.6", "0.8", "1.0"} << QStringLiteral("km");
}

void UnitTickLabelFormatterTest::labelsShareDecimalsOfPrefixedStep()
{
    QFETCH(qreal, viewportMin);
    QFETCH(qreal, viewportMax);
    QFETCH(int, tickCount);
    QFETCH(QStringList, labels);
    QFETCH(QString, unitText);
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    auto ticker = AxisTicker{};
    ticker.setTickCount(tickCount);
    ticker.setTickLabelFormatter(&formatter);

    const auto ticks = AxisTickPainter::computeTicks(viewportMin, viewportMax, false, &ticker);

    QCOMPARE(tickLabels(ticks), labels);
    QCOMPARE(ticks.unitText, unitText);
}

void UnitTickLabelFormatterTest::tickLabelCallbackReceivesPrefixedValues()
{
    auto engine = QJSEngine{};
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    formatter.setTickLabel(engine.evaluate(QStringLiteral("(function(value, tickStep) { return value + '|' + tickStep; })")));
    auto ticker = AxisTicker{};
    ticker.setTickCount(2);
    ticker.setTickLabelFormatter(&formatter);

    const auto ticks = AxisTickPainter::computeTicks(0.0, 0.004, false, &ticker);

    QCOMPARE(tickLabels(ticks), (QStringList{"0|2", "2|2", "4|2"}));
    QCOMPARE(ticks.unitText, QStringLiteral("mm"));
}

void UnitTickLabelFormatterTest::unitChangeEmitsFormatChanged()
{
    auto formatter = UnitTickLabelFormatter{};
    auto unitSpy = QSignalSpy{&formatter, &UnitTickLabelFormatter::unitChanged};
    auto formatSpy = QSignalSpy{&formatter, &TickLabelFormatter::formatChanged};

    formatter.setUnit(QStringLiteral("s"));
    formatter.setUnit(QStringLiteral("s"));

    QCOMPARE(formatter.unit(), QStringLiteral("s"));
    QCOMPARE(unitSpy.count(), 1);
    QCOMPARE(formatSpy.count(), 1);
}

void UnitTickLabelFormatterTest::prefixModeDefaultsToLargestMagnitude()
{
    auto formatter = UnitTickLabelFormatter{};
    auto formatSpy = QSignalSpy{&formatter, &TickLabelFormatter::formatChanged};

    QCOMPARE(formatter.prefixMode(), UnitTickLabelFormatter::LargestMagnitude);
    QVERIFY(formatter.setProperty("prefixMode", QStringLiteral("LargestMagnitude")));
    QCOMPARE(formatSpy.count(), 0);
}

void UnitTickLabelFormatterTest::allowedPrefixesRestrictTheSharedPrefix_data()
{
    QTest::addColumn<QString>("unit");
    QTest::addColumn<QStringList>("allowedPrefixes");
    QTest::addColumn<qreal>("viewportMax");
    QTest::addColumn<QStringList>("labels");
    QTest::addColumn<QString>("unitText");
    const auto pixelPrefixes = QStringList{"", "k", "M", "G"};
    QTest::newRow("megapixels") << QStringLiteral("px") << pixelPrefixes << 2.5e6 << QStringList{"0.0", "0.5", "1.0", "1.5", "2.0", "2.5"}
                                << QStringLiteral("Mpx");
    QTest::newRow("below the smallest prefix") << QStringLiteral("px") << pixelPrefixes << 0.5 << QStringList{"0.0", "0.1", "0.2", "0.3", "0.4", "0.5"}
                                               << QStringLiteral("px");
    const auto metricPrefixes = QStringList{"m", "c", ""};
    const auto integerLabels = QStringList{"0", "1", "2", "3", "4"};
    QTest::newRow("millimeters") << QStringLiteral("m") << metricPrefixes << 0.004 << integerLabels << QStringLiteral("mm");
    QTest::newRow("centimeters") << QStringLiteral("m") << metricPrefixes << 0.04 << integerLabels << QStringLiteral("cm");
    QTest::newRow("meters") << QStringLiteral("m") << metricPrefixes << 4.0 << integerLabels << QStringLiteral("m");
    const auto micrometers = QString{QChar{0x00B5}} + QStringLiteral("m");
    QTest::newRow("micro sign") << QStringLiteral("m") << QStringList{QString{QChar{0x00B5}}, ""} << 4e-6 << integerLabels << micrometers;
    QTest::newRow("greek mu") << QStringLiteral("m") << QStringList{QString{QChar{0x03BC}}, ""} << 4e-6 << integerLabels << micrometers;
    QTest::newRow("latin u") << QStringLiteral("m") << QStringList{"u", ""} << 4e-6 << integerLabels << micrometers;
    QTest::newRow("no prefixes") << QStringLiteral("m") << QStringList{} << 0.004 << QStringList{"0.000", "0.001", "0.002", "0.003", "0.004"}
                                 << QStringLiteral("m");
}

void UnitTickLabelFormatterTest::formatValueFollowsTheAllowedPrefixes_data()
{
    QTest::addColumn<QStringList>("allowedPrefixes");
    QTest::addColumn<qreal>("viewportMax");
    QTest::addColumn<qreal>("value");
    QTest::addColumn<QString>("text");
    const auto pixelPrefixes = QStringList{"", "k", "M", "G"};
    QTest::newRow("megapixels") << pixelPrefixes << 2.5e6 << 1.25e6 << QStringLiteral("1.2500 Mpx");
    QTest::newRow("below the smallest prefix") << pixelPrefixes << 0.5 << 0.25 << QStringLiteral("0.2500 px");
    QTest::newRow("no prefixes") << QStringList{} << 2.5e6 << 1.25e6 << QStringLiteral("1250000 px");
}

void UnitTickLabelFormatterTest::formatValueFollowsTheAllowedPrefixes()
{
    QFETCH(QStringList, allowedPrefixes);
    QFETCH(qreal, viewportMax);
    QFETCH(qreal, value);
    QFETCH(QString, text);
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("px"));
    formatter.setAllowedPrefixes(allowedPrefixes);
    auto ticker = AxisTicker{};
    ticker.setTickCount(5);
    ticker.setTickLabelFormatter(&formatter);

    const auto ticks = AxisTickPainter::computeTicks(0.0, viewportMax, false, &ticker);
    const auto resolution = viewportMax / 1000.0;

    QCOMPARE(formatter.formatValue(value, resolution, ticks.format), text);
}

void UnitTickLabelFormatterTest::allowedPrefixesReadBackSortedAndNormalized()
{
    auto formatter = UnitTickLabelFormatter{};
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("unknown unit prefix.*x")));

    formatter.setAllowedPrefixes({"k", "u", "", QString{QChar{0x03BC}}, "x", "da"});

    QCOMPARE(formatter.allowedPrefixes(), (QStringList{QString{QChar{0x00B5}}, "", "da", "k"}));
}

void UnitTickLabelFormatterTest::allowedPrefixesChangeEmitsFormatChanged()
{
    auto formatter = UnitTickLabelFormatter{};
    auto allowedSpy = QSignalSpy{&formatter, &UnitTickLabelFormatter::allowedPrefixesChanged};
    auto formatSpy = QSignalSpy{&formatter, &TickLabelFormatter::formatChanged};

    formatter.setAllowedPrefixes({"", "k"});
    formatter.setAllowedPrefixes({"k", ""});

    QCOMPARE(allowedSpy.count(), 1);
    QCOMPARE(formatSpy.count(), 1);
}

void UnitTickLabelFormatterTest::allowedPrefixesResetRestoresEveryPowerOf1000()
{
    const auto defaults
        = QStringList{"q", "r", "y", "z", "a", "f", "p", "n", QString{QChar{0x00B5}}, "m", "", "k", "M", "G", "T", "P", "E", "Z", "Y", "R", "Q"};
    auto formatter = UnitTickLabelFormatter{};
    QCOMPARE(formatter.allowedPrefixes(), defaults);
    formatter.setAllowedPrefixes({});
    auto allowedSpy = QSignalSpy{&formatter, &UnitTickLabelFormatter::allowedPrefixesChanged};
    auto formatSpy = QSignalSpy{&formatter, &TickLabelFormatter::formatChanged};
    const auto property = formatter.metaObject()->property(formatter.metaObject()->indexOfProperty("allowedPrefixes"));

    QVERIFY(property.reset(&formatter));
    QVERIFY(property.reset(&formatter));

    QCOMPARE(formatter.allowedPrefixes(), defaults);
    QCOMPARE(allowedSpy.count(), 1);
    QCOMPARE(formatSpy.count(), 1);
}

void UnitTickLabelFormatterTest::allowedPrefixesRestrictTheSharedPrefix()
{
    QFETCH(QString, unit);
    QFETCH(QStringList, allowedPrefixes);
    QFETCH(qreal, viewportMax);
    QFETCH(QStringList, labels);
    QFETCH(QString, unitText);
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(unit);
    formatter.setAllowedPrefixes(allowedPrefixes);
    auto ticker = AxisTicker{};
    ticker.setTickCount(5);
    ticker.setTickLabelFormatter(&formatter);

    const auto ticks = AxisTickPainter::computeTicks(0.0, viewportMax, false, &ticker);

    QCOMPARE(tickLabels(ticks), labels);
    QCOMPARE(ticks.unitText, unitText);
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::UnitTickLabelFormatterTest)
#include "tst_unit_tick_label_formatter.moc"
