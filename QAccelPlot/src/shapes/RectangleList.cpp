//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "shapes/RectangleList.hpp"

#include "QAccelPlotLogging.hpp"
#include "materials/DataTextureMaterial.hpp"
#include "materials/RectMaterial.hpp"

#include <QSGGeometry>
#include <QSGGeometryNode>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace QAccelPlot {

namespace {

bool hoverEnabled()
{
    auto isInteger = false;
    const auto value = qEnvironmentVariableIntValue("QACCELPLOT_HOVER_ENABLED", &isInteger);
    return !isInteger || value != 0;
}

}

RectangleList::RectangleList(QQuickItem* parent)
    : PlotSeries(parent)
{
    setFlag(ItemHasContents, true);
    setAcceptHoverEvents(hoverEnabled());
    setAcceptedMouseButtons(Qt::NoButton);
    setLegendSymbol(LegendSymbol::Fill);
}

QColor RectangleList::color() const
{
    return color_;
}

void RectangleList::setColor(const QColor& color)
{
    if (color_ == color) {
        return;
    }
    color_ = color;
    emit colorChanged();
    update();
}

int RectangleList::count() const
{
    return rectCount_;
}

int RectangleList::hoveredIndex() const
{
    return hoveredIndex_;
}

void RectangleList::setData(const QVariantList& rects)
{
    if (rects.size() != rectCount_) {
        vertexCacheValid_ = false;
    }
    rectCount_ = rects.size();
    data_.resize(static_cast<size_t>(rectCount_) * 4);

    for (auto i = 0; i < rectCount_; ++i) {
        const auto map = rects[i].toMap();
        const auto base = static_cast<size_t>(i) * 4;
        data_[base] = map.value(QStringLiteral("x1")).toDouble();
        data_[base + 1] = map.value(QStringLiteral("y1")).toDouble();
        data_[base + 2] = map.value(QStringLiteral("x2")).toDouble();
        data_[base + 3] = map.value(QStringLiteral("y2")).toDouble();
    }

    dataChanged_ = true;
    buildSpatialGrid();
    updateDataRanges();
    emit countChanged();
    update();
}

void RectangleList::setData(const float* data, const int rectCount)
{
    if (!validateRawDataArguments(data, rectCount)) {
        return;
    }
    if (rectCount != rectCount_) {
        vertexCacheValid_ = false;
    }
    rectCount_ = rectCount;
    const auto floatCount = static_cast<size_t>(rectCount) * 4;
    data_.resize(floatCount);
    for (size_t i = 0; i < floatCount; ++i) {
        data_[i] = static_cast<double>(data[i]);
    }

    dataChanged_ = true;
    buildSpatialGrid();
    updateDataRanges();
    emit countChanged();
    update();
}

void RectangleList::setData(const double* data, const int rectCount)
{
    if (!validateRawDataArguments(data, rectCount)) {
        return;
    }
    if (rectCount != rectCount_) {
        vertexCacheValid_ = false;
    }
    rectCount_ = rectCount;
    const auto doubleCount = static_cast<size_t>(rectCount) * 4;
    data_.resize(doubleCount);
    if (doubleCount > 0) {
        memcpy(data_.data(), data, doubleCount * sizeof(double));
    }

    dataChanged_ = true;
    buildSpatialGrid();
    updateDataRanges();
    emit countChanged();
    update();
}

bool RectangleList::validateRawDataArguments(const void* data, const int rectCount) const
{
    if (rectCount < 0) {
        qCWarning(lcQAccelPlot) << "RectangleList data rectangle count cannot be negative:" << rectCount;
        return false;
    }
    if (rectCount > 0 && !data) {
        qCWarning(lcQAccelPlot) << "RectangleList received a null data pointer for" << rectCount << "rectangles";
        return false;
    }
    return true;
}

QSGNode* RectangleList::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    if (rectCount_ <= 0) {
        delete oldNode;
        return nullptr;
    }
    if (!xAxis() || !yAxis() || plotRect().isEmpty()) {
        qCDebug(lcQAccelPlot) << "missing axes or empty plotRect, returning nullptr";
        delete oldNode;
        return nullptr;
    }

    auto* window = this->window();
    if (!window) {
        qCDebug(lcQAccelPlot) << "null window, returning nullptr";
        delete oldNode;
        return nullptr;
    }

    auto* node = static_cast<QSGGeometryNode*>(oldNode);
    RectMaterial* material = nullptr;
    const auto vertexCount = rectCount_ * 6;
    const auto nodeRecreated = !node;

    if (!node) {
        if (!vertexCacheValid_) {
            buildVertexCache();
        }

        auto* geometry = new QSGGeometry(DataTextureMaterial::attributeSet(), vertexCount);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        memcpy(geometry->vertexData(), vertexCache_.data(), static_cast<size_t>(vertexCount) * sizeof(RectVertex));

        material = new RectMaterial;
        node = new QSGGeometryNode;
        node->setGeometry(geometry);
        node->setFlag(QSGNode::OwnsGeometry);
        node->setMaterial(material);
        node->setFlag(QSGNode::OwnsMaterial);
        node->markDirty(QSGNode::DirtyGeometry);
    } else {
        material = static_cast<RectMaterial*>(node->material());

        // Vertex data is deterministic from rectCount_ alone — only rebuild when count changes.
        if (!vertexCacheValid_) {
            buildVertexCache();
            auto* geometry = node->geometry();
            geometry->allocate(vertexCount);
            memcpy(geometry->vertexData(), vertexCache_.data(), static_cast<size_t>(vertexCount) * sizeof(RectVertex));
            node->markDirty(QSGNode::DirtyGeometry);
        }
    }

    // Upload data texture
    if (dataChanged_ || nodeRecreated) {
        rebuildRenderData(xAxis()->logScale(), yAxis()->logScale());
        const auto numFloats = rectCount_ * 4;
        material->uploadTexture(material->dataTexture, window, renderData_.data(), numFloats);
        dataChanged_ = false;
    }

    // Set material uniforms
    material->color = color_;
    material->domainMin = QVector2D(static_cast<float>(xAxis()->viewportMin() - renderOriginX_), static_cast<float>(yAxis()->viewportMin() - renderOriginY_));
    material->domainMax = QVector2D(static_cast<float>(xAxis()->viewportMax() - renderOriginX_), static_cast<float>(yAxis()->viewportMax() - renderOriginY_));
    material->viewportSize = QVector2D(static_cast<float>(width()), static_cast<float>(height()));
    material->logScaleX = xAxis()->logScale() ? 1.0f : 0.0f;
    material->logScaleY = yAxis()->logScale() ? 1.0f : 0.0f;
    material->useVertexColor = 0.0f;
    material->rectCount = static_cast<float>(rectCount_);

    node->markDirty(QSGNode::DirtyMaterial);

    return node;
}

void RectangleList::hoverMoveEvent(QHoverEvent* event)
{
    if (!xAxis() || !yAxis() || plotRect().isEmpty() || rectCount_ <= 0) {
        if (hoveredIndex_ != -1) {
            hoveredIndex_ = -1;
            emit hoveredIndexChanged();
        }
        return;
    }

    // Map pixel position to data coordinates
    const auto pos = event->position();
    const auto dataX = xAxis()->pixelToCoord(pos.x(), width());
    const auto dataY = yAxis()->pixelToCoord(pos.y(), height());

    // Query spatial grid
    const auto candidate = spatialGrid_.query(static_cast<float>(dataX), static_cast<float>(dataY));

    auto newHovered = -1;
    if (candidate >= 0 && candidate < rectCount_) {
        // Containment test
        const auto base = static_cast<size_t>(candidate) * 4;
        const auto x1 = std::min(data_[base], data_[base + 2]);
        const auto y1 = std::min(data_[base + 1], data_[base + 3]);
        const auto x2 = std::max(data_[base], data_[base + 2]);
        const auto y2 = std::max(data_[base + 1], data_[base + 3]);

        if (dataX >= x1 && dataX <= x2 && dataY >= y1 && dataY <= y2) {
            newHovered = candidate;
        }
    }

    if (hoveredIndex_ != newHovered) {
        hoveredIndex_ = newHovered;
        emit hoveredIndexChanged();
    }
}

void RectangleList::hoverLeaveEvent(QHoverEvent* /*event*/)
{
    if (hoveredIndex_ != -1) {
        hoveredIndex_ = -1;
        emit hoveredIndexChanged();
    }
}

void RectangleList::buildSpatialGrid()
{
    // SpatialGrid only needs to narrow hover queries to a candidate rect — the final
    // containment test in hoverMoveEvent() re-checks against the full double-precision
    // data_, so a plain float cast here (no origin shift) is sufficient.
    auto floatData = std::vector<float>(data_.size());
    std::transform(data_.begin(), data_.end(), floatData.begin(), [](const double v) { return static_cast<float>(v); });
    spatialGrid_.build(floatData.data(), rectCount_);
}

void RectangleList::rebuildRenderData(const bool logScaleX, const bool logScaleY)
{
    // Log-scale coordinates aren't translation-invariant (log10(x - origin) != log10(x) -
    // log10(origin)), so origin-shifting is skipped for a log-scale axis, matching LineCurve.
    renderOriginX_ = 0.0;
    renderOriginY_ = 0.0;
    auto foundOriginX = logScaleX;
    auto foundOriginY = logScaleY;

    for (auto i = 0; i < rectCount_ && !(foundOriginX && foundOriginY); ++i) {
        const auto base = static_cast<size_t>(i) * 4;
        if (!foundOriginX) {
            if (std::isfinite(data_[base])) {
                renderOriginX_ = data_[base];
                foundOriginX = true;
            } else if (std::isfinite(data_[base + 2])) {
                renderOriginX_ = data_[base + 2];
                foundOriginX = true;
            }
        }
        if (!foundOriginY) {
            if (std::isfinite(data_[base + 1])) {
                renderOriginY_ = data_[base + 1];
                foundOriginY = true;
            } else if (std::isfinite(data_[base + 3])) {
                renderOriginY_ = data_[base + 3];
                foundOriginY = true;
            }
        }
    }

    renderData_.resize(data_.size());
    for (auto i = 0; i < rectCount_; ++i) {
        const auto base = static_cast<size_t>(i) * 4;
        renderData_[base] = static_cast<float>(data_[base] - renderOriginX_);
        renderData_[base + 1] = static_cast<float>(data_[base + 1] - renderOriginY_);
        renderData_[base + 2] = static_cast<float>(data_[base + 2] - renderOriginX_);
        renderData_[base + 3] = static_cast<float>(data_[base + 3] - renderOriginY_);
    }
}

void RectangleList::updateDataRanges()
{
    if (data_.empty()) {
        clearDataRanges();
        return;
    }

    auto xMin = std::numeric_limits<qreal>::max();
    auto xMax = std::numeric_limits<qreal>::lowest();
    auto yMin = std::numeric_limits<qreal>::max();
    auto yMax = std::numeric_limits<qreal>::lowest();
    for (int i = 0; i < rectCount_; ++i) {
        const auto base = static_cast<size_t>(i) * 4;
        xMin = std::min({xMin, static_cast<qreal>(data_[base]), static_cast<qreal>(data_[base + 2])});
        xMax = std::max({xMax, static_cast<qreal>(data_[base]), static_cast<qreal>(data_[base + 2])});
        yMin = std::min({yMin, static_cast<qreal>(data_[base + 1]), static_cast<qreal>(data_[base + 3])});
        yMax = std::max({yMax, static_cast<qreal>(data_[base + 1]), static_cast<qreal>(data_[base + 3])});
    }
    setDataRanges(xMin, xMax, yMin, yMax);
}

void RectangleList::buildVertexCache()
{
    const auto totalVerts = static_cast<size_t>(rectCount_) * 6;
    vertexCache_.resize(totalVerts);
    // Default RGBA: opaque white, which the shader multiplies by the per-rect color uniform.
    constexpr static auto kOpaqueChannelByte = quint8{255};
    for (auto i = 0; i < rectCount_; ++i) {
        const auto vbase = static_cast<size_t>(i) * 6;
        const auto fid = static_cast<float>(i);
        for (auto c = 0; c < 6; ++c) {
            auto& v = vertexCache_[vbase + static_cast<size_t>(c)];
            v.id = fid;
            v.corner = static_cast<float>(c);
            v.r = kOpaqueChannelByte;
            v.g = kOpaqueChannelByte;
            v.b = kOpaqueChannelByte;
            v.a = kOpaqueChannelByte;
        }
    }
    vertexCacheValid_ = true;
}

} // namespace QAccelPlot
