//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/PointCloud.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/GradientUtils.hpp"
#include "QAccelPlot/materials/PointCloudMaterial.hpp"
#include "QAccelPlot/series/LineCurve.hpp"

#include <QHoverEvent>
#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QSGRendererInterface>

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
#include <rhi/qrhi.h>
#endif

#include <algorithm>
#include <cmath>
#include <limits>

namespace QAccelPlot {

namespace {

// Shape integers are shared with LineCurve markers and the point_shapes.glsl shader include.
static_assert(static_cast<int>(PointCloud::MarkerShape::Circle) == static_cast<int>(LineCurve::PointShape::Circle));
static_assert(static_cast<int>(PointCloud::MarkerShape::Pentagon) == static_cast<int>(LineCurve::PointShape::Pentagon));

constexpr auto kPositionStride = 2;
constexpr auto kValueStride = 3;
constexpr auto kVerticesPerPoint = 6;
// Must match kTextureWidth in DataTextureMaterial.cpp and data_texture.glsl.
constexpr auto kDataTextureWidth = 2048;
// Texture height assumed when the RHI cannot be queried (Qt < 6.6); supported by every target GPU.
constexpr auto kFallbackMaxTextureSize = 8192;
// Point ids are passed as float vertex attributes, which represent integers exactly up to 2^24.
constexpr auto kMaxExactFloatInteger = 16777216;
constexpr auto kMinFeather = qreal{0.0};
constexpr auto kMaxFeather = qreal{10.0};
constexpr auto kNaN = std::numeric_limits<qreal>::quiet_NaN();

bool supportsCustomShaderRendering(const QQuickWindow* window)
{
    if (!window) {
        return false;
    }
    const auto* rendererInterface = window->rendererInterface();
    return rendererInterface && rendererInterface->graphicsApi() != QSGRendererInterface::Software;
}

bool hoverEnabled()
{
    auto isInteger = false;
    const auto value = qEnvironmentVariableIntValue("QACCELPLOT_HOVER_ENABLED", &isInteger);
    return !isInteger || value != 0;
}

int maxTextureSize(QQuickWindow* window)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    if (auto* rendererInterface = window->rendererInterface()) {
        const auto* rhi = static_cast<QRhi*>(rendererInterface->getResource(window, QSGRendererInterface::RhiResource));
        if (rhi) {
            return rhi->resourceLimit(QRhi::TextureSizeMax);
        }
    }
#else
    Q_UNUSED(window)
#endif
    return kFallbackMaxTextureSize;
}

int renderablePointCount(QQuickWindow* window, const int pointCount, const int stride)
{
    const auto maxFloats = static_cast<qint64>(kDataTextureWidth) * static_cast<qint64>(maxTextureSize(window));
    const auto capacity = std::min<qint64>(maxFloats / stride, kMaxExactFloatInteger);
    if (pointCount <= capacity) {
        return pointCount;
    }

    static auto warned = false;
    if (!warned) {
        qCWarning(lcQAccelPlot) << "PointCloud has" << pointCount << "points but the GPU data texture holds at most" << capacity
                                << "; the remaining points are not drawn.";
        warned = true;
    }
    return static_cast<int>(capacity);
}

QSGGeometryNode* createPointCloudNode()
{
    auto* geometry = new QSGGeometry(PointCloudMaterial::attributeSet(), 0);
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
void fillPointVertices(QSGGeometry* geometry, const int pointCount)
{
    struct PointCloudVertex {
        float pointId;
        float corner;
    };

    static_assert(sizeof(PointCloudVertex) == 8);

    auto* vertices = static_cast<PointCloudVertex*>(geometry->vertexData());
    for (auto point = 0; point < pointCount; ++point) {
        const auto base = static_cast<std::size_t>(point) * kVerticesPerPoint;
        const auto id = static_cast<float>(point);
        for (auto corner = 0; corner < kVerticesPerPoint; ++corner) {
            vertices[base + static_cast<std::size_t>(corner)] = {id, static_cast<float>(corner)};
        }
    }
}

const std::vector<GradientStopData>& neutralColorStops()
{
    static const auto stops = std::vector<GradientStopData>{{0.0f, Qt::white}, {1.0f, Qt::white}};
    return stops;
}

bool isDrawableCoordinate(const float value, const bool logarithmic)
{
    return std::isfinite(value) && (!logarithmic || value > 0.0f);
}

} // namespace

PointCloud::PointCloud(QQuickItem* parent)
    : PlotSeries(parent)
{
    setFlag(ItemHasContents, true);
    setAcceptHoverEvents(hoverEnabled());
    setAcceptedMouseButtons(Qt::NoButton);
    setLegendSymbol(LegendSymbol::Marker);

    connect(this, &PlotSeries::xAxisChanged, this, [this]() {
        reconnectAxisSignals();
        updateDataRanges();
        spatialIndexValid_ = false;
    });
    connect(this, &PlotSeries::yAxisChanged, this, [this]() {
        reconnectAxisSignals();
        updateDataRanges();
        spatialIndexValid_ = false;
    });
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

PointCloud::MarkerShape PointCloud::markerShape() const
{
    return markerShape_;
}

void PointCloud::setMarkerShape(const MarkerShape shape)
{
    if (markerShape_ == shape) {
        return;
    }
    markerShape_ = shape;
    emit markerShapeChanged();
    update();
}

qreal PointCloud::markerSize() const
{
    return markerSize_;
}

void PointCloud::setMarkerSize(const qreal size)
{
    const auto clamped = std::max(size, qreal{0.0});
    if (nearly_equal(markerSize_, clamped)) {
        return;
    }
    markerSize_ = clamped;
    emit markerSizeChanged();
    update();
}

bool PointCloud::markerFilled() const
{
    return markerFilled_;
}

void PointCloud::setMarkerFilled(const bool filled)
{
    if (markerFilled_ == filled) {
        return;
    }
    markerFilled_ = filled;
    emit markerFilledChanged();
    update();
}

qreal PointCloud::markerStrokeWidth() const
{
    return markerStrokeWidth_;
}

void PointCloud::setMarkerStrokeWidth(const qreal width)
{
    const auto clamped = std::max(width, qreal{0.0});
    if (nearly_equal(markerStrokeWidth_, clamped)) {
        return;
    }
    markerStrokeWidth_ = clamped;
    emit markerStrokeWidthChanged();
    update();
}

PointCloud::ColorMode PointCloud::colorMode() const
{
    return colorMode_;
}

void PointCloud::setColorMode(const ColorMode mode)
{
    if (colorMode_ == mode) {
        return;
    }
    colorMode_ = mode;
    emit colorModeChanged();
    update();
}

QObject* PointCloud::colorGradient() const
{
    return colorGradient_.data();
}

void PointCloud::setColorGradient(QObject* gradient)
{
    if (colorGradient_ == gradient) {
        return;
    }
    colorGradient_ = gradient;
    reconnectGradientSignals();
    refreshColorStops();
    emit colorGradientChanged();
    update();
}

GradientValueSource PointCloud::valueMinSource() const
{
    return valueMinSource_;
}

void PointCloud::setValueMinSource(const GradientValueSource source)
{
    if (valueMinSource_ == source) {
        return;
    }
    valueMinSource_ = source;
    emit valueMinSourceChanged();
    updateValueRange();
}

qreal PointCloud::valueMin() const
{
    return valueMin_;
}

void PointCloud::setValueMin(const qreal value)
{
    if (nearly_equal(valueMin_, value)) {
        return;
    }
    valueMin_ = value;
    emit valueMinChanged();
    updateValueRange();
}

GradientValueSource PointCloud::valueMaxSource() const
{
    return valueMaxSource_;
}

void PointCloud::setValueMaxSource(const GradientValueSource source)
{
    if (valueMaxSource_ == source) {
        return;
    }
    valueMaxSource_ = source;
    emit valueMaxSourceChanged();
    updateValueRange();
}

qreal PointCloud::valueMax() const
{
    return valueMax_;
}

void PointCloud::setValueMax(const qreal value)
{
    if (nearly_equal(valueMax_, value)) {
        return;
    }
    valueMax_ = value;
    emit valueMaxChanged();
    updateValueRange();
}

bool PointCloud::antialiasingEnabled() const
{
    return antialiasingEnabled_;
}

void PointCloud::setAntialiasingEnabled(const bool enabled)
{
    if (antialiasingEnabled_ == enabled) {
        return;
    }
    antialiasingEnabled_ = enabled;
    emit antialiasingEnabledChanged();
    update();
}

qreal PointCloud::antialiasingFeather() const
{
    return antialiasingFeather_;
}

void PointCloud::setAntialiasingFeather(const qreal feather)
{
    const auto clamped = std::clamp(feather, kMinFeather, kMaxFeather);
    if (nearly_equal(antialiasingFeather_, clamped)) {
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
    if (nearly_equal(hoverRadius_, clamped)) {
        return;
    }
    hoverRadius_ = clamped;
    emit hoverRadiusChanged();
}

int PointCloud::count() const
{
    return pointCount_;
}

bool PointCloud::hasValues() const
{
    return hasValues_;
}

int PointCloud::hoveredIndex() const
{
    return hoveredIndex_;
}

qreal PointCloud::dataValueMin() const
{
    return dataValueMin_;
}

qreal PointCloud::dataValueMax() const
{
    return dataValueMax_;
}

void PointCloud::setData(const QList<QPointF>& points)
{
    const auto pointCount = static_cast<int>(points.size());
    auto xy = std::vector<float>(static_cast<std::size_t>(pointCount) * kPositionStride);
    for (auto index = 0; index < pointCount; ++index) {
        xy[static_cast<std::size_t>(index) * 2] = static_cast<float>(points[index].x());
        xy[static_cast<std::size_t>(index) * 2 + 1] = static_cast<float>(points[index].y());
    }
    applyData(std::move(xy), {}, pointCount, true);
}

void PointCloud::setValues(const QList<qreal>& values)
{
    const auto hadValues = hasValues_;
    if (values.isEmpty()) {
        if (!hasValues_) {
            return;
        }
        // Compact (x, y, value) to (x, y) in place; destinations never overtake sources.
        for (auto index = std::size_t{0}; index < static_cast<std::size_t>(pointCount_); ++index) {
            data_[index * 2] = data_[index * 3];
            data_[index * 2 + 1] = data_[index * 3 + 1];
        }
        data_.resize(static_cast<std::size_t>(pointCount_) * kPositionStride);
        hasValues_ = false;
        finishDataChange(pointCount_, hadValues, false);
        return;
    }

    if (values.size() != pointCount_) {
        qCWarning(lcQAccelPlot) << "PointCloud received" << values.size() << "values for" << pointCount_ << "points";
        return;
    }

    if (!hasValues_) {
        // Expand (x, y) to (x, y, value) in place, back to front so no source is overwritten early.
        data_.resize(static_cast<std::size_t>(pointCount_) * kValueStride);
        for (auto index = static_cast<std::size_t>(pointCount_); index-- > 0;) {
            data_[index * 3 + 1] = data_[index * 2 + 1];
            data_[index * 3] = data_[index * 2];
        }
        hasValues_ = true;
    }
    for (auto index = std::size_t{0}; index < static_cast<std::size_t>(pointCount_); ++index) {
        data_[index * 3 + 2] = static_cast<float>(values[static_cast<qsizetype>(index)]);
    }
    finishDataChange(pointCount_, hadValues, false);
}

void PointCloud::clearData()
{
    applyData({}, {}, 0, true);
}

QPointF PointCloud::pointAt(const int index) const
{
    if (index < 0 || index >= pointCount_) {
        return {kNaN, kNaN};
    }
    const auto base = static_cast<std::size_t>(index) * static_cast<std::size_t>(stride());
    return {static_cast<qreal>(data_[base]), static_cast<qreal>(data_[base + 1])};
}

qreal PointCloud::valueAt(const int index) const
{
    if (!hasValues_ || index < 0 || index >= pointCount_) {
        return kNaN;
    }
    return static_cast<qreal>(data_[static_cast<std::size_t>(index) * kValueStride + 2]);
}

void PointCloud::setDataF(const float* xyInterleaved, const int pointCount)
{
    if (pointCount > 0 && !xyInterleaved) {
        qCWarning(lcQAccelPlot) << "PointCloud received a null data pointer for" << pointCount << "points";
        return;
    }
    if (!validateDataArguments(static_cast<std::size_t>(std::max(pointCount, 0)) * kPositionStride, 0, pointCount)) {
        return;
    }
    auto xy = std::vector<float>(xyInterleaved, xyInterleaved + static_cast<std::size_t>(pointCount) * kPositionStride);
    applyData(std::move(xy), {}, pointCount, true);
}

void PointCloud::setDataF(std::vector<float>&& xyInterleaved, const int pointCount)
{
    setDataF(std::move(xyInterleaved), {}, pointCount);
}

void PointCloud::setDataF(std::vector<float>&& xyInterleaved, std::vector<float>&& values, const int pointCount)
{
    if (!validateDataArguments(xyInterleaved.size(), values.size(), pointCount)) {
        return;
    }
    applyData(std::move(xyInterleaved), std::move(values), pointCount, true);
}

void PointCloud::setDataFNoRange(std::vector<float>&& xyInterleaved, std::vector<float>&& values, const int pointCount)
{
    if (!validateDataArguments(xyInterleaved.size(), values.size(), pointCount)) {
        return;
    }
    applyData(std::move(xyInterleaved), std::move(values), pointCount, false);
}

void PointCloud::postData(std::vector<float>&& xyInterleaved, const int pointCount)
{
    postData(std::move(xyInterleaved), {}, pointCount);
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

int PointCloud::pointIndexAt(const QPointF& position) const
{
    if (pointCount_ <= 0 || !xAxis() || !yAxis() || width() <= 0.0 || height() <= 0.0 || hoverRadius_ <= 0.0) {
        return -1;
    }

    ensureSpatialIndex();
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

    // Convert the pixel radius to mapped data units separately per axis.
    const auto radiusX = std::abs(viewportMaxX - viewportMinX) / width() * hoverRadius_;
    const auto radiusY = std::abs(viewportMaxY - viewportMinY) / height() * hoverRadius_;
    return spatialIndex_.nearest(cursorX, cursorY, radiusX, radiusY);
}

bool PointCloud::contains(const QPointF& point) const
{
    return boundingRect().contains(point) && pointIndexAt(point) >= 0;
}

QSGNode* PointCloud::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData)

    auto* window = this->window();
    if (!supportsCustomShaderRendering(window)) {
        static auto warned = false;
        if (!warned) {
            qCWarning(lcQAccelPlot) << "PointCloud custom rendering requires a hardware scene graph backend. Skipping updatePaintNode on the software backend.";
            warned = true;
        }
        delete oldNode;
        return nullptr;
    }
    const auto plotArea = resolvePlotRect();
    if (pointCount_ <= 0 || !xAxis() || !yAxis() || plotArea.width() <= 0.0 || plotArea.height() <= 0.0) {
        delete oldNode;
        return nullptr;
    }

    const auto renderCount = renderablePointCount(window, pointCount_, stride());
    auto* node = oldNode ? static_cast<QSGGeometryNode*>(oldNode) : createPointCloudNode();
    auto* material = static_cast<PointCloudMaterial*>(node->material());

    const auto vertexCount = renderCount * kVerticesPerPoint;
    if (node->geometry()->vertexCount() != vertexCount) {
        node->geometry()->allocate(vertexCount);
        fillPointVertices(node->geometry(), renderCount);
        node->markDirty(QSGNode::DirtyGeometry);
    }

    // A freshly created node owns a new material, so it always needs the data texture.
    if (dataChanged_ || !oldNode || !material->dataTexture) {
        material->uploadTexture(material->dataTexture, window, data_.data(), renderCount * stride());
        dataChanged_ = false;
    }

    const auto useValueColor = colorMode_ == ColorMode::ValueColor && hasValues_ && colorStops_.size() >= 2;
    // The fragment shader always samples the colormap, so a neutral texture keeps the sampler valid.
    material->colorMap.upload(window, useValueColor ? colorStops_ : neutralColorStops());

    material->color = color_;
    material->domainMin = QVector2D(static_cast<float>(xAxis()->viewportMin()), static_cast<float>(yAxis()->viewportMin()));
    material->domainMax = QVector2D(static_cast<float>(xAxis()->viewportMax()), static_cast<float>(yAxis()->viewportMax()));
    material->viewportSize = QVector2D(static_cast<float>(plotArea.width()), static_cast<float>(plotArea.height()));
    material->logScaleX = xAxis()->logScale() ? 1.0f : 0.0f;
    material->logScaleY = yAxis()->logScale() ? 1.0f : 0.0f;
    material->useVertexColor = useValueColor ? 1.0f : 0.0f;
    material->markerSize = static_cast<float>(markerSize_);
    material->markerFilled = markerFilled_ ? 1.0f : 0.0f;
    material->markerStrokeWidth = static_cast<float>(markerStrokeWidth_);
    material->antialiasingEnabled = antialiasingEnabled_ ? 1.0f : 0.0f;
    material->antialiasingFeather = static_cast<float>(antialiasingFeather_);
    material->valueMin = static_cast<float>(dataValueMin_);
    material->valueMax = static_cast<float>(dataValueMax_);
    material->stride = static_cast<float>(stride());
    material->shapeType = static_cast<int>(markerShape_) - 1;
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

void PointCloud::onColorGradientUpdated()
{
    refreshColorStops();
    update();
}

bool PointCloud::validateDataArguments(const std::size_t xyFloatCount, const std::size_t valueCount, const int pointCount) const
{
    if (pointCount < 0) {
        qCWarning(lcQAccelPlot) << "PointCloud data point count cannot be negative:" << pointCount;
        return false;
    }
    const auto expectedFloatCount = static_cast<std::size_t>(pointCount) * kPositionStride;
    if (xyFloatCount != expectedFloatCount) {
        qCWarning(lcQAccelPlot) << "PointCloud received" << xyFloatCount << "coordinates for" << pointCount << "points; expected" << expectedFloatCount;
        return false;
    }
    if (valueCount != 0 && valueCount != static_cast<std::size_t>(pointCount)) {
        qCWarning(lcQAccelPlot) << "PointCloud received" << valueCount << "values for" << pointCount << "points";
        return false;
    }
    return true;
}

void PointCloud::applyData(std::vector<float>&& xyInterleaved, std::vector<float>&& values, const int pointCount, const bool reportRanges)
{
    const auto previousCount = pointCount_;
    const auto hadValues = hasValues_;
    storeInterleaved(std::move(xyInterleaved), values, pointCount);
    finishDataChange(previousCount, hadValues, reportRanges);
}

void PointCloud::storeInterleaved(std::vector<float>&& xyInterleaved, const std::vector<float>& values, const int pointCount)
{
    pointCount_ = pointCount;
    hasValues_ = !values.empty();
    if (!hasValues_) {
        data_ = std::move(xyInterleaved);
        return;
    }

    // Interleave into the existing buffer; its capacity is reused across same-sized updates.
    data_.resize(static_cast<std::size_t>(pointCount) * kValueStride);
    for (auto index = std::size_t{0}; index < static_cast<std::size_t>(pointCount); ++index) {
        data_[index * 3] = xyInterleaved[index * 2];
        data_[index * 3 + 1] = xyInterleaved[index * 2 + 1];
        data_[index * 3 + 2] = values[index];
    }
}

void PointCloud::finishDataChange(const int previousCount, const bool hadValues, const bool reportRanges)
{
    dataChanged_ = true;
    spatialIndexValid_ = false;
    if (reportRanges) {
        updateDataRanges();
    }
    updateValueRange();
    if (hoveredIndex_ >= pointCount_) {
        setHoveredIndex(-1);
    }
    if (previousCount != pointCount_ || hadValues != hasValues_) {
        emit countChanged();
    }
    update();
}

void PointCloud::updateDataRanges()
{
    const auto logX = xAxis() && xAxis()->logScale();
    const auto logY = yAxis() && yAxis()->logScale();
    const auto pointStride = static_cast<std::size_t>(stride());

    auto xMin = std::numeric_limits<qreal>::max();
    auto xMax = std::numeric_limits<qreal>::lowest();
    auto yMin = std::numeric_limits<qreal>::max();
    auto yMax = std::numeric_limits<qreal>::lowest();
    auto anyValid = false;
    for (auto index = std::size_t{0}; index < static_cast<std::size_t>(pointCount_); ++index) {
        const auto x = data_[index * pointStride];
        const auto y = data_[index * pointStride + 1];
        if (!isDrawableCoordinate(x, logX) || !isDrawableCoordinate(y, logY)) {
            continue;
        }
        xMin = std::min(xMin, static_cast<qreal>(x));
        xMax = std::max(xMax, static_cast<qreal>(x));
        yMin = std::min(yMin, static_cast<qreal>(y));
        yMax = std::max(yMax, static_cast<qreal>(y));
        anyValid = true;
    }

    if (!anyValid) {
        clearDataRanges();
        return;
    }
    setDataRanges(xMin, xMax, yMin, yMax);
}

void PointCloud::updateValueRange()
{
    auto minimum = std::numeric_limits<qreal>::max();
    auto maximum = std::numeric_limits<qreal>::lowest();
    const auto needsDataRange = valueMinSource_ == GradientValueSource::DataRange || valueMaxSource_ == GradientValueSource::DataRange;
    if (hasValues_ && needsDataRange) {
        for (auto index = std::size_t{0}; index < static_cast<std::size_t>(pointCount_); ++index) {
            const auto value = data_[index * kValueStride + 2];
            if (!std::isfinite(value)) {
                continue;
            }
            minimum = std::min(minimum, static_cast<qreal>(value));
            maximum = std::max(maximum, static_cast<qreal>(value));
        }
    }
    const auto foundValues = minimum <= maximum;

    const auto resolvedMin = valueMinSource_ == GradientValueSource::Fixed ? valueMin_ : (foundValues ? minimum : 0.0);
    const auto resolvedMax = valueMaxSource_ == GradientValueSource::Fixed ? valueMax_ : (foundValues ? maximum : 1.0);
    if (nearly_equal(resolvedMin, dataValueMin_) && nearly_equal(resolvedMax, dataValueMax_)) {
        return;
    }
    dataValueMin_ = resolvedMin;
    dataValueMax_ = resolvedMax;
    emit valueRangeChanged();
    update();
}

void PointCloud::reconnectAxisSignals()
{
    for (const auto& connection : axisConnections_) {
        disconnect(connection);
    }
    axisConnections_.clear();

    const auto onLogScaleChanged = [this]() {
        updateDataRanges();
        spatialIndexValid_ = false;
        update();
    };
    for (auto* axis : {xAxis(), yAxis()}) {
        if (axis) {
            axisConnections_.append(connect(axis, &Axis::logScaleChanged, this, onLogScaleChanged));
        }
    }
}

void PointCloud::reconnectGradientSignals()
{
    for (const auto& connection : gradientConnections_) {
        disconnect(connection);
    }
    gradientConnections_.clear();
    if (!colorGradient_) {
        return;
    }

    auto* gradient = colorGradient_.data();
    const auto slot = metaObject()->method(metaObject()->indexOfSlot("onColorGradientUpdated()"));
    // QQuickGradient emits updated() for any stop change; the others cover custom gradient objects.
    for (const auto* signalName : {"updated()", "changed()", "stopsChanged()"}) {
        const auto signalIndex = gradient->metaObject()->indexOfSignal(signalName);
        if (signalIndex >= 0) {
            gradientConnections_.append(connect(gradient, gradient->metaObject()->method(signalIndex), this, slot));
        }
    }
    gradientConnections_.append(connect(gradient, &QObject::destroyed, this, [this]() {
        colorStops_.clear();
        emit colorGradientChanged();
        update();
    }));
}

void PointCloud::refreshColorStops()
{
    colorStops_ = readGradientStops(colorGradient_.data());
}

void PointCloud::setHoveredIndex(const int index)
{
    if (hoveredIndex_ == index) {
        return;
    }
    hoveredIndex_ = index;
    emit hoveredIndexChanged();
}

int PointCloud::stride() const
{
    return hasValues_ ? kValueStride : kPositionStride;
}

void PointCloud::ensureSpatialIndex() const
{
    const auto mapping = PointSpatialIndex::Mapping{xAxis() && xAxis()->logScale(), yAxis() && yAxis()->logScale()};
    if (spatialIndexValid_ && spatialIndex_.mapping() == mapping) {
        return;
    }
    spatialIndex_.build(data_.data(), pointCount_, stride(), mapping);
    spatialIndexValid_ = true;
}

} // namespace QAccelPlot
