//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/DataTexture.hpp"
#include "QAccelPlot/materials/PointMaterial.hpp"
#include "QAccelPlot/renderers/LineCurvePointRenderer.hpp"

#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QtTest/QtTest>

#include <cstring>
#include <memory>
#include <vector>

namespace QAccelPlot {

namespace {

PointCurveRenderParams makeParams(
    const std::vector<float>& data, const int pointCount, const GradientColorPayload& gradientPayload, std::shared_ptr<DataTexture> dataTexture = nullptr)
{
    return PointCurveRenderParams{nullptr, data, pointCount, true, std::move(dataTexture), QColor(Qt::white), false, 4.0, QVector2D(0, 0), QVector2D(1, 1),
        QVector2D(100, 100), false, false, false, 0.0, gradientPayload, nullptr, 0, 1.0, true};
}

} // namespace

class LineCurvePointRendererTest : public QObject {
    Q_OBJECT

private slots:
    void sameSizeDataUpdateKeepsVertices();
    void sharedDataTextureIsUsed();
    void verticalGradientSetsMaterialUniforms();
};

void LineCurvePointRendererTest::sameSizeDataUpdateKeepsVertices()
{
    const auto renderer = LineCurvePointRenderer{};
    const auto gradientPayload = GradientColorPayload{};
    const auto twoPoints = std::vector<float>{0.0f, 0.0f, 1.0f, 1.0f};
    auto node = std::unique_ptr<QSGGeometryNode>{static_cast<QSGGeometryNode*>(renderer.paint(nullptr, makeParams(twoPoints, 2, gradientPayload)))};
    auto* geometry = node->geometry();
    QCOMPARE(geometry->vertexCount(), 2 * LineCurvePointRenderer::kVerticesPerPoint);

    // Positions come from the data texture, so new data of the same size leaves the vertices alone.
    const auto byteCount = static_cast<std::size_t>(geometry->vertexCount()) * sizeof(PointVertex);
    std::memset(geometry->vertexData(), 0x5a, byteCount);
    const auto movedPoints = std::vector<float>{0.5f, 0.5f, 0.2f, 0.8f};
    QCOMPARE(renderer.paint(node.get(), makeParams(movedPoints, 2, gradientPayload)), node.get());
    QCOMPARE(static_cast<const unsigned char*>(geometry->vertexData())[byteCount - 1], 0x5a);

    const auto threePoints = std::vector<float>{0.0f, 0.0f, 0.5f, 0.5f, 1.0f, 1.0f};
    renderer.paint(node.get(), makeParams(threePoints, 3, gradientPayload));
    QCOMPARE(geometry->vertexCount(), 3 * LineCurvePointRenderer::kVerticesPerPoint);
    QCOMPARE(static_cast<const unsigned char*>(geometry->vertexData())[0], 0);
}

void LineCurvePointRendererTest::sharedDataTextureIsUsed()
{
    const auto renderer = LineCurvePointRenderer{};
    const auto gradientPayload = GradientColorPayload{};
    const auto data = std::vector<float>{0.0f, 0.0f};
    const auto lineTexture = std::make_shared<DataTexture>();
    const auto node
        = std::unique_ptr<QSGGeometryNode>{static_cast<QSGGeometryNode*>(renderer.paint(nullptr, makeParams(data, 1, gradientPayload, lineTexture)))};
    QCOMPARE(static_cast<PointMaterial*>(node->material())->dataTexture, lineTexture);
}

void LineCurvePointRendererTest::verticalGradientSetsMaterialUniforms()
{
    auto gradientPayload = GradientColorPayload{};
    gradientPayload.enabled = true;
    gradientPayload.direction = Qt::Vertical;
    gradientPayload.stops = {{0.0f, QColor(Qt::red)}, {1.0f, QColor(Qt::blue)}};
    gradientPayload.gradientValueMin = 0.0;
    gradientPayload.gradientValueMax = 10.0;

    const auto data = std::vector<float>{0.0f, 0.0f, 0.0f, 10.0f};
    const auto renderer = LineCurvePointRenderer{};
    const auto node = std::unique_ptr<QSGGeometryNode>{static_cast<QSGGeometryNode*>(renderer.paint(nullptr, makeParams(data, 2, gradientPayload)))};
    const auto* material = static_cast<const PointMaterial*>(node->material());
    QCOMPARE(material->useVertexColor, 1.0f);
    QCOMPARE(material->gradientDirection, 1.0f);
    QCOMPARE(material->gradientValueMin, 0.0f);
    QCOMPARE(material->gradientValueMax, 10.0f);

    gradientPayload.enabled = false;
    renderer.paint(node.get(), makeParams(data, 2, gradientPayload));
    QCOMPARE(material->useVertexColor, 0.0f);
}

} // namespace QAccelPlot

using QAccelPlot::LineCurvePointRendererTest;
QTEST_GUILESS_MAIN(LineCurvePointRendererTest)
#include "tst_line_curve_point_renderer.moc"
