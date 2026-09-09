//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "MathUtils.hpp"

#include <QtTest/QtTest>

#include <cmath>
#include <limits>

class TestMathUtils : public QObject {
    Q_OBJECT

private slots:
    // Exact equality
    void identical_values_areEqual();
    void positive_zero_negative_zero_areEqual();
    void positive_infinity_areEqual();
    void negative_infinity_areEqual();
    void opposite_infinities_areNotEqual();

    // Normal values
    void same_small_positive_values_areEqual();
    void same_large_positive_values_areEqual();
    void same_negative_values_areEqual();
    void different_values_areNotEqual();
    void values_within_default_epsilon_areEqual();
    void values_outside_default_epsilon_areNotEqual();

    // Near zero — the qFuzzyCompare(x + 1.0, y + 1.0) failure zone
    void near_zero_identical_areEqual();
    void near_zero_tiny_difference_areEqual();
    void near_zero_distinguishable_areNotEqual();
    void very_small_positive_values_areEqual();
    void very_small_but_different_values_areNotEqual();

    // Extreme magnitudes
    void very_large_values_equal();
    void very_large_values_slightlyDifferent_areNotEqual();
    void value_at_1e21_withinEpsilon_areEqual();

    // Mixed scales (no cross-scale false positives)
    void small_and_large_areNotEqual();
    void one_and_zero_areNotEqual();

    // NaN
    void nan_and_nan_areNotEqual();
    void nan_and_value_areNotEqual();
    void value_and_nan_areNotEqual();

    // Custom epsilon
    void custom_eps_rel_looser_considersEqual();
    void custom_eps_rel_tighter_considersNotEqual();
    void custom_eps_abs_catchesNearZeroDifference();
};

using QAccelPlot::nearly_equal;

// ---------------------------------------------------------------------------
// Exact equality
// ---------------------------------------------------------------------------

void TestMathUtils::identical_values_areEqual()
{
    QVERIFY(nearly_equal(1.0, 1.0));
    QVERIFY(nearly_equal(-5.0, -5.0));
    QVERIFY(nearly_equal(0.0, 0.0));
}

void TestMathUtils::positive_zero_negative_zero_areEqual()
{
    QVERIFY(nearly_equal(0.0, -0.0));
}

void TestMathUtils::positive_infinity_areEqual()
{
    constexpr auto inf = std::numeric_limits<double>::infinity();
    QVERIFY(nearly_equal(inf, inf));
}

void TestMathUtils::negative_infinity_areEqual()
{
    constexpr auto inf = std::numeric_limits<double>::infinity();
    QVERIFY(nearly_equal(-inf, -inf));
}

void TestMathUtils::opposite_infinities_areNotEqual()
{
    constexpr auto inf = std::numeric_limits<double>::infinity();
    QVERIFY(!nearly_equal(inf, -inf));
}

// ---------------------------------------------------------------------------
// Normal values
// ---------------------------------------------------------------------------

void TestMathUtils::same_small_positive_values_areEqual()
{
    QVERIFY(nearly_equal(0.001, 0.001));
}

void TestMathUtils::same_large_positive_values_areEqual()
{
    QVERIFY(nearly_equal(1234567.89, 1234567.89));
}

void TestMathUtils::same_negative_values_areEqual()
{
    QVERIFY(nearly_equal(-42.5, -42.5));
}

void TestMathUtils::different_values_areNotEqual()
{
    QVERIFY(!nearly_equal(1.0, 2.0));
    QVERIFY(!nearly_equal(-1.0, 1.0));
}

void TestMathUtils::values_within_default_epsilon_areEqual()
{
    // Difference is 1e-13, well within default eps_rel of 1e-12 at magnitude 1.0
    QVERIFY(nearly_equal(1.0, 1.0 + 1e-13));
}

void TestMathUtils::values_outside_default_epsilon_areNotEqual()
{
    // Difference is 1e-11, outside default eps_rel of 1e-12 at magnitude 1.0
    QVERIFY(!nearly_equal(1.0, 1.0 + 1e-11));
}

// ---------------------------------------------------------------------------
// Near zero — the qFuzzyCompare(x + 1.0, y + 1.0) failure zone
// ---------------------------------------------------------------------------

void TestMathUtils::near_zero_identical_areEqual()
{
    QVERIFY(nearly_equal(1e-20, 1e-20));
}

void TestMathUtils::near_zero_tiny_difference_areEqual()
{
    // Both values are so small that the absolute floor (double::min) handles them
    const auto a = std::numeric_limits<double>::min(); // ~2.2e-308
    QVERIFY(nearly_equal(a, a));
}

void TestMathUtils::near_zero_distinguishable_areNotEqual()
{
    // 1e-20 and 2e-20 differ by 100% — should not be considered equal
    QVERIFY(!nearly_equal(1e-20, 2e-20));
}

void TestMathUtils::very_small_positive_values_areEqual()
{
    const auto v = 3e-200;
    QVERIFY(nearly_equal(v, v));
}

void TestMathUtils::very_small_but_different_values_areNotEqual()
{
    QVERIFY(!nearly_equal(1e-100, 2e-100));
}

// ---------------------------------------------------------------------------
// Extreme magnitudes
// ---------------------------------------------------------------------------

void TestMathUtils::very_large_values_equal()
{
    QVERIFY(nearly_equal(1e21, 1e21));
}

void TestMathUtils::very_large_values_slightlyDifferent_areNotEqual()
{
    // Differs by 1e9 at magnitude 1e21 — larger than eps_rel * mag = 1e9,
    // so should be considered not equal (boundary: 1e21 * 1e-12 = 1e9).
    QVERIFY(!nearly_equal(1e21, 1e21 + 2e9));
}

void TestMathUtils::value_at_1e21_withinEpsilon_areEqual()
{
    // Differs by 1e8, well within 1e21 * 1e-12 = 1e9
    QVERIFY(nearly_equal(1e21, 1e21 + 1e8));
}

// ---------------------------------------------------------------------------
// Mixed scales
// ---------------------------------------------------------------------------

void TestMathUtils::small_and_large_areNotEqual()
{
    QVERIFY(!nearly_equal(1e-20, 1e21));
}

void TestMathUtils::one_and_zero_areNotEqual()
{
    QVERIFY(!nearly_equal(0.0, 1.0));
    QVERIFY(!nearly_equal(1.0, 0.0));
}

// ---------------------------------------------------------------------------
// NaN
// ---------------------------------------------------------------------------

void TestMathUtils::nan_and_nan_areNotEqual()
{
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    QVERIFY(!nearly_equal(nan, nan));
}

void TestMathUtils::nan_and_value_areNotEqual()
{
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    QVERIFY(!nearly_equal(nan, 1.0));
}

void TestMathUtils::value_and_nan_areNotEqual()
{
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    QVERIFY(!nearly_equal(1.0, nan));
}

// ---------------------------------------------------------------------------
// Custom epsilon
// ---------------------------------------------------------------------------

void TestMathUtils::custom_eps_rel_looser_considersEqual()
{
    // 1e-6 difference at magnitude 1.0 — outside default, inside loose 1e-5
    QVERIFY(!nearly_equal(1.0, 1.0 + 1e-6));
    QVERIFY(nearly_equal(1.0, 1.0 + 1e-6, 1e-5));
}

void TestMathUtils::custom_eps_rel_tighter_considersNotEqual()
{
    // 1e-14 difference at magnitude 1.0 — inside default 1e-12, outside tight 1e-15
    QVERIFY(nearly_equal(1.0, 1.0 + 1e-14));
    QVERIFY(!nearly_equal(1.0, 1.0 + 1e-14, 1e-15));
}

void TestMathUtils::custom_eps_abs_catchesNearZeroDifference()
{
    // With a generous absolute floor, two tiny-but-different values compare equal
    QVERIFY(nearly_equal(1e-20, 2e-20, 1e-12, 1e-19));
}

QTEST_GUILESS_MAIN(TestMathUtils)
#include "tst_math_utils.moc"
