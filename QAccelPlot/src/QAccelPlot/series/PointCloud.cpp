//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/PointCloud.hpp"
#include "QAccelPlot/inspection/internal/InspectionTypes.hpp"
#include "QAccelPlot/legend/LegendEntry.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/GradientUtils.hpp"
#include "QAccelPlot/materials/PointCloudMaterial.hpp"
#include "QAccelPlot/materials/internal/DataTextureLayout.hpp"
#include "QAccelPlot/series/LineCurve.hpp"
#include "QAccelPlot/series/internal/HoverIndexBudget.hpp"
#include "QAccelPlot/series/internal/SeriesSupport.hpp"
#include "QAccelPlot/transitions/TransitionRunner.hpp"

#include <QHoverEvent>
#include <QQuickWindow>
#include <QSGGeometryNode>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <limits>
#include <utility>

namespace QAccelPlot {

namespace {

// Cost of indexing one point, measured on a desktop CPU and assumed until a build has been timed.
constexpr auto kAssumedIndexCostPerPoint = std::chrono::nanoseconds{15};

// The shape integers come from MarkerShape, shared with LineCurve markers and the
// point_shapes.glsl shader include, which selects a shape by the value minus one.

constexpr auto kPositionStride = 2;
constexpr auto kVerticesPerPoint = 6;
constexpr auto kNaN = std::numeric_limits<qreal>::quiet_NaN();

// Number of points of stride floats each that the GPU data texture holds.
int pointCapacity(QQuickWindow* window, const int stride)
{
    // QSGGeometry sizes its vertex buffer in int bytes.
    const auto maxGeometryPoints = std::numeric_limits<int>::max() / (kVerticesPerPoint * DataTextureMaterial::vertexIndexAttributeSet().stride);
    return static_cast<int>(std::min<qint64>(Internal::dataTextureItemCapacity(Internal::maxTextureSize(window), stride), maxGeometryPoints));
}

QSGGeometryNode* createPointCloudNode()
{
    auto* geometry = new QSGGeometry(DataTextureMaterial::vertexIndexAttributeSet(), 0);
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);
    // The buffer only changes with the point count; let the backend keep it in static GPU memory.
    geometry->setVertexDataPattern(QSGGeometry::StaticPattern);

    auto* node = new QSGGeometryNode;
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(new PointCloudMaterial);
    node->setFlag(QSGNode::OwnsMaterial);
    return node;
}

// Two unindexed triangles per point. A 4-vertex indexed quad was measured ~11% slower on desktop
// OpenGL (44 vs 49 FPS panning 1M points), despite fewer vertex shader invocations.
// The shader derives everything from gl_VertexIndex and ignores the vertex content; zeroing only
// avoids uploading uninitialized memory.
void clearPointVertices(QSGGeometry* geometry)
{
    const auto byteCount = static_cast<std::size_t>(geometry->vertexCount()) * static_cast<std::size_t>(geometry->sizeOfVertex());
    std::memset(geometry->vertexData(), 0, byteCount);
}

} // namespace

PointCloud::PointCloud(QQuickItem* parent)
    : ColormappedSeries(DataPointStore::Layout{{DataPointStore::Dimension::X, DataPointStore::Dimension::Y}, DataPointStore::RangeRule::WholeDataPoint, 1},
          DataPointWording{"data point", "points", "coordinates", "coordinates"}, parent)
    , transition_(std::make_unique<TransitionRunner>(this))
    , spatialIndexBudget_(std::make_unique<Internal::HoverIndexBudget>(kAssumedIndexCostPerPoint))
{
    setFlag(ItemHasContents, true);
    setAntialiasing(true);
    supportHover();
    setAcceptedMouseButtons(Qt::NoButton);
    setLegendSymbol(LegendSymbol::Marker);

    for (const auto signal : {&SeriesMarker::shapeChanged, &SeriesMarker::sizeChanged, &SeriesMarker::filledChanged, &SeriesMarker::strokeWidthChanged}) {
        connect(marker_, signal, this, &QQuickItem::update);
        connect(marker_, signal, this, &PlotSeries::legendEntryChanged);
    }
    for (const auto signal : {&PointCloud::colorChanged, &PointCloud::antialiasingFeatherChanged}) {
        connect(this, signal, this, &PlotSeries::legendEntryChanged);
    }
    connect(this, &QQuickItem::antialiasingChanged, this, &PlotSeries::legendEntryChanged);
    connect(transition_.get(), &TransitionRunner::frameDue, this, &PointCloud::advanceTransition);
    connect(transition_.get(), &TransitionRunner::interrupted, this, &PointCloud::endTransition);
    connect(transition_.get(), &TransitionRunner::transitionDestroyed, this, &PointCloud::transitionChanged);
}

PointCloud::~PointCloud()
{
    // runningChanged handlers can still read the series until QObject destruction.
    transition_->cancel();
}

QColor PointCloud::color() const
{
    return color_;
}

void PointCloud::setColor(const QColor& color)
{
    if (color_ == color) {
        return;
    }
    color_ = color;
    emit colorChanged();
    update();
}

SeriesMarker* PointCloud::marker() const
{
    return marker_;
}

qreal PointCloud::antialiasingFeather() const
{
    return antialiasingFeather_;
}

void PointCloud::setAntialiasingFeather(const qreal feather)
{
    const auto clamped = Internal::clampAntialiasingFeather(feather);
    if (nearlyEqual(antialiasingFeather_, clamped)) {
        return;
    }
    antialiasingFeather_ = clamped;
    emit antialiasingFeatherChanged();
    update();
}

qreal PointCloud::hoverRadius() const
{
    return hoverRadius_;
}

void PointCloud::setHoverRadius(const qreal radius)
{
    const auto clamped = std::max(radius, qreal{0.0});
    if (nearlyEqual(hoverRadius_, clamped)) {
        return;
    }
    hoverRadius_ = clamped;
    emit hoverRadiusChanged();
    // Marks the item dirty so the window re-delivers hover at the cursor and updates hoveredIndex.
    update();
}

DataTransition* PointCloud::transition() const
{
    return transition_->transition();
}

void PointCloud::setTransition(DataTransition* transition)
{
    if (transition_->setTransition(transition)) {
        emit transitionChanged();
    }
}

int PointCloud::hoveredIndex() const
{
    return hoveredIndex_;
}

void PointCloud::setData(const QList<QPointF>& points)
{
    const auto pointCount = static_cast<int>(points.size());
    auto xy = std::vector<double>(static_cast<std::size_t>(pointCount) * kPositionStride);
    for (auto index = 0; index < pointCount; ++index) {
        xy[static_cast<std::size_t>(index) * 2] = points[index].x();
        xy[static_cast<std::size_t>(index) * 2 + 1] = points[index].y();
    }
    // QPointF is double precision, so keep it: applyDoubleData() origin-shifts the upload.
    applyDoubleData(std::move(xy), {}, pointCount);
}

void PointCloud::setValues(const QList<qreal>& values)
{
    if (values.isEmpty() && !hasValues()) {
        return;
    }
    if (!values.isEmpty() && !validateValues(static_cast<std::size_t>(values.size()), count())) {
        return;
    }
    auto pointValues = std::vector<float>(static_cast<std::size_t>(values.size()));
    std::transform(values.cbegin(), values.cend(), pointValues.begin(), [](const qreal value) { return static_cast<float>(value); });
    if (transition_->enabled()) {
        animateValues(std::move(pointValues));
        return;
    }
    const auto previousCount = count();
    endTransition();
    dataPoints().setAttributes(std::move(pointValues));
    finishValueChange(previousCount);
}

QPointF PointCloud::pointAt(const int index) const
{
    if (index < 0 || index >= count()) {
        return {kNaN, kNaN};
    }
    return {dataPoints().value(index, 0), dataPoints().value(index, 1)};
}

qreal PointCloud::valueAt(const int index) const
{
    if (!hasValues() || index < 0 || index >= count()) {
        return kNaN;
    }
    return static_cast<qreal>(dataPoints().attribute(index));
}

void PointCloud::setData(std::vector<double>&& xyInterleaved, std::vector<float>&& values, const int pointCount)
{
    if (!validateDataSize(xyInterleaved.size(), pointCount, false) || !validateValues(values.size(), pointCount)) {
        return;
    }
    applyDoubleData(std::move(xyInterleaved), std::move(values), pointCount);
}

void PointCloud::setDataF(std::vector<float>&& xyInterleaved, std::vector<float>&& values, const int pointCount)
{
    if (!validateDataSize(xyInterleaved.size(), pointCount, true) || !validateValues(values.size(), pointCount)) {
        return;
    }
    applyData(std::move(xyInterleaved), std::move(values), pointCount);
}

void PointCloud::postData(std::vector<double>&& xyInterleaved, std::vector<float>&& values, const int pointCount)
{
    QMetaObject::invokeMethod(
        this,
        [this, xy = std::move(xyInterleaved), pointValues = std::move(values), pointCount]() mutable {
            setData(std::move(xy), std::move(pointValues), pointCount);
        },
        Qt::QueuedConnection);
}

void PointCloud::postData(std::vector<float>&& xyInterleaved, std::vector<float>&& values, const int pointCount)
{
    QMetaObject::invokeMethod(
        this,
        [this, xy = std::move(xyInterleaved), pointValues = std::move(values), pointCount]() mutable {
            setDataF(std::move(xy), std::move(pointValues), pointCount);
        },
        Qt::QueuedConnection);
}

void PointCloud::clearData()
{
    applyFloatData({}, {}, 0);
}

std::optional<LegendEntry> PointCloud::legendEntry() const
{
    auto entry = LegendEntry{};
    entry.symbol = legendSymbol();
    entry.color = color_;
    entry.markerShape = marker_->shape();
    entry.markerSize = marker_->size();
    entry.markerFilled = marker_->filled();
    entry.markerStrokeWidth = marker_->strokeWidth();
    entry.antialiasing = antialiasing();
    entry.antialiasingFeather = antialiasingFeather_;
    return entry;
}

int PointCloud::pointIndexAt(const QPointF& position) const
{
    if (count() <= 0 || !xAxis() || !yAxis() || width() <= 0.0 || height() <= 0.0 || hoverRadius_ <= 0.0) {
        return -1;
    }

    const auto logX = xAxis()->logScale();
    const auto logY = yAxis()->logScale();
    auto cursorX = 0.0;
    auto cursorY = 0.0;
    auto viewportMinX = 0.0;
    auto viewportMaxX = 0.0;
    auto viewportMinY = 0.0;
    auto viewportMaxY = 0.0;
    if (!PointSpatialIndex::mapCoordinate(xAxis()->pixelToCoord(position.x(), width()), logX, cursorX)
        || !PointSpatialIndex::mapCoordinate(yAxis()->pixelToCoord(position.y(), height()), logY, cursorY)
        || !PointSpatialIndex::mapCoordinate(xAxis()->viewportMin(), logX, viewportMinX)
        || !PointSpatialIndex::mapCoordinate(xAxis()->viewportMax(), logX, viewportMaxX)
        || !PointSpatialIndex::mapCoordinate(yAxis()->viewportMin(), logY, viewportMinY)
        || !PointSpatialIndex::mapCoordinate(yAxis()->viewportMax(), logY, viewportMaxY)) {
        return -1;
    }

    // Convert the pixel radius to mapped data units separately per axis. The origins cancel in
    // these differences, but the cursor itself must move into the index's origin-relative space.
    const auto radiusX = std::abs(viewportMaxX - viewportMinX) / width() * hoverRadius_;
    const auto radiusY = std::abs(viewportMaxY - viewportMinY) / height() * hoverRadius_;
    // The index holds the render data, which is relative to the render origin.
    const auto originX = dataPoints().renderOrigin(DataPointStore::Dimension::X);
    const auto originY = dataPoints().renderOrigin(DataPointStore::Dimension::Y);
    const auto query = HoverQuery{cursorX - originX, cursorY - originY, radiusX, radiusY, {logX, logY}};
    if (indexedRevision_ != dataPoints().renderRevision()) {
        spatialIndexValid_ = false;
        lastHoverQuery_.reset();
        indexedRevision_ = dataPoints().renderRevision();
    }
    if (!(lastHoverQuery_ == query)) {
        lastHoverIndex_ = nearestPoint(query);
        lastHoverQuery_ = query;
    }
    return lastHoverIndex_;
}

bool PointCloud::contains(const QPointF& point) const
{
    return boundingRect().contains(point) && pointIndexAt(point) >= 0;
}

Internal::InspectionSource PointCloud::inspectionSource() const
{
    const auto points = dataPoints().view();
    auto source = Internal::InspectionSource{};
    source.count = points.count;
    source.logX = logScaleX();
    source.logY = logScaleY();
    source.doubles = points.precise;
    source.floats = points.floats;
    source.floatStride = points.stride;
    const auto values = dataPoints().attributes();
    if (values.data) {
        source.values = values.data;
        source.valueStride = values.stride;
    }
    return source;
}

QSGNode* PointCloud::updatePaintNode(QSGNode* oldNode, [[maybe_unused]] UpdatePaintNodeData* updatePaintNodeData)
{
    auto* window = this->window();
    if (!Internal::supportsCustomShaderRendering(window)) {
        static auto warned = false;
        if (!warned) {
            qCWarning(lcQAccelPlot) << "PointCloud custom rendering requires a hardware scene graph backend. Skipping updatePaintNode on the software backend.";
            warned = true;
        }
        delete oldNode;
        return nullptr;
    }
    const auto& points = dataPoints();
    const auto stride = points.renderStride();
    const auto capacity = pointCapacity(window, stride);
    if (capacity != renderCapacity_) {
        invalidateSpatialIndex();
    }
    updateRenderCapacity(capacity, count());
    const auto plotArea = resolvePlotRect();
    if (count() <= 0 || !xAxis() || !yAxis() || plotArea.width() <= 0.0 || plotArea.height() <= 0.0) {
        delete oldNode;
        return nullptr;
    }

    const auto renderCount = std::min(points.renderCount(), renderCapacity_);
    auto* node = oldNode ? static_cast<QSGGeometryNode*>(oldNode) : createPointCloudNode();
    auto* material = static_cast<PointCloudMaterial*>(node->material());

    const auto vertexCount = renderCount * kVerticesPerPoint;
    if (node->geometry()->vertexCount() != vertexCount) {
        node->geometry()->allocate(vertexCount);
        clearPointVertices(node->geometry());
        node->markDirty(QSGNode::DirtyGeometry);
    }

    // A freshly created node owns a new material, so it always needs the data texture.
    if (points.renderRevision() != uploadedRevision_ || !oldNode || !material->dataTexture) {
        material->uploadTexture(window, points.renderData().data(), renderCount * stride);
        uploadedRevision_ = points.renderRevision();
    }

    const auto* const valueColormap = colormap();
    const auto useValueColor = valueColormap && hasValues() && colorStops().size() >= 2;
    // The fragment shader always samples the colormap, so a neutral texture keeps the sampler valid.
    material->colorMap.upload(window, useValueColor ? colorStops() : GradientTexture::neutralStops());

    material->color = color_;
    // The render data is origin-relative, so the domain is shifted by the same origin.
    const auto originX = points.renderOrigin(DataPointStore::Dimension::X);
    const auto originY = points.renderOrigin(DataPointStore::Dimension::Y);
    material->domainMin = QVector2D(static_cast<float>(xAxis()->viewportMin() - originX), static_cast<float>(yAxis()->viewportMin() - originY));
    material->domainMax = QVector2D(static_cast<float>(xAxis()->viewportMax() - originX), static_cast<float>(yAxis()->viewportMax() - originY));
    material->viewportSize = QVector2D(static_cast<float>(plotArea.width()), static_cast<float>(plotArea.height()));
    material->logScaleX = xAxis()->logScale() ? 1.0f : 0.0f;
    material->logScaleY = yAxis()->logScale() ? 1.0f : 0.0f;
    material->useVertexColor = useValueColor ? 1.0f : 0.0f;
    material->valueLogScale = useValueColor && valueColormap->norm() == Colormap::Normalization::Log ? 1.0f : 0.0f;
    material->markerSize = static_cast<float>(marker_->size());
    material->markerFilled = marker_->filled() ? 1.0f : 0.0f;
    material->markerStrokeWidth = static_cast<float>(marker_->strokeWidth());
    material->antialiasingEnabled = antialiasing() ? 1.0f : 0.0f;
    material->antialiasingFeather = static_cast<float>(antialiasingFeather_);
    material->valueMin = static_cast<float>(dataValueMin());
    material->valueMax = static_cast<float>(dataValueMax());
    material->stride = static_cast<float>(stride);
    material->shapeType = static_cast<int>(marker_->shape()) - 1;
    node->markDirty(QSGNode::DirtyMaterial);

    return node;
}

void PointCloud::hoverEnterEvent(QHoverEvent* event)
{
    setHoveredIndex(pointIndexAt(event->position()));
    QQuickItem::hoverEnterEvent(event);
}

void PointCloud::hoverMoveEvent(QHoverEvent* event)
{
    setHoveredIndex(pointIndexAt(event->position()));
    QQuickItem::hoverMoveEvent(event);
}

void PointCloud::hoverLeaveEvent(QHoverEvent* event)
{
    setHoveredIndex(-1);
    QQuickItem::hoverLeaveEvent(event);
}

bool PointCloud::hasValues() const
{
    return dataPoints().hasAttributes();
}

bool PointCloud::validateValues(const std::size_t valueCount, const int pointCount) const
{
    if (valueCount != 0 && valueCount != static_cast<std::size_t>(pointCount)) {
        qCWarning(lcQAccelPlot) << "PointCloud received" << valueCount << "values for" << pointCount << "points";
        return false;
    }
    return true;
}

void PointCloud::applyData(std::vector<float>&& xyInterleaved, std::vector<float>&& values, const int pointCount)
{
    if (transition_->enabled()) {
        // Transitions animate doubles.
        applyDoubleData(std::vector<double>(xyInterleaved.cbegin(), xyInterleaved.cend()), std::move(values), pointCount);
        return;
    }
    applyFloatData(std::move(xyInterleaved), std::move(values), pointCount);
}

void PointCloud::applyFloatData(std::vector<float>&& xyInterleaved, std::vector<float>&& values, const int pointCount)
{
    const auto previousCount = count();
    discardTransition();
    dataPoints().setFloat(std::move(xyInterleaved), pointCount, std::move(values));
    notifyDataPointsChanged(DataChange::Replaced, previousCount);
}

void PointCloud::applyDoubleData(std::vector<double>&& xyInterleaved, std::vector<float>&& values, const int pointCount)
{
    const auto previousCount = count();
    const auto animate = transition_->enabled();
    // Copied before the data changes: a transition starts from the points drawn now.
    auto drawn = animate ? drawnPoints() : Points{};
    dataPoints().setPrecise(std::move(xyInterleaved), pointCount, std::move(values));
    if (animate) {
        startTransition(std::move(drawn));
    } else {
        discardTransition();
    }
    notifyDataPointsChanged(DataChange::Replaced, previousCount);
}

void PointCloud::finishValueChange(const int previousCount)
{
    onDataPointsChanged(DataChange::Replaced, previousCount);
    inspectionDataChanged();
    update();
}

void PointCloud::applyDataPoints(DataPointInput&& input)
{
    const auto pointCount = input.count;
    if (input.precise) {
        applyDoubleData(std::move(input).toDoubles(), {}, pointCount);
        return;
    }
    if (!input.floatArray) {
        applyData(std::move(input.floats), {}, pointCount);
        return;
    }
    if (transition_->enabled()) {
        // Transitions animate doubles.
        applyDoubleData(std::move(input).toDoubles(), {}, pointCount);
        return;
    }
    const auto previousCount = count();
    discardTransition();
    // Reuses the allocation of the stored points.
    dataPoints().assignFloat(input.floatArray, pointCount);
    notifyDataPointsChanged(DataChange::Replaced, previousCount);
}

void PointCloud::onAxisScaleChanged()
{
    invalidateSpatialIndex();
    update();
}

void PointCloud::onDataPointsChanged([[maybe_unused]] const DataChange change, [[maybe_unused]] const int previousCount)
{
    invalidateSpatialIndex();
    updateValueRange();
    if (hoveredIndex_ >= count()) {
        setHoveredIndex(-1);
    }
}

std::optional<ColormappedSeries::ValueSpan> PointCloud::scanValueRange() const
{
    const auto values = dataPoints().attributes();
    if (!values.data) {
        return std::nullopt;
    }
    auto minimum = std::numeric_limits<qreal>::max();
    auto maximum = std::numeric_limits<qreal>::lowest();
    for (auto index = std::size_t{0}; index < static_cast<std::size_t>(count()); ++index) {
        const auto value = values.data[index * static_cast<std::size_t>(values.stride)];
        if (!std::isfinite(value)) {
            continue;
        }
        minimum = std::min(minimum, static_cast<qreal>(value));
        maximum = std::max(maximum, static_cast<qreal>(value));
    }
    if (minimum > maximum) {
        return std::nullopt;
    }
    return ValueSpan{minimum, maximum};
}

void PointCloud::setHoveredIndex(const int index)
{
    if (hoveredIndex_ == index) {
        return;
    }
    hoveredIndex_ = index;
    setHovered(index >= 0);
    emit hoveredIndexChanged();
}

int PointCloud::hoverPointCount() const
{
    return std::min(dataPoints().renderCount(), renderCapacity_);
}

void PointCloud::invalidateSpatialIndex()
{
    spatialIndexValid_ = false;
    spatialIndexBudget_->reset();
    lastHoverQuery_.reset();
}

bool PointCloud::spatialIndexReady(const PointSpatialIndex::Mapping mapping) const
{
    if (spatialIndexValid_ && spatialIndex_.mapping() == mapping) {
        return true;
    }
    const auto count = hoverPointCount();
    if (!spatialIndexBudget_->buildDue(count)) {
        return false;
    }
    const auto& points = dataPoints();
    spatialIndexBudget_->timeBuild(count, [&] { spatialIndex_.build(points.renderData().data(), count, points.renderStride(), mapping); });
    spatialIndexValid_ = true;
    return true;
}

bool PointCloud::HoverQuery::operator==(const HoverQuery& other) const
{
    return x == other.x && y == other.y && radiusX == other.radiusX && radiusY == other.radiusY && mapping == other.mapping;
}

int PointCloud::nearestPoint(const HoverQuery& query) const
{
    if (spatialIndexReady(query.mapping)) {
        return spatialIndex_.nearest(query.x, query.y, query.radiusX, query.radiusY);
    }
    const auto& points = dataPoints();
    return spatialIndexBudget_->timeScan([&] {
        return PointSpatialIndex::nearestByScan(
            points.renderData().data(), hoverPointCount(), points.renderStride(), query.mapping, query.x, query.y, query.radiusX, query.radiusY);
    });
}

std::vector<double> PointCloud::interleavedData() const
{
    const auto& points = dataPoints();
    const auto view = points.view();
    const auto stride = static_cast<std::size_t>(points.renderStride());
    auto data = std::vector<double>(static_cast<std::size_t>(view.count) * stride);
    for (auto index = 0; index < view.count; ++index) {
        auto* point = data.data() + static_cast<std::size_t>(index) * stride;
        point[0] = view.value(index, 0);
        point[1] = view.value(index, 1);
        if (stride > kPositionStride) {
            point[2] = points.attribute(index);
        }
    }
    return data;
}

PointCloud::Points PointCloud::drawnPoints() const
{
    const auto& points = dataPoints();
    if (!points.hasDrawn()) {
        return {interleavedData(), count(), points.renderStride()};
    }
    const auto drawn = points.drawnView();
    const auto size = static_cast<std::size_t>(drawn.count) * static_cast<std::size_t>(drawn.stride);
    return {std::vector<double>(drawn.precise, drawn.precise + size), drawn.count, drawn.stride};
}

void PointCloud::animateValues(std::vector<float>&& values)
{
    const auto previousCount = count();
    auto drawn = drawnPoints();
    dataPoints().promoteToPrecise();
    dataPoints().setAttributes(std::move(values));
    startTransition(std::move(drawn));
    finishValueChange(previousCount);
}

void PointCloud::startTransition(Points&& drawn)
{
    const auto pointCount = count();
    if (pointCount == 0) {
        discardTransition();
        return;
    }
    const auto pointStride = static_cast<std::size_t>(dataPoints().renderStride());
    const auto drawnStride = static_cast<std::size_t>(drawn.stride);
    // A value animates only when the drawn point and the new one both have one.
    const auto sharedValues = std::min(pointStride, drawnStride);
    auto target = interleavedData();
    auto start = target;
    for (auto index = std::size_t{0}; index < static_cast<std::size_t>(std::min(drawn.count, pointCount)); ++index) {
        const auto drawnPoint = drawn.values.cbegin() + static_cast<std::ptrdiff_t>(index * drawnStride);
        std::copy_n(drawnPoint, sharedValues, start.begin() + static_cast<std::ptrdiff_t>(index * pointStride));
    }

    transition_->start(start, pointCount, std::move(target), pointCount, static_cast<int>(pointStride));
    frameBuffer_ = dataPoints().setDrawn(std::move(start), pointCount);
    invalidateSpatialIndex();
}

void PointCloud::advanceTransition()
{
    auto shownCount = 0;
    transition_->advance(frameBuffer_, shownCount);
    if (!transition_->pending()) {
        endTransition();
        return;
    }
    // A transition may show fewer points, as DrawTransition does; the others are not drawn.
    const auto pointCount = count();
    frameBuffer_.resize(static_cast<std::size_t>(pointCount) * static_cast<std::size_t>(dataPoints().renderStride()), kNaN);
    frameBuffer_ = dataPoints().setDrawn(std::move(frameBuffer_), pointCount);
    invalidateSpatialIndex();
    update();
}

void PointCloud::endTransition()
{
    if (!dataPoints().hasDrawn()) {
        return;
    }
    discardTransition();
    update();
}

void PointCloud::discardTransition()
{
    transition_->cancel();
    dataPoints().clearDrawn();
    frameBuffer_ = std::vector<double>{};
    invalidateSpatialIndex();
}

} // namespace QAccelPlot
