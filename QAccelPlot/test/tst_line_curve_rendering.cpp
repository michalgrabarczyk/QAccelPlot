//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/PointMaterial.hpp"
#include "QAccelPlot/renderers/LineCurveLineRenderer.hpp"
#include "QAccelPlot/series/LineCurve.hpp"

#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QtTest/QtTest>

#include <algorithm>
#include <cstring>
#include <memory>

namespace {
class RenderableCurve final : public QAccelPlot::LineCurve {
public:
    using LineCurve::updatePaintNode;
};
}

class LineCurveRenderingTest : public QObject {
    Q_OBJECT
private slots:
    void markersSurviveSinglePointDatasets();
    void prebuiltVertexCacheIsUploaded();
};

void LineCurveRenderingTest::markersSurviveSinglePointDatasets()
{
    auto window = QQuickWindow{};
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom curve materials require a hardware scene graph backend");
    }
    auto xAxis = QAccelPlot::Axis{};
    auto yAxis = QAccelPlot::Axis{};
    auto curve = RenderableCurve{};
    curve.setParentItem(window.contentItem());
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);
    curve.setPlotRect({0, 0, 100, 100});
    curve.marker()->setShape(QAccelPlot::LineCurve::MarkerShape::Circle);
    auto node = std::unique_ptr<QSGNode>{};

    for (const auto count : {1, 2, 1}) {
        curve.setDataF(std::vector<float>(count * 2, 0.5f), count);
        node.reset(curve.updatePaintNode(node.release(), nullptr));
        QVERIFY(node);
        QCOMPARE(node->childCount(), 2);
        auto* markers = static_cast<QSGGeometryNode*>(node->lastChild());
        QVERIFY(dynamic_cast<QAccelPlot::PointMaterial*>(markers->material()));
        QCOMPARE(markers->geometry()->vertexCount(), count * 6);
    }
    curve.clearData();
    node.reset(curve.updatePaintNode(node.release(), nullptr));
    QVERIFY(!node);
}

void LineCurveRenderingTest::prebuiltVertexCacheIsUploaded()
{
    auto window = QQuickWindow{};
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom curve materials require a hardware scene graph backend");
    }
    auto xAxis = QAccelPlot::Axis{};
    auto yAxis = QAccelPlot::Axis{};
    auto curve = RenderableCurve{};
    curve.setParentItem(window.contentItem());
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);
    curve.setPlotRect({0, 0, 100, 100});
    // Distinguish uploaded caches from ones the curve would build itself by reversing their bytes.
    const auto reversedCache = [](const std::vector<float>& data, const int pointCount) {
        auto cache = std::vector<char>{};
        QAccelPlot::LineCurveLineRenderer{}.buildVertexCache(data, pointCount, cache);
        std::reverse(cache.begin(), cache.end());
        return cache;
    };
    const auto lineVerticesMatch = [](const QSGNode* root, const std::vector<char>& cache) {
        const auto* geometry = static_cast<const QSGGeometryNode*>(root->lastChild())->geometry();
        return static_cast<std::size_t>(geometry->vertexCount() * geometry->sizeOfVertex()) == cache.size()
            && std::memcmp(geometry->vertexData(), cache.data(), cache.size()) == 0;
    };

    const auto threePoints = std::vector<float>{0.0f, 0.1f, 0.5f, 0.9f, 1.0f, 0.2f};
    const auto threePointCache = reversedCache(threePoints, 3);
    curve.setDataFNoRangeWithCache(std::vector<float>(threePoints), 3, std::vector<char>(threePointCache));
    auto node = std::unique_ptr<QSGNode>{curve.updatePaintNode(nullptr, nullptr)};
    QVERIFY(node);
    QVERIFY(lineVerticesMatch(node.get(), threePointCache));

    // Line vertices depend only on the point count, so a new count re-uploads the cache.
    const auto fourPoints = std::vector<float>{0.0f, 0.1f, 0.3f, 0.9f, 0.6f, 0.2f, 1.0f, 0.5f};
    const auto fourPointCache = reversedCache(fourPoints, 4);
    curve.setDataFNoRangeWithCache(fourPoints.data(), 4, std::vector<char>(fourPointCache));
    node.reset(curve.updatePaintNode(node.release(), nullptr));
    QVERIFY(lineVerticesMatch(node.get(), fourPointCache));
}

QTEST_MAIN(LineCurveRenderingTest)
#include "tst_line_curve_rendering.moc"
