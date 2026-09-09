//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "SampleDataGenerators.hpp"

#include <QtTest>

#include <cmath>

using namespace QAccelPlotExample;

class TestSampleDataGenerators : public QObject {
    Q_OBJECT

private slots:
    void rejectsNonPositiveCounts();
    void supportsSinglePointCurve();
    void appliesFrequencyScale();
    void acousticDataHasExpectedShape();
};

void TestSampleDataGenerators::rejectsNonPositiveCounts()
{
    QVERIFY(generateSinusoid(0).empty());
    QVERIFY(generateSinusoid(-1).empty());
    QVERIFY(generateRectangles(0).empty());
    QVERIFY(generateDiscreteWaveSamples(false, -1).empty());
    QVERIFY(generateContinuousWaveSamples(false, 0).empty());
    QVERIFY(generateAcousticData(-1).empty());
}

void TestSampleDataGenerators::supportsSinglePointCurve()
{
    const auto sine = generateContinuousWaveSamples(false, 1);
    QCOMPARE(sine.size(), size_t{2});
    QCOMPARE(sine[0], 0.0f);
    QCOMPARE(sine[1], 0.0f);

    const auto cosine = generateContinuousWaveSamples(true, 1);
    QCOMPARE(cosine.size(), size_t{2});
    QCOMPARE(cosine[0], 0.0f);
    QCOMPARE(cosine[1], 1.0f);
}

void TestSampleDataGenerators::appliesFrequencyScale()
{
    constexpr auto pointCount = 100;
    constexpr auto frequencyScale = 0.5f;
    const auto data = generateSinusoid(pointCount, 0.0f, 1.0f, 1000.0f, frequencyScale);

    QCOMPARE(data.size(), size_t{pointCount * 2});
    const auto expected = std::sin(25.0f * frequencyScale);
    QVERIFY(std::abs(data[51] - expected) < 1.0e-6f);
}

void TestSampleDataGenerators::acousticDataHasExpectedShape()
{
    constexpr auto pointCount = 32;
    const auto data = generateAcousticData(pointCount);

    QCOMPARE(data.size(), size_t{pointCount * 2});
    for (const auto value : data) {
        QVERIFY(std::isfinite(value));
    }
}

QTEST_APPLESS_MAIN(TestSampleDataGenerators)

#include "tst_sample_data_generators.moc"
