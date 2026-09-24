//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/materials/PointMaterial.hpp"
#include "QAccelPlot/series/LineCurve.hpp"

#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QtTest/QtTest>

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

QTEST_MAIN(LineCurveRenderingTest)
#include "tst_line_curve_rendering.moc"
