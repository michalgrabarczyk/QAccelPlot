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

} // namespace QAccelPlot

QTEST_MAIN(QAccelPlot::UnitTickLabelFormatterTest)
#include "tst_unit_tick_label_formatter.moc"
