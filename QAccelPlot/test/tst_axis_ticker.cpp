//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/AxisTicker.hpp"
#include "QAccelPlot/formatters/NumericTickLabelFormatter.hpp"
#include "QAccelPlot/formatters/TextTickLabelFormatter.hpp"

#include <QJSEngine>
#include <QSignalSpy>
#include <QtTest/QtTest>

class TestAxisTicker : public QObject {
    Q_OBJECT

private slots:
    void defaults();
    void tickLength_splitsEvenlyAndNotifiesOnce();
    void tickLength_readsSumOfAsymmetricLengths();
    void subtickLength_splitsEvenlyAndNotifiesOnce();
    void unchangedValues_doNotNotify();
    void invalidTickLabelColor_restoresFallback();
    void assigningDefaultFormatter_isNoOp();
    void defaultFormatterChanges_forwardOnlyWithoutCustomFormatter();
    void replacedFormatterChanges_areNotForwarded();
};

void TestAxisTicker::defaults()
{
    const auto ticker = QAccelPlot::AxisTicker{};

    QCOMPARE(ticker.tickCount(), 5);
    QCOMPARE(ticker.subtickCount(), 10);
    QCOMPARE(ticker.tickLengthIn(), 8.0);
    QCOMPARE(ticker.tickLengthOut(), 8.0);
    QCOMPARE(ticker.tickLength(), 16.0);
    QCOMPARE(ticker.subtickLength(), 8.0);
    QVERIFY(!ticker.tickLabelColor().isValid());
    QVERIFY(qobject_cast<QAccelPlot::NumericTickLabelFormatter*>(ticker.tickLabelFormatter()) != nullptr);
}

void TestAxisTicker::tickLength_splitsEvenlyAndNotifiesOnce()
{
    auto ticker = QAccelPlot::AxisTicker{};
    auto lengthSpy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickLengthChanged};
    auto inSpy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickLengthInChanged};
    auto outSpy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickLengthOutChanged};

    ticker.setTickLength(10.0);

    QCOMPARE(ticker.tickLengthIn(), 5.0);
    QCOMPARE(ticker.tickLengthOut(), 5.0);
    QCOMPARE(ticker.tickLength(), 10.0);
    QCOMPARE(lengthSpy.count(), 1);
    QCOMPARE(inSpy.count(), 1);
    QCOMPARE(outSpy.count(), 1);

    ticker.setTickLength(10.0);
    QCOMPARE(lengthSpy.count(), 1);
}

void TestAxisTicker::tickLength_readsSumOfAsymmetricLengths()
{
    auto ticker = QAccelPlot::AxisTicker{};
    ticker.setTickLengthIn(0.0);
    ticker.setTickLengthOut(6.0);
    QCOMPARE(ticker.tickLength(), 6.0);

    // Re-applying the same total still rebalances the halves.
    auto inSpy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickLengthInChanged};
    ticker.setTickLength(6.0);
    QCOMPARE(ticker.tickLengthIn(), 3.0);
    QCOMPARE(ticker.tickLengthOut(), 3.0);
    QCOMPARE(inSpy.count(), 1);
}

void TestAxisTicker::subtickLength_splitsEvenlyAndNotifiesOnce()
{
    auto ticker = QAccelPlot::AxisTicker{};
    auto lengthSpy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::subtickLengthChanged};
    auto inSpy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::subtickLengthInChanged};
    auto outSpy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::subtickLengthOutChanged};

    ticker.setSubtickLength(3.0);

    QCOMPARE(ticker.subtickLengthIn(), 1.5);
    QCOMPARE(ticker.subtickLengthOut(), 1.5);
    QCOMPARE(lengthSpy.count(), 1);
    QCOMPARE(inSpy.count(), 1);
    QCOMPARE(outSpy.count(), 1);
}

void TestAxisTicker::unchangedValues_doNotNotify()
{
    auto ticker = QAccelPlot::AxisTicker{};
    auto countSpy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickCountChanged};
    auto widthSpy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickWidthChanged};
    auto colorSpy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickColorChanged};
    auto fontSpy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickLabelFontChanged};

    ticker.setTickCount(ticker.tickCount());
    ticker.setTickWidth(ticker.tickWidth());
    ticker.setTickColor(ticker.tickColor());
    ticker.setTickLabelFont(ticker.tickLabelFont());

    QCOMPARE(countSpy.count(), 0);
    QCOMPARE(widthSpy.count(), 0);
    QCOMPARE(colorSpy.count(), 0);
    QCOMPARE(fontSpy.count(), 0);
}

void TestAxisTicker::invalidTickLabelColor_restoresFallback()
{
    auto ticker = QAccelPlot::AxisTicker{};
    auto spy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickLabelColorChanged};

    ticker.setTickLabelColor(Qt::red);
    QCOMPARE(ticker.tickLabelColor(), QColor(Qt::red));

    ticker.setTickLabelColor(QColor{});
    QVERIFY(!ticker.tickLabelColor().isValid());
    QCOMPARE(spy.count(), 2);
}

void TestAxisTicker::assigningDefaultFormatter_isNoOp()
{
    auto ticker = QAccelPlot::AxisTicker{};
    auto* defaultFormatter = ticker.tickLabelFormatter();
    auto spy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickLabelFormatterChanged};

    ticker.setTickLabelFormatter(defaultFormatter);
    ticker.setTickLabelFormatter(nullptr);

    QCOMPARE(ticker.tickLabelFormatter(), defaultFormatter);
    QCOMPARE(spy.count(), 0);
}

void TestAxisTicker::defaultFormatterChanges_forwardOnlyWithoutCustomFormatter()
{
    auto engine = QJSEngine{};
    auto ticker = QAccelPlot::AxisTicker{};
    auto* defaultFormatter = ticker.tickLabelFormatter();
    auto spy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickLabelFormatChanged};

    defaultFormatter->setTickLabel(engine.evaluate(QStringLiteral("(function(value) { return 'a' + value; })")));
    QCOMPARE(spy.count(), 1);

    auto custom = QAccelPlot::TextTickLabelFormatter{};
    ticker.setTickLabelFormatter(&custom);
    defaultFormatter->setTickLabel(engine.evaluate(QStringLiteral("(function(value) { return 'b' + value; })")));
    QCOMPARE(spy.count(), 1);

    ticker.setTickLabelFormatter(nullptr);
    QCOMPARE(ticker.tickLabelFormatter(), defaultFormatter);
    QCOMPARE(ticker.tickLabelFormatter()->format(1.0, 1.0), QStringLiteral("b1"));
}

void TestAxisTicker::replacedFormatterChanges_areNotForwarded()
{
    auto ticker = QAccelPlot::AxisTicker{};
    auto first = QAccelPlot::TextTickLabelFormatter{};
    auto second = QAccelPlot::TextTickLabelFormatter{};
    ticker.setTickLabelFormatter(&first);
    ticker.setTickLabelFormatter(&second);
    auto formatSpy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickLabelFormatChanged};
    auto formatterSpy = QSignalSpy{&ticker, &QAccelPlot::AxisTicker::tickLabelFormatterChanged};

    first.setLabels({QStringLiteral("stale")});
    QCOMPARE(formatSpy.count(), 0);

    second.setLabels({QStringLiteral("current")});
    QCOMPARE(formatSpy.count(), 1);

    // Destroying a formatter that is no longer assigned must not reset the current one.
    {
        auto transient = QAccelPlot::TextTickLabelFormatter{};
        ticker.setTickLabelFormatter(&transient);
        ticker.setTickLabelFormatter(&second);
    }
    QCOMPARE(ticker.tickLabelFormatter(), &second);
    QCOMPARE(formatterSpy.count(), 2);
}

QTEST_GUILESS_MAIN(TestAxisTicker)
#include "tst_axis_ticker.moc"
