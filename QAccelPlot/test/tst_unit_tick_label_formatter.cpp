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

AxisTicks computeTicks(UnitTickLabelFormatter& formatter, const qreal viewportMin, const qreal viewportMax)
{
    auto ticker = AxisTicker{};
    ticker.setTickCount(5);
    ticker.setTickLabelFormatter(&formatter);
    return AxisTickPainter::computeTicks(viewportMin, viewportMax, false, &ticker);
}

AxisTicks computeLogTicks(UnitTickLabelFormatter& formatter, const qreal viewportMin, const qreal viewportMax)
{
    auto ticker = AxisTicker{};
    ticker.setTickLabelFormatter(&formatter);
    return AxisTickPainter::computeTicks(viewportMin, viewportMax, true, &ticker);
}

// Exposes formatter to engine as the global "formatter", as QML does, and sets its prefix callback from source.
void setPrefixCallback(QJSEngine& engine, UnitTickLabelFormatter& formatter, const QString& source)
{
    QJSEngine::setObjectOwnership(&formatter, QJSEngine::CppOwnership);
    engine.globalObject().setProperty(QStringLiteral("formatter"), engine.newQObject(&formatter));
    const auto result = engine.evaluate(QStringLiteral("formatter.prefixCallback = ") + source);
    QVERIFY2(!result.isError(), qPrintable(result.toString()));
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
    void fixedPrefixShowsAtEveryZoomLevel_data();
    void fixedPrefixShowsAtEveryZoomLevel();
    void fixedPrefixSnapsToTheAllowedPrefixes_data();
    void fixedPrefixSnapsToTheAllowedPrefixes();
    void fixedPrefixReadsBackNormalized();
    void fixedPrefixChangeEmitsFormatChanged();
    void prefixModeChangeEmitsFormatChanged();
    void customPrefixCallbackReceivesRangeAndTicksInBaseUnit();
    void customPrefixSnapsToTheAllowedPrefixes_data();
    void customPrefixSnapsToTheAllowedPrefixes();
    void customPrefixFallsBackToLargestMagnitude_data();
    void customPrefixFallsBackToLargestMagnitude();
    void prefixCallbackIsUsedOnlyInCustomMode_data();
    void prefixCallbackIsUsedOnlyInCustomMode();
    void prefixCallbackChangeEmitsFormatChanged();
    void logAxisSharesThePrefixOfTheLargestTick();
    void logAxisScalesDecadesToTheFixedPrefix();
    void logAxisPassesBaseUnitTicksToThePrefixCallback();
    void logAxisSnapsToTheAllowedPrefixes_data();
    void logAxisSnapsToTheAllowedPrefixes();
    void logAxisTickLabelCallbackReceivesPrefixedValues();
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

void UnitTickLabelFormatterTest::fixedPrefixShowsAtEveryZoomLevel_data()
{
    QTest::addColumn<qreal>("viewportMax");
    QTest::addColumn<QStringList>("labels");
    QTest::addColumn<QString>("valueText");
    QTest::newRow("micrometers") << 4e-6 << QStringList{"0.000", "0.001", "0.002", "0.003", "0.004"} << QStringLiteral("0.001000 mm");
    QTest::newRow("millimeters") << 0.004 << QStringList{"0", "1", "2", "3", "4"} << QStringLiteral("1.000 mm");
    QTest::newRow("meters") << 4.0 << QStringList{"0", "1000", "2000", "3000", "4000"} << QStringLiteral("1000 mm");
}

void UnitTickLabelFormatterTest::fixedPrefixShowsAtEveryZoomLevel()
{
    QFETCH(qreal, viewportMax);
    QFETCH(QStringList, labels);
    QFETCH(QString, valueText);
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    formatter.setPrefixMode(UnitTickLabelFormatter::Fixed);
    formatter.setFixedPrefix(QStringLiteral("m"));
    auto ticker = AxisTicker{};
    ticker.setTickCount(5);
    ticker.setTickLabelFormatter(&formatter);

    const auto ticks = AxisTickPainter::computeTicks(0.0, viewportMax, false, &ticker);

    QCOMPARE(tickLabels(ticks), labels);
    QCOMPARE(ticks.unitText, QStringLiteral("mm"));
    QCOMPARE(formatter.formatValue(viewportMax / 4.0, viewportMax / 1000.0, ticks.format), valueText);
}

void UnitTickLabelFormatterTest::fixedPrefixSnapsToTheAllowedPrefixes_data()
{
    QTest::addColumn<QStringList>("allowedPrefixes");
    QTest::addColumn<QString>("fixedPrefix");
    QTest::addColumn<QString>("unitText");
    QTest::addColumn<QString>("valueText");
    const auto metricPrefixes = QStringList{"m", "", "k"};
    QTest::newRow("allowed") << metricPrefixes << QStringLiteral("k") << QStringLiteral("km") << QStringLiteral("1.0000 km");
    QTest::newRow("above the largest") << metricPrefixes << QStringLiteral("M") << QStringLiteral("km") << QStringLiteral("1.0000 km");
    QTest::newRow("between two") << metricPrefixes << QStringLiteral("c") << QStringLiteral("mm") << QStringLiteral("1000000 mm");
    QTest::newRow("below the smallest") << QStringList{"", "k"} << QStringLiteral("m") << QStringLiteral("m") << QStringLiteral("1000.0 m");
    QTest::newRow("no prefixes") << QStringList{} << QStringLiteral("k") << QStringLiteral("m") << QStringLiteral("1000.0 m");
}

void UnitTickLabelFormatterTest::fixedPrefixSnapsToTheAllowedPrefixes()
{
    QFETCH(QStringList, allowedPrefixes);
    QFETCH(QString, fixedPrefix);
    QFETCH(QString, unitText);
    QFETCH(QString, valueText);
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    formatter.setAllowedPrefixes(allowedPrefixes);
    formatter.setPrefixMode(UnitTickLabelFormatter::Fixed);
    formatter.setFixedPrefix(fixedPrefix);

    const auto ticks = computeTicks(formatter, 0.0, 4000.0);

    QCOMPARE(ticks.unitText, unitText);
    QCOMPARE(formatter.formatValue(1000.0, 0.4, ticks.format), valueText);
}

void UnitTickLabelFormatterTest::fixedPrefixReadsBackNormalized()
{
    auto formatter = UnitTickLabelFormatter{};
    QCOMPARE(formatter.fixedPrefix(), QString{});

    formatter.setFixedPrefix(QStringLiteral("u"));
    QCOMPARE(formatter.fixedPrefix(), QString{QChar{0x00B5}});

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("unknown unit prefix.*x")));
    formatter.setFixedPrefix(QStringLiteral("x"));
    QCOMPARE(formatter.fixedPrefix(), QString{QChar{0x00B5}});
}

void UnitTickLabelFormatterTest::fixedPrefixChangeEmitsFormatChanged()
{
    auto formatter = UnitTickLabelFormatter{};
    auto fixedSpy = QSignalSpy{&formatter, &UnitTickLabelFormatter::fixedPrefixChanged};
    auto formatSpy = QSignalSpy{&formatter, &TickLabelFormatter::formatChanged};

    formatter.setFixedPrefix(QStringLiteral("u"));
    formatter.setFixedPrefix(QString{QChar{0x03BC}});

    QCOMPARE(fixedSpy.count(), 1);
    QCOMPARE(formatSpy.count(), 1);
}

void UnitTickLabelFormatterTest::prefixModeChangeEmitsFormatChanged()
{
    auto formatter = UnitTickLabelFormatter{};
    auto modeSpy = QSignalSpy{&formatter, &UnitTickLabelFormatter::prefixModeChanged};
    auto formatSpy = QSignalSpy{&formatter, &TickLabelFormatter::formatChanged};

    QVERIFY(formatter.setProperty("prefixMode", QStringLiteral("Fixed")));
    formatter.setPrefixMode(UnitTickLabelFormatter::Fixed);
    QVERIFY(formatter.setProperty("prefixMode", QStringLiteral("Custom")));

    QCOMPARE(formatter.prefixMode(), UnitTickLabelFormatter::Custom);
    QCOMPARE(modeSpy.count(), 2);
    QCOMPARE(formatSpy.count(), 2);
}

void UnitTickLabelFormatterTest::customPrefixCallbackReceivesRangeAndTicksInBaseUnit()
{
    auto engine = QJSEngine{};
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    formatter.setPrefixMode(UnitTickLabelFormatter::Custom);
    setPrefixCallback(engine, formatter, QStringLiteral("(function(min, max, ticks) { calls.push([min, max, ticks.join(',')].join('|')); return 'k'; })"));
    engine.globalObject().setProperty(QStringLiteral("calls"), engine.newArray());

    const auto ticks = computeTicks(formatter, 0.0, 0.004);

    QCOMPARE(engine.evaluate(QStringLiteral("calls.join(';')")).toString(), QStringLiteral("0|0.004|0,0.001,0.002,0.003,0.004"));
    QCOMPARE(tickLabels(ticks), (QStringList{"0.000000", "0.000001", "0.000002", "0.000003", "0.000004"}));
    QCOMPARE(ticks.unitText, QStringLiteral("km"));
    QCOMPARE(formatter.formatValue(0.002, 0.001, ticks.format), QStringLiteral("0.000002 km"));
}

void UnitTickLabelFormatterTest::customPrefixSnapsToTheAllowedPrefixes_data()
{
    QTest::addColumn<QString>("result");
    QTest::addColumn<QString>("unitText");
    QTest::newRow("allowed") << QStringLiteral("k") << QStringLiteral("km");
    QTest::newRow("greek mu") << QString{QChar{0x03BC}} << QString{QChar{0x00B5}} + QStringLiteral("m");
    QTest::newRow("above the largest") << QStringLiteral("G") << QStringLiteral("km");
    QTest::newRow("between two") << QStringLiteral("m") << QString{QChar{0x00B5}} + QStringLiteral("m");
    QTest::newRow("below the smallest") << QStringLiteral("n") << QString{QChar{0x00B5}} + QStringLiteral("m");
}

void UnitTickLabelFormatterTest::customPrefixSnapsToTheAllowedPrefixes()
{
    QFETCH(QString, result);
    QFETCH(QString, unitText);
    auto engine = QJSEngine{};
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    formatter.setAllowedPrefixes({QStringLiteral("u"), QStringLiteral(""), QStringLiteral("k")});
    formatter.setPrefixMode(UnitTickLabelFormatter::Custom);
    setPrefixCallback(engine, formatter, QStringLiteral("(function() { return result; })"));
    engine.globalObject().setProperty(QStringLiteral("result"), result);

    const auto ticks = computeTicks(formatter, 0.0, 4.0);

    QCOMPARE(ticks.unitText, unitText);
}

void UnitTickLabelFormatterTest::customPrefixFallsBackToLargestMagnitude_data()
{
    QTest::addColumn<QString>("callback");
    QTest::addColumn<QString>("warning");
    QTest::newRow("error") << QStringLiteral("(function() { throw new Error('boom'); })") << QStringLiteral("prefix callback failed.*boom");
    QTest::newRow("number") << QStringLiteral("(function() { return 3; })") << QStringLiteral("prefix callback returned a non-string");
    QTest::newRow("undefined") << QStringLiteral("(function() {})") << QStringLiteral("prefix callback returned a non-string");
    QTest::newRow("unknown symbol") << QStringLiteral("(function() { return 'x'; })") << QStringLiteral("unknown unit prefix.*x");
    QTest::newRow("not a function") << QStringLiteral("'k'") << QString{};
}

void UnitTickLabelFormatterTest::customPrefixFallsBackToLargestMagnitude()
{
    QFETCH(QString, callback);
    QFETCH(QString, warning);
    auto engine = QJSEngine{};
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    formatter.setPrefixMode(UnitTickLabelFormatter::Custom);
    setPrefixCallback(engine, formatter, callback);
    if (!warning.isEmpty()) {
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(warning));
    }

    const auto ticks = computeTicks(formatter, 0.0, 0.004);

    QCOMPARE(tickLabels(ticks), (QStringList{"0", "1", "2", "3", "4"}));
    QCOMPARE(ticks.unitText, QStringLiteral("mm"));
    QCOMPARE(formatter.formatValue(0.002, 0.000001, ticks.format), QStringLiteral("2.000 mm"));
}

void UnitTickLabelFormatterTest::prefixCallbackIsUsedOnlyInCustomMode_data()
{
    QTest::addColumn<UnitTickLabelFormatter::PrefixMode>("prefixMode");
    QTest::addColumn<QString>("unitText");
    QTest::newRow("largest magnitude") << UnitTickLabelFormatter::LargestMagnitude << QStringLiteral("mm");
    QTest::newRow("fixed") << UnitTickLabelFormatter::Fixed << QStringLiteral("m");
}

void UnitTickLabelFormatterTest::prefixCallbackIsUsedOnlyInCustomMode()
{
    QFETCH(UnitTickLabelFormatter::PrefixMode, prefixMode);
    QFETCH(QString, unitText);
    auto engine = QJSEngine{};
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    formatter.setPrefixMode(prefixMode);
    setPrefixCallback(engine, formatter, QStringLiteral("(function() { called = true; return 'k'; })"));
    engine.globalObject().setProperty(QStringLiteral("called"), false);

    const auto ticks = computeTicks(formatter, 0.0, 0.004);

    QCOMPARE(ticks.unitText, unitText);
    QCOMPARE(engine.globalObject().property(QStringLiteral("called")).toBool(), false);
}

void UnitTickLabelFormatterTest::prefixCallbackChangeEmitsFormatChanged()
{
    auto engine = QJSEngine{};
    auto formatter = UnitTickLabelFormatter{};
    auto callbackSpy = QSignalSpy{&formatter, &UnitTickLabelFormatter::prefixCallbackChanged};
    auto formatSpy = QSignalSpy{&formatter, &TickLabelFormatter::formatChanged};

    formatter.setPrefixCallback(engine.evaluate(QStringLiteral("(function() { return 'k'; })")));

    QVERIFY(formatter.prefixCallback().isCallable());
    QCOMPARE(callbackSpy.count(), 1);
    QCOMPARE(formatSpy.count(), 1);
}

void UnitTickLabelFormatterTest::logAxisSharesThePrefixOfTheLargestTick()
{
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));

    const auto ticks = computeLogTicks(formatter, 1e-6, 1e-3);

    QCOMPARE(tickLabels(ticks),
        (QStringList{QStringLiteral("10\u207B\u00B3"), QStringLiteral("10\u207B\u00B2"), QStringLiteral("10\u207B\u00B9"), QStringLiteral("10\u2070")}));
    QCOMPARE(ticks.unitText, QStringLiteral("mm"));
    QCOMPARE(formatter.formatValue(2e-4, 1e-6, ticks.format), QStringLiteral("0.200 mm"));
}

void UnitTickLabelFormatterTest::logAxisScalesDecadesToTheFixedPrefix()
{
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    formatter.setPrefixMode(UnitTickLabelFormatter::Fixed);
    formatter.setFixedPrefix(QStringLiteral("m"));

    const auto ticks = computeLogTicks(formatter, 1e-6, 1.0);

    QCOMPARE(tickLabels(ticks),
        (QStringList{QStringLiteral("10\u207B\u00B3"), QStringLiteral("10\u207B\u00B2"), QStringLiteral("10\u207B\u00B9"), QStringLiteral("10\u2070"),
            QStringLiteral("10\u00B9"), QStringLiteral("10\u00B2"), QStringLiteral("10\u00B3")}));
    QCOMPARE(ticks.unitText, QStringLiteral("mm"));
    QCOMPARE(formatter.formatValue(0.5, 0.001, ticks.format), QStringLiteral("500 mm"));
}

void UnitTickLabelFormatterTest::logAxisPassesBaseUnitTicksToThePrefixCallback()
{
    auto engine = QJSEngine{};
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    formatter.setPrefixMode(UnitTickLabelFormatter::Custom);
    setPrefixCallback(engine, formatter, QStringLiteral("(function(min, max, ticks) { calls.push([min, max, ticks.join(',')].join('|')); return 'k'; })"));
    engine.globalObject().setProperty(QStringLiteral("calls"), engine.newArray());

    const auto ticks = computeLogTicks(formatter, 10.0, 1000.0);

    QCOMPARE(engine.evaluate(QStringLiteral("calls.join(';')")).toString(), QStringLiteral("10|1000|10,100,1000"));
    QCOMPARE(tickLabels(ticks), (QStringList{QStringLiteral("10\u207B\u00B2"), QStringLiteral("10\u207B\u00B9"), QStringLiteral("10\u2070")}));
    QCOMPARE(ticks.unitText, QStringLiteral("km"));
}

void UnitTickLabelFormatterTest::logAxisSnapsToTheAllowedPrefixes_data()
{
    QTest::addColumn<QStringList>("allowedPrefixes");
    QTest::addColumn<QStringList>("labels");
    QTest::addColumn<QString>("unitText");
    QTest::newRow("centimeters") << QStringList{"m", "c", ""}
                                 << QStringList{QStringLiteral("10\u207B\u00B9"), QStringLiteral("10\u2070"), QStringLiteral("10\u00B9")}
                                 << QStringLiteral("cm");
    QTest::newRow("below the smallest prefix") << QStringList{"",
        "k"} << QStringList{QStringLiteral("10\u207B\u00B3"), QStringLiteral("10\u207B\u00B2"), QStringLiteral("10\u207B\u00B9")}
                                               << QStringLiteral("m");
    QTest::newRow("no prefixes") << QStringList{}
                                 << QStringList{QStringLiteral("10\u207B\u00B3"), QStringLiteral("10\u207B\u00B2"), QStringLiteral("10\u207B\u00B9")}
                                 << QStringLiteral("m");
}

void UnitTickLabelFormatterTest::logAxisSnapsToTheAllowedPrefixes()
{
    QFETCH(QStringList, allowedPrefixes);
    QFETCH(QStringList, labels);
    QFETCH(QString, unitText);
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    formatter.setAllowedPrefixes(allowedPrefixes);

    const auto ticks = computeLogTicks(formatter, 1e-3, 0.1);

    QCOMPARE(tickLabels(ticks), labels);
    QCOMPARE(ticks.unitText, unitText);
}

void UnitTickLabelFormatterTest::logAxisTickLabelCallbackReceivesPrefixedValues()
{
    auto engine = QJSEngine{};
    auto formatter = UnitTickLabelFormatter{};
    formatter.setUnit(QStringLiteral("m"));
    formatter.setTickLabel(engine.evaluate(QStringLiteral("(function(value, tickStep) { return value + '|' + tickStep; })")));

    const auto ticks = computeLogTicks(formatter, 1e-5, 1e-3);

    QCOMPARE(tickLabels(ticks), (QStringList{"0.01|0.01", "0.1|0.1", "1|1"}));
    QCOMPARE(ticks.unitText, QStringLiteral("mm"));
}

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::UnitTickLabelFormatterTest)
#include "tst_unit_tick_label_formatter.moc"
