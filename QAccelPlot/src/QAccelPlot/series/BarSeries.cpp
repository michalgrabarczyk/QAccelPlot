//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/BarSeries.hpp"
#include "QAccelPlot/legend/LegendEntry.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/materials/BarMaterial.hpp"
#include "QAccelPlot/materials/DataTextureMaterial.hpp"
#include "QAccelPlot/series/internal/CategoryList.hpp"
#include "QAccelPlot/series/internal/RectGeometry.hpp"
#include "QAccelPlot/series/internal/RectVertexCache.hpp"
#include "QAccelPlot/series/internal/SeriesSupport.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"
#include "QAccelPlot/transitions/TransitionRunner.hpp"

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

// The coordinate columns of a bar: the position ones on the position axis, then the value.
std::vector<DataPointStore::Dimension> barColumns(const Qt::Orientation orientation, const bool ranged)
{
    using Dimension = DataPointStore::Dimension;
    const auto position = orientation == Qt::Horizontal ? Dimension::Y : Dimension::X;
    const auto value = orientation == Qt::Horizontal ? Dimension::X : Dimension::Y;
    return ranged ? std::vector<Dimension>{position, position, value} : std::vector<Dimension>{position, value};
}

// BarSeries reports its own data ranges, which add the bar width and the baseline.
DataPointStore::Layout barLayout(const Qt::Orientation orientation, const bool ranged)
{
    return {barColumns(orientation, ranged), DataPointStore::RangeRule::EachCoordinate, 0};
}

// Appends bars \a begin to \a end of \a source to \a bars with \a restValue as their value: the
// state a bar grows from, or shrinks to.
void appendRestingBars(std::vector<double>& bars, const std::vector<double>& source, const int begin, const int end, const int stride, const double restValue)
{
    for (auto index = begin; index < end; ++index) {
        const auto bar = source.cbegin() + static_cast<std::ptrdiff_t>(index) * stride;
        bars.insert(bars.end(), bar, bar + stride);
        bars.back() = restValue;
    }
}

}

BarSeries::BarSeries(QQuickItem* parent)
    : ColormappedSeries(barLayout(Qt::Vertical, false), DataPointWording{"data bar", "bars", "values", "values"}, parent)
    , color_(ColorPalette::dark().seriesPrimary)
    , categories_(std::make_unique<Internal::CategoryList>("BarSeries"))
    , transition_(std::make_unique<TransitionRunner>(this))
    , vertexCache_(std::make_unique<Internal::RectVertexCache>())
{
    setFlag(ItemHasContents, true);
    supportHover();
    setAcceptedMouseButtons(Qt::NoButton);
    setLegendSymbol(LegendSymbol::Fill);
    connect(border_, &Border::widthChanged, this, &QQuickItem::update);
    connect(border_, &Border::colorChanged, this, &QQuickItem::update);
    connect(transition_.get(), &TransitionRunner::frameDue, this, &BarSeries::advanceTransition);
    connect(transition_.get(), &TransitionRunner::interrupted, this, &BarSeries::endTransition);
    connect(transition_.get(), &TransitionRunner::transitionDestroyed, this, &BarSeries::transitionChanged);
    connect(this, &BarSeries::colorChanged, this, &PlotSeries::legendEntryChanged);
    // A colormap replaces the per-category legend entries.
    connect(this, &ColormappedSeries::colormapChanged, this, &PlotSeries::legendEntryChanged);
    connect(categories_.get(), &Internal::CategoryList::changed, this, &BarSeries::onCategoriesChanged);
}

BarSeries::~BarSeries()
{
    // runningChanged handlers can still read the series until QObject destruction.
    transition_->cancel();
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
    dataPoints().remapColumns(barColumns(orientation_, valuesPerBar() == 3));
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
    if (nearlyEqual(barWidth_, clamped)) {
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
    if (nearlyEqual(barOffset_, offset)) {
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
    if (std::isnan(baselineValue) || nearlyEqual(baselineValue_, baselineValue)) {
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
    if (nearlyEqual(minimumWidth_, clamped)) {
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
    if (!drawnCategories().empty()) {
        vertexCache_->invalidate();
    }
    emit colorChanged();
    update();
}

QQmlListProperty<Category> BarSeries::categories()
{
    return categories_->listProperty(this);
}

void BarSeries::setCategories(const QList<Category*>& categories)
{
    categories_->setCategories(categories);
}

BarSeries::ColorSource BarSeries::colorSource() const
{
    return colorSource_;
}

void BarSeries::setColorSource(const ColorSource source)
{
    if (colorSource_ == source) {
        return;
    }
    colorSource_ = source;
    updateValueRange();
    emit colorSourceChanged();
    update();
}

Border* BarSeries::border() const
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

DataTransition* BarSeries::transition() const
{
    return transition_->transition();
}

void BarSeries::setTransition(DataTransition* transition)
{
    if (transition_->setTransition(transition)) {
        emit transitionChanged();
    }
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
        if (const auto category = categories_->table().dataPointCategory(map.value(QStringLiteral("category")), "BarSeries", i)) {
            categories[static_cast<size_t>(i)] = *category;
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

void BarSeries::setData(std::vector<double>&& data, std::vector<int>&& categories, const int barCount)
{
    if (!validateDataArguments(data.size(), categories.size(), barCount)) {
        return;
    }
    applyData(std::move(data), std::move(categories), barCount);
}

void BarSeries::setDataF(std::vector<float>&& data, std::vector<int>&& categories, const int barCount)
{
    if (!validateDataArguments(data.size(), categories.size(), barCount)) {
        return;
    }
    applyFloatData(std::move(data), std::move(categories), barCount);
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
    applyBars({}, false, {}, 0, false);
}

std::optional<LegendEntry> BarSeries::legendEntry() const
{
    auto entry = LegendEntry{};
    entry.symbol = legendSymbol();
    entry.color = color_;
    return entry;
}

QList<LegendEntry> BarSeries::legendEntries() const
{
    if (categories_->table().empty() || colormap()) {
        return PlotSeries::legendEntries();
    }
    return categories_->table().legendEntries(*legendEntry());
}

void BarSeries::setDataPointCategories(const QVariantList& categories)
{
    if (!categories.isEmpty() && categories.size() != count()) {
        qCWarning(lcQAccelPlot) << "BarSeries received" << categories.size() << "categories for" << count() << "bars";
        return;
    }
    auto values = std::vector<int>(static_cast<size_t>(categories.size()), -1);
    for (qsizetype i = 0; i < categories.size(); ++i) {
        values[static_cast<size_t>(i)] = categories_->table().dataPointCategory(categories[i], "BarSeries", i).value_or(-1);
    }
    dataPoints().setCategories(std::move(values));
    if (animating()) {
        transitionCategories_ = transitionCategories(transitionCategories_, drawnBarCount());
    }
    vertexCache_->invalidate();
    inspectionDataChanged();
    update();
}

QVariantMap BarSeries::barAt(const int index) const
{
    if (index < 0 || index >= count()) {
        return {};
    }
    auto bar = QVariantMap{{QStringLiteral("value"), value(index)}};
    if (valuesPerBar() == 3) {
        bar.insert(QStringLiteral("from"), component(index, 0));
        bar.insert(QStringLiteral("to"), component(index, 1));
    } else {
        bar.insert(QStringLiteral("position"), component(index, 0));
    }
    if (hasCategories()) {
        const auto category = dataPoints().categories()[static_cast<size_t>(index)];
        bar.insert(QStringLiteral("category"), category);
        const auto name = categories_->table().name(category);
        if (!name.isEmpty()) {
            bar.insert(QStringLiteral("categoryName"), name);
        }
    }
    return bar;
}

int BarSeries::barIndexAt(const QPointF& position) const
{
    if (count() <= 0 || !xAxis() || !yAxis() || plotRect().isEmpty()) {
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
    const auto drawnCount = std::min(count(), renderCapacity_);
    return spatialGrid_.queryTopmost(std::min(x1, x2), std::min(y1, y2), std::max(x1, x2), std::max(y1, y2),
        [this, &position, drawnCount](const int index) { return index < drawnCount && containsInPixels(index, position); });
}

bool BarSeries::contains(const QPointF& point) const
{
    return boundingRect().contains(point) && barIndexAt(point) >= 0;
}

bool BarSeries::recordInspectionSupported() const
{
    return true;
}

InspectionRecord BarSeries::inspectionRecord(const int index) const
{
    auto result = InspectionRecord{};
    if (index < 0 || index >= count()) {
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
    const auto drawnCount = drawnBarCount();
    const auto capacity = Internal::recordCapacity(window, valuesPerBar(), Internal::RectVertexCache::kBytesPerRect);
    updateRenderCapacity(capacity, drawnCount);
    if (drawnCount <= 0 || !xAxis() || !yAxis() || plotRect().isEmpty() || !window) {
        return releaseNode(oldNode);
    }
    if (!Internal::supportsCustomShaderRendering(window)) {
        static auto warned = false;
        if (!warned) {
            qCWarning(lcQAccelPlot) << "BarSeries custom rendering requires a hardware scene graph backend. Skipping updatePaintNode on the software backend.";
            warned = true;
        }
        delete oldNode;
        return nullptr;
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

    const auto renderCount = std::min(drawnCount, capacity);

    // Vertices hold only ids and category colors, so coordinate changes don't rebuild them.
    const auto rebuildVertices = !vertexCache_->valid() || vertexCache_->vertexCount() != renderCount * Internal::RectVertexCache::kVerticesPerRect;
    if (rebuildVertices) {
        buildVertexCache(renderCount);
    }
    if (rebuildVertices || nodeRecreated) {
        vertexCache_->copyTo(*node->geometry());
        node->markDirty(QSGNode::DirtyGeometry);
    }

    auto* material = static_cast<BarMaterial*>(node->material());
    if (dataPoints().renderRevision() != uploadedRevision_ || nodeRecreated) {
        material->uploadTexture(window, dataPoints().renderData().data(), renderCount * valuesPerBar());
        uploadedRevision_ = dataPoints().renderRevision();
    }

    updateMaterial(*material, renderCount);
    updateColormapUniforms(*material, window);
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

void BarSeries::applyDataPoints(DataPointInput&& input)
{
    const auto barCount = input.count;
    if (input.floatArray) {
        setDataFFromArray(input.floatArray, barCount);
    } else if (!input.precise) {
        applyFloatData(std::move(input.floats), {}, barCount);
    } else {
        applyData(std::move(input).toDoubles(), {}, barCount);
    }
}

int BarSeries::dataPointValueCount() const
{
    return 2;
}

void BarSeries::onAxisScaleChanged()
{
    update();
}

void BarSeries::onCategoriesChanged()
{
    if (!drawnCategories().empty()) {
        vertexCache_->invalidate();
    }
    if (hasCategories()) {
        inspectionDataChanged();
    }
    emit categoriesChanged();
    emit legendEntryChanged();
    update();
}

bool BarSeries::validateDataArguments(const std::size_t valueCount, const std::size_t categoryCount, const int barCount, const int barValueCount) const
{
    if (!validateDataSize(valueCount, barCount, false, barValueCount)) {
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
    applyBars(std::move(data), false, std::move(categories), barCount, transition_->enabled());
}

void BarSeries::applyRangedData(std::vector<double>&& data, std::vector<int>&& categories, const int barCount)
{
    applyBars(std::move(data), true, std::move(categories), barCount, transition_->enabled());
}

void BarSeries::setRanged(const bool ranged)
{
    if ((valuesPerBar() == 3) != ranged) {
        dataPoints().setLayout(barLayout(orientation_, ranged));
    }
}

void BarSeries::applyBars(std::vector<double>&& data, const bool ranged, std::vector<int>&& categories, const int barCount, const bool animate)
{
    // Copied before the data changes: a transition starts from the bars drawn now.
    auto drawn = animate ? drawnBars() : Bars{};
    const auto previousCount = count();
    // Ended first: bars of another layout drop the drawn ones along with the data.
    if (!animate) {
        endTransition();
    }
    setRanged(ranged);
    dataPoints().setPrecise(std::move(data), barCount);
    dataPoints().setCategories(std::move(categories));
    if (animate) {
        startTransition(std::move(drawn));
    }
    notifyDataPointsChanged(DataChange::Replaced, previousCount);
}

void BarSeries::applyFloatData(std::vector<float>&& data, std::vector<int>&& categories, const int barCount)
{
    if (transition_->enabled()) {
        // Transitions animate doubles.
        applyBars(std::vector<double>(data.cbegin(), data.cend()), false, std::move(categories), barCount, true);
        return;
    }
    applyFloatBars(std::move(data), std::move(categories), barCount);
}

void BarSeries::applyFloatBars(std::vector<float>&& data, std::vector<int>&& categories, const int barCount)
{
    const auto previousCount = count();
    endTransition();
    setRanged(false);
    dataPoints().setFloat(std::move(data), barCount);
    dataPoints().setCategories(std::move(categories));
    notifyDataPointsChanged(DataChange::Replaced, previousCount);
}

void BarSeries::setDataFFromArray(const float* data, const int barCount)
{
    if (transition_->enabled()) {
        // Transitions animate doubles.
        applyBars(std::vector<double>(data, data + static_cast<size_t>(barCount) * 2), false, {}, barCount, true);
        return;
    }
    const auto previousCount = count();
    endTransition();
    setRanged(false);
    // Reuses the allocation of the stored bars.
    dataPoints().assignFloat(data, barCount);
    notifyDataPointsChanged(DataChange::Replaced, previousCount);
}

void BarSeries::onDataPointsChanged([[maybe_unused]] const DataChange change, const int previousCount)
{
    // Vertex colors depend on the categories, but not on the coordinates.
    if (count() != previousCount || vertexCacheHasCategories_ || hasCategories()) {
        vertexCache_->invalidate();
    }
    spatialGridValid_ = false;
    if (hoveredIndex_ >= count()) {
        setHoveredIndex(-1);
    }
    updateValueRange();
}

std::optional<ColormappedSeries::ValueSpan> BarSeries::scanValueRange() const
{
    if (!colormap()) {
        return std::nullopt;
    }
    const auto ranged = valuesPerBar() == 3;
    const auto byPosition = colorSource_ == ColorSource::Position;
    auto span = Extent{};
    const auto include = [&span](const double coordinate) {
        if (std::isfinite(coordinate)) {
            span.include(coordinate);
        }
    };
    for (auto i = 0; i < count(); ++i) {
        if (std::isnan(positionSpan(i)[0]) || std::isnan(value(i))) {
            continue;
        }
        if (!byPosition) {
            include(value(i));
            continue;
        }
        include(component(i, 0));
        if (ranged) {
            include(component(i, 1));
        }
    }
    if (span.isEmpty()) {
        return std::nullopt;
    }
    return ValueSpan{span.min, span.max};
}

bool BarSeries::animating() const
{
    return dataPoints().hasDrawn();
}

std::vector<double> BarSeries::storedBars() const
{
    const auto bars = dataPoints().view();
    const auto size = static_cast<size_t>(bars.count) * static_cast<size_t>(bars.stride);
    if (bars.precise) {
        return std::vector<double>(bars.precise, bars.precise + size);
    }
    return std::vector<double>(bars.floats, bars.floats + size);
}

BarSeries::Bars BarSeries::drawnBars() const
{
    if (!animating()) {
        return Bars{storedBars(), dataPoints().categories(), count(), valuesPerBar()};
    }
    const auto drawn = dataPoints().drawnView();
    const auto size = static_cast<size_t>(drawn.count) * static_cast<size_t>(drawn.stride);
    return Bars{std::vector<double>(drawn.precise, drawn.precise + size), transitionCategories_, drawn.count, drawn.stride};
}

int BarSeries::drawnBarCount() const
{
    return dataPoints().renderCount();
}

const std::vector<int>& BarSeries::drawnCategories() const
{
    return animating() ? transitionCategories_ : dataPoints().categories();
}

std::vector<int> BarSeries::transitionCategories(const std::vector<int>& previous, const int transitionBarCount) const
{
    const auto& current = dataPoints().categories();
    if (current.empty() && previous.empty()) {
        return {};
    }
    auto categories = std::vector<int>(static_cast<size_t>(transitionBarCount), -1);
    std::copy(current.cbegin(), current.cend(), categories.begin());
    for (auto index = static_cast<size_t>(count()); index < std::min(previous.size(), categories.size()); ++index) {
        categories[index] = previous[index];
    }
    return categories;
}

void BarSeries::startTransition(Bars&& drawn)
{
    const auto stride = valuesPerBar();
    // Bars in the other layout have no counterpart, so all the new ones grow in.
    if (drawn.stride != stride) {
        drawn = Bars{};
    }
    const auto transitionBarCount = std::max(drawn.count, count());
    if (transitionBarCount == 0) {
        endTransition();
        return;
    }
    const auto stored = storedBars();
    auto target = stored;
    appendRestingBars(target, drawn.values, count(), transitionBarCount, stride, baselineValue_);
    appendRestingBars(drawn.values, stored, drawn.count, transitionBarCount, stride, baselineValue_);

    transitionCategories_ = transitionCategories(drawn.categories, transitionBarCount);
    vertexCache_->invalidate();
    transition_->start(drawn.values, transitionBarCount, std::move(target), transitionBarCount, stride);
    frameBuffer_ = dataPoints().setDrawn(std::move(drawn.values), transitionBarCount);
}

void BarSeries::advanceTransition()
{
    const auto transitionBarCount = drawnBarCount();
    auto shownCount = 0;
    transition_->advance(frameBuffer_, shownCount);
    if (!transition_->pending()) {
        endTransition();
        return;
    }
    // A transition may show fewer bars, as DrawTransition does; the others are not drawn.
    frameBuffer_.resize(static_cast<size_t>(transitionBarCount) * static_cast<size_t>(valuesPerBar()), kNaN);
    frameBuffer_ = dataPoints().setDrawn(std::move(frameBuffer_), transitionBarCount);
    update();
}

void BarSeries::endTransition()
{
    // Cancelled even without drawn bars: new bars of another layout drop them first.
    transition_->cancel();
    if (!animating()) {
        return;
    }
    dataPoints().clearDrawn();
    transitionCategories_.clear();
    frameBuffer_ = std::vector<double>{};
    vertexCache_->invalidate();
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

int BarSeries::valuesPerBar() const
{
    return dataPoints().columnCount();
}

double BarSeries::component(const int index, const int offset) const
{
    return dataPoints().value(index, offset);
}

double BarSeries::value(const int index) const
{
    return component(index, valuesPerBar() - 1);
}

std::array<double, 2> BarSeries::positionSpan(const int index) const
{
    if (valuesPerBar() == 3) {
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
    return dataPoints().hasCategories();
}

QColor BarSeries::barColor(const int index) const
{
    const auto& categories = drawnCategories();
    const auto categoryColor = categories.empty() ? QColor{} : categories_->table().color(categories[static_cast<size_t>(index)]);
    return categoryColor.isValid() ? categoryColor : color_;
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
    setHovered(index >= 0);
    emit hoveredIndexChanged();
    if (hoverColor_.isValid()) {
        update();
    }
}

QSGNode* BarSeries::releaseNode(QSGNode* oldNode) const
{
    if (count() > 0) {
        qCDebug(lcQAccelPlot) << "BarSeries has no axes, plot area, or window; not drawing";
    }
    delete oldNode;
    return nullptr;
}

void BarSeries::updateMaterial(BarMaterial& material, const int renderCount) const
{
    const auto horizontal = isHorizontal();
    const auto originX = dataPoints().renderOrigin(DataPointStore::Dimension::X);
    const auto originY = dataPoints().renderOrigin(DataPointStore::Dimension::Y);
    const auto minimumSize = static_cast<float>(minimumWidth_);
    material.color = color_;
    material.domainMin = QVector2D(static_cast<float>(xAxis()->viewportMin() - originX), static_cast<float>(yAxis()->viewportMin() - originY));
    material.domainMax = QVector2D(static_cast<float>(xAxis()->viewportMax() - originX), static_cast<float>(yAxis()->viewportMax() - originY));
    material.viewportSize = QVector2D(static_cast<float>(width()), static_cast<float>(height()));
    material.logScaleX = xAxis()->logScale() ? 1.0f : 0.0f;
    material.logScaleY = yAxis()->logScale() ? 1.0f : 0.0f;
    material.useVertexColor = drawnCategories().empty() ? 0.0f : 1.0f;
    material.rectCount = static_cast<float>(renderCount);
    material.minimumSize = horizontal ? QVector2D(0.0f, minimumSize) : QVector2D(minimumSize, 0.0f);
    material.borderWidth = static_cast<float>(border_->width());
    material.borderColor = border_->color();
    material.hoverColor = hoverColor_.isValid() ? hoverColor_ : QColor{Qt::transparent};
    material.hoveredIndex = hoverColor_.isValid() ? static_cast<float>(hoveredIndex_) : -1.0f;
    material.antialiasing = antialiasing() ? 1.0f : 0.0f;
    material.barWidth = static_cast<float>(barWidth_);
    material.barOffset = static_cast<float>(barOffset_);
    material.baseline = static_cast<float>(baselineValue_ - (horizontal ? originX : originY));
    material.horizontal = horizontal ? 1.0f : 0.0f;
    material.ranged = valuesPerBar() == 3 ? 1.0f : 0.0f;
}

void BarSeries::updateColormapUniforms(BarMaterial& material, QQuickWindow* window) const
{
    using Dimension = DataPointStore::Dimension;
    const auto mapped = colormapped();
    // bar.vert always declares the colormap sampler, so a neutral ramp keeps it valid.
    material.colorMap.upload(window, mapped ? colorStops() : GradientTexture::neutralStops());
    const auto byValue = colorSource_ == ColorSource::Value;
    material.colorSource = mapped ? (byValue ? 1.0f : 2.0f) : 0.0f;

    // Linear normalization shifts the range by the render origin in double precision. A log one
    // needs the absolute value, so the shader adds the origin back first.
    const auto logScale = mapped && colormap()->logScale();
    const auto origin = dataPoints().renderOrigin(byValue == isHorizontal() ? Dimension::X : Dimension::Y);
    const auto shift = logScale ? 0.0 : origin;
    material.valueMin = static_cast<float>(dataValueMin() - shift);
    material.valueMax = static_cast<float>(dataValueMax() - shift);
    material.valueOrigin = logScale ? static_cast<float>(origin) : 0.0f;
    material.valueLogScale = logScale ? 1.0f : 0.0f;
}

bool BarSeries::colormapped() const
{
    return colormap() && colorStops().size() >= 2;
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
    hitRects_.resize(static_cast<size_t>(count()) * 4);
    for (auto i = 0; i < count(); ++i) {
        auto rect = barRect(i);
        if (!std::isnan(rect[0])) {
            rect[static_cast<size_t>(valueStart)] = -kInf;
            rect[static_cast<size_t>(valueStart) + 2] = kInf;
        }
        std::copy(rect.cbegin(), rect.cend(), hitRects_.begin() + static_cast<std::ptrdiff_t>(i) * 4);
    }
    spatialGrid_.build(hitRects_.data(), count());
    spatialGridValid_ = true;
}

void BarSeries::buildVertexCache(const int barCount)
{
    auto colorAt = Internal::RectVertexCache::ColorFunction{};
    if (!drawnCategories().empty()) {
        colorAt = [this](const int index) { return barColor(index); };
    }
    vertexCache_->rebuild(barCount, colorAt);
    vertexCacheHasCategories_ = !drawnCategories().empty();
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
    const auto ranged = valuesPerBar() == 3;
    for (auto i = 0; i < count(); ++i) {
        const auto first = component(i, 0);
        const auto last = ranged ? component(i, 1) : first;
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
    if (!ranged) {
        // Widen by half a bar so the outer bars are fully in view, unless that leaves a log axis' domain.
        const auto widenedMin = positions.min + barOffset_ - 0.5 * barWidth_;
        positionMin = isPlaceable(widenedMin, logPosition) ? widenedMin : positions.min;
        positionMax = positions.max + barOffset_ + 0.5 * barWidth_;
    }
    const auto positionExtent = std::optional<DataExtent>{DataExtent{positionMin, positionMax}};
    const auto valueExtent = values.isEmpty() ? std::optional<DataExtent>{} : DataExtent{values.min, values.max};
    return isHorizontal() ? DataRanges{valueExtent, positionExtent} : DataRanges{positionExtent, valueExtent};
}

} // namespace QAccelPlot
