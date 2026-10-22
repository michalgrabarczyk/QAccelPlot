//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/RectangleSeries.hpp"
#include "QAccelPlot/legend/LegendEntry.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/materials/DataTextureMaterial.hpp"
#include "QAccelPlot/materials/RectMaterial.hpp"
#include "QAccelPlot/series/internal/HoverIndexBudget.hpp"
#include "QAccelPlot/series/internal/RectGeometry.hpp"
#include "QAccelPlot/series/internal/SeriesSupport.hpp"

#include <QSGGeometry>
#include <QSGGeometryNode>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <utility>

namespace QAccelPlot {

namespace {

// Cost of indexing one rectangle, measured on a desktop CPU and assumed until a build has been timed.
constexpr auto kAssumedIndexCostPerRectangle = std::chrono::nanoseconds{400};

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

constexpr auto kValuesPerRectangle = 4;

// Infinite edges don't widen the data ranges, so a full-height span leaves Y autoscaling alone.
DataPointStore::Layout rectangleLayout()
{
    using Dimension = DataPointStore::Dimension;
    return {{Dimension::X, Dimension::Y, Dimension::X, Dimension::Y}, DataPointStore::RangeRule::SkipOnNaN, 0};
}

}

RectangleSeries::RectangleSeries(QQuickItem* parent)
    : PlotSeries(rectangleLayout(), DataPointWording{"data rectangle", "rectangles", "coordinates", "coordinates"}, parent)
    , color_(defaultRectangleColor())
    , spatialGridBudget_(std::make_unique<Internal::HoverIndexBudget>(kAssumedIndexCostPerRectangle))
{
    setFlag(ItemHasContents, true);
    supportHover();
    setAcceptedMouseButtons(Qt::NoButton);
    setLegendSymbol(LegendSymbol::Fill);
    connect(border_, &RectangleBorder::widthChanged, this, &QQuickItem::update);
    connect(border_, &RectangleBorder::colorChanged, this, &QQuickItem::update);
    connect(this, &RectangleSeries::colorChanged, this, &PlotSeries::legendEntryChanged);
}

RectangleSeries::~RectangleSeries() = default;

QColor RectangleSeries::color() const
{
    return color_;
}

void RectangleSeries::setColor(const QColor& color)
{
    if (color_ == color) {
        return;
    }
    color_ = color;
    if (hasCategories()) {
        vertexCache_.invalidate();
    }
    emit colorChanged();
    update();
}

QList<QColor> RectangleSeries::categoryColors() const
{
    return categoryColors_;
}

void RectangleSeries::setCategoryColors(const QList<QColor>& colors)
{
    if (categoryColors_ == colors) {
        return;
    }
    categoryColors_ = colors;
    if (hasCategories()) {
        vertexCache_.invalidate();
    }
    emit categoryColorsChanged();
    update();
}

RectangleBorder* RectangleSeries::border() const
{
    return border_;
}

QColor RectangleSeries::hoverColor() const
{
    return hoverColor_;
}

void RectangleSeries::setHoverColor(const QColor& color)
{
    if (hoverColor_ == color) {
        return;
    }
    hoverColor_ = color;
    emit hoverColorChanged();
    update();
}

qreal RectangleSeries::minimumWidth() const
{
    return minimumWidth_;
}

void RectangleSeries::setMinimumWidth(const qreal width)
{
    const auto clamped = std::max(width, qreal{0.0});
    if (nearlyEqual(minimumWidth_, clamped)) {
        return;
    }
    minimumWidth_ = clamped;
    emit minimumWidthChanged();
    update();
}

qreal RectangleSeries::minimumHeight() const
{
    return minimumHeight_;
}

void RectangleSeries::setMinimumHeight(const qreal height)
{
    const auto clamped = std::max(height, qreal{0.0});
    if (nearlyEqual(minimumHeight_, clamped)) {
        return;
    }
    minimumHeight_ = clamped;
    emit minimumHeightChanged();
    update();
}

int RectangleSeries::count() const
{
    return dataPoints().count();
}

int RectangleSeries::hoveredIndex() const
{
    return hoveredIndex_;
}

void RectangleSeries::setData(const QVariantList& rects)
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
    applyData(std::move(data), std::move(categories), static_cast<int>(rects.size()));
}

void RectangleSeries::setData(std::vector<double>&& data, std::vector<int>&& categories, const int rectCount)
{
    if (!validateDataSize(data.size(), rectCount, false) || !validateCategories(categories.size(), rectCount)) {
        return;
    }
    applyData(std::move(data), std::move(categories), rectCount);
}

void RectangleSeries::setDataF(std::vector<float>&& data, std::vector<int>&& categories, const int rectCount)
{
    if (!validateDataSize(data.size(), rectCount, true) || !validateCategories(categories.size(), rectCount)) {
        return;
    }
    applyFloatData(std::move(data), std::move(categories), rectCount);
}

void RectangleSeries::postData(std::vector<double>&& data, std::vector<int>&& categories, const int rectCount)
{
    QMetaObject::invokeMethod(
        this,
        [this, rects = std::move(data), rectCategories = std::move(categories), rectCount]() mutable {
            setData(std::move(rects), std::move(rectCategories), rectCount);
        },
        Qt::QueuedConnection);
}

void RectangleSeries::postData(std::vector<float>&& data, std::vector<int>&& categories, const int rectCount)
{
    QMetaObject::invokeMethod(
        this,
        [this, rects = std::move(data), rectCategories = std::move(categories), rectCount]() mutable {
            setDataF(std::move(rects), std::move(rectCategories), rectCount);
        },
        Qt::QueuedConnection);
}

void RectangleSeries::clearData()
{
    PlotSeries::clearData();
}

std::optional<LegendEntry> RectangleSeries::legendEntry() const
{
    auto entry = LegendEntry{};
    entry.symbol = legendSymbol();
    entry.color = color_;
    return entry;
}

void RectangleSeries::setCategories(const QList<int>& categories)
{
    if (!validateCategories(static_cast<std::size_t>(categories.size()), count())) {
        return;
    }
    dataPoints().setCategories(std::vector<int>(categories.cbegin(), categories.cend()));
    vertexCache_.invalidate();
    inspectionDataChanged();
    update();
}

QVariantMap RectangleSeries::rectangleAt(const int index) const
{
    if (index < 0 || index >= count()) {
        return {};
    }
    const auto& keys = rectangleKeys();
    auto rect = QVariantMap{};
    for (size_t k = 0; k < keys.size(); ++k) {
        rect.insert(keys[k], coordinate(index, static_cast<int>(k)));
    }
    if (hasCategories()) {
        rect.insert(QStringLiteral("category"), dataPoints().categories()[static_cast<size_t>(index)]);
    }
    return rect;
}

int RectangleSeries::rectangleIndexAt(const QPointF& position) const
{
    if (count() <= 0 || !xAxis() || !yAxis() || plotRect().isEmpty()) {
        return -1;
    }
    const auto inputs = hitTestInputs(position);
    if (!(lastHitTest_ == inputs)) {
        lastHitIndex_ = topmostRectangleAt(position);
        lastHitTest_ = inputs;
    }
    return lastHitIndex_;
}

bool RectangleSeries::contains(const QPointF& point) const
{
    return boundingRect().contains(point) && rectangleIndexAt(point) >= 0;
}

bool RectangleSeries::recordInspectionSupported() const
{
    return true;
}

InspectionRecord RectangleSeries::inspectionRecord(const int index) const
{
    auto result = InspectionRecord{};
    if (index < 0 || index >= count()) {
        result.status = InspectionStatus::InvalidArgument;
        return result;
    }
    result.index = index;
    result.fields = rectangleAt(index);
    const auto& keys = rectangleKeys();
    const auto valid = std::none_of(keys.begin(), keys.end(), [&result](const auto& key) { return std::isnan(result.fields.value(key).toDouble()); });
    result.status = valid ? InspectionStatus::Ready : InspectionStatus::NoMatch;
    return result;
}

InspectionRecord RectangleSeries::inspectionRecordAt(const QPointF& position) const
{
    const auto index = rectangleIndexAt(position);
    return index < 0 ? InspectionRecord{} : inspectionRecord(index);
}

bool RectangleSeries::validateCategories(const std::size_t categoryCount, const int rectCount) const
{
    if (categoryCount != 0 && categoryCount != static_cast<std::size_t>(rectCount)) {
        qCWarning(lcQAccelPlot) << "RectangleSeries received" << categoryCount << "categories for" << rectCount << "rectangles";
        return false;
    }
    return true;
}

void RectangleSeries::applyData(std::vector<double>&& data, std::vector<int>&& categories, const int rectCount)
{
    const auto previousCount = count();
    dataPoints().setPrecise(std::move(data), rectCount);
    dataPoints().setCategories(std::move(categories));
    notifyDataPointsChanged(DataChange::Replaced, previousCount);
}

void RectangleSeries::applyFloatData(std::vector<float>&& data, std::vector<int>&& categories, const int rectCount)
{
    const auto previousCount = count();
    dataPoints().setFloat(std::move(data), rectCount);
    dataPoints().setCategories(std::move(categories));
    notifyDataPointsChanged(DataChange::Replaced, previousCount);
}

void RectangleSeries::onDataPointsChanged(const DataChange /*change*/, const int previousCount)
{
    // Vertex colors depend on the categories, but not on the coordinates.
    if (count() != previousCount || vertexCacheHasCategories_ || hasCategories()) {
        vertexCache_.invalidate();
    }
    spatialGridValid_ = false;
    spatialGridBudget_->reset();
    lastHitTest_.reset();
    if (hoveredIndex_ >= count()) {
        setHoveredIndex(-1);
    }
    if (previousCount != count()) {
        emit countChanged();
    }
}

double RectangleSeries::coordinate(const int index, const int component) const
{
    return dataPoints().value(index, component);
}

bool RectangleSeries::hasCategories() const
{
    return dataPoints().hasCategories();
}

QColor RectangleSeries::rectangleColor(const int index) const
{
    const auto category = hasCategories() ? dataPoints().categories()[static_cast<size_t>(index)] : -1;
    return category >= 0 && category < categoryColors_.size() ? categoryColors_[category] : color_;
}

bool RectangleSeries::containsInPixels(const int index, const QPointF& position) const
{
    using Internal::edgePixel;
    using Internal::widenedSpan;
    const auto [left, right]
        = widenedSpan(edgePixel(coordinate(index, 0), *xAxis(), width()), edgePixel(coordinate(index, 2), *xAxis(), width()), minimumWidth_);
    const auto [top, bottom]
        = widenedSpan(edgePixel(coordinate(index, 1), *yAxis(), height()), edgePixel(coordinate(index, 3), *yAxis(), height()), minimumHeight_);
    return position.x() >= left && position.x() <= right && position.y() >= top && position.y() <= bottom;
}

void RectangleSeries::setHoveredIndex(const int index)
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

QSGNode* RectangleSeries::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    auto* window = this->window();
    const auto capacity = Internal::recordCapacity(window, kValuesPerRectangle, RectVertexCache::kBytesPerRect);
    if (capacity != renderCapacity_) {
        lastHitTest_.reset();
    }
    updateRenderCapacity(capacity, count());
    if (count() <= 0) {
        delete oldNode;
        return nullptr;
    }
    if (!xAxis() || !yAxis() || plotRect().isEmpty()) {
        qCDebug(lcQAccelPlot) << "missing axes or empty plotRect, returning nullptr";
        delete oldNode;
        return nullptr;
    }

    if (!window) {
        qCDebug(lcQAccelPlot) << "null window, returning nullptr";
        delete oldNode;
        return nullptr;
    }
    if (!Internal::supportsCustomShaderRendering(window)) {
        static auto warned = false;
        if (!warned) {
            qCWarning(lcQAccelPlot)
                << "RectangleSeries custom rendering requires a hardware scene graph backend. Skipping updatePaintNode on the software backend.";
            warned = true;
        }
        delete oldNode;
        return nullptr;
    }

    auto* node = static_cast<QSGGeometryNode*>(oldNode);
    RectMaterial* material = nullptr;
    const auto nodeRecreated = !node;
    const auto drawnCount = std::min(count(), capacity);
    if (vertexCache_.vertexCount() != drawnCount * RectVertexCache::kVerticesPerRect) {
        vertexCache_.invalidate();
    }

    if (!node) {
        if (!vertexCache_.valid()) {
            buildVertexCache(drawnCount);
        }

        auto* geometry = new QSGGeometry(DataTextureMaterial::attributeSet(), 0);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        vertexCache_.copyTo(*geometry);

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
        if (!vertexCache_.valid()) {
            buildVertexCache(drawnCount);
            vertexCache_.copyTo(*node->geometry());
            node->markDirty(QSGNode::DirtyGeometry);
        }
    }

    if (dataPoints().renderRevision() != uploadedRevision_ || nodeRecreated) {
        material->uploadTexture(window, dataPoints().renderData().data(), drawnCount * kValuesPerRectangle);
        uploadedRevision_ = dataPoints().renderRevision();
    }

    updateMaterial(*material, drawnCount);
    node->markDirty(QSGNode::DirtyMaterial);

    return node;
}

void RectangleSeries::hoverEnterEvent(QHoverEvent* event)
{
    setHoveredIndex(rectangleIndexAt(event->position()));
    QQuickItem::hoverEnterEvent(event);
}

void RectangleSeries::hoverMoveEvent(QHoverEvent* event)
{
    setHoveredIndex(rectangleIndexAt(event->position()));
    QQuickItem::hoverMoveEvent(event);
}

void RectangleSeries::hoverLeaveEvent(QHoverEvent* event)
{
    setHoveredIndex(-1);
    QQuickItem::hoverLeaveEvent(event);
}

void RectangleSeries::onAxisScaleChanged()
{
    update();
}

void RectangleSeries::updateMaterial(RectMaterial& material, const int drawnCount) const
{
    const auto originX = dataPoints().renderOrigin(DataPointStore::Dimension::X);
    const auto originY = dataPoints().renderOrigin(DataPointStore::Dimension::Y);
    material.color = color_;
    material.domainMin = QVector2D(static_cast<float>(xAxis()->viewportMin() - originX), static_cast<float>(yAxis()->viewportMin() - originY));
    material.domainMax = QVector2D(static_cast<float>(xAxis()->viewportMax() - originX), static_cast<float>(yAxis()->viewportMax() - originY));
    material.viewportSize = QVector2D(static_cast<float>(width()), static_cast<float>(height()));
    material.logScaleX = xAxis()->logScale() ? 1.0f : 0.0f;
    material.logScaleY = yAxis()->logScale() ? 1.0f : 0.0f;
    material.useVertexColor = hasCategories() ? 1.0f : 0.0f;
    material.rectCount = static_cast<float>(drawnCount);
    material.minimumSize = QVector2D(static_cast<float>(minimumWidth_), static_cast<float>(minimumHeight_));
    material.borderWidth = static_cast<float>(border_->width());
    material.borderColor = border_->color();
    material.hoverColor = hoverColor_.isValid() ? hoverColor_ : QColor{Qt::transparent};
    material.hoveredIndex = hoverColor_.isValid() ? static_cast<float>(hoveredIndex_) : -1.0f;
}

bool RectangleSeries::spatialGridReady() const
{
    if (spatialGridValid_) {
        return true;
    }
    const auto rectangles = dataPoints().view();
    if (!spatialGridBudget_->buildDue(rectangles.count)) {
        return false;
    }
    spatialGridBudget_->timeBuild(rectangles.count, [&] {
        if (rectangles.precise) {
            spatialGrid_.build(rectangles.precise, rectangles.count);
        } else {
            spatialGrid_.buildF(rectangles.floats, rectangles.count);
        }
    });
    spatialGridValid_ = true;
    return true;
}

bool RectangleSeries::HitTestInputs::operator==(const HitTestInputs& other) const
{
    return position == other.position && itemSize == other.itemSize && minimumSize == other.minimumSize && xAtLeft == other.xAtLeft
        && xAtRight == other.xAtRight && yAtTop == other.yAtTop && yAtBottom == other.yAtBottom && logScaleX == other.logScaleX && logScaleY == other.logScaleY;
}

RectangleSeries::HitTestInputs RectangleSeries::hitTestInputs(const QPointF& position) const
{
    return {position, {width(), height()}, {minimumWidth_, minimumHeight_}, xAxis()->pixelToCoord(0.0, width()), xAxis()->pixelToCoord(width(), width()),
        yAxis()->pixelToCoord(0.0, height()), yAxis()->pixelToCoord(height(), height()), xAxis()->logScale(), yAxis()->logScale()};
}

int RectangleSeries::topmostRectangleAt(const QPointF& position) const
{
    // A rectangle widened to contain the cursor has its center, and so part of itself, within half
    // the minimum size of it. That box in data space bounds the candidates for the pixel test.
    const auto x1 = xAxis()->pixelToCoord(position.x() - 0.5 * minimumWidth_, width());
    const auto x2 = xAxis()->pixelToCoord(position.x() + 0.5 * minimumWidth_, width());
    const auto y1 = yAxis()->pixelToCoord(position.y() - 0.5 * minimumHeight_, height());
    const auto y2 = yAxis()->pixelToCoord(position.y() + 0.5 * minimumHeight_, height());
    const auto minX = std::min(x1, x2);
    const auto minY = std::min(y1, y2);
    const auto maxX = std::max(x1, x2);
    const auto maxY = std::max(y1, y2);
    const auto drawnCount = std::min(count(), renderCapacity_);
    const auto accept = [this, &position, drawnCount](const int index) { return index < drawnCount && containsInPixels(index, position); };
    if (spatialGridReady()) {
        return spatialGrid_.queryTopmost(minX, minY, maxX, maxY, accept);
    }
    const auto rectangles = dataPoints().view();
    return spatialGridBudget_->timeScan([&] {
        return rectangles.precise ? SpatialGrid::scanTopmost(rectangles.precise, rectangles.count, minX, minY, maxX, maxY, accept)
                                  : SpatialGrid::scanTopmostF(rectangles.floats, rectangles.count, minX, minY, maxX, maxY, accept);
    });
}

void RectangleSeries::buildVertexCache(const int rectCount)
{
    auto colorAt = RectVertexCache::ColorFunction{};
    if (hasCategories()) {
        colorAt = [this](const int index) { return rectangleColor(index); };
    }
    vertexCache_.rebuild(rectCount, colorAt);
    vertexCacheHasCategories_ = hasCategories();
}

} // namespace QAccelPlot
