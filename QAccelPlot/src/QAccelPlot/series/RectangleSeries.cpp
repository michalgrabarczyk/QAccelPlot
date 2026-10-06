//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/RectangleSeries.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/materials/DataTextureMaterial.hpp"
#include "QAccelPlot/materials/RectMaterial.hpp"
#include "QAccelPlot/series/internal/RectGeometry.hpp"
#include "QAccelPlot/series/internal/SeriesSupport.hpp"

#include <QSGGeometry>
#include <QSGGeometryNode>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>

namespace QAccelPlot {

namespace {

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

RectangleSeries::RectangleSeries(QQuickItem* parent)
    : PlotSeries(parent)
    , color_(defaultRectangleColor())
{
    setFlag(ItemHasContents, true);
    setAcceptHoverEvents(Internal::hoverEnabled());
    setAcceptedMouseButtons(Qt::NoButton);
    setLegendSymbol(LegendSymbol::Fill);
    connect(border_, &RectangleBorder::widthChanged, this, &QQuickItem::update);
    connect(border_, &RectangleBorder::colorChanged, this, &QQuickItem::update);
}

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
    if (nearly_equal(minimumWidth_, clamped)) {
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
    if (nearly_equal(minimumHeight_, clamped)) {
        return;
    }
    minimumHeight_ = clamped;
    emit minimumHeightChanged();
    update();
}

int RectangleSeries::count() const
{
    return rectCount_;
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
    applyData(std::move(data), std::move(categories), static_cast<int>(rects.size()), true);
}

void RectangleSeries::setData(const double* data, const int rectCount)
{
    if (!validateRawDataArguments(data, rectCount)) {
        return;
    }
    auto buffer = std::vector<double>{};
    if (rectCount > 0) {
        buffer.assign(data, data + static_cast<size_t>(rectCount) * 4);
    }
    applyData(std::move(buffer), {}, rectCount, true);
}

void RectangleSeries::setData(std::vector<double>&& data, const int rectCount)
{
    setData(std::move(data), {}, rectCount);
}

void RectangleSeries::setData(std::vector<double>&& data, std::vector<int>&& categories, const int rectCount)
{
    if (!validateDataArguments(data.size(), categories.size(), rectCount)) {
        return;
    }
    applyData(std::move(data), std::move(categories), rectCount, true);
}

void RectangleSeries::setDataNoRange(const double* data, const int rectCount)
{
    if (!validateRawDataArguments(data, rectCount)) {
        return;
    }
    auto buffer = std::vector<double>{};
    if (rectCount > 0) {
        buffer.assign(data, data + static_cast<size_t>(rectCount) * 4);
    }
    setDataNoRange(std::move(buffer), rectCount);
}

void RectangleSeries::setDataNoRange(std::vector<double>&& data, const int rectCount)
{
    setDataNoRange(std::move(data), {}, rectCount);
}

void RectangleSeries::setDataNoRange(std::vector<double>&& data, std::vector<int>&& categories, const int rectCount)
{
    if (!validateDataArguments(data.size(), categories.size(), rectCount)) {
        return;
    }
    applyData(std::move(data), std::move(categories), rectCount, false);
}

void RectangleSeries::setDataF(const float* data, const int rectCount)
{
    setDataFFromArray(data, rectCount, true);
}

void RectangleSeries::setDataF(std::vector<float>&& data, const int rectCount)
{
    setDataF(std::move(data), {}, rectCount);
}

void RectangleSeries::setDataF(std::vector<float>&& data, std::vector<int>&& categories, const int rectCount)
{
    if (!validateDataArguments(data.size(), categories.size(), rectCount)) {
        return;
    }
    applyFloatData(std::move(data), std::move(categories), rectCount, true);
}

void RectangleSeries::setDataFNoRange(const float* data, const int rectCount)
{
    setDataFFromArray(data, rectCount, false);
}

void RectangleSeries::setDataFNoRange(std::vector<float>&& data, const int rectCount)
{
    setDataFNoRange(std::move(data), {}, rectCount);
}

void RectangleSeries::setDataFNoRange(std::vector<float>&& data, std::vector<int>&& categories, const int rectCount)
{
    if (!validateDataArguments(data.size(), categories.size(), rectCount)) {
        return;
    }
    applyFloatData(std::move(data), std::move(categories), rectCount, false);
}

void RectangleSeries::postData(std::vector<double>&& data, const int rectCount)
{
    postData(std::move(data), {}, rectCount);
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

void RectangleSeries::postData(std::vector<float>&& data, const int rectCount)
{
    postData(std::move(data), {}, rectCount);
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
    applyData({}, {}, 0, true);
}

void RectangleSeries::setCategories(const QList<int>& categories)
{
    if (!categories.isEmpty() && categories.size() != rectCount_) {
        qCWarning(lcQAccelPlot) << "RectangleSeries received" << categories.size() << "categories for" << rectCount_ << "rectangles";
        return;
    }
    categories_.assign(categories.cbegin(), categories.cend());
    vertexCache_.invalidate();
    inspectionDataChanged();
    update();
}

QVariantMap RectangleSeries::rectangleAt(const int index) const
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

int RectangleSeries::rectangleIndexAt(const QPointF& position) const
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
    const auto minX = std::min(x1, x2);
    const auto minY = std::min(y1, y2);
    const auto maxX = std::max(x1, x2);
    const auto maxY = std::max(y1, y2);
    const auto accept = [this, &position](const int index) { return containsInPixels(index, position); };
    if (spatialGridReady()) {
        return spatialGrid_.queryTopmost(minX, minY, maxX, maxY, accept);
    }
    return hasPreciseData() ? SpatialGrid::scanTopmost(data_.data(), rectCount_, minX, minY, maxX, maxY, accept)
                            : SpatialGrid::scanTopmostF(renderData_.data(), rectCount_, minX, minY, maxX, maxY, accept);
}

bool RectangleSeries::contains(const QPointF& point) const
{
    return boundingRect().contains(point) && rectangleIndexAt(point) >= 0;
}

InspectionRecord RectangleSeries::inspectionRecord(const int index) const
{
    auto result = InspectionRecord{};
    if (index < 0 || index >= rectCount_) {
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

bool RectangleSeries::validateRawDataArguments(const void* data, const int rectCount) const
{
    if (rectCount < 0) {
        qCWarning(lcQAccelPlot) << "RectangleSeries data rectangle count cannot be negative:" << rectCount;
        return false;
    }
    if (rectCount > 0 && !data) {
        qCWarning(lcQAccelPlot) << "RectangleSeries received a null data pointer for" << rectCount << "rectangles";
        return false;
    }
    return true;
}

bool RectangleSeries::validateDataArguments(const std::size_t valueCount, const std::size_t categoryCount, const int rectCount) const
{
    if (rectCount < 0) {
        qCWarning(lcQAccelPlot) << "RectangleSeries data rectangle count cannot be negative:" << rectCount;
        return false;
    }
    const auto expectedValueCount = static_cast<std::size_t>(rectCount) * 4;
    if (valueCount != expectedValueCount) {
        qCWarning(lcQAccelPlot) << "RectangleSeries received" << valueCount << "coordinates for" << rectCount << "rectangles; expected" << expectedValueCount;
        return false;
    }
    if (categoryCount != 0 && categoryCount != static_cast<std::size_t>(rectCount)) {
        qCWarning(lcQAccelPlot) << "RectangleSeries received" << categoryCount << "categories for" << rectCount << "rectangles";
        return false;
    }
    return true;
}

void RectangleSeries::applyData(std::vector<double>&& data, std::vector<int>&& categories, const int rectCount, const bool reportRanges)
{
    data_ = std::move(data);
    finishDataChange(std::move(categories), rectCount, reportRanges);
}

void RectangleSeries::applyFloatData(std::vector<float>&& data, std::vector<int>&& categories, const int rectCount, const bool reportRanges)
{
    data_ = std::vector<double>{};
    renderData_ = std::move(data);
    renderOriginX_ = 0.0;
    renderOriginY_ = 0.0;
    finishDataChange(std::move(categories), rectCount, reportRanges);
}

void RectangleSeries::setDataFFromArray(const float* data, const int rectCount, const bool reportRanges)
{
    if (!validateRawDataArguments(data, rectCount)) {
        return;
    }
    auto buffer = std::move(renderData_);
    if (rectCount > 0) {
        buffer.assign(data, data + static_cast<size_t>(rectCount) * 4);
    } else {
        buffer.clear();
    }
    applyFloatData(std::move(buffer), {}, rectCount, reportRanges);
}

void RectangleSeries::finishDataChange(std::vector<int>&& categories, const int rectCount, const bool reportRanges)
{
    const auto previousCount = rectCount_;
    // Vertex colors depend on the categories, but not on the coordinates.
    if (rectCount != rectCount_ || hasCategories() || !categories.empty()) {
        vertexCache_.invalidate();
    }
    categories_ = std::move(categories);
    rectCount_ = rectCount;
    dataChanged_ = true;
    spatialGridValid_ = false;
    scansSinceInvalidation_ = 0;
    if (reportRanges) {
        updateDataRanges();
    }
    if (hoveredIndex_ >= rectCount_) {
        setHoveredIndex(-1);
    }
    if (previousCount != rectCount_) {
        emit countChanged();
    }
    inspectionDataChanged();
    update();
}

bool RectangleSeries::hasPreciseData() const
{
    return !data_.empty();
}

double RectangleSeries::coordinate(const int index, const int component) const
{
    const auto offset = static_cast<size_t>(index) * 4 + static_cast<size_t>(component);
    return hasPreciseData() ? data_[offset] : static_cast<double>(renderData_[offset]);
}

bool RectangleSeries::hasCategories() const
{
    return !categories_.empty();
}

QColor RectangleSeries::rectangleColor(const int index) const
{
    const auto category = hasCategories() ? categories_[static_cast<size_t>(index)] : -1;
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
    const auto nodeRecreated = !node;

    if (!node) {
        if (!vertexCache_.valid()) {
            buildVertexCache();
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
            buildVertexCache();
            vertexCache_.copyTo(*node->geometry());
            node->markDirty(QSGNode::DirtyGeometry);
        }
    }

    // Upload data texture
    if (dataChanged_ || nodeRecreated) {
        if (hasPreciseData()) {
            rebuildRenderData(xAxis()->logScale(), yAxis()->logScale());
        }
        const auto numFloats = rectCount_ * 4;
        material->uploadTexture(window, renderData_.data(), numFloats);
        dataChanged_ = false;
    }

    updateMaterial(*material);
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
    // Only origin-shifted double data depends on the scale; float data is uploaded as is.
    if (hasPreciseData()) {
        dataChanged_ = true;
    }
    update();
}

void RectangleSeries::updateMaterial(RectMaterial& material) const
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

// Building the grid costs as much as many scans of the rectangles, so it only pays off for data
// that stays. The first queries after a change scan instead, which keeps live data cheap.
bool RectangleSeries::spatialGridReady() const
{
    static constexpr auto kScansBeforeIndexing = 8;
    if (spatialGridValid_) {
        return true;
    }
    if (scansSinceInvalidation_ < kScansBeforeIndexing) {
        ++scansSinceInvalidation_;
        return false;
    }
    if (hasPreciseData()) {
        spatialGrid_.build(data_.data(), rectCount_);
    } else {
        spatialGrid_.buildF(renderData_.data(), rectCount_);
    }
    spatialGridValid_ = true;
    return true;
}

void RectangleSeries::rebuildRenderData(const bool logScaleX, const bool logScaleY)
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

void RectangleSeries::updateDataRanges()
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

void RectangleSeries::buildVertexCache()
{
    auto colorAt = RectVertexCache::ColorFunction{};
    if (hasCategories()) {
        colorAt = [this](const int index) { return rectangleColor(index); };
    }
    vertexCache_.rebuild(rectCount_, colorAt);
}

} // namespace QAccelPlot
