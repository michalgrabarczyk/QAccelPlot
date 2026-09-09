//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "GradientCoordinateUtils.hpp"

#include <QtTest/QtTest>

#include <algorithm>

class TestGradientCoordinates : public QObject {
    Q_OBJECT

private slots:
    void verticalCoordinate_remainsUnboundedUntilFragmentSampling();
    void verticalCoordinate_interpolationReachesMaximumAtConfiguredValue();
};

void TestGradientCoordinates::verticalCoordinate_remainsUnboundedUntilFragmentSampling()
{
    constexpr auto minimum = -4.0f;
    constexpr auto maximum = 1.0f;

    QCOMPARE(QAccelPlot::unboundedGradientCoordinate(QAccelPlot::GradientDirection::Vertical, minimum, minimum, maximum), 1.0f);
    QCOMPARE(QAccelPlot::unboundedGradientCoordinate(QAccelPlot::GradientDirection::Vertical, maximum, minimum, maximum), 0.0f);
    QVERIFY(QAccelPlot::unboundedGradientCoordinate(QAccelPlot::GradientDirection::Vertical, 5.2f, minimum, maximum) < 0.0f);
}

void TestGradientCoordinates::verticalCoordinate_interpolationReachesMaximumAtConfiguredValue()
{
    constexpr auto baseline = -4.0f;
    constexpr auto curve = 5.2f;
    constexpr auto maximum = 1.0f;

    const auto baselineCoordinate = QAccelPlot::unboundedGradientCoordinate(QAccelPlot::GradientDirection::Vertical, baseline, baseline, maximum);
    const auto curveCoordinate = QAccelPlot::unboundedGradientCoordinate(QAccelPlot::GradientDirection::Vertical, curve, baseline, maximum);
    const auto interpolation = (maximum - baseline) / (curve - baseline);
    const auto coordinateAtMaximum = baselineCoordinate + interpolation * (curveCoordinate - baselineCoordinate);

    QVERIFY(qAbs(coordinateAtMaximum) < 1e-6f);
    QCOMPARE(std::clamp(curveCoordinate, 0.0f, 1.0f), 0.0f);
}

QTEST_GUILESS_MAIN(TestGradientCoordinates)
#include "tst_gradient_coordinates.moc"
