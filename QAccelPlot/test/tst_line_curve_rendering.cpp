//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/effects/Colormap.hpp"
#include "QAccelPlot/effects/GradientStroke.hpp"
#include "QAccelPlot/materials/PointMaterial.hpp"
#include "QAccelPlot/renderers/LineCurveLineRenderer.hpp"
#include "QAccelPlot/renderers/LineCurvePointRenderer.hpp"
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
    void gradientDataRangeIsCurveExtent();
    void markerCacheSurvivesSameSizeDataUpdates();
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
    curve.marker()->setShape(QAccelPlot::MarkerShape::Circle);
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
    curve.setDataFWithCache(std::vector<float>(threePoints), 3, std::vector<char>(threePointCache));
    auto node = std::unique_ptr<QSGNode>{curve.updatePaintNode(nullptr, nullptr)};
    QVERIFY(node);
    QVERIFY(lineVerticesMatch(node.get(), threePointCache));

    // Line vertices depend only on the point count, so a new count re-uploads the cache.
    const auto fourPoints = std::vector<float>{0.0f, 0.1f, 0.3f, 0.9f, 0.6f, 0.2f, 1.0f, 0.5f};
    const auto fourPointCache = reversedCache(fourPoints, 4);
    curve.setDataFWithCache(fourPoints.data(), 4, std::vector<char>(fourPointCache));
    node.reset(curve.updatePaintNode(node.release(), nullptr));
    QVERIFY(lineVerticesMatch(node.get(), fourPointCache));
}

void LineCurveRenderingTest::gradientDataRangeIsCurveExtent()
{
    auto window = QQuickWindow{};
    if (window.rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        QSKIP("Custom curve materials require a hardware scene graph backend");
    }
    auto xAxis = QAccelPlot::Axis{};
    auto yAxis = QAccelPlot::Axis{};
    auto colormap = QAccelPlot::Colormap{};
    colormap.setPreset(QAccelPlot::Colormap::Preset::Grayscale);
    auto curve = RenderableCurve{};
    curve.setParentItem(window.contentItem());
    curve.setXAxis(&xAxis);
    curve.setYAxis(&yAxis);
    curve.setPlotRect({0, 0, 100, 100});
    curve.setLineStyle(nullptr);
    curve.marker()->setShape(QAccelPlot::MarkerShape::Square);
    auto* stroke = new QAccelPlot::GradientStroke(&curve);
    stroke->setDirection(Qt::Vertical);
    stroke->setColormap(&colormap);
    auto effects = curve.effects();
    effects.append(&effects, stroke);
    curve.setData(QList<QPointF>{{0.0, 0.0}, {1.0, 1.0}});

    // Another series on the same axis widens the axis data range to [0, 10].
    auto other = QAccelPlot::LineCurve{};
    other.setXAxis(&xAxis);
    other.setYAxis(&yAxis);
    other.setData(QList<QPointF>{{0.0, 0.0}, {1.0, 10.0}});

    const auto node = std::unique_ptr<QSGNode>{curve.updatePaintNode(nullptr, nullptr)};
    QVERIFY(node);
    const auto* material = static_cast<const QAccelPlot::PointMaterial*>(static_cast<const QSGGeometryNode*>(node.get())->material());
    QCOMPARE(material->useVertexColor, 1.0f);
    QCOMPARE(material->gradientDirection, 1.0f);
    QCOMPARE(material->gradientValueMax - material->gradientValueMin, 1.0f);
}

void LineCurveRenderingTest::markerCacheSurvivesSameSizeDataUpdates()
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
    curve.setLineStyle(nullptr);
    curve.marker()->setShape(QAccelPlot::MarkerShape::Circle);

    // A marked cache tells an uploaded cache apart from the zeroed one the curve would build itself.
    constexpr auto kPointCount = 3;
    const auto markedCache = std::vector<char>(kPointCount * QAccelPlot::LineCurvePointRenderer::kVerticesPerPoint * sizeof(QAccelPlot::PointVertex), 0x5a);
    curve.setDataFWithCache(std::vector<float>(kPointCount * 2, 0.5f), kPointCount, std::vector<char>(markedCache));
    // Marker vertices hold no positions, so new data of the same size keeps the cache.
    curve.setDataF(std::vector<float>(kPointCount * 2, 0.25f), kPointCount);

    const auto node = std::unique_ptr<QSGNode>{curve.updatePaintNode(nullptr, nullptr)};
    QVERIFY(node);
    const auto* geometry = static_cast<const QSGGeometryNode*>(node.get())->geometry();
    QCOMPARE(static_cast<std::size_t>(geometry->vertexCount() * geometry->sizeOfVertex()), markedCache.size());
    QVERIFY(std::memcmp(geometry->vertexData(), markedCache.data(), markedCache.size()) == 0);
}

QTEST_MAIN(LineCurveRenderingTest)
#include "tst_line_curve_rendering.moc"
