//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/linestyles/DashLine.hpp"
#include "QAccelPlot/linestyles/NoLine.hpp"
#include "QAccelPlot/linestyles/SolidLine.hpp"

#include <QSignalSpy>
#include <QtTest/QtTest>

namespace QAccelPlot {

class LineStylesTest : public QObject {
    Q_OBJECT

private slots:
    void dashPatternIsAppliedVerbatim();
    void negativeDashSegmentIsRejected();
    void overlongDashPatternWarnsAboutTruncation();
    void maximumLengthDashPatternIsNotTruncated();
    void emptyDashPatternDisablesDashing();
    void zeroLengthDashPatternKeepsPositivePeriod();
    void unchangedDashPatternDoesNotNotify();
    void solidAndNoLineStyles();
};

void LineStylesTest::dashPatternIsAppliedVerbatim()
{
    auto dash = DashLine{};
    dash.setPattern({10.0, 5.0});

    const auto params = dash.dashParameters();
    QVERIFY(params.enabled);
    QCOMPARE(params.patternSize, 2);
    QCOMPARE(params.pattern[0], 10.0f);
    QCOMPARE(params.pattern[1], 5.0f);
    QCOMPARE(params.period, 15.0f);
}

void LineStylesTest::negativeDashSegmentIsRejected()
{
    auto dash = DashLine{};
    dash.setPattern({10.0, 5.0});
    auto patternChanged = QSignalSpy{&dash, &DashLine::patternChanged};

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("DashLine pattern segments cannot be negative.*"));
    dash.setPattern({10.0, -5.0});

    QCOMPARE(dash.pattern(), QList<qreal>({10.0, 5.0}));
    QCOMPARE(patternChanged.count(), 0);
}

void LineStylesTest::overlongDashPatternWarnsAboutTruncation()
{
    auto dash = DashLine{};
    const auto tenSegments = QList<qreal>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("DashLine pattern has 10 segments.*"));
    dash.setPattern(tenSegments);

    const auto params = dash.dashParameters();
    QCOMPARE(params.patternSize, 8);
}

void LineStylesTest::maximumLengthDashPatternIsNotTruncated()
{
    auto dash = DashLine{};
    dash.setPattern({1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0});

    const auto params = dash.dashParameters();
    QCOMPARE(params.patternSize, kMaxDashPatternSize);
    QCOMPARE(params.pattern[kMaxDashPatternSize - 1], 8.0f);
    QCOMPARE(params.period, 36.0f);
}

void LineStylesTest::emptyDashPatternDisablesDashing()
{
    auto dash = DashLine{};
    dash.setPattern({10.0, 5.0});
    auto styleChanged = QSignalSpy{&dash, &LineStyle::styleChanged};

    dash.setPattern({});

    QCOMPARE(styleChanged.count(), 1);
    QVERIFY(dash.showLine());
    QVERIFY(!dash.dashParameters().enabled);
}

void LineStylesTest::zeroLengthDashPatternKeepsPositivePeriod()
{
    auto dash = DashLine{};
    dash.setPattern({0.0, 0.0});

    // The shader takes the arc length modulo the period, so it must stay positive.
    const auto params = dash.dashParameters();
    QVERIFY(params.enabled);
    QCOMPARE(params.period, 1.0f);
}

void LineStylesTest::unchangedDashPatternDoesNotNotify()
{
    auto dash = DashLine{};
    dash.setPattern({4.0, 2.0});
    auto patternChanged = QSignalSpy{&dash, &DashLine::patternChanged};
    auto styleChanged = QSignalSpy{&dash, &LineStyle::styleChanged};

    dash.setPattern({4.0, 2.0});

    QCOMPARE(patternChanged.count(), 0);
    QCOMPARE(styleChanged.count(), 0);
}

void LineStylesTest::solidAndNoLineStyles()
{
    const auto solid = SolidLine{};
    QVERIFY(solid.showLine());
    QVERIFY(!solid.dashParameters().enabled);

    const auto none = NoLine{};
    QVERIFY(!none.showLine());
    QVERIFY(!none.dashParameters().enabled);
}

} // namespace QAccelPlot

using QAccelPlot::LineStylesTest;
QTEST_MAIN(LineStylesTest)
#include "tst_line_styles.moc"
