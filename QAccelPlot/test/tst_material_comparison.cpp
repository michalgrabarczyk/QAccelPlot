//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/DataTextureMaterial.hpp"
#include "QAccelPlot/materials/LineMaterial.hpp"
#include "QAccelPlot/materials/PointCloudMaterial.hpp"
#include "QAccelPlot/materials/PointMaterial.hpp"

#include <QSGTexture>
#include <QtTest/QtTest>

#include <functional>
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
    void pointMaterialsCompareMarkerStyleUniforms();
    void dataTextureIsDestroyedWithMaterial();
    void identicalPointCloudMaterialsCompareEqual();
    void pointCloudMaterialsCompareMarkerAndColorUniforms();
    void pointCloudMaterialsCompareDataTextureIdentity();
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

void MaterialComparisonTest::pointMaterialsCompareMarkerStyleUniforms()
{
    const auto left = PointMaterial{};

    auto differentShape = PointMaterial{};
    differentShape.shapeType = 9;
    QVERIFY(left.compare(&differentShape) != 0);

    auto differentStrokeWidth = PointMaterial{};
    differentStrokeWidth.markerStrokeWidth = 2.0f;
    QVERIFY(left.compare(&differentStrokeWidth) != 0);

    auto hollow = PointMaterial{};
    hollow.markerFilled = 0.0f;
    QVERIFY(left.compare(&hollow) != 0);
    QVERIFY(hollow.compare(&left) == -left.compare(&hollow));
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

void MaterialComparisonTest::identicalPointCloudMaterialsCompareEqual()
{
    auto left = PointCloudMaterial{};
    auto right = PointCloudMaterial{};

    QCOMPARE(left.compare(&right), 0);
    QCOMPARE(right.compare(&left), 0);
}

void MaterialComparisonTest::pointCloudMaterialsCompareMarkerAndColorUniforms()
{
    const auto left = PointCloudMaterial{};
    const auto differsFrom = [&left](const std::function<void(PointCloudMaterial&)>& change) {
        auto other = PointCloudMaterial{};
        change(other);
        return left.compare(&other) != 0 && other.compare(&left) == -left.compare(&other);
    };

    QVERIFY(differsFrom([](PointCloudMaterial& m) { m.markerSize = 7.0f; }));
    QVERIFY(differsFrom([](PointCloudMaterial& m) { m.antialiasingEnabled = 0.0f; }));
    QVERIFY(differsFrom([](PointCloudMaterial& m) { m.antialiasingFeather = 2.0f; }));
    QVERIFY(differsFrom([](PointCloudMaterial& m) { m.valueMin = -1.0f; }));
    QVERIFY(differsFrom([](PointCloudMaterial& m) { m.valueMax = 5.0f; }));
    QVERIFY(differsFrom([](PointCloudMaterial& m) { m.stride = 3.0f; }));
    QVERIFY(differsFrom([](PointCloudMaterial& m) { m.shapeType = 4; }));
    QVERIFY(differsFrom([](PointCloudMaterial& m) { m.markerFilled = 0.0f; }));
    QVERIFY(differsFrom([](PointCloudMaterial& m) { m.markerStrokeWidth = 2.0f; }));
    QVERIFY(differsFrom([](PointCloudMaterial& m) { m.useVertexColor = 1.0f; }));
    QVERIFY(differsFrom([](PointCloudMaterial& m) { m.domainMax.setX(3.0f); }));
}

void MaterialComparisonTest::pointCloudMaterialsCompareDataTextureIdentity()
{
    auto left = PointCloudMaterial{};
    auto right = PointCloudMaterial{};
    left.dataTexture = std::make_unique<TestTexture>(10);
    right.dataTexture = std::make_unique<TestTexture>(11);

    QVERIFY(left.compare(&right) != 0);
}

} // namespace QAccelPlot

using QAccelPlot::MaterialComparisonTest;
QTEST_GUILESS_MAIN(MaterialComparisonTest)
#include "tst_material_comparison.moc"
