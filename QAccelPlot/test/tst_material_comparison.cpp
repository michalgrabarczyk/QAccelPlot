//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "DataTextureMaterial.hpp"
#include "LineMaterial.hpp"
#include "PointMaterial.hpp"

#include <QSGTexture>
#include <QtTest/QtTest>

#include <memory>

namespace QAccelPlot {

class TestTexture final : public QSGTexture {
public:
    explicit TestTexture(const qint64 key, bool* destroyed = nullptr)
        : key_(key)
        , destroyed_(destroyed)
    {
    }

    ~TestTexture() override
    {
        if (destroyed_) {
            *destroyed_ = true;
        }
    }

    qint64 comparisonKey() const override
    {
        return key_;
    }

    QSize textureSize() const override
    {
        return {1, 1};
    }

    bool hasAlphaChannel() const override
    {
        return false;
    }

    bool hasMipmaps() const override
    {
        return false;
    }

private:
    qint64 key_;
    bool* destroyed_;
};

class MaterialComparisonTest : public QObject {
    Q_OBJECT

private slots:
    void identicalLineMaterialsCompareEqual();
    void lineMaterialsCompareCommonUniforms();
    void lineMaterialsCompareTextureIdentity();
    void lineMaterialsCompareCompleteDashState();
    void pointMaterialsCompareMappingUniforms();
    void dataTextureIsDestroyedWithMaterial();
};

void MaterialComparisonTest::identicalLineMaterialsCompareEqual()
{
    auto left = LineMaterial{};
    auto right = LineMaterial{};

    QCOMPARE(left.compare(&right), 0);
    QCOMPARE(right.compare(&left), 0);
}

void MaterialComparisonTest::lineMaterialsCompareCommonUniforms()
{
    auto left = LineMaterial{};
    auto differentDomain = LineMaterial{};

    differentDomain.domainMin.setX(-1.0f);
    QVERIFY(left.compare(&differentDomain) != 0);

    auto differentViewport = LineMaterial{};
    differentViewport.viewportSize.setY(720.0f);
    QVERIFY(left.compare(&differentViewport) != 0);

    auto differentScale = LineMaterial{};
    differentScale.logScaleX = 1.0f;
    QVERIFY(left.compare(&differentScale) != 0);
}

void MaterialComparisonTest::lineMaterialsCompareTextureIdentity()
{
    auto left = LineMaterial{};
    auto right = LineMaterial{};
    left.dataTexture = std::make_unique<TestTexture>(1);
    right.dataTexture = std::make_unique<TestTexture>(2);

    QVERIFY(left.compare(&right) != 0);
    QVERIFY(right.compare(&left) != 0);
}

void MaterialComparisonTest::lineMaterialsCompareCompleteDashState()
{
    auto left = LineMaterial{};
    auto right = LineMaterial{};
    left.dashPatternSize = 2;
    right.dashPatternSize = 2;
    left.dashPeriod = right.dashPeriod = 5.0f;
    left.dashPattern[0] = 2.0f;
    left.dashPattern[1] = 3.0f;
    right.dashPattern[0] = 1.0f;
    right.dashPattern[1] = 4.0f;

    QVERIFY(left.compare(&right) != 0);

    right.dashPattern[0] = left.dashPattern[0];
    right.dashPattern[1] = left.dashPattern[1];
    right.dashOffset = 1.0f;
    QVERIFY(left.compare(&right) != 0);
}

void MaterialComparisonTest::pointMaterialsCompareMappingUniforms()
{
    auto left = PointMaterial{};
    auto differentDomain = PointMaterial{};

    differentDomain.domainMax.setY(2.0f);
    QVERIFY(left.compare(&differentDomain) != 0);

    auto differentColorMode = PointMaterial{};
    differentColorMode.useVertexColor = 1.0f;
    QVERIFY(left.compare(&differentColorMode) != 0);
}

void MaterialComparisonTest::dataTextureIsDestroyedWithMaterial()
{
    auto destroyed = false;
    {
        auto material = LineMaterial{};
        material.dataTexture = std::make_unique<TestTexture>(1, &destroyed);
    }

    QVERIFY(destroyed);
}

} // namespace QAccelPlot

using QAccelPlot::MaterialComparisonTest;
QTEST_GUILESS_MAIN(MaterialComparisonTest)
#include "tst_material_comparison.moc"
