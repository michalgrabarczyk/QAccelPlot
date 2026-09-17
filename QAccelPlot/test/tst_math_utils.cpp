//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/MathUtils.hpp"

#include <QtTest/QtTest>

#include <cmath>
#include <limits>

class TestMathUtils : public QObject {
    Q_OBJECT

private slots:
    void defaultTolerance_data();
    void defaultTolerance();
    void customTolerance_data();
    void customTolerance();
    void isValidSample_data();
    void isValidSample();
};

using QAccelPlot::nearly_equal;

void TestMathUtils::defaultTolerance_data()
{
    QTest::addColumn<double>("left");
    QTest::addColumn<double>("right");
    QTest::addColumn<bool>("expectedEqual");

    const auto infinity = std::numeric_limits<double>::infinity();
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    const auto epsilon = std::numeric_limits<double>::epsilon();
    const auto adjacentOne = std::nextafter(1.0, 2.0);
    const auto adjacentLarge = std::nextafter(1e21, infinity);
    auto separatedOne = 1.0;
    auto separatedLarge = 1e21;
    for (auto i = 0; i < 4; ++i) {
        separatedOne = std::nextafter(separatedOne, 2.0);
        separatedLarge = std::nextafter(separatedLarge, infinity);
    }

    QTest::newRow("identical-positive") << 1.0 << 1.0 << true;
    QTest::newRow("identical-negative") << -5.0 << -5.0 << true;
    QTest::newRow("positive-and-negative-zero") << 0.0 << -0.0 << true;
    QTest::newRow("positive-infinity") << infinity << infinity << true;
    QTest::newRow("negative-infinity") << -infinity << -infinity << true;
    QTest::newRow("opposite-infinities") << infinity << -infinity << false;
    QTest::newRow("same-small-positive") << 0.001 << 0.001 << true;
    QTest::newRow("same-large-positive") << 1234567.89 << 1234567.89 << true;
    QTest::newRow("same-negative") << -42.5 << -42.5 << true;
    QTest::newRow("different-positive") << 1.0 << 2.0 << false;
    QTest::newRow("different-sign") << -1.0 << 1.0 << false;
    QTest::newRow("adjacent-one") << 1.0 << adjacentOne << true;
    QTest::newRow("four-steps-from-one") << 1.0 << separatedOne << false;
    QTest::newRow("same-near-zero") << 1e-20 << 1e-20 << true;
    QTest::newRow("zero-and-epsilon") << 0.0 << epsilon << true;
    QTest::newRow("distinguishable-near-zero") << 1e-12 << 2e-12 << false;
    QTest::newRow("same-very-small") << 3e-200 << 3e-200 << true;
    QTest::newRow("absolute-tolerance-near-zero") << 1e-100 << 2e-100 << true;
    QTest::newRow("same-very-large") << 1e21 << 1e21 << true;
    QTest::newRow("adjacent-very-large") << 1e21 << adjacentLarge << true;
    QTest::newRow("four-steps-from-very-large") << 1e21 << separatedLarge << false;
    QTest::newRow("mixed-scales") << 1e-20 << 1e21 << false;
    QTest::newRow("zero-and-one") << 0.0 << 1.0 << false;
    QTest::newRow("one-and-zero") << 1.0 << 0.0 << false;
    QTest::newRow("nan-and-nan") << nan << nan << false;
    QTest::newRow("nan-and-value") << nan << 1.0 << false;
    QTest::newRow("value-and-nan") << 1.0 << nan << false;
    QTest::newRow("outside-default-tolerance") << 1.0 << 1.0 + 1e-6 << false;
}

void TestMathUtils::defaultTolerance()
{
    QFETCH(double, left);
    QFETCH(double, right);
    QFETCH(bool, expectedEqual);

    QCOMPARE(nearly_equal(left, right), expectedEqual);
}

void TestMathUtils::customTolerance_data()
{
    QTest::addColumn<double>("left");
    QTest::addColumn<double>("right");
    QTest::addColumn<double>("relativeTolerance");
    QTest::addColumn<double>("absoluteTolerance");
    QTest::addColumn<bool>("expectedEqual");

    const auto adjacent = std::nextafter(1.0, 2.0);
    QTest::newRow("looser-relative-tolerance") << 1.0 << 1.0 + 1e-6 << 1e-5 << QAccelPlot::kNearlyEqualEpsilon << true;
    QTest::newRow("tighter-relative-tolerance") << 1.0 << adjacent << std::numeric_limits<double>::epsilon() / 2.0 << std::numeric_limits<double>::min()
                                                << false;
    QTest::newRow("absolute-tolerance-near-zero") << 1e-20 << 2e-20 << 1e-12 << 1e-19 << true;
}

void TestMathUtils::customTolerance()
{
    QFETCH(double, left);
    QFETCH(double, right);
    QFETCH(double, relativeTolerance);
    QFETCH(double, absoluteTolerance);
    QFETCH(bool, expectedEqual);

    QCOMPARE(nearly_equal(left, right, relativeTolerance, absoluteTolerance), expectedEqual);
}

void TestMathUtils::isValidSample_data()
{
    QTest::addColumn<double>("value");
    QTest::addColumn<bool>("logScale");
    QTest::addColumn<bool>("expected");

    const auto nan = std::numeric_limits<double>::quiet_NaN();
    const auto inf = std::numeric_limits<double>::infinity();
    const auto denormal = std::numeric_limits<double>::denorm_min();
    const auto maxValue = std::numeric_limits<double>::max();

    QTest::newRow("finite-linear") << 1.5 << false << true;
    QTest::newRow("zero-linear") << 0.0 << false << true;
    QTest::newRow("negative-linear") << -3.0 << false << true;
    QTest::newRow("max-linear") << maxValue << false << true;
    QTest::newRow("denormal-linear") << denormal << false << true;
    QTest::newRow("nan-linear") << nan << false << false;
    QTest::newRow("positive-infinity-linear") << inf << false << false;
    QTest::newRow("negative-infinity-linear") << -inf << false << false;
    QTest::newRow("positive-log") << 10.0 << true << true;
    QTest::newRow("denormal-log") << denormal << true << true;
    QTest::newRow("zero-log") << 0.0 << true << false;
    QTest::newRow("negative-zero-log") << -0.0 << true << false;
    QTest::newRow("negative-log") << -1.0 << true << false;
    QTest::newRow("nan-log") << nan << true << false;
    QTest::newRow("positive-infinity-log") << inf << true << false;
}

void TestMathUtils::isValidSample()
{
    QFETCH(double, value);
    QFETCH(bool, logScale);
    QFETCH(bool, expected);

    QCOMPARE(QAccelPlot::isValidSample(value, logScale), expected);
}

QTEST_GUILESS_MAIN(TestMathUtils)
#include "tst_math_utils.moc"
