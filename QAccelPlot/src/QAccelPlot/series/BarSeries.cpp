//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/BarSeries.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/materials/BarMaterial.hpp"
#include "QAccelPlot/materials/DataTextureMaterial.hpp"
#include "QAccelPlot/series/internal/RectGeometry.hpp"
#include "QAccelPlot/series/internal/SeriesSupport.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"

#include <QSGGeometry>
#include <QSGGeometryNode>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>

namespace QAccelPlot {

namespace {

constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();

// Returns \a value as a double, or \a fallback when it is missing, null, or not a number.
double numberOr(const QVariant& value, const double fallback)
{
    if (!value.isValid() || value.isNull()) {
        return fallback;
    }
    auto ok = false;
    const auto number = value.toDouble(&ok);
    return ok ? number : fallback;
}

// True for a bar object that gives its own extent along the position axis.
bool isRangedBar(const QVariant& bar)
{
    if (bar.metaType().id() != QMetaType::QVariantMap) {
        return false;
    }
    const auto map = bar.toMap();
    return map.contains(QStringLiteral("from")) || map.contains(QStringLiteral("to"));
}

// A coordinate the axis can place: finite, and positive on a log axis.
bool isPlaceable(const double coordinate, const bool logScale)
{
    return std::isfinite(coordinate) && (!logScale || coordinate > 0.0);
}

struct Extent {
    qreal min{std::numeric_limits<qreal>::max()};
    qreal max{std::numeric_limits<qreal>::lowest()};

    void include(const qreal coordinate)
    {
        min = std::min(min, coordinate);
        max = std::max(max, coordinate);
    }

    bool isEmpty() const
    {
        return min > max;
    }
};

}

BarSeries::BarSeries(QQuickItem* parent)
    : PlotSeries(parent)
    , color_(ColorPalette::dark().seriesPrimary)
{
    setFlag(ItemHasContents, true);
    setAcceptHoverEvents(Internal::hoverEnabled());
    setAcceptedMouseButtons(Qt::NoButton);
    setLegendSymbol(LegendSymbol::Fill);
    connect(border_, &RectangleBorder::widthChanged, this, &QQuickItem::update);
    connect(border_, &RectangleBorder::colorChanged, this, &QQuickItem::update);
}

Qt::Orientation BarSeries::orientation() const
{
    return orientation_;
}

void BarSeries::setOrientation(const Qt::Orientation orientation)
{
    if (orientation_ == orientation) {
        return;
    }
    orientation_ = orientation;
    // The render origins depend on which axis, and so which log scale, each coordinate maps to.
    if (hasPreciseData()) {
        dataChanged_ = true;
    }
    onGeometryChanged();
    emit orientationChanged();
}

qreal BarSeries::barWidth() const
{
    return barWidth_;
}

void BarSeries::setBarWidth(const qreal width)
{
    const auto clamped = std::max(width, qreal{0.0});
    if (nearly_equal(barWidth_, clamped)) {
        return;
    }
    barWidth_ = clamped;
    onGeometryChanged();
    emit barWidthChanged();
}

qreal BarSeries::barOffset() const
{
    return barOffset_;
}

void BarSeries::setBarOffset(const qreal offset)
{
    if (nearly_equal(barOffset_, offset)) {
        return;
    }
    barOffset_ = offset;
    onGeometryChanged();
    emit barOffsetChanged();
}

qreal BarSeries::baselineValue() const
{
    return baselineValue_;
}

void BarSeries::setBaselineValue(const qreal baselineValue)
{
    if (std::isnan(baselineValue) || nearly_equal(baselineValue_, baselineValue)) {
        return;
    }
    baselineValue_ = baselineValue;
    onGeometryChanged();
    emit baselineValueChanged();
}

qreal BarSeries::minimumWidth() const
{
    return minimumWidth_;
}

void BarSeries::setMinimumWidth(const qreal width)
{
    const auto clamped = std::max(width, qreal{0.0});
    if (nearly_equal(minimumWidth_, clamped)) {
        return;
    }
    minimumWidth_ = clamped;
    emit minimumWidthChanged();
    update();
}

QColor BarSeries::color() const
{
    return color_;
}

void BarSeries::setColor(const QColor& color)
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

QList<QColor> BarSeries::categoryColors() const
{
    return categoryColors_;
}

void BarSeries::setCategoryColors(const QList<QColor>& colors)
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

RectangleBorder* BarSeries::border() const
{
    return border_;
}

QColor BarSeries::hoverColor() const
{
    return hoverColor_;
}

void BarSeries::setHoverColor(const QColor& color)
{
    if (hoverColor_ == color) {
        return;
    }
    hoverColor_ = color;
    emit hoverColorChanged();
    update();
}

int BarSeries::count() const
{
    return barCount_;
}

int BarSeries::hoveredIndex() const
{
    return hoveredIndex_;
}

void BarSeries::setData(const QVariantList& bars)
{
    const auto ranged = std::any_of(bars.cbegin(), bars.cend(), isRangedBar);
    const auto stride = ranged ? size_t{3} : size_t{2};
    auto data = std::vector<double>(static_cast<size_t>(bars.size()) * stride);
    auto categories = std::vector<int>(static_cast<size_t>(bars.size()), -1);
    auto anyCategory = false;
    for (qsizetype i = 0; i < bars.size(); ++i) {
        const auto& bar = bars[i];
        auto* values = data.data() + static_cast<size_t>(i) * stride;
        const auto isObject = bar.metaType().id() == QMetaType::QVariantMap;
        const auto map = isObject ? bar.toMap() : QVariantMap{};
        if (ranged) {
            values[0] = numberOr(map.value(QStringLiteral("from")), kNaN);
            values[1] = numberOr(map.value(QStringLiteral("to")), kNaN);
        } else {
            values[0] = numberOr(map.value(QStringLiteral("position")), static_cast<double>(i));
        }
        values[stride - 1] = numberOr(isObject ? map.value(QStringLiteral("value")) : bar, kNaN);
        const auto category = map.value(QStringLiteral("category"));
        if (category.isValid() && !category.isNull()) {
            categories[static_cast<size_t>(i)] = category.toInt();
            anyCategory = true;
        }
    }
    if (!anyCategory) {
        categories.clear();
    }
    if (ranged) {
        applyRangedData(std::move(data), std::move(categories), static_cast<int>(bars.size()));
    } else {
        applyData(std::move(data), std::move(categories), static_cast<int>(bars.size()));
    }
}

void BarSeries::setData(const double* data, const int barCount)
{
    if (!validateRawDataArguments(data, barCount)) {
        return;
    }
    applyData(std::vector<double>(data, data + static_cast<size_t>(barCount) * 2), {}, barCount);
}

void BarSeries::setData(std::vector<double>&& data, const int barCount)
{
    setData(std::move(data), {}, barCount);
}

void BarSeries::setData(std::vector<double>&& data, std::vector<int>&& categories, const int barCount)
{
    if (!validateDataArguments(data.size(), categories.size(), barCount)) {
        return;
    }
    applyData(std::move(data), std::move(categories), barCount);
}

void BarSeries::setDataF(const float* data, const int barCount)
{
    setDataFFromArray(data, barCount);
}

void BarSeries::setDataF(std::vector<float>&& data, const int barCount)
{
    setDataF(std::move(data), {}, barCount);
}

void BarSeries::setDataF(std::vector<float>&& data, std::vector<int>&& categories, const int barCount)
{
    if (!validateDataArguments(data.size(), categories.size(), barCount)) {
        return;
    }
    applyFloatData(std::move(data), std::move(categories), barCount);
}

void BarSeries::postData(std::vector<double>&& data, const int barCount)
{
    postData(std::move(data), {}, barCount);
}

void BarSeries::postData(std::vector<double>&& data, std::vector<int>&& categories, const int barCount)
{
    QMetaObject::invokeMethod(
        this,
        [this, bars = std::move(data), barCategories = std::move(categories), barCount]() mutable {
            setData(std::move(bars), std::move(barCategories), barCount);
        },
        Qt::QueuedConnection);
}

void BarSeries::postData(std::vector<float>&& data, const int barCount)
{
    postData(std::move(data), {}, barCount);
}

void BarSeries::postData(std::vector<float>&& data, std::vector<int>&& categories, const int barCount)
{
    QMetaObject::invokeMethod(
        this,
        [this, bars = std::move(data), barCategories = std::move(categories), barCount]() mutable {
            setDataF(std::move(bars), std::move(barCategories), barCount);
        },
        Qt::QueuedConnection);
}

void BarSeries::setRangedData(std::vector<double>&& data, const int barCount)
{
    setRangedData(std::move(data), {}, barCount);
}

void BarSeries::setRangedData(std::vector<double>&& data, std::vector<int>&& categories, const int barCount)
{
    if (!validateDataArguments(data.size(), categories.size(), barCount, 3)) {
        return;
    }
    applyRangedData(std::move(data), std::move(categories), barCount);
}

void BarSeries::postRangedData(std::vector<double>&& data, const int barCount)
{
    QMetaObject::invokeMethod(this, [this, bars = std::move(data), barCount]() mutable { setRangedData(std::move(bars), barCount); }, Qt::QueuedConnection);
}

void BarSeries::clearData()
{
    applyData({}, {}, 0);
}

void BarSeries::setCategories(const QList<int>& categories)
{
    if (!categories.isEmpty() && categories.size() != barCount_) {
        qCWarning(lcQAccelPlot) << "BarSeries received" << categories.size() << "categories for" << barCount_ << "bars";
        return;
    }
    categories_.assign(categories.cbegin(), categories.cend());
    vertexCache_.invalidate();
    inspectionDataChanged();
    update();
}

QVariantMap BarSeries::barAt(const int index) const
{
    if (index < 0 || index >= barCount_) {
        return {};
    }
    auto bar = QVariantMap{{QStringLiteral("value"), value(index)}};
    if (ranged_) {
        bar.insert(QStringLiteral("from"), component(index, 0));
        bar.insert(QStringLiteral("to"), component(index, 1));
    } else {
        bar.insert(QStringLiteral("position"), component(index, 0));
    }
    if (hasCategories()) {
        bar.insert(QStringLiteral("category"), categories_[static_cast<size_t>(index)]);
    }
    return bar;
}

int BarSeries::barIndexAt(const QPointF& position) const
{
    if (barCount_ <= 0 || !xAxis() || !yAxis() || plotRect().isEmpty()) {
        return -1;
    }
    // A bar widened to contain the cursor has its center within half the minimum width of it,
    // across the position axis. That box in data space bounds the candidates for the pixel test.
    const auto halfX = isHorizontal() ? 0.0 : 0.5 * minimumWidth_;
    const auto halfY = isHorizontal() ? 0.5 * minimumWidth_ : 0.0;
    const auto x1 = xAxis()->pixelToCoord(position.x() - halfX, width());
    const auto x2 = xAxis()->pixelToCoord(position.x() + halfX, width());
    const auto y1 = yAxis()->pixelToCoord(position.y() - halfY, height());
    const auto y2 = yAxis()->pixelToCoord(position.y() + halfY, height());
    ensureSpatialGrid();
    return spatialGrid_.queryTopmost(std::min(x1, x2), std::min(y1, y2), std::max(x1, x2), std::max(y1, y2),
        [this, &position](const int index) { return containsInPixels(index, position); });
}

bool BarSeries::contains(const QPointF& point) const
{
    return boundingRect().contains(point) && barIndexAt(point) >= 0;
}

InspectionRecord BarSeries::inspectionRecord(const int index) const
{
    auto result = InspectionRecord{};
    if (index < 0 || index >= barCount_) {
        result.status = InspectionStatus::InvalidArgument;
        return result;
    }
    result.index = index;
    result.fields = barAt(index);
    result.status = std::isnan(barRect(index)[0]) ? InspectionStatus::NoMatch : InspectionStatus::Ready;
    return result;
}

InspectionRecord BarSeries::inspectionRecordAt(const QPointF& position) const
{
    const auto index = barIndexAt(position);
    return index < 0 ? InspectionRecord{} : inspectionRecord(index);
}

QSGNode* BarSeries::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    auto* window = this->window();
    if (barCount_ <= 0 || !xAxis() || !yAxis() || plotRect().isEmpty() || !window) {
        return releaseNode(oldNode);
    }

    auto* node = static_cast<QSGGeometryNode*>(oldNode);
    const auto nodeRecreated = !node;
    if (!node) {
        auto* geometry = new QSGGeometry(DataTextureMaterial::attributeSet(), 0);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        node = new QSGGeometryNode;
        node->setGeometry(geometry);
        node->setFlag(QSGNode::OwnsGeometry);
        node->setMaterial(new BarMaterial);
        node->setFlag(QSGNode::OwnsMaterial);
    }

    // Vertices hold only ids and category colors, so coordinate changes don't rebuild them.
    const auto rebuildVertices = !vertexCache_.valid();
    if (rebuildVertices) {
        buildVertexCache();
    }
    if (rebuildVertices || nodeRecreated) {
        vertexCache_.copyTo(*node->geometry());
        node->markDirty(QSGNode::DirtyGeometry);
    }

    auto* material = static_cast<BarMaterial*>(node->material());
    if (dataChanged_ || nodeRecreated) {
        if (hasPreciseData()) {
            const auto logPosition = isHorizontal() ? yAxis()->logScale() : xAxis()->logScale();
            const auto logValue = isHorizontal() ? xAxis()->logScale() : yAxis()->logScale();
            rebuildRenderData(logPosition, logValue);
        }
        material->uploadTexture(window, renderData_.data(), barCount_ * valuesPerBar());
        dataChanged_ = false;
    }

    updateMaterial(*material);
    node->markDirty(QSGNode::DirtyMaterial);
    return node;
}

void BarSeries::hoverEnterEvent(QHoverEvent* event)
{
    setHoveredIndex(barIndexAt(event->position()));
    QQuickItem::hoverEnterEvent(event);
}

void BarSeries::hoverMoveEvent(QHoverEvent* event)
{
    setHoveredIndex(barIndexAt(event->position()));
    QQuickItem::hoverMoveEvent(event);
}

void BarSeries::hoverLeaveEvent(QHoverEvent* event)
{
    setHoveredIndex(-1);
    QQuickItem::hoverLeaveEvent(event);
}

void BarSeries::onAxisScaleChanged()
{
    // Only origin-shifted double data depends on the scale; float data is uploaded as is.
    if (hasPreciseData()) {
        dataChanged_ = true;
    }
    invalidateDataRanges();
    update();
}

bool BarSeries::validateRawDataArguments(const void* data, const int barCount) const
{
    if (barCount < 0) {
        qCWarning(lcQAccelPlot) << "BarSeries data bar count cannot be negative:" << barCount;
        return false;
    }
    if (barCount > 0 && !data) {
        qCWarning(lcQAccelPlot) << "BarSeries received a null data pointer for" << barCount << "bars";
        return false;
    }
    return true;
}

bool BarSeries::validateDataArguments(const std::size_t valueCount, const std::size_t categoryCount, const int barCount, const int barValueCount) const
{
    if (barCount < 0) {
        qCWarning(lcQAccelPlot) << "BarSeries data bar count cannot be negative:" << barCount;
        return false;
    }
    const auto expectedValueCount = static_cast<std::size_t>(barCount) * static_cast<std::size_t>(barValueCount);
    if (valueCount != expectedValueCount) {
        qCWarning(lcQAccelPlot) << "BarSeries received" << valueCount << "values for" << barCount << "bars; expected" << expectedValueCount;
        return false;
    }
    if (categoryCount != 0 && categoryCount != static_cast<std::size_t>(barCount)) {
        qCWarning(lcQAccelPlot) << "BarSeries received" << categoryCount << "categories for" << barCount << "bars";
        return false;
    }
    return true;
}

void BarSeries::applyData(std::vector<double>&& data, std::vector<int>&& categories, const int barCount)
{
    data_ = std::move(data);
    ranged_ = false;
    finishDataChange(std::move(categories), barCount);
}

void BarSeries::applyRangedData(std::vector<double>&& data, std::vector<int>&& categories, const int barCount)
{
    data_ = std::move(data);
    ranged_ = true;
    finishDataChange(std::move(categories), barCount);
}

void BarSeries::applyFloatData(std::vector<float>&& data, std::vector<int>&& categories, const int barCount)
{
    data_ = std::vector<double>{};
    ranged_ = false;
    renderData_ = std::move(data);
    renderOriginPosition_ = 0.0;
    renderOriginValue_ = 0.0;
    finishDataChange(std::move(categories), barCount);
}

void BarSeries::setDataFFromArray(const float* data, const int barCount)
{
    if (!validateRawDataArguments(data, barCount)) {
        return;
    }
    auto buffer = std::move(renderData_);
    buffer.assign(data, data + static_cast<size_t>(barCount) * 2);
    applyFloatData(std::move(buffer), {}, barCount);
}

void BarSeries::finishDataChange(std::vector<int>&& categories, const int barCount)
{
    const auto previousCount = barCount_;
    // Vertex colors depend on the categories, but not on the coordinates.
    if (barCount != barCount_ || hasCategories() || !categories.empty()) {
        vertexCache_.invalidate();
    }
    categories_ = std::move(categories);
    barCount_ = barCount;
    dataChanged_ = true;
    spatialGridValid_ = false;
    invalidateDataRanges();
    if (hoveredIndex_ >= barCount_) {
        setHoveredIndex(-1);
    }
    if (previousCount != barCount_) {
        emit countChanged();
    }
    inspectionDataChanged();
    update();
}

void BarSeries::onGeometryChanged()
{
    spatialGridValid_ = false;
    invalidateDataRanges();
    update();
}

bool BarSeries::isHorizontal() const
{
    return orientation_ == Qt::Horizontal;
}

bool BarSeries::hasPreciseData() const
{
    return !data_.empty();
}

int BarSeries::valuesPerBar() const
{
    return ranged_ ? 3 : 2;
}

double BarSeries::component(const int index, const int offset) const
{
    const auto position = static_cast<size_t>(index) * static_cast<size_t>(valuesPerBar()) + static_cast<size_t>(offset);
    return hasPreciseData() ? data_[position] : static_cast<double>(renderData_[position]);
}

double BarSeries::value(const int index) const
{
    return component(index, valuesPerBar() - 1);
}

std::array<double, 2> BarSeries::positionSpan(const int index) const
{
    if (ranged_) {
        const auto from = component(index, 0);
        const auto to = component(index, 1);
        return std::isnan(from) || std::isnan(to) ? std::array<double, 2>{kNaN, kNaN} : std::array<double, 2>{from, to};
    }
    const auto center = component(index, 0) + barOffset_;
    if (!std::isfinite(center)) {
        return {kNaN, kNaN};
    }
    return {center - 0.5 * barWidth_, center + 0.5 * barWidth_};
}

std::array<double, 4> BarSeries::barRect(const int index) const
{
    const auto span = positionSpan(index);
    const auto barValue = value(index);
    if (std::isnan(span[0]) || std::isnan(barValue)) {
        return {kNaN, kNaN, kNaN, kNaN};
    }
    if (isHorizontal()) {
        return {baselineValue_, span[0], barValue, span[1]};
    }
    return {span[0], baselineValue_, span[1], barValue};
}

bool BarSeries::hasCategories() const
{
    return !categories_.empty();
}

QColor BarSeries::barColor(const int index) const
{
    const auto category = hasCategories() ? categories_[static_cast<size_t>(index)] : -1;
    return category >= 0 && category < categoryColors_.size() ? categoryColors_[category] : color_;
}

bool BarSeries::containsInPixels(const int index, const QPointF& position) const
{
    using Internal::edgePixel;
    using Internal::widenedSpan;
    const auto rect = barRect(index);
    if (std::isnan(rect[0])) {
        return false;
    }
    const auto minimumX = isHorizontal() ? 0.0 : minimumWidth_;
    const auto minimumY = isHorizontal() ? minimumWidth_ : 0.0;
    const auto [left, right] = widenedSpan(edgePixel(rect[0], *xAxis(), width()), edgePixel(rect[2], *xAxis(), width()), minimumX);
    const auto [top, bottom] = widenedSpan(edgePixel(rect[1], *yAxis(), height()), edgePixel(rect[3], *yAxis(), height()), minimumY);
    return position.x() >= left && position.x() <= right && position.y() >= top && position.y() <= bottom;
}

void BarSeries::setHoveredIndex(const int index)
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

QSGNode* BarSeries::releaseNode(QSGNode* oldNode) const
{
    if (barCount_ > 0) {
        qCDebug(lcQAccelPlot) << "BarSeries has no axes, plot area, or window; not drawing";
    }
    delete oldNode;
    return nullptr;
}

void BarSeries::updateMaterial(BarMaterial& material) const
{
    const auto horizontal = isHorizontal();
    const auto originX = horizontal ? renderOriginValue_ : renderOriginPosition_;
    const auto originY = horizontal ? renderOriginPosition_ : renderOriginValue_;
    const auto minimumSize = static_cast<float>(minimumWidth_);
    material.color = color_;
    material.domainMin = QVector2D(static_cast<float>(xAxis()->viewportMin() - originX), static_cast<float>(yAxis()->viewportMin() - originY));
    material.domainMax = QVector2D(static_cast<float>(xAxis()->viewportMax() - originX), static_cast<float>(yAxis()->viewportMax() - originY));
    material.viewportSize = QVector2D(static_cast<float>(width()), static_cast<float>(height()));
    material.logScaleX = xAxis()->logScale() ? 1.0f : 0.0f;
    material.logScaleY = yAxis()->logScale() ? 1.0f : 0.0f;
    material.useVertexColor = hasCategories() ? 1.0f : 0.0f;
    material.rectCount = static_cast<float>(barCount_);
    material.minimumSize = horizontal ? QVector2D(0.0f, minimumSize) : QVector2D(minimumSize, 0.0f);
    material.borderWidth = static_cast<float>(border_->width());
    material.borderColor = border_->color();
    material.hoverColor = hoverColor_.isValid() ? hoverColor_ : QColor{Qt::transparent};
    material.hoveredIndex = hoverColor_.isValid() ? static_cast<float>(hoveredIndex_) : -1.0f;
    material.barWidth = static_cast<float>(barWidth_);
    material.barOffset = static_cast<float>(barOffset_);
    material.baseline = static_cast<float>(baselineValue_ - renderOriginValue_);
    material.horizontal = horizontal ? 1.0f : 0.0f;
    material.ranged = ranged_ ? 1.0f : 0.0f;
}

void BarSeries::ensureSpatialGrid() const
{
    if (spatialGridValid_) {
        return;
    }
    // Only the position extent is indexed. Bars usually span much of the value axis, so indexing it
    // would push tall bars past the grid's per-item cell limit into its linear scan; the pixel test
    // checks the value extent instead.
    constexpr auto kInf = std::numeric_limits<double>::infinity();
    const auto valueStart = isHorizontal() ? 0 : 1;
    hitRects_.resize(static_cast<size_t>(barCount_) * 4);
    for (auto i = 0; i < barCount_; ++i) {
        auto rect = barRect(i);
        if (!std::isnan(rect[0])) {
            rect[static_cast<size_t>(valueStart)] = -kInf;
            rect[static_cast<size_t>(valueStart) + 2] = kInf;
        }
        std::copy(rect.cbegin(), rect.cend(), hitRects_.begin() + static_cast<std::ptrdiff_t>(i) * 4);
    }
    spatialGrid_.build(hitRects_.data(), barCount_);
    spatialGridValid_ = true;
}

void BarSeries::buildVertexCache()
{
    auto colorAt = RectVertexCache::ColorFunction{};
    if (hasCategories()) {
        colorAt = [this](const int index) { return barColor(index); };
    }
    vertexCache_.rebuild(barCount_, colorAt);
}

PlotSeries::DataRanges BarSeries::computeDataRanges() const
{
    const auto* positionAxis = isHorizontal() ? yAxis() : xAxis();
    const auto* valueAxis = isHorizontal() ? xAxis() : yAxis();
    const auto logPosition = positionAxis && positionAxis->logScale();
    const auto logValue = valueAxis && valueAxis->logScale();

    // A ranged bar contributes its two edges, any other bar its position.
    auto positions = Extent{};
    auto values = Extent{};
    for (auto i = 0; i < barCount_; ++i) {
        const auto first = component(i, 0);
        const auto last = ranged_ ? component(i, 1) : first;
        const auto barValue = value(i);
        if (std::isnan(first) || std::isnan(last) || std::isnan(barValue)) {
            continue;
        }
        const auto firstPlaced = isPlaceable(first, logPosition);
        const auto lastPlaced = isPlaceable(last, logPosition);
        if (firstPlaced) {
            positions.include(first);
        }
        if (lastPlaced) {
            positions.include(last);
        }
        if ((firstPlaced || lastPlaced) && isPlaceable(barValue, logValue)) {
            values.include(barValue);
        }
    }
    if (positions.isEmpty()) {
        return {};
    }
    if (isPlaceable(baselineValue_, logValue)) {
        values.include(baselineValue_);
    }

    auto positionMin = positions.min;
    auto positionMax = positions.max;
    if (!ranged_) {
        // Widen by half a bar so the outer bars are fully in view, unless that leaves a log axis' domain.
        const auto widenedMin = positions.min + barOffset_ - 0.5 * barWidth_;
        positionMin = isPlaceable(widenedMin, logPosition) ? widenedMin : positions.min;
        positionMax = positions.max + barOffset_ + 0.5 * barWidth_;
    }
    const auto positionExtent = std::optional<DataExtent>{DataExtent{positionMin, positionMax}};
    const auto valueExtent = values.isEmpty() ? std::optional<DataExtent>{} : DataExtent{values.min, values.max};
    return isHorizontal() ? DataRanges{valueExtent, positionExtent} : DataRanges{positionExtent, valueExtent};
}

void BarSeries::rebuildRenderData(const bool logScalePosition, const bool logScaleValue)
{
    // Log-scale coordinates aren't translation-invariant, so origin-shifting is skipped for a
    // log-scale axis, matching RectangleSeries.
    renderOriginPosition_ = 0.0;
    renderOriginValue_ = 0.0;
    // The value is the last component of a bar; the ones before it lie on the position axis.
    const auto stride = static_cast<size_t>(valuesPerBar());
    const auto valueOffset = stride - 1;
    auto foundPositionOrigin = logScalePosition;
    auto foundValueOrigin = logScaleValue;
    for (auto base = size_t{0}; base < data_.size() && !(foundPositionOrigin && foundValueOrigin); base += stride) {
        for (auto offset = size_t{0}; offset < valueOffset && !foundPositionOrigin; ++offset) {
            if (std::isfinite(data_[base + offset])) {
                renderOriginPosition_ = data_[base + offset];
                foundPositionOrigin = true;
            }
        }
        if (!foundValueOrigin && std::isfinite(data_[base + valueOffset])) {
            renderOriginValue_ = data_[base + valueOffset];
            foundValueOrigin = true;
        }
    }

    renderData_.resize(data_.size());
    for (auto base = size_t{0}; base < data_.size(); base += stride) {
        for (auto offset = size_t{0}; offset < valueOffset; ++offset) {
            renderData_[base + offset] = static_cast<float>(data_[base + offset] - renderOriginPosition_);
        }
        renderData_[base + valueOffset] = static_cast<float>(data_[base + valueOffset] - renderOriginValue_);
    }
}

} // namespace QAccelPlot
