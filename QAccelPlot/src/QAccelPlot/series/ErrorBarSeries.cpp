//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/ErrorBarSeries.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/materials/ErrorBarMaterial.hpp"
#include "QAccelPlot/series/internal/RectGeometry.hpp"
#include "QAccelPlot/series/internal/SeriesSupport.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"
#include "QAccelPlot/transitions/TransitionRunner.hpp"

#include <QHoverEvent>
#include <QQuickWindow>
#include <QSGGeometryNode>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <utility>

namespace QAccelPlot {

namespace {

// Values per data point: x, y, low, high.
constexpr auto kStride = 4;
constexpr auto kLowColumn = 2;
constexpr auto kHighColumn = 3;
constexpr auto kVertexBytesPerErrorBar = ErrorBarMaterial::kVerticesPerErrorBar * 4;
constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();
constexpr auto kInf = std::numeric_limits<double>::infinity();

// The bounds lie on the Y axis of vertical error bars and on the X axis of horizontal ones.
std::vector<DataPointStore::Dimension> errorBarColumns(const Qt::Orientation orientation)
{
    using Dimension = DataPointStore::Dimension;
    const auto bounds = orientation == Qt::Horizontal ? Dimension::X : Dimension::Y;
    return {Dimension::X, Dimension::Y, bounds, bounds};
}

template <typename List> std::size_t shortestLength(std::initializer_list<const List*> lists)
{
    auto length = std::numeric_limits<std::size_t>::max();
    for (const auto* list : lists) {
        length = std::min(length, static_cast<std::size_t>(list->size()));
    }
    return length;
}

// Interleaves the first min(sizes) values into (x, y, low, high) quadruples.
template <typename List> std::vector<double> interleave(const List& xs, const List& ys, const List& lows, const List& highs)
{
    const auto count = shortestLength<List>({&xs, &ys, &lows, &highs});
    auto data = std::vector<double>(count * kStride);
    for (auto i = std::size_t{0}; i < count; ++i) {
        const auto dataPoint = std::array<double, kStride>{xs[i], ys[i], lows[i], highs[i]};
        std::copy(dataPoint.cbegin(), dataPoint.cend(), data.begin() + static_cast<std::ptrdiff_t>(i * kStride));
    }
    return data;
}

// Returns the bounds of \a value with \a minus below and \a plus above it, ignoring their signs.
std::array<double, 2> boundsAround(const double value, const double minus, const double plus)
{
    return {value - std::abs(minus), value + std::abs(plus)};
}

// Interleaves the first min(sizes) data points into (x, y, low, high) quadruples, with bounds around
// y for vertical and around x for horizontal error bars.
template <typename List> std::vector<double> interleaveErrors(const List& xs, const List& ys, const List& minuses, const List& pluses, const bool horizontal)
{
    const auto count = shortestLength<List>({&xs, &ys, &minuses, &pluses});
    auto data = std::vector<double>(count * kStride);
    for (auto i = std::size_t{0}; i < count; ++i) {
        const auto [low, high] = boundsAround(horizontal ? xs[i] : ys[i], minuses[i], pluses[i]);
        const auto dataPoint = std::array<double, kStride>{xs[i], ys[i], low, high};
        std::copy(dataPoint.cbegin(), dataPoint.cend(), data.begin() + static_cast<std::ptrdiff_t>(i * kStride));
    }
    return data;
}

// Error bars the vertex buffer holds room for: the next power of two, so appending one data point
// at a time reallocates it only when the count crosses one.
int reservedErrorBarCount(const int drawnCount)
{
    constexpr auto kMinReserved = 256;
    auto reserved = kMinReserved;
    while (reserved < drawnCount) {
        reserved *= 2;
    }
    return reserved;
}

// Pixel distance from (\a u, \a v) to the box spanning \a us and \a vs; 0 inside.
qreal distanceToBox(const qreal u, const qreal v, const std::pair<qreal, qreal>& us, const std::pair<qreal, qreal>& vs)
{
    const auto du = std::max({us.first - u, qreal{0.0}, u - us.second});
    const auto dv = std::max({vs.first - v, qreal{0.0}, v - vs.second});
    return std::hypot(du, dv);
}

QSGGeometryNode* createErrorBarNode()
{
    auto* geometry = new QSGGeometry(ErrorBarMaterial::attributeSet(), 0);
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);
    // The buffer only changes with the reserved count; let the backend keep it in static GPU memory.
    geometry->setVertexDataPattern(QSGGeometry::StaticPattern);
    auto* node = new QSGGeometryNode;
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(new ErrorBarMaterial);
    node->setFlag(QSGNode::OwnsMaterial);
    return node;
}

// Copies the data points of \a view as doubles, for float and double data alike.
std::vector<double> doubleData(const DataPointStore::View& view)
{
    auto data = std::vector<double>(static_cast<std::size_t>(view.count) * kStride);
    for (auto index = 0; index < view.count; ++index) {
        for (auto column = 0; column < kStride; ++column) {
            data[static_cast<std::size_t>(index) * kStride + static_cast<std::size_t>(column)] = view.value(index, column);
        }
    }
    return data;
}

} // namespace

ErrorBarSeries::ErrorBarSeries(QQuickItem* parent)
    : PlotSeries(DataPointStore::Layout{errorBarColumns(Qt::Vertical), DataPointStore::RangeRule::EachCoordinate, 0},
          DataPointWording{"data point", "data points", "values", "values"}, parent)
    , color_(ColorPalette::dark().seriesPrimary)
    , hoverColor_(ColorPalette::dark().hover)
    , transition_(std::make_unique<TransitionRunner>(this))
{
    setFlag(ItemHasContents, true);
    supportHover();
    setAcceptedMouseButtons(Qt::NoButton);
    connect(transition_.get(), &TransitionRunner::frameDue, this, &ErrorBarSeries::advanceTransition);
    connect(transition_.get(), &TransitionRunner::interrupted, this, &ErrorBarSeries::endTransition);
    connect(transition_.get(), &TransitionRunner::transitionDestroyed, this, &ErrorBarSeries::transitionChanged);
}

ErrorBarSeries::~ErrorBarSeries()
{
    // runningChanged handlers can still read the series until QObject destruction.
    transition_->cancel();
}

Qt::Orientation ErrorBarSeries::orientation() const
{
    return orientation_;
}

void ErrorBarSeries::setOrientation(const Qt::Orientation orientation)
{
    if (orientation_ == orientation) {
        return;
    }
    orientation_ = orientation;
    dataPoints().remapColumns(errorBarColumns(orientation_));
    onGeometryChanged();
    emit orientationChanged();
}

QColor ErrorBarSeries::color() const
{
    return color_;
}

void ErrorBarSeries::setColor(const QColor& color)
{
    if (color_ == color) {
        return;
    }
    color_ = color;
    emit colorChanged();
    update();
}

qreal ErrorBarSeries::lineWidth() const
{
    return lineWidth_;
}

void ErrorBarSeries::setLineWidth(const qreal width)
{
    const auto clamped = std::max(width, qreal{0.0});
    if (nearlyEqual(lineWidth_, clamped)) {
        return;
    }
    lineWidth_ = clamped;
    emit lineWidthChanged();
    update();
}

qreal ErrorBarSeries::capLength() const
{
    return capLength_;
}

void ErrorBarSeries::setCapLength(const qreal length)
{
    const auto clamped = std::max(length, qreal{0.0});
    if (nearlyEqual(capLength_, clamped)) {
        return;
    }
    capLength_ = clamped;
    emit capLengthChanged();
    update();
}

qreal ErrorBarSeries::offset() const
{
    return offset_;
}

void ErrorBarSeries::setOffset(const qreal offset)
{
    if (!std::isfinite(offset) || nearlyEqual(offset_, offset)) {
        return;
    }
    offset_ = offset;
    onGeometryChanged();
    emit offsetChanged();
}

QColor ErrorBarSeries::hoverColor() const
{
    return hoverColor_;
}

void ErrorBarSeries::setHoverColor(const QColor& color)
{
    if (hoverColor_ == color) {
        return;
    }
    hoverColor_ = color;
    emit hoverColorChanged();
    update();
}

qreal ErrorBarSeries::hoverRadius() const
{
    return hoverRadius_;
}

void ErrorBarSeries::setHoverRadius(const qreal radius)
{
    const auto clamped = std::max(radius, qreal{0.0});
    if (nearlyEqual(hoverRadius_, clamped)) {
        return;
    }
    hoverRadius_ = clamped;
    emit hoverRadiusChanged();
}

DataTransition* ErrorBarSeries::transition() const
{
    return transition_->transition();
}

void ErrorBarSeries::setTransition(DataTransition* transition)
{
    if (transition_->setTransition(transition)) {
        emit transitionChanged();
    }
}

int ErrorBarSeries::count() const
{
    return dataPoints().count();
}

int ErrorBarSeries::hoveredIndex() const
{
    return hoveredIndex_;
}

void ErrorBarSeries::appendData(const qreal x, const qreal y, const qreal low, const qreal high)
{
    endTransition();
    const auto dataPoint = std::array<double, kStride>{x, y, low, high};
    appendDataPoint(dataPoint.data());
}

void ErrorBarSeries::setData(const QList<qreal>& xs, const QList<qreal>& ys, const QList<qreal>& lows, const QList<qreal>& highs)
{
    applyInterleavedData(interleave(xs, ys, lows, highs));
}

void ErrorBarSeries::setData(const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& lows, const std::vector<double>& highs)
{
    applyInterleavedData(interleave(xs, ys, lows, highs));
}

void ErrorBarSeries::appendErrorData(const qreal x, const qreal y, const qreal error)
{
    appendErrorData(x, y, error, error);
}

void ErrorBarSeries::appendErrorData(const qreal x, const qreal y, const qreal minus, const qreal plus)
{
    const auto [low, high] = boundsAround(isHorizontal() ? x : y, minus, plus);
    appendData(x, y, low, high);
}

void ErrorBarSeries::setErrorData(const QList<qreal>& xs, const QList<qreal>& ys, const QList<qreal>& errors)
{
    applyInterleavedData(interleaveErrors(xs, ys, errors, errors, isHorizontal()));
}

void ErrorBarSeries::setErrorData(const QList<qreal>& xs, const QList<qreal>& ys, const QList<qreal>& minuses, const QList<qreal>& pluses)
{
    applyInterleavedData(interleaveErrors(xs, ys, minuses, pluses, isHorizontal()));
}

void ErrorBarSeries::setErrorData(const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& errors)
{
    applyInterleavedData(interleaveErrors(xs, ys, errors, errors, isHorizontal()));
}

void ErrorBarSeries::setErrorData(
    const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<double>& minuses, const std::vector<double>& pluses)
{
    applyInterleavedData(interleaveErrors(xs, ys, minuses, pluses, isHorizontal()));
}

void ErrorBarSeries::clearData()
{
    discardTransition();
    PlotSeries::clearData();
}

QVariantMap ErrorBarSeries::errorBarAt(const int index) const
{
    if (index < 0 || index >= count()) {
        return {};
    }
    return {{QStringLiteral("x"), component(index, 0)}, {QStringLiteral("y"), component(index, 1)}, {QStringLiteral("low"), component(index, kLowColumn)},
        {QStringLiteral("high"), component(index, kHighColumn)}};
}

int ErrorBarSeries::errorBarIndexAt(const QPointF& position) const
{
    if (count() <= 0 || !xAxis() || !yAxis() || width() <= 0.0 || height() <= 0.0) {
        return -1;
    }
    // Error bars within reach have their position within this many pixels of the cursor.
    const auto reach = hoverRadius_ + 0.5 * std::max(lineWidth_, capLength_) + 1.0;
    const auto x1 = xAxis()->pixelToCoord(position.x() - reach, width());
    const auto x2 = xAxis()->pixelToCoord(position.x() + reach, width());
    const auto y1 = yAxis()->pixelToCoord(position.y() - reach, height());
    const auto y2 = yAxis()->pixelToCoord(position.y() + reach, height());
    ensureSpatialGrid();

    const auto drawnCount = std::min(dataPoints().drawnView().count, renderCapacity_);
    auto nearest = -1;
    auto nearestDistance = hoverRadius_;
    // Accepts no candidate, so that the grid offers each one in the box to find the nearest.
    spatialGrid_.queryTopmost(std::min(x1, x2), std::min(y1, y2), std::max(x1, x2), std::max(y1, y2), [&](const int index) {
        const auto distance = index < drawnCount ? pixelDistance(index, position) : std::nullopt;
        if (distance && *distance <= hoverRadius_ && (nearest < 0 || *distance < nearestDistance || (*distance == nearestDistance && index > nearest))) {
            nearest = index;
            nearestDistance = *distance;
        }
        return false;
    });
    return nearest;
}

bool ErrorBarSeries::contains(const QPointF& point) const
{
    return boundingRect().contains(point) && errorBarIndexAt(point) >= 0;
}

bool ErrorBarSeries::recordInspectionSupported() const
{
    return true;
}

InspectionRecord ErrorBarSeries::inspectionRecord(const int index) const
{
    auto result = InspectionRecord{};
    if (index < 0 || index >= count()) {
        result.status = InspectionStatus::InvalidArgument;
        return result;
    }
    result.index = index;
    result.fields = errorBarAt(index);
    result.status = dataBar(dataPoints().view(), index) ? InspectionStatus::Ready : InspectionStatus::NoMatch;
    return result;
}

InspectionRecord ErrorBarSeries::inspectionRecordAt(const QPointF& position) const
{
    const auto index = errorBarIndexAt(position);
    return index < 0 ? InspectionRecord{} : inspectionRecord(index);
}

QSGNode* ErrorBarSeries::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* /*updatePaintNodeData*/)
{
    auto* window = this->window();
    updateRenderCapacity(Internal::recordCapacity(window, kStride, kVertexBytesPerErrorBar), count());
    if (count() <= 0 || !xAxis() || !yAxis() || resolvePlotRect().isEmpty() || !window) {
        delete oldNode;
        return nullptr;
    }
    if (!Internal::supportsCustomShaderRendering(window)) {
        static auto warned = false;
        if (!warned) {
            qCWarning(lcQAccelPlot)
                << "ErrorBarSeries custom rendering requires a hardware scene graph backend. Skipping updatePaintNode on the software backend.";
            warned = true;
        }
        delete oldNode;
        return nullptr;
    }

    auto* node = oldNode ? static_cast<QSGGeometryNode*>(oldNode) : createErrorBarNode();
    const auto drawnCount = std::min(dataPoints().renderCount(), renderCapacity_);
    updateVertices(*node, drawnCount);

    auto* material = static_cast<ErrorBarMaterial*>(node->material());
    // A freshly created node owns a new material, so it always needs the data texture.
    if (dataPoints().renderRevision() != uploadedRevision_ || !oldNode || !material->dataTexture) {
        material->uploadTexture(window, dataPoints().renderData().data(), drawnCount * kStride);
        uploadedRevision_ = dataPoints().renderRevision();
    }
    updateMaterial(*material, drawnCount);
    node->markDirty(QSGNode::DirtyMaterial);
    return node;
}

void ErrorBarSeries::hoverEnterEvent(QHoverEvent* event)
{
    setHoveredIndex(errorBarIndexAt(event->position()));
    QQuickItem::hoverEnterEvent(event);
}

void ErrorBarSeries::hoverMoveEvent(QHoverEvent* event)
{
    setHoveredIndex(errorBarIndexAt(event->position()));
    QQuickItem::hoverMoveEvent(event);
}

void ErrorBarSeries::hoverLeaveEvent(QHoverEvent* event)
{
    setHoveredIndex(-1);
    QQuickItem::hoverLeaveEvent(event);
}

void ErrorBarSeries::applyDataPoints(DataPointInput&& input)
{
    if (transition_->enabled()) {
        // Transitions animate doubles.
        const auto dataPointCount = input.count;
        applyDoubleData(std::move(input).toDoubles(), dataPointCount);
        return;
    }
    discardTransition();
    storeDataPoints(std::move(input));
}

void ErrorBarSeries::onAxisScaleChanged()
{
    // Log scale changes which data points are drawn.
    spatialGridValid_ = false;
    update();
}

void ErrorBarSeries::onDataPointsChanged(const DataChange /*change*/, const int previousCount)
{
    spatialGridValid_ = false;
    if (hoveredIndex_ >= count()) {
        setHoveredIndex(-1);
    }
    if (previousCount != count()) {
        emit countChanged();
    }
}

bool ErrorBarSeries::isHorizontal() const
{
    return orientation_ == Qt::Horizontal;
}

double ErrorBarSeries::component(const int index, const int column) const
{
    return dataPoints().value(index, column);
}

std::optional<ErrorBarSeries::DataBar> ErrorBarSeries::dataBar(const DataPointStore::View& dataPoints, const int index) const
{
    const auto horizontal = isHorizontal();
    const auto position = dataPoints.value(index, horizontal ? 1 : 0) + offset_;
    const auto value = dataPoints.value(index, horizontal ? 0 : 1);
    if (!isValidCoordinate(position, horizontal ? logScaleY() : logScaleX()) || std::isnan(value)) {
        return std::nullopt;
    }
    const auto valueLogScale = horizontal ? logScaleX() : logScaleY();
    const auto low = dataPoints.value(index, kLowColumn);
    const auto high = dataPoints.value(index, kHighColumn);
    return DataBar{position, std::isnan(low) ? value : low, std::isnan(high) ? value : high, isValidCoordinate(low, valueLogScale),
        isValidCoordinate(high, valueLogScale)};
}

std::optional<qreal> ErrorBarSeries::pixelDistance(const int index, const QPointF& position) const
{
    const auto bar = dataBar(dataPoints().drawnView(), index);
    if (!bar || lineWidth_ <= 0.0) {
        return std::nullopt;
    }
    // u runs across the bounds and v along them.
    const auto horizontal = isHorizontal();
    const auto& positionAxis = horizontal ? *yAxis() : *xAxis();
    const auto& valueAxis = horizontal ? *xAxis() : *yAxis();
    const auto alongLength = horizontal ? width() : height();
    const auto across = positionAxis.coordToPixel(bar->position, horizontal ? height() : width());
    const auto start = Internal::edgePixel(bar->start, valueAxis, alongLength);
    const auto end = Internal::edgePixel(bar->end, valueAxis, alongLength);
    const auto u = horizontal ? position.y() : position.x();
    const auto v = horizontal ? position.x() : position.y();

    const auto halfWidth = 0.5 * lineWidth_;
    auto distance = distanceToBox(u, v, {across - halfWidth, across + halfWidth}, {std::min(start, end), std::max(start, end)});
    if (capLength_ > 0.0) {
        const auto halfLength = 0.5 * std::max(capLength_, lineWidth_);
        for (const auto& [bound, capped] : {std::pair{start, bar->startCapped}, std::pair{end, bar->endCapped}}) {
            if (capped) {
                distance = std::min(distance, distanceToBox(u, v, {across - halfLength, across + halfLength}, {bound - halfWidth, bound + halfWidth}));
            }
        }
    }
    return distance;
}

void ErrorBarSeries::applyInterleavedData(std::vector<double>&& data)
{
    const auto dataPointCount = static_cast<int>(data.size() / kStride);
    setData(std::move(data), dataPointCount);
}

void ErrorBarSeries::applyDoubleData(std::vector<double>&& data, const int dataPointCount)
{
    const auto previousCount = count();
    // Copied before the data changes: a transition starts from the error bars drawn now.
    const auto drawnView = dataPoints().drawnView();
    const auto drawn = doubleData(drawnView);
    dataPoints().setPrecise(std::move(data), dataPointCount);
    startTransition(drawn, drawnView.count);
    notifyDataPointsChanged(DataChange::Replaced, previousCount);
}

void ErrorBarSeries::startTransition(const std::vector<double>& drawn, const int drawnCount)
{
    const auto errorBarCount = count();
    if (errorBarCount == 0) {
        discardTransition();
        return;
    }
    // New error bars appear at once, so only those drawn now start elsewhere.
    auto target = doubleData(dataPoints().view());
    auto start = target;
    const auto sharedCount = static_cast<std::ptrdiff_t>(std::min(drawnCount, errorBarCount)) * kStride;
    std::copy_n(drawn.cbegin(), sharedCount, start.begin());

    transition_->start(start, errorBarCount, std::move(target), errorBarCount, kStride);
    frameBuffer_ = dataPoints().setDrawn(std::move(start), errorBarCount);
    spatialGridValid_ = false;
}

void ErrorBarSeries::advanceTransition()
{
    auto shownCount = 0;
    transition_->advance(frameBuffer_, shownCount);
    if (!transition_->pending()) {
        endTransition();
        return;
    }
    // A transition may show fewer error bars, as DrawTransition does; the others are not drawn.
    frameBuffer_.resize(static_cast<std::size_t>(count()) * kStride, kNaN);
    frameBuffer_ = dataPoints().setDrawn(std::move(frameBuffer_), count());
    spatialGridValid_ = false;
    update();
}

void ErrorBarSeries::endTransition()
{
    if (!dataPoints().hasDrawn()) {
        return;
    }
    discardTransition();
    update();
}

void ErrorBarSeries::discardTransition()
{
    transition_->cancel();
    dataPoints().clearDrawn();
    frameBuffer_ = std::vector<double>{};
    spatialGridValid_ = false;
}

void ErrorBarSeries::setHoveredIndex(const int index)
{
    if (hoveredIndex_ == index) {
        return;
    }
    hoveredIndex_ = index;
    emit hoveredIndexChanged();
    update();
}

void ErrorBarSeries::onGeometryChanged()
{
    spatialGridValid_ = false;
    invalidateDataRanges();
    update();
}

void ErrorBarSeries::ensureSpatialGrid() const
{
    if (spatialGridValid_) {
        return;
    }
    // Only the position is indexed. Error bars often span much of the value axis, which would push
    // them past the grid's per-item cell limit into its linear scan; the pixel test checks the rest.
    const auto horizontal = isHorizontal();
    const auto drawn = dataPoints().drawnView();
    hitRects_.resize(static_cast<std::size_t>(drawn.count) * 4);
    for (auto i = 0; i < drawn.count; ++i) {
        auto rect = std::array<double, 4>{kNaN, kNaN, kNaN, kNaN};
        if (const auto bar = dataBar(drawn, i)) {
            rect = horizontal ? std::array<double, 4>{-kInf, bar->position, kInf, bar->position}
                              : std::array<double, 4>{bar->position, -kInf, bar->position, kInf};
        }
        std::copy(rect.cbegin(), rect.cend(), hitRects_.begin() + static_cast<std::ptrdiff_t>(i) * 4);
    }
    spatialGrid_.build(hitRects_.data(), drawn.count);
    spatialGridValid_ = true;
}

void ErrorBarSeries::updateVertices(QSGGeometryNode& node, const int drawnCount) const
{
    auto* geometry = node.geometry();
    const auto vertexCount = reservedErrorBarCount(drawnCount) * ErrorBarMaterial::kVerticesPerErrorBar;
    if (geometry->vertexCount() == vertexCount) {
        return;
    }
    geometry->allocate(vertexCount);
    // The shader derives everything from the vertex index; zeroing only avoids uploading uninitialized memory.
    std::memset(geometry->vertexData(), 0, static_cast<std::size_t>(vertexCount) * static_cast<std::size_t>(geometry->sizeOfVertex()));
    node.markDirty(QSGNode::DirtyGeometry);
}

void ErrorBarSeries::updateMaterial(ErrorBarMaterial& material, const int drawnCount) const
{
    const auto originX = dataPoints().renderOrigin(DataPointStore::Dimension::X);
    const auto originY = dataPoints().renderOrigin(DataPointStore::Dimension::Y);
    const auto rect = resolvePlotRect();
    material.color = color_;
    material.domainMin = QVector2D(static_cast<float>(xAxis()->viewportMin() - originX), static_cast<float>(yAxis()->viewportMin() - originY));
    material.domainMax = QVector2D(static_cast<float>(xAxis()->viewportMax() - originX), static_cast<float>(yAxis()->viewportMax() - originY));
    material.viewportSize = QVector2D(static_cast<float>(rect.width()), static_cast<float>(rect.height()));
    material.logScaleX = logScaleX() ? 1.0f : 0.0f;
    material.logScaleY = logScaleY() ? 1.0f : 0.0f;
    material.hoverColor = hoverColor_.isValid() ? hoverColor_ : color_;
    material.hoveredIndex = static_cast<float>(hoveredIndex_);
    material.lineWidth = static_cast<float>(lineWidth_);
    material.capLength = static_cast<float>(capLength_);
    material.offset = static_cast<float>(offset_);
    material.horizontal = isHorizontal() ? 1.0f : 0.0f;
    material.errorBarCount = static_cast<float>(drawnCount);
}

} // namespace QAccelPlot
