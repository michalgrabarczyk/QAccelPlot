//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "grid/GridNode.hpp"
#include "axis/Axis.hpp"
#include "axis/AxisTickPainter.hpp"
#include "axis/AxisTicker.hpp"
#include "grid/Grid.hpp"

#include <QSGFlatColorMaterial>
#include <QSGGeometry>
#include <QSGGeometryNode>

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

static QSGNode* buildRectangleNode(const QVector<QRectF>& rects, const QColor& color)
{
    if (rects.isEmpty()) {
        return nullptr;
    }
    static constexpr auto kVerticesPerRect = 6; // 2 triangles × 3 vertices
    const auto rectCount = rects.size();
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
    const auto* t = p.axis->ticker();
    const auto viewportLow = std::min(p.axis->viewportMin(), p.axis->viewportMax());
    const auto viewportHigh = std::max(p.axis->viewportMin(), p.axis->viewportMax());
    if (p.axis->logScale() && viewportLow > 0 && viewportHigh > 0) {
        const auto logMin = static_cast<int>(std::floor(std::log10(viewportLow)));
        const auto logMax = static_cast<int>(std::ceil(std::log10(viewportHigh)));
        for (int e = logMin; e <= logMax; ++e) {
            const auto val = std::pow(10.0, e);
            if (p.gridLinesVisible && val >= viewportLow && val <= viewportHigh) {
                mainRects.append(p.makeRect(val, grid->lineWidth() / 2.0));
            }
            if (p.subGridLinesVisible && e < logMax) {
                for (int k = 2; k <= 9; ++k) {
                    const auto subVal = k * val;
                    if (subVal >= viewportLow && subVal <= viewportHigh) {
                        subRects.append(p.makeRect(subVal, grid->subGridLineWidth() / 2.0));
                    }
                }
            }
        }
    } else {
        const auto ticks = t ? t->tickCount() : 5;
        const auto subticks = t ? t->subtickCount() : 0;
        static constexpr auto kRelativeTolerance = 1e-9; // avoids dropping ticks that land exactly on viewport boundary
        const auto step = AxisTickPainter::computeNiceStep(viewportLow, viewportHigh, ticks);
        const auto firstTick = std::ceil(viewportLow / step) * step;
        const auto loopStart = firstTick - step;
        auto tickIndex = 0;
        for (auto val = loopStart; val <= viewportHigh + step * kRelativeTolerance; val = loopStart + (++tickIndex) * step) {
            if (p.gridLinesVisible && val >= viewportLow - step * kRelativeTolerance) {
                mainRects.append(p.makeRect(val, grid->lineWidth() / 2.0));
            }
            if (p.subGridLinesVisible && subticks > 0) {
                const auto subStep = step / (subticks + 1);
                for (int j = 1; j <= subticks; ++j) {
                    const auto subVal = val + j * subStep;
                    if (subVal < viewportLow - subStep * kRelativeTolerance) {
                        continue;
                    }
                    if (subVal > viewportHigh + subStep * kRelativeTolerance) {
                        break;
                    }
                    subRects.append(p.makeRect(subVal, grid->subGridLineWidth() / 2.0));
                }
            }
        }
    }
}

} // namespace QAccelPlot
