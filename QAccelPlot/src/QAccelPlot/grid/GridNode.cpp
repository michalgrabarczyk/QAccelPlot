//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/grid/GridNode.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/axis/AxisTickPainter.hpp"
#include "QAccelPlot/grid/Grid.hpp"

#include <QSGFlatColorMaterial>
#include <QSGGeometry>
#include <QSGGeometryNode>

namespace QAccelPlot {

static QSGNode* buildRectangleNode(const QVector<QRectF>& rects, const QColor& color)
{
    if (rects.isEmpty()) {
        return nullptr;
    }
    static constexpr auto kVerticesPerRect = 6; // 2 triangles × 3 vertices
    const auto rectCount = static_cast<int>(rects.size());
    auto* g = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), rectCount * kVerticesPerRect);
    g->setDrawingMode(QSGGeometry::DrawTriangles);
    auto* v = g->vertexDataAsPoint2D();
    auto idx = 0;
    for (const QRectF& r : rects) {
        const auto x1 = static_cast<float>(r.x());
        const auto y1 = static_cast<float>(r.y());
        const auto x2 = static_cast<float>(r.x() + r.width());
        const auto y2 = static_cast<float>(r.y() + r.height());
        v[idx++] = {x1, y1};
        v[idx++] = {x2, y1};
        v[idx++] = {x2, y2};
        v[idx++] = {x1, y1};
        v[idx++] = {x2, y2};
        v[idx++] = {x1, y2};
    }
    auto* node = new QSGGeometryNode;
    node->setGeometry(g);
    node->setFlag(QSGNode::OwnsGeometry);
    auto* mat = new QSGFlatColorMaterial;
    mat->setColor(color);
    node->setMaterial(mat);
    node->setFlag(QSGNode::OwnsMaterial);
    return node;
}

void GridNode::update(const Grid* grid, const Axis* xAxis, const Axis* yAxis, const QRectF& plotRect)
{
    while (childCount() > 0) {
        QSGNode* c = firstChild();
        removeChildNode(c);
        delete c;
    }

    plotRect_ = plotRect;

    if (plotRect_.isEmpty() || (!xAxis && !yAxis)) {
        return;
    }
    if (!grid->gridVisible() && !grid->subGridVisible()) {
        return;
    }
    if (!grid->gridHorizontalLinesVisible() && !grid->gridVerticalLinesVisible() && !grid->subGridHorizontalLinesVisible()
        && !grid->subGridVerticalLinesVisible()) {
        return;
    }

    auto mainRects = QVector<QRectF>{};
    auto subRects = QVector<QRectF>{};

    // X axis: produces vertical grid lines
    collectAxisGridLines(mainRects, subRects, grid,
        GridLineCollectionParams{xAxis, grid->gridVerticalLinesVisible(), grid->subGridVerticalLinesVisible(), [&](const auto val, const auto halfW) {
                                     const auto x = plotRect_.x() + xAxis->coordToPixel(val, plotRect_.width());
                                     return QRectF(x - halfW, plotRect_.y(), halfW * 2.0, plotRect_.height());
                                 }});

    // Y axis: produces horizontal grid lines
    collectAxisGridLines(mainRects, subRects, grid,
        GridLineCollectionParams{yAxis, grid->gridHorizontalLinesVisible(), grid->subGridHorizontalLinesVisible(), [&](const auto val, const auto halfW) {
                                     const auto y = plotRect_.y() + yAxis->coordToPixel(val, plotRect_.height());
                                     return QRectF(plotRect_.x(), y - halfW, plotRect_.width(), halfW * 2.0);
                                 }});

    if (grid->subGridVisible()) {
        if (auto* node = buildRectangleNode(subRects, grid->subGridColor())) {
            appendChildNode(node);
        }
    }
    if (grid->gridVisible()) {
        if (auto* node = buildRectangleNode(mainRects, grid->gridColor())) {
            appendChildNode(node);
        }
    }
}

void GridNode::collectAxisGridLines(QVector<QRectF>& mainRects, QVector<QRectF>& subRects, const Grid* grid, const GridLineCollectionParams& p) const
{
    if (!p.axis || (!p.gridLinesVisible && !p.subGridLinesVisible)) {
        return;
    }
    // Values only: computeTicks() formats labels, which may call into QML/JS and must not run on the render thread.
    const auto ticks = AxisTickPainter::computeTickValues(p.axis->viewportMin(), p.axis->viewportMax(), p.axis->logScale(), p.axis->ticker());
    if (p.gridLinesVisible) {
        for (const auto value : ticks.majorValues) {
            mainRects.append(p.makeRect(value, grid->lineWidth() / 2.0));
        }
    }
    if (p.subGridLinesVisible) {
        for (const auto value : ticks.subtickValues) {
            subRects.append(p.makeRect(value, grid->subGridLineWidth() / 2.0));
        }
    }
}

} // namespace QAccelPlot
