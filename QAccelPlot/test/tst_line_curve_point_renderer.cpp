//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "renderers/LineCurvePointRenderer.hpp"

#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QtTest/QtTest>

namespace QAccelPlot {

class LineCurvePointRendererTest : public QObject {
    Q_OBJECT

private slots:
    void verticalGradientMatchesLineAndFillOrientation();
};

void LineCurvePointRendererTest::verticalGradientMatchesLineAndFillOrientation()
{
    auto gradientPayload = GradientColorPayload{};
    gradientPayload.enabled = true;
    gradientPayload.direction = GradientDirection::Vertical;
    gradientPayload.stops = {{0.0f, QColor(Qt::red)}, {1.0f, QColor(Qt::blue)}};
    gradientPayload.gradientValueMin = 0.0;
    gradientPayload.gradientValueMax = 10.0;

    const auto data = std::vector<float>{0.0f, 0.0f, 0.0f, 10.0f};
    const auto sourceData = CurveDataView{data.data(), nullptr};

    const auto params = PointCurveRenderParams{data, sourceData, 2, true, QColor(Qt::white), false, 4.0, QVector2D(0, 0), QVector2D(1, 1), QVector2D(100, 100),
        false, false, false, 0.0, gradientPayload, nullptr, 0};

    auto renderer = LineCurvePointRenderer{};
    auto* node = static_cast<QSGGeometryNode*>(renderer.paint(nullptr, params));
    const auto* vertices = static_cast<const PointVertex*>(node->geometry()->vertexData());

    // gradient_line.vert flips the Vertical coordinate (1.0 - normalized) so that the
    // axis-minimum end of the data samples the last stop, matching how line strokes and
    // fills render a vertical gradient. Markers must use the same orientation.
    const auto minValueColor = QColor::fromRgbF(vertices[0].r, vertices[0].g, vertices[0].b, vertices[0].a);
    const auto maxValueColor = QColor::fromRgbF(vertices[6].r, vertices[6].g, vertices[6].b, vertices[6].a);

    QCOMPARE(minValueColor, QColor(Qt::blue));
    QCOMPARE(maxValueColor, QColor(Qt::red));

    delete node;
}

} // namespace QAccelPlot

using QAccelPlot::LineCurvePointRendererTest;
QTEST_GUILESS_MAIN(LineCurvePointRendererTest)
#include "tst_line_curve_point_renderer.moc"
