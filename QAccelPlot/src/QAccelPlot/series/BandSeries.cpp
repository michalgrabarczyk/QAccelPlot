//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/BandSeries.hpp"
#include "QAccelPlot/legend/LegendEntry.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/materials/BandMaterial.hpp"
#include "QAccelPlot/materials/internal/DataTextureLayout.hpp"
#include "QAccelPlot/series/internal/SeriesSupport.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"

#include <QHoverEvent>
#include <QQuickWindow>
#include <QSGGeometryNode>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>

namespace QAccelPlot {

namespace {

// Values per data point: x, low, high.
constexpr auto kStride = 3;
constexpr auto kLowComponent = 1;
constexpr auto kHighComponent = 2;

QColor defaultBandColor()
{
    auto color = ColorPalette::dark().seriesPrimary;
    color.setAlpha(64);
    return color;
}

// Maps a coordinate to the space the renderer interpolates in: log10 on a log-scale axis.
qreal toAxisSpace(const qreal value, const bool logScale)
{
    return logScale ? std::log10(value) : value;
}

qreal fromAxisSpace(const qreal value, const bool logScale)
{
    return logScale ? std::pow(10.0, value) : value;
}

template <typename T> bool isAscending(const T* data, const int dataPointCount)
{
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(dataPointCount); ++i) {
        const auto x = data[i * kStride];
        if (std::isnan(x) || (i > 0 && x < data[(i - 1) * kStride])) {
            return false;
        }
    }
    return true;
}

// Interleaves the first min(xs, lows, highs) values into (x, low, high) triples.
template <typename List> std::vector<double> interleave(const List& xs, const List& lows, const List& highs)
{
    const auto count = std::min({static_cast<std::size_t>(xs.size()), static_cast<std::size_t>(lows.size()), static_cast<std::size_t>(highs.size())});
    auto data = std::vector<double>(count * kStride);
    for (auto i = std::size_t{0}; i < count; ++i) {
        data[i * kStride] = xs[i];
        data[i * kStride + kLowComponent] = lows[i];
        data[i * kStride + kHighComponent] = highs[i];
    }
    return data;
}

// Number of data points the GPU draws: float vertex ids are exact up to 2^24, and the data texture
// holds a limited number of (x, low, high) triples.
int dataPointCapacity(QQuickWindow* window)
{
    constexpr auto kMaxExactVertexId = qint64{1} << 24;
    return static_cast<int>(std::min(Internal::dataTextureItemCapacity(Internal::maxTextureSize(window), kStride), kMaxExactVertexId));
}

// Data points the vertex buffers hold room for: the next power of two, so appending one data point at a
// time rebuilds the vertices only when the count crosses one. Vertex ids stay exact floats.
int reservedDataPointCount(const int drawnCount)
{
    constexpr auto kMinReserved = 256;
    constexpr auto kMaxExactVertexId = 1 << 24;
    auto reserved = kMinReserved;
    while (reserved < drawnCount && reserved < kMaxExactVertexId) {
        reserved *= 2;
    }
    return std::max(reserved, drawnCount);
}

QSGGeometryNode* createFillNode()
{
    auto* geometry = new QSGGeometry(BandMaterial::vertexAttributes(), 0);
    geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);
    auto* node = new QSGGeometryNode;
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(new BandMaterial);
    node->setFlag(QSGNode::OwnsMaterial);
    return node;
}

} // namespace

BandSeries::BandSeries(QQuickItem* parent)
    : PlotSeries(DataPointStore::Layout{{DataPointStore::Dimension::X, DataPointStore::Dimension::Y, DataPointStore::Dimension::Y},
                     DataPointStore::RangeRule::EachCoordinate, 0},
          DataPointWording{"data point", "data points", "values", "values"}, parent)
    , color_(defaultBandColor())
{
    setFlag(ItemHasContents, true);
    supportHover();
    setAcceptedMouseButtons(Qt::NoButton);
    setLegendSymbol(LegendSymbol::Fill);
    connect(edges_, &BandEdges::widthChanged, this, &QQuickItem::update);
    connect(edges_, &BandEdges::colorChanged, this, &QQuickItem::update);
    connect(edges_, &BandEdges::lineStyleChanged, this, &BandSeries::onEdgeLineStyleChanged);
    onEdgeLineStyleChanged();
    connect(this, &BandSeries::colorChanged, this, &PlotSeries::legendEntryChanged);
}

QColor BandSeries::color() const
{
    return color_;
}

void BandSeries::setColor(const QColor& color)
{
    if (color_ == color) {
        return;
    }
    color_ = color;
    emit colorChanged();
    update();
}

BandEdges* BandSeries::edges() const
{
    return edges_;
}

int BandSeries::count() const
{
    return dataPoints().count();
}

bool BandSeries::hovered() const
{
    return hovered_;
}

void BandSeries::appendData(const qreal x, const qreal low, const qreal high)
{
    const auto dataPointCount = dataPoints().count();
    xAscending_ = xAscending_ && !std::isnan(x) && (dataPointCount == 0 || x >= value(dataPointCount - 1, 0));
    const auto dataPoint = std::array<double, kStride>{x, low, high};
    appendDataPoint(dataPoint.data());
}

void BandSeries::setData(const QList<qreal>& xs, const QList<qreal>& lows, const QList<qreal>& highs)
{
    applyInterleavedData(interleave(xs, lows, highs));
}

void BandSeries::setData(const std::vector<double>& xs, const std::vector<double>& lows, const std::vector<double>& highs)
{
    applyInterleavedData(interleave(xs, lows, highs));
}

void BandSeries::clearData()
{
    PlotSeries::clearData();
}

std::optional<LegendEntry> BandSeries::legendEntry() const
{
    auto entry = LegendEntry{};
    entry.symbol = legendSymbol();
    entry.color = color_;
    return entry;
}

QVariantMap BandSeries::valueAt(const qreal x) const
{
    const auto span = spanAt(x, dataPoints().count());
    if (!span) {
        return {};
    }
    return {{QStringLiteral("x"), x}, {QStringLiteral("low"), span->low}, {QStringLiteral("high"), span->high}};
}

bool BandSeries::contains(const QPointF& point) const
{
    if (!xAxis() || !yAxis() || !boundingRect().contains(point)) {
        return false;
    }
    const auto span = spanAt(xAxis()->pixelToCoord(point.x(), width()), std::min(dataPoints().count(), renderCapacity_));
    if (!span) {
        return false;
    }
    const auto tolerance = edgesVisible() ? edges_->width() * 0.5 : 0.0;
    const auto lowPixel = yAxis()->coordToPixel(span->low, height());
    const auto highPixel = yAxis()->coordToPixel(span->high, height());
    return point.y() >= std::min(lowPixel, highPixel) - tolerance && point.y() <= std::max(lowPixel, highPixel) + tolerance;
}

bool BandSeries::recordInspectionSupported() const
{
    return true;
}

InspectionRecord BandSeries::inspectionRecord(const int index) const
{
    auto result = InspectionRecord{};
    if (index < 0 || index >= dataPoints().count()) {
        result.status = InspectionStatus::InvalidArgument;
        return result;
    }
    result.index = index;
    result.fields = {{QStringLiteral("x"), value(index, 0)}, {QStringLiteral("low"), value(index, 1)}, {QStringLiteral("high"), value(index, 2)}};
    result.status = dataPointValid(index) ? InspectionStatus::Ready : InspectionStatus::NoMatch;
    return result;
}

InspectionRecord BandSeries::inspectionRecordAt(const QPointF& position) const
{
    auto result = InspectionRecord{};
    if (!xAxis() || width() <= 0 || !contains(position)) {
        return result;
    }
    // A band is continuous between its data points, so a hit reports interpolated limits rather than a source record.
    result.fields = valueAt(xAxis()->pixelToCoord(position.x(), width()));
    if (!result.fields.isEmpty()) {
        result.interpolated = true;
        result.status = InspectionStatus::Ready;
    }
    return result;
}

QSGNode* BandSeries::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* /*updatePaintNodeData*/)
{
    if (!Internal::supportsCustomShaderRendering(window())) {
        static auto warned = false;
        if (!warned) {
            qCWarning(lcQAccelPlot) << "BandSeries custom rendering requires a hardware scene graph backend. Skipping updatePaintNode on the software backend.";
            warned = true;
        }
        delete oldNode;
        return nullptr;
    }
    updateRenderCapacity(dataPointCapacity(window()), dataPoints().count());
    if (dataPoints().count() < 2 || !xAxis() || !yAxis() || resolvePlotRect().isEmpty()) {
        delete oldNode;
        return nullptr;
    }
    const auto dataChanged = dataPoints().renderRevision() != uploadedRevision_;

    auto* root = oldNode;
    if (!root) {
        root = new QSGNode;
        root->appendChildNode(createFillNode());
    }
    const auto view = renderView();
    if (view.drawnCount < 2) {
        delete root;
        return nullptr;
    }
    auto* fillNode = static_cast<QSGGeometryNode*>(root->firstChild());
    updateFillNode(fillNode, view, dataChanged);

    auto* lowerOld = static_cast<QSGGeometryNode*>(fillNode->nextSibling());
    auto* upperOld = lowerOld ? static_cast<QSGGeometryNode*>(lowerOld->nextSibling()) : nullptr;
    if (edgesVisible()) {
        // The edge lines read the fill's data texture instead of uploading their own copies.
        const auto& dataTexture = static_cast<BandMaterial*>(fillNode->material())->dataTexture;
        auto* lower = paintEdge(lowerOld, lowerEdgeRenderer_, view, dataTexture, dataChanged);
        if (!lowerOld) {
            root->appendChildNode(lower);
        }
        auto* upper = paintEdge(upperOld, upperEdgeRenderer_, view, dataTexture, dataChanged);
        if (!upperOld) {
            root->appendChildNode(upper);
        }
    } else {
        delete lowerOld;
        delete upperOld;
    }
    uploadedRevision_ = dataPoints().renderRevision();
    return root;
}

void BandSeries::hoverEnterEvent(QHoverEvent* event)
{
    hoverPosition_ = event->position();
    setHovered(true);
    QQuickItem::hoverEnterEvent(event);
}

void BandSeries::hoverMoveEvent(QHoverEvent* event)
{
    hoverPosition_ = event->position();
    setHovered(true);
    QQuickItem::hoverMoveEvent(event);
}

void BandSeries::hoverLeaveEvent(QHoverEvent* event)
{
    hoverPosition_.reset();
    setHovered(false);
    QQuickItem::hoverLeaveEvent(event);
}

void BandSeries::onAxisScaleChanged()
{
    refreshHovered();
    update();
}

void BandSeries::onAxisRangeChanged()
{
    PlotSeries::onAxisRangeChanged();
    refreshHovered();
}

void BandSeries::onDataPointsChanged(const DataChange change, const int previousCount)
{
    if (change == DataChange::Replaced) {
        updateXAscending();
    }
    refreshHovered();
    if (dataPoints().count() != previousCount) {
        emit countChanged();
    }
}

void BandSeries::applyInterleavedData(std::vector<double>&& data)
{
    const auto dataPointCount = static_cast<int>(data.size() / kStride);
    setData(std::move(data), dataPointCount);
}

double BandSeries::value(const int index, const int component) const
{
    return dataPoints().value(index, component);
}

bool BandSeries::dataPointValid(const int index) const
{
    return isValidCoordinate(value(index, 0), logScaleX()) && isValidCoordinate(value(index, kLowComponent), logScaleY())
        && isValidCoordinate(value(index, kHighComponent), logScaleY());
}

void BandSeries::updateXAscending()
{
    const auto points = dataPoints().view();
    xAscending_ = points.precise ? isAscending(points.precise, points.count) : isAscending(points.floats, points.count);
}

std::optional<BandSeries::Span> BandSeries::spanAt(const qreal x, const int dataPointCount) const
{
    const auto logX = logScaleX();
    const auto logY = logScaleY();
    if (!xAscending_ || !isValidCoordinate(x, logX)) {
        return std::nullopt;
    }
    // First data point whose X is not below x.
    auto first = int{0};
    auto last = dataPointCount;
    while (first < last) {
        const auto middle = first + (last - first) / 2;
        if (value(middle, 0) < x) {
            first = middle + 1;
        } else {
            last = middle;
        }
    }
    const auto bounds = [this](const int index) {
        const auto low = value(index, kLowComponent);
        const auto high = value(index, kHighComponent);
        return Span{std::min(low, high), std::max(low, high)};
    };
    if (first < dataPointCount && value(first, 0) == x) {
        // Data points sharing this X draw a vertical step; the band there covers all of them.
        auto span = std::optional<Span>{};
        for (auto index = first; index < dataPointCount && value(index, 0) == x; ++index) {
            if (dataPointValid(index)) {
                const auto pointSpan = bounds(index);
                span = span ? Span{std::min(span->low, pointSpan.low), std::max(span->high, pointSpan.high)} : pointSpan;
            }
        }
        return span;
    }
    if (first == 0 || first == dataPointCount || !dataPointValid(first - 1) || !dataPointValid(first)) {
        return std::nullopt;
    }
    // Interpolate in axis space, as the shader draws straight edges between the mapped data points.
    const auto x0 = toAxisSpace(value(first - 1, 0), logX);
    const auto t = (toAxisSpace(x, logX) - x0) / (toAxisSpace(value(first, 0), logX) - x0);
    const auto lerp = [t, logY](const qreal from, const qreal to) {
        const auto start = toAxisSpace(from, logY);
        return fromAxisSpace(start + (toAxisSpace(to, logY) - start) * t, logY);
    };
    const auto before = bounds(first - 1);
    const auto after = bounds(first);
    return Span{lerp(before.low, after.low), lerp(before.high, after.high)};
}

bool BandSeries::edgesVisible() const
{
    const auto* style = edges_->lineStyle();
    return edges_->width() > 0.0 && style && style->showLine();
}

QColor BandSeries::edgeColor() const
{
    if (edges_->color().isValid()) {
        return edges_->color();
    }
    auto color = color_;
    color.setAlpha(255);
    return color;
}

void BandSeries::setHovered(const bool hovered)
{
    if (hovered_ == hovered) {
        return;
    }
    hovered_ = hovered;
    emit hoveredChanged();
}

void BandSeries::refreshHovered()
{
    setHovered(hoverPosition_ && contains(*hoverPosition_));
}

void BandSeries::onEdgeLineStyleChanged()
{
    disconnect(edgeStyleConnection_);
    if (auto* style = edges_->lineStyle()) {
        edgeStyleConnection_ = connect(style, &LineStyle::styleChanged, this, &QQuickItem::update);
    }
    update();
}

BandSeries::RenderView BandSeries::renderView()
{
    const auto rect = resolvePlotRect();
    const auto drawnCount = std::min(dataPoints().count(), renderCapacity_);
    const auto originX = dataPoints().renderOrigin(DataPointStore::Dimension::X);
    const auto originY = dataPoints().renderOrigin(DataPointStore::Dimension::Y);
    return RenderView{
        QVector2D(static_cast<float>(xAxis()->viewportMin() - originX), static_cast<float>(yAxis()->viewportMin() - originY)),
        QVector2D(static_cast<float>(xAxis()->viewportMax() - originX), static_cast<float>(yAxis()->viewportMax() - originY)),
        QVector2D(static_cast<float>(rect.width()), static_cast<float>(rect.height())),
        drawnCount,
        reservedDataPointCount(drawnCount),
    };
}

void BandSeries::updateFillNode(QSGGeometryNode* node, const RenderView& view, const bool dataChanged)
{
    auto* geometry = node->geometry();
    const auto vertexCount = view.reservedCount * 2;
    // Vertices hold only data point indices and edge selectors, so only a change of the reserved room
    // rebuilds them. band.vert hides the vertices past the drawn data points.
    if (geometry->vertexCount() != vertexCount) {
        geometry->allocate(vertexCount);
        auto* vertices = static_cast<BandMaterial::Vertex*>(geometry->vertexData());
        for (auto i = int{0}; i < view.reservedCount; ++i) {
            vertices[i * 2] = {static_cast<float>(i), 0.0f};
            vertices[i * 2 + 1] = {static_cast<float>(i), 1.0f};
        }
        node->markDirty(QSGNode::DirtyGeometry);
    }

    auto* material = static_cast<BandMaterial*>(node->material());
    if (dataChanged || material->dataPointCount != static_cast<float>(view.drawnCount) || !material->sampledTexture()) {
        material->uploadTexture(window(), dataPoints().renderData().data(), view.drawnCount * kStride);
    }
    material->color = color_;
    material->domainMin = view.domainMin;
    material->domainMax = view.domainMax;
    material->viewportSize = view.viewportSize;
    material->logScaleX = logScaleX() ? 1.0f : 0.0f;
    material->logScaleY = logScaleY() ? 1.0f : 0.0f;
    material->dataPointCount = static_cast<float>(view.drawnCount);
    node->markDirty(QSGNode::DirtyMaterial);
}

QSGGeometryNode* BandSeries::paintEdge(QSGGeometryNode* oldNode, const BandEdgeRenderer& renderer, const RenderView& view,
    const std::shared_ptr<DataTexture>& dataTexture, const bool dataChanged) const
{
    const auto count = view.drawnCount;
    auto points = dataPoints().view();
    points.count = count;
    const auto* style = edges_->lineStyle();
    const auto uniforms = LineStroke::Uniforms{edgeColor(), edges_->width(), view.domainMin, view.domainMax, view.viewportSize, logScaleX(), logScaleY(), count,
        true, 1.0, style ? style->dashParameters() : DashParameters{}};
    return renderer.paint(oldNode, BandEdgeRenderParams{dataTexture, points, uniforms, xAxis(), yAxis(), dataChanged, view.reservedCount});
}

} // namespace QAccelPlot
