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

#include <cstring>
#include <vector>

namespace QAccelPlot {

namespace {

QSGGeometryNode* createRectanglesNode()
{
    auto* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);
    auto* node = new QSGGeometryNode;
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(new QSGFlatColorMaterial);
    node->setFlag(QSGNode::OwnsMaterial);
    return node;
}

std::vector<QSGGeometry::Point2D> rectangleVertices(const QVector<QRectF>& rects)
{
    auto vertices = std::vector<QSGGeometry::Point2D>{};
    vertices.reserve(static_cast<std::size_t>(rects.size()) * 6);
    for (const auto& r : rects) {
        const auto x1 = static_cast<float>(r.x());
        const auto y1 = static_cast<float>(r.y());
        const auto x2 = static_cast<float>(r.x() + r.width());
        const auto y2 = static_cast<float>(r.y() + r.height());
        vertices.insert(vertices.end(), {{x1, y1}, {x2, y1}, {x2, y2}, {x1, y1}, {x2, y2}, {x1, y2}});
    }
    return vertices;
}

// Marks \a node dirty only for what changed, so an unchanged grid leaves the renderer's batches intact.
void updateRectanglesNode(QSGGeometryNode* node, const QVector<QRectF>& rects, const QColor& color)
{
    const auto vertices = rectangleVertices(rects);
    const auto vertexCount = static_cast<int>(vertices.size());
    const auto byteSize = vertices.size() * sizeof(QSGGeometry::Point2D);
    auto* geometry = node->geometry();
    const auto countChanged = geometry->vertexCount() != vertexCount;
    if (countChanged || (byteSize > 0 && std::memcmp(geometry->vertexData(), vertices.data(), byteSize) != 0)) {
        if (countChanged) {
            geometry->allocate(vertexCount);
        }
        if (byteSize > 0) {
            std::memcpy(geometry->vertexData(), vertices.data(), byteSize);
        }
        node->markDirty(QSGNode::DirtyGeometry);
    }
    auto* material = static_cast<QSGFlatColorMaterial*>(node->material());
    if (material->color() != color) {
        material->setColor(color);
        node->markDirty(QSGNode::DirtyMaterial);
    }
}

} // namespace

GridNode::GridNode()
    : subGridNode_(createRectanglesNode())
    , mainGridNode_(createRectanglesNode())
{
    appendChildNode(subGridNode_);
    appendChildNode(mainGridNode_);
}

void GridNode::update(const Grid* grid, const Axis* xAxis, const Axis* yAxis, const QRectF& plotRect)
{
    plotRect_ = plotRect;

    auto mainRects = QVector<QRectF>{};
    auto subRects = QVector<QRectF>{};
    collectGridLines(mainRects, subRects, grid, xAxis, yAxis);

    updateRectanglesNode(subGridNode_, grid->subGridVisible() ? subRects : QVector<QRectF>{}, grid->subGridColor());
    updateRectanglesNode(mainGridNode_, grid->gridVisible() ? mainRects : QVector<QRectF>{}, grid->gridColor());
}

void GridNode::collectGridLines(QVector<QRectF>& mainRects, QVector<QRectF>& subRects, const Grid* grid, const Axis* xAxis, const Axis* yAxis) const
{
    if (plotRect_.isEmpty() || (!xAxis && !yAxis)) {
        return;
    }
    if (!grid->gridVisible() && !grid->subGridVisible()) {
        return;
    }

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
