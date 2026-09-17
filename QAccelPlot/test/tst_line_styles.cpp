//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/linestyles/DashLine.hpp"

#include <QSignalSpy>
#include <QtTest/QtTest>

namespace QAccelPlot {

class LineStylesTest : public QObject {
    Q_OBJECT

private slots:
    void dashPatternIsAppliedVerbatim();
    void negativeDashSegmentIsRejected();
    void overlongDashPatternWarnsAboutTruncation();
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

} // namespace QAccelPlot

using QAccelPlot::LineStylesTest;
QTEST_MAIN(LineStylesTest)
#include "tst_line_styles.moc"
