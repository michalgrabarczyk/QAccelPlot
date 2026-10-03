//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/BandSeries.hpp"

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
#include <cmath>
#include <cstring>
#include <limits>
#include <utility>

namespace QAccelPlot {

namespace {

// Values per sample: x, low, high.
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

struct DataExtents {
    qreal xMin{std::numeric_limits<qreal>::max()};
    qreal xMax{std::numeric_limits<qreal>::lowest()};
    qreal yMin{std::numeric_limits<qreal>::max()};
    qreal yMax{std::numeric_limits<qreal>::lowest()};
};

struct SampleScan {
    DataExtents extents;
    bool xAscending{true};
};

// One pass over interleaved (x, low, high) data: the extents of the valid values, and whether X
// ascends. Each value is judged on its own, so a sample with an invalid low still extends the
// ranges with its x and high.
template <typename T> SampleScan scanSamples(const T* data, const int sampleCount, const bool logScaleX, const bool logScaleY)
{
    auto scan = SampleScan{};
    auto& extents = scan.extents;
    const auto include = [](const qreal value, const bool logScale, qreal& min, qreal& max) {
        if (isValidSample(value, logScale)) {
            min = std::min(min, value);
            max = std::max(max, value);
        }
    };
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(sampleCount); ++i) {
        const auto* sample = data + i * kStride;
        if (std::isnan(sample[0]) || (i > 0 && sample[0] < sample[-kStride])) {
            scan.xAscending = false;
        }
        include(static_cast<qreal>(sample[0]), logScaleX, extents.xMin, extents.xMax);
        include(static_cast<qreal>(sample[kLowComponent]), logScaleY, extents.yMin, extents.yMax);
        include(static_cast<qreal>(sample[kHighComponent]), logScaleY, extents.yMin, extents.yMax);
    }
    return scan;
}

template <typename T> bool isAscending(const T* data, const int sampleCount)
{
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(sampleCount); ++i) {
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

// Number of samples the GPU draws: float vertex ids are exact up to 2^24, and the data texture
// holds a limited number of (x, low, high) triples.
int sampleCapacity(QQuickWindow* window)
{
    constexpr auto kMaxExactVertexId = qint64{1} << 24;
    return static_cast<int>(std::min(Internal::dataTextureItemCapacity(Internal::maxTextureSize(window), kStride), kMaxExactVertexId));
}

// Samples the vertex buffers hold room for: the next power of two, so appending one sample at a
// time rebuilds the vertices only when the count crosses one. Vertex ids stay exact floats.
int reservedSampleCount(const int drawnSampleCount)
{
    constexpr auto kMinReserved = 256;
    constexpr auto kMaxExactVertexId = 1 << 24;
    auto reserved = kMinReserved;
    while (reserved < drawnSampleCount && reserved < kMaxExactVertexId) {
        reserved *= 2;
    }
    return std::max(reserved, drawnSampleCount);
}

void warnOnceIfOverCapacity(const int sampleCount, const int capacity)
{
    static auto warned = false;
    if (sampleCount > capacity && !warned) {
        qCWarning(lcQAccelPlot) << "BandSeries has" << sampleCount << "samples but draws at most" << capacity << "; the remaining samples are not drawn.";
        warned = true;
    }
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
    : PlotSeries(parent)
    , color_(defaultBandColor())
{
    setFlag(ItemHasContents, true);
    setAcceptHoverEvents(Internal::hoverEnabled());
    setAcceptedMouseButtons(Qt::NoButton);
    setLegendSymbol(LegendSymbol::Fill);
    connect(edges_, &BandEdges::widthChanged, this, &QQuickItem::update);
    connect(edges_, &BandEdges::colorChanged, this, &QQuickItem::update);
    connect(edges_, &BandEdges::lineStyleChanged, this, &BandSeries::onEdgeLineStyleChanged);
    onEdgeLineStyleChanged();
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
    return sampleCount_;
}

bool BandSeries::hovered() const
{
    return hovered_;
}

void BandSeries::appendData(const qreal x, const qreal low, const qreal high)
{
    promoteFloatDataToDouble();
    const auto canExtendRenderData = renderDataValid_ && renderOriginSettled_ && renderData_.size() == data_.size();
    const auto ascending = xAscending_ && !std::isnan(x) && (sampleCount_ == 0 || x >= value(sampleCount_ - 1, 0));

    data_.insert(data_.end(), {static_cast<double>(x), static_cast<double>(low), static_cast<double>(high)});
    ++sampleCount_;
    xAscending_ = ascending;

    if (!autoDataRanges_) {
        // The ranges may be stale after a NoRange update; rescan once.
        updateDataRanges();
    } else {
        if (isValidSample(x, logScaleX())) {
            extendXDataRange(x);
        }
        for (const auto y : {low, high}) {
            if (isValidSample(y, logScaleY())) {
                extendYDataRange(y);
            }
        }
    }
    if (canExtendRenderData) {
        renderData_.push_back(static_cast<float>(x - renderOriginX_));
        renderData_.push_back(static_cast<float>(low - renderOriginY_));
        renderData_.push_back(static_cast<float>(high - renderOriginY_));
    } else {
        renderDataValid_ = false;
    }
    dataChanged_ = true;
    refreshHovered();
    emit countChanged();
    inspectionDataChanged();
    update();
}

void BandSeries::setData(const QList<qreal>& xs, const QList<qreal>& lows, const QList<qreal>& highs)
{
    applyInterleavedData(interleave(xs, lows, highs));
}

void BandSeries::setData(const std::vector<double>& xs, const std::vector<double>& lows, const std::vector<double>& highs)
{
    applyInterleavedData(interleave(xs, lows, highs));
}

void BandSeries::setData(const double* data, const int sampleCount)
{
    copyData(data, sampleCount, true);
}

void BandSeries::setData(std::vector<double>&& data, const int sampleCount)
{
    if (validateVectorArguments(data.size(), sampleCount)) {
        applyData(std::move(data), sampleCount, true);
    }
}

void BandSeries::setDataNoRange(const double* data, const int sampleCount)
{
    copyData(data, sampleCount, false);
}

void BandSeries::setDataNoRange(std::vector<double>&& data, const int sampleCount)
{
    if (validateVectorArguments(data.size(), sampleCount)) {
        applyData(std::move(data), sampleCount, false);
    }
}

void BandSeries::setDataF(const float* data, const int sampleCount)
{
    copyFloatData(data, sampleCount, true);
}

void BandSeries::setDataF(std::vector<float>&& data, const int sampleCount)
{
    if (validateVectorArguments(data.size(), sampleCount)) {
        applyFloatData(std::move(data), sampleCount, true);
    }
}

void BandSeries::setDataFNoRange(const float* data, const int sampleCount)
{
    copyFloatData(data, sampleCount, false);
}

void BandSeries::setDataFNoRange(std::vector<float>&& data, const int sampleCount)
{
    if (validateVectorArguments(data.size(), sampleCount)) {
        applyFloatData(std::move(data), sampleCount, false);
    }
}

void BandSeries::postData(std::vector<double>&& data, const int sampleCount)
{
    QMetaObject::invokeMethod(
        this, [this, samples = std::move(data), sampleCount]() mutable { setData(std::move(samples), sampleCount); }, Qt::QueuedConnection);
}

void BandSeries::postData(std::vector<float>&& data, const int sampleCount)
{
    QMetaObject::invokeMethod(
        this, [this, samples = std::move(data), sampleCount]() mutable { setDataF(std::move(samples), sampleCount); }, Qt::QueuedConnection);
}

void BandSeries::clearData()
{
    applyData({}, 0, true);
}

QVariantMap BandSeries::valueAt(const qreal x) const
{
    const auto span = spanAt(x);
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
    const auto span = spanAt(xAxis()->pixelToCoord(point.x(), width()));
    if (!span) {
        return false;
    }
    const auto tolerance = edgesVisible() ? edges_->width() * 0.5 : 0.0;
    const auto lowPixel = yAxis()->coordToPixel(span->low, height());
    const auto highPixel = yAxis()->coordToPixel(span->high, height());
    return point.y() >= std::min(lowPixel, highPixel) - tolerance && point.y() <= std::max(lowPixel, highPixel) + tolerance;
}

InspectionRecord BandSeries::inspectionRecord(const int index) const
{
    auto result = InspectionRecord{};
    if (index < 0 || index >= sampleCount_) {
        result.status = InspectionStatus::InvalidArgument;
        return result;
    }
    result.index = index;
    result.fields = {{QStringLiteral("x"), value(index, 0)}, {QStringLiteral("low"), value(index, 1)}, {QStringLiteral("high"), value(index, 2)}};
    result.status = sampleValid(index) ? InspectionStatus::Ready : InspectionStatus::NoMatch;
    return result;
}

InspectionRecord BandSeries::inspectionRecordAt(const QPointF& position) const
{
    auto result = InspectionRecord{};
    if (!xAxis() || width() <= 0 || !contains(position)) {
        return result;
    }
    // A band is continuous between its samples, so a hit reports interpolated limits rather than a source record.
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
    if (sampleCount_ < 2 || !xAxis() || !yAxis() || resolvePlotRect().isEmpty()) {
        delete oldNode;
        return nullptr;
    }
    if (hasPreciseData() && !renderDataValid_) {
        rebuildRenderData();
    }

    auto* root = oldNode;
    if (!root) {
        root = new QSGNode;
        root->appendChildNode(createFillNode());
    }
    const auto view = renderView();
    if (view.drawnSampleCount < 2) {
        delete root;
        return nullptr;
    }
    auto* fillNode = static_cast<QSGGeometryNode*>(root->firstChild());
    updateFillNode(fillNode, view);

    auto* lowerOld = static_cast<QSGGeometryNode*>(fillNode->nextSibling());
    auto* upperOld = lowerOld ? static_cast<QSGGeometryNode*>(lowerOld->nextSibling()) : nullptr;
    if (edgesVisible()) {
        // The edge lines read the fill's data texture instead of uploading their own copies.
        const auto& dataTexture = static_cast<BandMaterial*>(fillNode->material())->dataTexture;
        auto* lower = paintEdge(lowerOld, lowerEdgeRenderer_, view, dataTexture);
        if (!lowerOld) {
            root->appendChildNode(lower);
        }
        auto* upper = paintEdge(upperOld, upperEdgeRenderer_, view, dataTexture);
        if (!upperOld) {
            root->appendChildNode(upper);
        }
    } else {
        delete lowerOld;
        delete upperOld;
    }
    dataChanged_ = false;
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
    if (autoDataRanges_) {
        updateDataRanges();
    }
    // Only origin-shifted double data depends on the scale; float data is uploaded as is.
    if (hasPreciseData()) {
        renderDataValid_ = false;
    }
    refreshHovered();
    update();
}

void BandSeries::onAxisRangeChanged()
{
    PlotSeries::onAxisRangeChanged();
    refreshHovered();
    if (!hasPreciseData() || !renderDataValid_) {
        return;
    }
    const auto rebaseX = !renderLogScaleX_ && xAxis() && renderOriginTooFar(renderOriginX_, xAxis()->viewportMin(), xAxis()->viewportMax());
    const auto rebaseY = !renderLogScaleY_ && yAxis() && renderOriginTooFar(renderOriginY_, yAxis()->viewportMin(), yAxis()->viewportMax());
    if (rebaseX || rebaseY) {
        renderDataValid_ = false;
    }
}

bool BandSeries::validateRawDataArguments(const void* data, const int sampleCount) const
{
    if (sampleCount < 0) {
        qCWarning(lcQAccelPlot) << "BandSeries sample count cannot be negative:" << sampleCount;
        return false;
    }
    if (sampleCount > 0 && !data) {
        qCWarning(lcQAccelPlot) << "BandSeries received a null data pointer for" << sampleCount << "samples";
        return false;
    }
    return true;
}

bool BandSeries::validateVectorArguments(const std::size_t valueCount, const int sampleCount) const
{
    if (sampleCount < 0) {
        qCWarning(lcQAccelPlot) << "BandSeries sample count cannot be negative:" << sampleCount;
        return false;
    }
    const auto expectedValueCount = static_cast<std::size_t>(sampleCount) * kStride;
    if (valueCount != expectedValueCount) {
        qCWarning(lcQAccelPlot) << "BandSeries received" << valueCount << "values for" << sampleCount << "samples; expected" << expectedValueCount;
        return false;
    }
    return true;
}

void BandSeries::copyData(const double* data, const int sampleCount, const bool reportRanges)
{
    if (validateRawDataArguments(data, sampleCount)) {
        applyData(std::vector<double>(data, data + static_cast<std::size_t>(sampleCount) * kStride), sampleCount, reportRanges);
    }
}

void BandSeries::copyFloatData(const float* data, const int sampleCount, const bool reportRanges)
{
    if (!validateRawDataArguments(data, sampleCount)) {
        return;
    }
    // Reuses the render buffer's allocation.
    auto buffer = std::move(renderData_);
    buffer.assign(data, data + static_cast<std::size_t>(sampleCount) * kStride);
    applyFloatData(std::move(buffer), sampleCount, reportRanges);
}

void BandSeries::applyInterleavedData(std::vector<double>&& data)
{
    const auto sampleCount = static_cast<int>(data.size() / kStride);
    applyData(std::move(data), sampleCount, true);
}

void BandSeries::applyData(std::vector<double>&& data, const int sampleCount, const bool reportRanges)
{
    dataType_ = DataType::Double;
    data_ = std::move(data);
    renderDataValid_ = false;
    finishDataChange(sampleCount, reportRanges);
}

void BandSeries::applyFloatData(std::vector<float>&& data, const int sampleCount, const bool reportRanges)
{
    dataType_ = DataType::Float;
    data_ = std::vector<double>{};
    renderData_ = std::move(data);
    renderOriginX_ = 0.0;
    renderOriginY_ = 0.0;
    renderDataValid_ = true;
    finishDataChange(sampleCount, reportRanges);
}

void BandSeries::finishDataChange(const int sampleCount, const bool reportRanges)
{
    const auto countDiffers = sampleCount_ != sampleCount;
    sampleCount_ = sampleCount;
    dataChanged_ = true;
    if (reportRanges) {
        updateDataRanges();
    } else {
        updateXAscending();
        autoDataRanges_ = false;
    }
    refreshHovered();
    if (countDiffers) {
        emit countChanged();
    }
    inspectionDataChanged();
    update();
}

void BandSeries::promoteFloatDataToDouble()
{
    if (hasPreciseData()) {
        return;
    }
    dataType_ = DataType::Double;
    data_.assign(renderData_.begin(), renderData_.end());
    renderDataValid_ = false;
}

bool BandSeries::hasPreciseData() const
{
    return dataType_ == DataType::Double;
}

double BandSeries::value(const int index, const int component) const
{
    const auto offset = static_cast<std::size_t>(index) * kStride + static_cast<std::size_t>(component);
    return hasPreciseData() ? data_[offset] : static_cast<double>(renderData_[offset]);
}

bool BandSeries::sampleValid(const int index) const
{
    return isValidSample(value(index, 0), logScaleX()) && isValidSample(value(index, kLowComponent), logScaleY())
        && isValidSample(value(index, kHighComponent), logScaleY());
}

bool BandSeries::logScaleX() const
{
    return xAxis() && xAxis()->logScale();
}

bool BandSeries::logScaleY() const
{
    return yAxis() && yAxis()->logScale();
}

void BandSeries::updateDataRanges()
{
    const auto scan = hasPreciseData() ? scanSamples(data_.data(), sampleCount_, logScaleX(), logScaleY())
                                       : scanSamples(renderData_.data(), sampleCount_, logScaleX(), logScaleY());
    const auto& extents = scan.extents;
    xAscending_ = scan.xAscending;
    autoDataRanges_ = true;
    if (extents.xMin <= extents.xMax) {
        setXDataRange(extents.xMin, extents.xMax);
    } else {
        clearXDataRange();
    }
    if (extents.yMin <= extents.yMax) {
        setYDataRange(extents.yMin, extents.yMax);
    } else {
        clearYDataRange();
    }
}

void BandSeries::updateXAscending()
{
    xAscending_ = hasPreciseData() ? isAscending(data_.data(), sampleCount_) : isAscending(renderData_.data(), sampleCount_);
}

std::optional<BandSeries::Span> BandSeries::spanAt(const qreal x) const
{
    const auto logX = logScaleX();
    const auto logY = logScaleY();
    if (!xAscending_ || !isValidSample(x, logX)) {
        return std::nullopt;
    }
    // First sample whose X is not below x.
    auto first = int{0};
    auto last = sampleCount_;
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
    if (first < sampleCount_ && value(first, 0) == x) {
        // Samples sharing this X draw a vertical step; the band there covers all of them.
        auto span = std::optional<Span>{};
        for (auto index = first; index < sampleCount_ && value(index, 0) == x; ++index) {
            if (sampleValid(index)) {
                const auto sample = bounds(index);
                span = span ? Span{std::min(span->low, sample.low), std::max(span->high, sample.high)} : sample;
            }
        }
        return span;
    }
    if (first == 0 || first == sampleCount_ || !sampleValid(first - 1) || !sampleValid(first)) {
        return std::nullopt;
    }
    // Interpolate in axis space, as the shader draws straight edges between the mapped samples.
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

void BandSeries::rebuildRenderData()
{
    renderLogScaleX_ = logScaleX();
    renderLogScaleY_ = logScaleY();
    // Log-scale values aren't translation-invariant, so a log-scale dimension is never shifted.
    renderOriginX_ = 0.0;
    renderOriginY_ = 0.0;
    auto foundOriginX = renderLogScaleX_;
    auto foundOriginY = renderLogScaleY_;
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(sampleCount_) && !(foundOriginX && foundOriginY); ++i) {
        const auto* sample = data_.data() + i * kStride;
        if (!foundOriginX && std::isfinite(sample[0])) {
            renderOriginX_ = sample[0];
            foundOriginX = true;
        }
        for (const auto component : {kLowComponent, kHighComponent}) {
            if (!foundOriginY && std::isfinite(sample[component])) {
                renderOriginY_ = sample[component];
                foundOriginY = true;
            }
        }
    }
    if (!renderLogScaleX_ && xAxis()) {
        renderOriginX_ = renderOriginForViewport(renderOriginX_, xAxis()->viewportMin(), xAxis()->viewportMax());
    }
    if (!renderLogScaleY_ && yAxis()) {
        renderOriginY_ = renderOriginForViewport(renderOriginY_, yAxis()->viewportMin(), yAxis()->viewportMax());
    }
    renderOriginSettled_ = foundOriginX && foundOriginY;

    renderData_.resize(data_.size());
    for (auto offset = std::size_t{0}; offset < data_.size(); offset += kStride) {
        renderData_[offset] = static_cast<float>(data_[offset] - renderOriginX_);
        renderData_[offset + kLowComponent] = static_cast<float>(data_[offset + kLowComponent] - renderOriginY_);
        renderData_[offset + kHighComponent] = static_cast<float>(data_[offset + kHighComponent] - renderOriginY_);
    }
    renderDataValid_ = true;
    dataChanged_ = true;
}

BandSeries::RenderView BandSeries::renderView() const
{
    const auto rect = resolvePlotRect();
    const auto capacity = sampleCapacity(window());
    warnOnceIfOverCapacity(sampleCount_, capacity);
    return RenderView{
        QVector2D(static_cast<float>(xAxis()->viewportMin() - renderOriginX_), static_cast<float>(yAxis()->viewportMin() - renderOriginY_)),
        QVector2D(static_cast<float>(xAxis()->viewportMax() - renderOriginX_), static_cast<float>(yAxis()->viewportMax() - renderOriginY_)),
        QVector2D(static_cast<float>(rect.width()), static_cast<float>(rect.height())),
        std::min(sampleCount_, capacity),
        reservedSampleCount(std::min(sampleCount_, capacity)),
    };
}

void BandSeries::updateFillNode(QSGGeometryNode* node, const RenderView& view)
{
    auto* geometry = node->geometry();
    const auto vertexCount = view.reservedSampleCount * 2;
    // Vertices hold only sample indices and edge selectors, so only a change of the reserved room
    // rebuilds them. band.vert hides the vertices past the drawn samples.
    if (geometry->vertexCount() != vertexCount) {
        geometry->allocate(vertexCount);
        auto* vertices = static_cast<BandMaterial::Vertex*>(geometry->vertexData());
        for (auto i = int{0}; i < view.reservedSampleCount; ++i) {
            vertices[i * 2] = {static_cast<float>(i), 0.0f};
            vertices[i * 2 + 1] = {static_cast<float>(i), 1.0f};
        }
        node->markDirty(QSGNode::DirtyGeometry);
    }

    auto* material = static_cast<BandMaterial*>(node->material());
    if (dataChanged_ || material->sampleCount != static_cast<float>(view.drawnSampleCount) || !material->sampledTexture()) {
        material->uploadTexture(window(), renderData_.data(), view.drawnSampleCount * kStride);
    }
    material->color = color_;
    material->domainMin = view.domainMin;
    material->domainMax = view.domainMax;
    material->viewportSize = view.viewportSize;
    material->logScaleX = logScaleX() ? 1.0f : 0.0f;
    material->logScaleY = logScaleY() ? 1.0f : 0.0f;
    material->sampleCount = static_cast<float>(view.drawnSampleCount);
    node->markDirty(QSGNode::DirtyMaterial);
}

QSGGeometryNode* BandSeries::paintEdge(
    QSGGeometryNode* oldNode, const BandEdgeRenderer& renderer, const RenderView& view, const std::shared_ptr<DataTexture>& dataTexture) const
{
    const auto count = view.drawnSampleCount;
    const auto samples = hasPreciseData() ? BandSamples{nullptr, data_.data(), count} : BandSamples{renderData_.data(), nullptr, count};
    const auto* style = edges_->lineStyle();
    const auto uniforms = LineStroke::Uniforms{edgeColor(), edges_->width(), view.domainMin, view.domainMax, view.viewportSize, logScaleX(), logScaleY(), count,
        true, 1.0, style ? style->dashParameters() : DashParameters{}};
    return renderer.paint(oldNode, BandEdgeRenderParams{dataTexture, samples, uniforms, xAxis(), yAxis(), dataChanged_, view.reservedSampleCount});
}

} // namespace QAccelPlot
