//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"

#include <QQmlParserStatus>
#include <QSignalSpy>
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
    void nonFlatNonPositiveLogScaleData_synthesizesPositiveRange();
    void rescaleToData_emitsRangeChangedOnce();
    void autoRescale_isOffByDefault();
    void autoRescale_enablingFitsCurrentDataRange();
    void autoRescale_followsDataRangeChanges();
    void autoRescale_replacesManualViewportOnNextDataRangeChange();
    void autoRescale_emitsRangeChangedOncePerDataRangeUpdate();
    void autoRescale_flatDataSynthesizesRange();
    void autoRescale_logScaleKeepsViewportPositive();
    void autoRescale_fitsDeclaredRangeOnComponentCompletion();
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

void AxisRescaleTest::nonFlatNonPositiveLogScaleData_synthesizesPositiveRange()
{
    auto axis = Axis{};
    axis.setLogScale(true);
    axis.setDataMin(-10.0);
    axis.setDataMax(-1.0);

    axis.rescaleToData();

    QVERIFY(axis.viewportMin() > 0.0);
    QVERIFY(axis.viewportMax() > axis.viewportMin());
    QCOMPARE(axis.coordToPixel(axis.viewportMax(), 100.0), 100.0);
}

void AxisRescaleTest::rescaleToData_emitsRangeChangedOnce()
{
    auto axis = Axis{};
    axis.setDataMin(-3.0);
    axis.setDataMax(7.0);
    auto rangeSpy = QSignalSpy{&axis, &Axis::rangeChanged};

    axis.rescaleToData();

    QCOMPARE(rangeSpy.count(), 1);
    QCOMPARE(axis.viewportMin(), -3.0);
    QCOMPARE(axis.viewportMax(), 7.0);
}

void AxisRescaleTest::autoRescale_isOffByDefault()
{
    auto axis = Axis{};
    QVERIFY(!axis.autoRescale());

    axis.setDataRange(-3.0, 7.0);

    QCOMPARE(axis.viewportMin(), 0.0);
    QCOMPARE(axis.viewportMax(), 1.0);
}

void AxisRescaleTest::autoRescale_enablingFitsCurrentDataRange()
{
    auto axis = Axis{};
    axis.setDataRange(-3.0, 7.0);
    auto changedSpy = QSignalSpy{&axis, &Axis::autoRescaleChanged};

    axis.setAutoRescale(true);

    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(axis.viewportMin(), -3.0);
    QCOMPARE(axis.viewportMax(), 7.0);

    axis.setAutoRescale(true);
    QCOMPARE(changedSpy.count(), 1);
}

void AxisRescaleTest::autoRescale_followsDataRangeChanges()
{
    auto axis = Axis{};
    axis.setAutoRescale(true);

    axis.setDataRange(2.0, 4.0);
    QCOMPARE(axis.viewportMin(), 2.0);
    QCOMPARE(axis.viewportMax(), 4.0);

    axis.setDataMax(9.0);
    QCOMPARE(axis.viewportMin(), 2.0);
    QCOMPARE(axis.viewportMax(), 9.0);

    axis.setDataMin(-1.0);
    QCOMPARE(axis.viewportMin(), -1.0);
    QCOMPARE(axis.viewportMax(), 9.0);

    axis.setAutoRescale(false);
    axis.setDataRange(0.0, 100.0);
    QCOMPARE(axis.viewportMin(), -1.0);
    QCOMPARE(axis.viewportMax(), 9.0);
}

void AxisRescaleTest::autoRescale_replacesManualViewportOnNextDataRangeChange()
{
    auto axis = Axis{};
    axis.setAutoRescale(true);
    axis.setDataRange(2.0, 4.0);

    axis.setViewportMin(2.5);
    axis.setViewportMax(3.5);
    QCOMPARE(axis.viewportMin(), 2.5);
    QCOMPARE(axis.viewportMax(), 3.5);

    axis.setDataRange(2.0, 5.0);
    QCOMPARE(axis.viewportMin(), 2.0);
    QCOMPARE(axis.viewportMax(), 5.0);
}

void AxisRescaleTest::autoRescale_emitsRangeChangedOncePerDataRangeUpdate()
{
    auto axis = Axis{};
    axis.setAutoRescale(true);
    auto rangeSpy = QSignalSpy{&axis, &Axis::rangeChanged};

    axis.setDataRange(-3.0, 7.0);
    QCOMPARE(rangeSpy.count(), 1);

    axis.setDataRange(-3.0, 7.0);
    QCOMPARE(rangeSpy.count(), 1);
}

void AxisRescaleTest::autoRescale_flatDataSynthesizesRange()
{
    auto axis = Axis{};
    axis.setAutoRescale(true);

    axis.setDataRange(5.0, 5.0);

    QVERIFY(axis.viewportMin() < 5.0);
    QVERIFY(axis.viewportMax() > 5.0);
}

void AxisRescaleTest::autoRescale_logScaleKeepsViewportPositive()
{
    auto axis = Axis{};
    axis.setLogScale(true);
    axis.setAutoRescale(true);

    axis.setDataRange(-10.0, 100.0);

    QVERIFY(axis.viewportMin() > 0.0);
    QCOMPARE(axis.viewportMax(), 100.0);
}

void AxisRescaleTest::autoRescale_fitsDeclaredRangeOnComponentCompletion()
{
    // Mirrors a QML declaration that assigns the viewport after autoRescale and the data range.
    auto axis = Axis{};
    auto& parserStatus = static_cast<QQmlParserStatus&>(axis);
    parserStatus.classBegin();
    axis.setAutoRescale(true);
    axis.setDataMin(-2.5);
    axis.setDataMax(2.5);
    axis.setViewportMin(0.0);
    axis.setViewportMax(1.0);

    parserStatus.componentComplete();

    QCOMPARE(axis.viewportMin(), -2.5);
    QCOMPARE(axis.viewportMax(), 2.5);
}

} // namespace QAccelPlot

using QAccelPlot::AxisRescaleTest;
QTEST_MAIN(AxisRescaleTest)
#include "tst_axis_rescale.moc"
