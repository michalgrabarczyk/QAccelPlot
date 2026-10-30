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
#include <cstddef>
#include <cstring>
#include <memory>
#include <vector>

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
    void lineVerticesFollowPointCount();
    void gradientDataRangeIsCurveExtent();
    void markerVerticesFollowPointCount();
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

void LineCurveRenderingTest::lineVerticesFollowPointCount()
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
    const auto expectedVertices = [](const int pointCount) {
        auto cache
            = std::vector<char>(static_cast<std::size_t>(pointCount) * QAccelPlot::LineCurveLineRenderer::vertexCacheBytesPerPoint(QAccelPlot::StepMode::None));
        QAccelPlot::LineCurveLineRenderer::writeVertexCache(cache, 0, pointCount, QAccelPlot::StepMode::None);
        return cache;
    };
    const auto lineVerticesMatch = [](const QSGNode* root, const std::vector<char>& cache) {
        const auto* geometry = static_cast<const QSGGeometryNode*>(root->lastChild())->geometry();
        return static_cast<std::size_t>(geometry->vertexCount() * geometry->sizeOfVertex()) == cache.size()
            && std::memcmp(geometry->vertexData(), cache.data(), cache.size()) == 0;
    };

    // The curve extends its vertex cache when the point count grows and trims it when the count shrinks.
    auto node = std::unique_ptr<QSGNode>{};
    for (const auto count : {3, 5, 5, 2, 4}) {
        curve.setDataF(std::vector<float>(static_cast<std::size_t>(count) * 2, 0.5f), count);
        node.reset(curve.updatePaintNode(node.release(), nullptr));
        QVERIFY(node);
        QVERIFY(lineVerticesMatch(node.get(), expectedVertices(count)));
    }
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

void LineCurveRenderingTest::markerVerticesFollowPointCount()
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

    auto node = std::unique_ptr<QSGNode>{};
    for (const auto count : {3, 3, 5, 2}) {
        curve.setDataF(std::vector<float>(static_cast<std::size_t>(count) * 2, 0.5f), count);
        node.reset(curve.updatePaintNode(node.release(), nullptr));
        QVERIFY(node);
        const auto* geometry = static_cast<const QSGGeometryNode*>(node.get())->geometry();
        QCOMPARE(geometry->vertexCount(), count * QAccelPlot::LineCurvePointRenderer::kVerticesPerPoint);
        const auto* bytes = static_cast<const char*>(geometry->vertexData());
        const auto byteCount = static_cast<std::size_t>(geometry->vertexCount() * geometry->sizeOfVertex());
        QVERIFY(std::all_of(bytes, bytes + byteCount, [](const char byte) { return byte == 0; }));
    }
}

QTEST_MAIN(LineCurveRenderingTest)
#include "tst_line_curve_rendering.moc"
