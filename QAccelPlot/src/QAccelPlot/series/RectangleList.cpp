//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/RectangleList.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/materials/DataTextureMaterial.hpp"
#include "QAccelPlot/materials/RectMaterial.hpp"

#include <QSGGeometry>
#include <QSGGeometryNode>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <limits>
#include <utility>

namespace QAccelPlot {

namespace {

bool hoverEnabled()
{
    auto isInteger = false;
    const auto value = qEnvironmentVariableIntValue("QACCELPLOT_HOVER_ENABLED", &isInteger);
    return !isInteger || value != 0;
}

const std::array<QString, 4>& rectangleKeys()
{
    static const auto keys = std::array<QString, 4>{QStringLiteral("x1"), QStringLiteral("y1"), QStringLiteral("x2"), QStringLiteral("y2")};
    return keys;
}

QColor defaultRectangleColor()
{
    auto color = ColorPalette::dark().seriesPrimary;
    color.setAlpha(50);
    return color;
}

// Maps an edge to item pixels. Infinite edges, and non-positive edges on a log axis, map to
// infinity on the matching side, as the shader extends them past the plot edge.
qreal edgePixel(const double value, const Axis& axis, const qreal length)
{
    constexpr auto kInf = std::numeric_limits<qreal>::infinity();
    const auto belowRange = value == -kInf || (axis.logScale() && value <= 0.0);
    if (!belowRange && value != kInf) {
        return axis.coordToPixel(value, length);
    }
    const auto pixelsGrowWithData = axis.coordToPixel(axis.viewportMax(), length) >= axis.coordToPixel(axis.viewportMin(), length);
    return pixelsGrowWithData == belowRange ? -kInf : kInf;
}

// Returns the pixel span between edges a and b, widened around its center to at least minimumSize.
std::pair<qreal, qreal> widenedSpan(const qreal a, const qreal b, const qreal minimumSize)
{
    const auto low = std::min(a, b);
    const auto high = std::max(a, b);
    const auto grow = 0.5 * std::max(minimumSize - (high - low), qreal{0.0});
    return {low - grow, high + grow};
}

struct FiniteBounds {
    qreal xMin{std::numeric_limits<qreal>::max()};
    qreal xMax{std::numeric_limits<qreal>::lowest()};
    qreal yMin{std::numeric_limits<qreal>::max()};
    qreal yMax{std::numeric_limits<qreal>::lowest()};
};

// Infinite edges don't widen the bounds, so a full-height span leaves Y autoscaling alone.
template <typename T> FiniteBounds finiteBounds(const T* data, const int rectCount)
{
    auto bounds = FiniteBounds{};
    const auto include = [](const T value, qreal& min, qreal& max) {
        if (std::isfinite(value)) {
            min = std::min(min, static_cast<qreal>(value));
            max = std::max(max, static_cast<qreal>(value));
        }
    };
    for (int i = 0; i < rectCount; ++i) {
        const auto* rect = data + static_cast<size_t>(i) * 4;
        if (std::isnan(rect[0]) || std::isnan(rect[1]) || std::isnan(rect[2]) || std::isnan(rect[3])) {
            continue;
        }
        include(rect[0], bounds.xMin, bounds.xMax);
        include(rect[2], bounds.xMin, bounds.xMax);
        include(rect[1], bounds.yMin, bounds.yMax);
        include(rect[3], bounds.yMin, bounds.yMax);
    }
    return bounds;
}

}

RectangleList::RectangleList(QQuickItem* parent)
    : PlotSeries(parent)
    , color_(defaultRectangleColor())
{
    setFlag(ItemHasContents, true);
    setAcceptHoverEvents(hoverEnabled());
    setAcceptedMouseButtons(Qt::NoButton);
    setLegendSymbol(LegendSymbol::Fill);
    connect(border_, &RectangleBorder::widthChanged, this, &QQuickItem::update);
    connect(border_, &RectangleBorder::colorChanged, this, &QQuickItem::update);
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
    if (hasCategories()) {
        vertexCacheValid_ = false;
    }
    emit colorChanged();
    update();
}

QList<QColor> RectangleList::categoryColors() const
{
    return categoryColors_;
}

void RectangleList::setCategoryColors(const QList<QColor>& colors)
{
    if (categoryColors_ == colors) {
        return;
    }
    categoryColors_ = colors;
    if (hasCategories()) {
        vertexCacheValid_ = false;
    }
    emit categoryColorsChanged();
    update();
}

RectangleBorder* RectangleList::border() const
{
    return border_;
}

QColor RectangleList::hoverColor() const
{
    return hoverColor_;
}

void RectangleList::setHoverColor(const QColor& color)
{
    if (hoverColor_ == color) {
        return;
    }
    hoverColor_ = color;
    emit hoverColorChanged();
    update();
}

qreal RectangleList::minimumWidth() const
{
    return minimumWidth_;
}

void RectangleList::setMinimumWidth(const qreal width)
{
    const auto clamped = std::max(width, qreal{0.0});
    if (nearly_equal(minimumWidth_, clamped)) {
        return;
    }
    minimumWidth_ = clamped;
    emit minimumWidthChanged();
    update();
}

qreal RectangleList::minimumHeight() const
{
    return minimumHeight_;
}

void RectangleList::setMinimumHeight(const qreal height)
{
    const auto clamped = std::max(height, qreal{0.0});
    if (nearly_equal(minimumHeight_, clamped)) {
        return;
    }
    minimumHeight_ = clamped;
    emit minimumHeightChanged();
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
    constexpr auto kInf = std::numeric_limits<double>::infinity();
    const auto& keys = rectangleKeys();
    const auto categoryKey = QStringLiteral("category");
    auto data = std::vector<double>(static_cast<size_t>(rects.size()) * keys.size());
    auto categories = std::vector<int>(static_cast<size_t>(rects.size()), -1);
    auto anyCategory = false;
    for (qsizetype i = 0; i < rects.size(); ++i) {
        const auto map = rects[i].toMap();
        const auto base = static_cast<size_t>(i) * keys.size();
        for (size_t k = 0; k < keys.size(); ++k) {
            const auto value = map.value(keys[k]);
            // A missing or null x1/y1 is unbounded below, and a missing x2/y2 unbounded above.
            const auto unbounded = k < 2 ? -kInf : kInf;
            data[base + k] = value.isValid() && !value.isNull() ? value.toDouble() : unbounded;
        }
        const auto category = map.value(categoryKey);
        if (category.isValid() && !category.isNull()) {
            categories[static_cast<size_t>(i)] = category.toInt();
            anyCategory = true;
        }
    }
    if (!anyCategory) {
        categories.clear();
    }
    applyData(std::move(data), std::move(categories), static_cast<int>(rects.size()), true);
}

void RectangleList::setData(const double* data, const int rectCount)
{
    if (!validateRawDataArguments(data, rectCount)) {
        return;
    }
    const auto doubleCount = static_cast<size_t>(rectCount) * 4;
    applyData(std::vector<double>(data, data + doubleCount), {}, rectCount, true);
}

void RectangleList::setData(std::vector<double>&& data, const int rectCount)
{
    setData(std::move(data), {}, rectCount);
}

void RectangleList::setData(std::vector<double>&& data, std::vector<int>&& categories, const int rectCount)
{
    if (!validateDataArguments(data.size(), categories.size(), rectCount)) {
        return;
    }
    applyData(std::move(data), std::move(categories), rectCount, true);
}

void RectangleList::setDataNoRange(std::vector<double>&& data, const int rectCount)
{
    setDataNoRange(std::move(data), {}, rectCount);
}

void RectangleList::setDataNoRange(std::vector<double>&& data, std::vector<int>&& categories, const int rectCount)
{
    if (!validateDataArguments(data.size(), categories.size(), rectCount)) {
        return;
    }
    applyData(std::move(data), std::move(categories), rectCount, false);
}

void RectangleList::setDataF(const float* data, const int rectCount)
{
    setDataFFromArray(data, rectCount, true);
}

void RectangleList::setDataF(std::vector<float>&& data, const int rectCount)
{
    setDataF(std::move(data), {}, rectCount);
}

void RectangleList::setDataF(std::vector<float>&& data, std::vector<int>&& categories, const int rectCount)
{
    if (!validateDataArguments(data.size(), categories.size(), rectCount)) {
        return;
    }
    applyFloatData(std::move(data), std::move(categories), rectCount, true);
}

void RectangleList::setDataFNoRange(std::vector<float>&& data, const int rectCount)
{
    setDataFNoRange(std::move(data), {}, rectCount);
}

void RectangleList::setDataFNoRange(std::vector<float>&& data, std::vector<int>&& categories, const int rectCount)
{
    if (!validateDataArguments(data.size(), categories.size(), rectCount)) {
        return;
    }
    applyFloatData(std::move(data), std::move(categories), rectCount, false);
}

void RectangleList::setDataFNoRange(const float* data, const int rectCount)
{
    setDataFFromArray(data, rectCount, false);
}

void RectangleList::postData(std::vector<double>&& data, const int rectCount)
{
    postData(std::move(data), {}, rectCount);
}

void RectangleList::postData(std::vector<double>&& data, std::vector<int>&& categories, const int rectCount)
{
    QMetaObject::invokeMethod(
        this,
        [this, rects = std::move(data), rectCategories = std::move(categories), rectCount]() mutable {
            setData(std::move(rects), std::move(rectCategories), rectCount);
        },
        Qt::QueuedConnection);
}

void RectangleList::postData(std::vector<float>&& data, const int rectCount)
{
    postData(std::move(data), {}, rectCount);
}

void RectangleList::postData(std::vector<float>&& data, std::vector<int>&& categories, const int rectCount)
{
    QMetaObject::invokeMethod(
        this,
        [this, rects = std::move(data), rectCategories = std::move(categories), rectCount]() mutable {
            setDataF(std::move(rects), std::move(rectCategories), rectCount);
        },
        Qt::QueuedConnection);
}

void RectangleList::setCategories(const QList<int>& categories)
{
    if (!categories.isEmpty() && categories.size() != rectCount_) {
        qCWarning(lcQAccelPlot) << "RectangleList received" << categories.size() << "categories for" << rectCount_ << "rectangles";
        return;
    }
    categories_.assign(categories.cbegin(), categories.cend());
    vertexCacheValid_ = false;
    update();
}

void RectangleList::clearData()
{
    applyData({}, {}, 0, true);
}

QVariantMap RectangleList::rectangleAt(const int index) const
{
    if (index < 0 || index >= rectCount_) {
        return {};
    }
    const auto& keys = rectangleKeys();
    auto rect = QVariantMap{};
    for (size_t k = 0; k < keys.size(); ++k) {
        rect.insert(keys[k], coordinate(index, static_cast<int>(k)));
    }
    if (hasCategories()) {
        rect.insert(QStringLiteral("category"), categories_[static_cast<size_t>(index)]);
    }
    return rect;
}

int RectangleList::rectangleIndexAt(const QPointF& position) const
{
    if (rectCount_ <= 0 || !xAxis() || !yAxis() || plotRect().isEmpty()) {
        return -1;
    }
    // A rectangle widened to contain the cursor has its center, and so part of itself, within half
    // the minimum size of it. That box in data space bounds the candidates for the pixel test.
    const auto x1 = xAxis()->pixelToCoord(position.x() - 0.5 * minimumWidth_, width());
    const auto x2 = xAxis()->pixelToCoord(position.x() + 0.5 * minimumWidth_, width());
    const auto y1 = yAxis()->pixelToCoord(position.y() - 0.5 * minimumHeight_, height());
    const auto y2 = yAxis()->pixelToCoord(position.y() + 0.5 * minimumHeight_, height());
    ensureSpatialGrid();
    return spatialGrid_.queryTopmost(std::min(x1, x2), std::min(y1, y2), std::max(x1, x2), std::max(y1, y2),
        [this, &position](const int index) { return containsInPixels(index, position); });
}

bool RectangleList::contains(const QPointF& point) const
{
    return boundingRect().contains(point) && rectangleIndexAt(point) >= 0;
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

bool RectangleList::validateDataArguments(const std::size_t valueCount, const std::size_t categoryCount, const int rectCount) const
{
    if (rectCount < 0) {
        qCWarning(lcQAccelPlot) << "RectangleList data rectangle count cannot be negative:" << rectCount;
        return false;
    }
    const auto expectedValueCount = static_cast<std::size_t>(rectCount) * 4;
    if (valueCount != expectedValueCount) {
        qCWarning(lcQAccelPlot) << "RectangleList received" << valueCount << "coordinates for" << rectCount << "rectangles; expected" << expectedValueCount;
        return false;
    }
    if (categoryCount != 0 && categoryCount != static_cast<std::size_t>(rectCount)) {
        qCWarning(lcQAccelPlot) << "RectangleList received" << categoryCount << "categories for" << rectCount << "rectangles";
        return false;
    }
    return true;
}

void RectangleList::applyData(std::vector<double>&& data, std::vector<int>&& categories, const int rectCount, const bool reportRanges)
{
    data_ = std::move(data);
    finishDataChange(std::move(categories), rectCount, reportRanges);
}

void RectangleList::applyFloatData(std::vector<float>&& data, std::vector<int>&& categories, const int rectCount, const bool reportRanges)
{
    data_ = std::vector<double>{};
    renderData_ = std::move(data);
    renderOriginX_ = 0.0;
    renderOriginY_ = 0.0;
    finishDataChange(std::move(categories), rectCount, reportRanges);
}

void RectangleList::setDataFFromArray(const float* data, const int rectCount, const bool reportRanges)
{
    if (!validateRawDataArguments(data, rectCount)) {
        return;
    }
    auto buffer = std::move(renderData_);
    buffer.assign(data, data + static_cast<size_t>(rectCount) * 4);
    applyFloatData(std::move(buffer), {}, rectCount, reportRanges);
}

void RectangleList::finishDataChange(std::vector<int>&& categories, const int rectCount, const bool reportRanges)
{
    const auto previousCount = rectCount_;
    // Vertex colors depend on the categories, but not on the coordinates.
    if (rectCount != rectCount_ || hasCategories() || !categories.empty()) {
        vertexCacheValid_ = false;
    }
    categories_ = std::move(categories);
    rectCount_ = rectCount;
    dataChanged_ = true;
    spatialGridValid_ = false;
    if (reportRanges) {
        updateDataRanges();
    }
    if (hoveredIndex_ >= rectCount_) {
        setHoveredIndex(-1);
    }
    if (previousCount != rectCount_) {
        emit countChanged();
    }
    update();
}

bool RectangleList::hasPreciseData() const
{
    return !data_.empty();
}

double RectangleList::coordinate(const int index, const int component) const
{
    const auto offset = static_cast<size_t>(index) * 4 + static_cast<size_t>(component);
    return hasPreciseData() ? data_[offset] : static_cast<double>(renderData_[offset]);
}

bool RectangleList::hasCategories() const
{
    return !categories_.empty();
}

QColor RectangleList::rectangleColor(const int index) const
{
    const auto category = hasCategories() ? categories_[static_cast<size_t>(index)] : -1;
    return category >= 0 && category < categoryColors_.size() ? categoryColors_[category] : color_;
}

bool RectangleList::containsInPixels(const int index, const QPointF& position) const
{
    const auto [left, right]
        = widenedSpan(edgePixel(coordinate(index, 0), *xAxis(), width()), edgePixel(coordinate(index, 2), *xAxis(), width()), minimumWidth_);
    const auto [top, bottom]
        = widenedSpan(edgePixel(coordinate(index, 1), *yAxis(), height()), edgePixel(coordinate(index, 3), *yAxis(), height()), minimumHeight_);
    return position.x() >= left && position.x() <= right && position.y() >= top && position.y() <= bottom;
}

void RectangleList::setHoveredIndex(const int index)
{
    if (hoveredIndex_ == index) {
        return;
    }
    hoveredIndex_ = index;
    emit hoveredIndexChanged();
    if (hoverColor_.isValid()) {
        update();
    }
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

        // Vertices hold only ids and category colors, so coordinate changes don't rebuild them.
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
        if (hasPreciseData()) {
            rebuildRenderData(xAxis()->logScale(), yAxis()->logScale());
        }
        const auto numFloats = rectCount_ * 4;
        material->uploadTexture(material->dataTexture, window, renderData_.data(), numFloats);
        dataChanged_ = false;
    }

    updateMaterial(*material);
    node->markDirty(QSGNode::DirtyMaterial);

    return node;
}

void RectangleList::hoverEnterEvent(QHoverEvent* event)
{
    setHoveredIndex(rectangleIndexAt(event->position()));
    QQuickItem::hoverEnterEvent(event);
}

void RectangleList::hoverMoveEvent(QHoverEvent* event)
{
    setHoveredIndex(rectangleIndexAt(event->position()));
    QQuickItem::hoverMoveEvent(event);
}

void RectangleList::hoverLeaveEvent(QHoverEvent* event)
{
    setHoveredIndex(-1);
    QQuickItem::hoverLeaveEvent(event);
}

void RectangleList::onAxisScaleChanged()
{
    // Only origin-shifted double data depends on the scale; float data is uploaded as is.
    if (hasPreciseData()) {
        dataChanged_ = true;
    }
    update();
}

void RectangleList::updateMaterial(RectMaterial& material) const
{
    material.color = color_;
    material.domainMin = QVector2D(static_cast<float>(xAxis()->viewportMin() - renderOriginX_), static_cast<float>(yAxis()->viewportMin() - renderOriginY_));
    material.domainMax = QVector2D(static_cast<float>(xAxis()->viewportMax() - renderOriginX_), static_cast<float>(yAxis()->viewportMax() - renderOriginY_));
    material.viewportSize = QVector2D(static_cast<float>(width()), static_cast<float>(height()));
    material.logScaleX = xAxis()->logScale() ? 1.0f : 0.0f;
    material.logScaleY = yAxis()->logScale() ? 1.0f : 0.0f;
    material.useVertexColor = hasCategories() ? 1.0f : 0.0f;
    material.rectCount = static_cast<float>(rectCount_);
    material.minimumSize = QVector2D(static_cast<float>(minimumWidth_), static_cast<float>(minimumHeight_));
    material.borderWidth = static_cast<float>(border_->width());
    material.borderColor = border_->color();
    material.hoverColor = hoverColor_.isValid() ? hoverColor_ : QColor{Qt::transparent};
    material.hoveredIndex = hoverColor_.isValid() ? static_cast<float>(hoveredIndex_) : -1.0f;
}

void RectangleList::ensureSpatialGrid() const
{
    if (spatialGridValid_) {
        return;
    }
    if (hasPreciseData()) {
        spatialGrid_.build(data_.data(), rectCount_);
    } else {
        spatialGrid_.buildF(renderData_.data(), rectCount_);
    }
    spatialGridValid_ = true;
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
    const auto bounds = hasPreciseData() ? finiteBounds(data_.data(), rectCount_) : finiteBounds(renderData_.data(), rectCount_);
    if (bounds.xMin <= bounds.xMax) {
        setXDataRange(bounds.xMin, bounds.xMax);
    } else {
        clearXDataRange();
    }
    if (bounds.yMin <= bounds.yMax) {
        setYDataRange(bounds.yMin, bounds.yMax);
    } else {
        clearYDataRange();
    }
}

void RectangleList::buildVertexCache()
{
    const auto totalVerts = static_cast<size_t>(rectCount_) * 6;
    vertexCache_.resize(totalVerts);
    for (auto i = 0; i < rectCount_; ++i) {
        const auto vbase = static_cast<size_t>(i) * 6;
        const auto fid = static_cast<float>(i);
        const auto rgba = hasCategories() ? rectangleColor(i).toRgb() : QColor{Qt::white};
        for (auto c = 0; c < 6; ++c) {
            auto& v = vertexCache_[vbase + static_cast<size_t>(c)];
            v.id = fid;
            v.corner = static_cast<float>(c);
            v.r = static_cast<unsigned char>(rgba.red());
            v.g = static_cast<unsigned char>(rgba.green());
            v.b = static_cast<unsigned char>(rgba.blue());
            v.a = static_cast<unsigned char>(rgba.alpha());
        }
    }
    vertexCacheValid_ = true;
}

} // namespace QAccelPlot
