//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "axis/Axis.hpp"

#include <QtTest/QtTest>

namespace QAccelPlot {

class AxisRescaleTest : public QObject {
    Q_OBJECT

private slots:
    void flatLinearData_synthesizesRangeAroundValue();
    void flatLinearZeroData_usesFixedFallbackRange();
    void flatLinearTinyValue_keepsRangeProportionalToMagnitude();
    void flatLogScaleData_synthesizesPositiveRange();
    void flatNonPositiveLogScaleData_synthesizesPositiveRange();
};

void AxisRescaleTest::flatLinearData_synthesizesRangeAroundValue()
{
    auto axis = Axis{};
    axis.setDataMin(5.0);
    axis.setDataMax(5.0);

    axis.rescaleToData();

    QVERIFY(axis.viewportMin() < 5.0);
    QVERIFY(axis.viewportMax() > 5.0);
}

void AxisRescaleTest::flatLinearZeroData_usesFixedFallbackRange()
{
    auto axis = Axis{};
    axis.setDataMin(0.0);
    axis.setDataMax(0.0);

    axis.rescaleToData();

    QCOMPARE(axis.viewportMin(), -1.0);
    QCOMPARE(axis.viewportMax(), 1.0);
}

void AxisRescaleTest::flatLinearTinyValue_keepsRangeProportionalToMagnitude()
{
    auto axis = Axis{};
    axis.setDataMin(1e-12);
    axis.setDataMax(1e-12);

    axis.rescaleToData();

    // A fixed +/-1 fallback would swallow a value this small, rendering it
    // visually indistinguishable from zero. The synthesized range must instead
    // stay proportional to the value's own magnitude.
    QVERIFY(axis.viewportMin() < 1e-12);
    QVERIFY(axis.viewportMax() > 1e-12);
    QVERIFY(axis.viewportMax() - axis.viewportMin() < 1e-10);
}

void AxisRescaleTest::flatLogScaleData_synthesizesPositiveRange()
{
    auto axis = Axis{};
    axis.setLogScale(true);
    axis.setDataMin(10.0);
    axis.setDataMax(10.0);

    axis.rescaleToData();

    QVERIFY(axis.viewportMin() > 0.0);
    QVERIFY(axis.viewportMax() > axis.viewportMin());
}

void AxisRescaleTest::flatNonPositiveLogScaleData_synthesizesPositiveRange()
{
    auto axis = Axis{};
    axis.setLogScale(true);
    axis.setDataMin(-5.0);
    axis.setDataMax(-5.0);

    axis.rescaleToData();

    QVERIFY(axis.viewportMin() > 0.0);
    QVERIFY(axis.viewportMax() > axis.viewportMin());
}

} // namespace QAccelPlot

using QAccelPlot::AxisRescaleTest;
QTEST_MAIN(AxisRescaleTest)
#include "tst_axis_rescale.moc"
