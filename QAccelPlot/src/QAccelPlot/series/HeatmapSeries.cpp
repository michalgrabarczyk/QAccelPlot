//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/HeatmapSeries.hpp"
#include "QAccelPlot/inspection/internal/InspectionTypes.hpp"

#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/materials/HeatmapMaterial.hpp"
#include "QAccelPlot/materials/internal/DataTextureLayout.hpp"
#include "QAccelPlot/series/internal/SeriesSupport.hpp"

#include <QQuickWindow>
#include <QSGGeometryNode>

#include <algorithm>
#include <cmath>
#include <limits>

namespace QAccelPlot {

namespace {

constexpr auto kNaN = std::numeric_limits<qreal>::quiet_NaN();
constexpr auto kQuadVertexCount = 4;

QSGGeometryNode* createHeatmapNode()
{
    auto* geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), kQuadVertexCount);
    geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);

    auto* node = new QSGGeometryNode;
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(new HeatmapMaterial);
    node->setFlag(QSGNode::OwnsMaterial);
    return node;
}

// Sets the quad to cover an item of \a size and returns whether it changed. A \a fresh geometry
// holds no vertices yet.
bool setQuad(QSGGeometry* geometry, const QSizeF& size, const bool fresh)
{
    auto* vertices = geometry->vertexDataAsPoint2D();
    const auto width = static_cast<float>(size.width());
    const auto height = static_cast<float>(size.height());
    if (!fresh && vertices[3].x == width && vertices[3].y == height) {
        return false;
    }
    vertices[0].set(0.0f, 0.0f);
    vertices[1].set(width, 0.0f);
    vertices[2].set(0.0f, height);
    vertices[3].set(width, height);
    return true;
}

// Returns the color the ramp of \a stops shows at its middle.
QColor middleColor(const std::vector<GradientStopData>& stops)
{
    if (stops.empty()) {
        return Qt::transparent;
    }
    constexpr auto kMiddle = 0.5f;
    const auto after = std::find_if(stops.cbegin(), stops.cend(), [](const GradientStopData& stop) { return stop.position >= kMiddle; });
    if (after == stops.cbegin()) {
        return after->color;
    }
    if (after == stops.cend()) {
        return stops.back().color;
    }
    const auto& before = *(after - 1);
    const auto span = after->position - before.position;
    const auto t = span > 0.0f ? (kMiddle - before.position) / span : 0.0f;
    const auto mix = [t](const float from, const float to) { return from + (to - from) * t; };
    return QColor::fromRgbF(mix(before.color.redF(), after->color.redF()), mix(before.color.greenF(), after->color.greenF()),
        mix(before.color.blueF(), after->color.blueF()), mix(before.color.alphaF(), after->color.alphaF()));
}

std::optional<PlotSeries::DataExtent> orderedExtent(const double from, const double to)
{
    return PlotSeries::DataExtent{std::min(from, to), std::max(from, to)};
}

} // namespace

HeatmapSeries::HeatmapSeries(QQuickItem* parent)
    : ColormappedSeries(DataPointStore::Layout{}, DataPointWording{"cell", "cells", "values", "values"}, parent)
    , xMin_(kNaN)
    , xMax_(kNaN)
    , yMin_(kNaN)
    , yMax_(kNaN)
{
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::NoButton);
    setLegendSymbol(LegendSymbol::Fill);
    // Owned by the series, so it renders without setup; a shared colormap replaces it.
    setColormap(new Colormap(this));
}

int HeatmapSeries::columns() const
{
    return columns_;
}

void HeatmapSeries::setColumns(const int columns)
{
    const auto clamped = std::max(columns, 0);
    if (columns_ == clamped) {
        return;
    }
    applyShape(clamped);
}

int HeatmapSeries::rows() const
{
    return rows_;
}

qreal HeatmapSeries::xMin() const
{
    return xMin_;
}

void HeatmapSeries::setXMin(const qreal value)
{
    setBound(xMin_, value, &HeatmapSeries::xMinChanged);
}

qreal HeatmapSeries::xMax() const
{
    return xMax_;
}

void HeatmapSeries::setXMax(const qreal value)
{
    setBound(xMax_, value, &HeatmapSeries::xMaxChanged);
}

qreal HeatmapSeries::yMin() const
{
    return yMin_;
}

void HeatmapSeries::setYMin(const qreal value)
{
    setBound(yMin_, value, &HeatmapSeries::yMinChanged);
}

qreal HeatmapSeries::yMax() const
{
    return yMax_;
}

void HeatmapSeries::setYMax(const qreal value)
{
    setBound(yMax_, value, &HeatmapSeries::yMaxChanged);
}

QColor HeatmapSeries::color() const
{
    return color_;
}

void HeatmapSeries::setValues(const QList<qreal>& values, const int columns)
{
    if (values.isEmpty()) {
        clearData();
        return;
    }
    if (columns <= 0 || values.size() % columns != 0) {
        qCWarning(lcQAccelPlot) << "HeatmapSeries received" << values.size() << "values, which is not a whole number of rows of" << columns << "columns";
        return;
    }
    const auto rows = static_cast<int>(values.size() / columns);
    if (!validateShape(static_cast<std::size_t>(values.size()), columns, rows)) {
        return;
    }
    auto cellValues = std::vector<float>(static_cast<std::size_t>(values.size()));
    std::transform(values.cbegin(), values.cend(), cellValues.begin(), [](const qreal value) { return static_cast<float>(value); });
    storeValues(std::move(cellValues), columns);
}

void HeatmapSeries::setValues(std::vector<float>&& values, const int columns, const int rows)
{
    if (!validateShape(values.size(), columns, rows)) {
        return;
    }
    storeValues(std::move(values), columns);
}

void HeatmapSeries::postValues(std::vector<float>&& values, const int columns, const int rows)
{
    // The series is the context object, so Qt drops the call if the series is destroyed first.
    QMetaObject::invokeMethod(
        this, [this, cellValues = std::move(values), columns, rows]() mutable { setValues(std::move(cellValues), columns, rows); }, Qt::QueuedConnection);
}

qreal HeatmapSeries::valueAt(const int row, const int column) const
{
    if (row < 0 || row >= rows_ || column < 0 || column >= columns_) {
        return kNaN;
    }
    return static_cast<qreal>(values_[static_cast<std::size_t>(row) * static_cast<std::size_t>(columns_) + static_cast<std::size_t>(column)]);
}

void HeatmapSeries::clearData()
{
    storeValues({}, columns_);
}

QSGNode* HeatmapSeries::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData)

    auto* window = this->window();
    if (!Internal::supportsCustomShaderRendering(window)) {
        static auto warned = false;
        if (!warned) {
            qCWarning(lcQAccelPlot)
                << "HeatmapSeries custom rendering requires a hardware scene graph backend. Skipping updatePaintNode on the software backend.";
            warned = true;
        }
        delete oldNode;
        return nullptr;
    }
    const auto plotArea = resolvePlotRect();
    const auto extent = resolvedExtent();
    const auto drawable = cellCount() > 0 && xAxis() && yAxis() && plotArea.width() > 0.0 && plotArea.height() > 0.0 && colorStops().size() >= 2
        && extent.isFinite() && extent.xMin != extent.xMax && extent.yMin != extent.yMax;
    if (!drawable || !canDraw(window)) {
        delete oldNode;
        return nullptr;
    }

    auto* node = oldNode ? static_cast<QSGGeometryNode*>(oldNode) : createHeatmapNode();
    if (setQuad(node->geometry(), plotArea.size(), !oldNode)) {
        node->markDirty(QSGNode::DirtyGeometry);
    }
    updateMaterial(*static_cast<HeatmapMaterial*>(node->material()), window, !oldNode);
    node->markDirty(QSGNode::DirtyMaterial);
    return node;
}

void HeatmapSeries::applyDataPoints(DataPointInput&& input)
{
    const auto count = input.count;
    if (count > 0 && (columns_ <= 0 || count % columns_ != 0)) {
        qCWarning(lcQAccelPlot) << "HeatmapSeries received" << count << "values, which is not a whole number of rows of" << columns_ << "columns";
        return;
    }
    const auto size = static_cast<std::size_t>(count);
    auto cellValues = std::vector<float>{};
    if (input.floatArray) {
        cellValues.assign(input.floatArray, input.floatArray + size);
    } else if (input.doubleArray) {
        cellValues.assign(input.doubleArray, input.doubleArray + size);
    } else if (input.precise) {
        cellValues.assign(input.doubles.cbegin(), input.doubles.cend());
    } else {
        cellValues = std::move(input.floats);
    }
    storeValues(std::move(cellValues), columns_);
}

int HeatmapSeries::dataPointValueCount() const
{
    return 1;
}

InspectionRecord HeatmapSeries::inspectionRecord(const int index) const
{
    auto result = InspectionRecord{};
    if (index < 0 || index >= cellCount()) {
        result.status = InspectionStatus::InvalidArgument;
        return result;
    }
    const auto extent = resolvedExtent();
    if (!extent.isFinite()) {
        result.status = InspectionStatus::Unavailable;
        return result;
    }
    const auto row = index / columns_;
    const auto column = index % columns_;
    const auto cellWidth = (extent.xMax - extent.xMin) / columns_;
    const auto cellHeight = (extent.yMax - extent.yMin) / rows_;
    result.index = index;
    result.fields = QVariantMap{
        {QStringLiteral("x"), extent.xMin + (column + 0.5) * cellWidth},
        {QStringLiteral("y"), extent.yMin + (row + 0.5) * cellHeight},
        {QStringLiteral("value"), valueAt(row, column)},
        {QStringLiteral("row"), row},
        {QStringLiteral("column"), column},
    };
    result.status = InspectionStatus::Ready;
    return result;
}

InspectionRecord HeatmapSeries::inspectionRecordAt(const QPointF& position) const
{
    const auto cell = cellAt(position);
    return cell ? inspectionRecord(cell->first * columns_ + cell->second) : InspectionRecord{};
}

PlotSeries::DataRanges HeatmapSeries::computeDataRanges() const
{
    if (cellCount() <= 0) {
        return {};
    }
    const auto extent = resolvedExtent();
    auto ranges = DataRanges{};
    if (!logScaleX()) {
        ranges.x = orderedExtent(extent.xMin, extent.xMax);
    }
    if (!logScaleY()) {
        ranges.y = orderedExtent(extent.yMin, extent.yMax);
    }
    return ranges;
}

void HeatmapSeries::onAxisScaleChanged()
{
    update();
}

std::optional<ColormappedSeries::ValueSpan> HeatmapSeries::scanValueRange() const
{
    auto minimum = std::numeric_limits<qreal>::max();
    auto maximum = std::numeric_limits<qreal>::lowest();
    const auto end = values_.cbegin() + cellCount();
    for (auto value = values_.cbegin(); value != end; ++value) {
        if (!std::isfinite(*value)) {
            continue;
        }
        minimum = std::min(minimum, static_cast<qreal>(*value));
        maximum = std::max(maximum, static_cast<qreal>(*value));
    }
    if (minimum > maximum) {
        return std::nullopt;
    }
    return ValueSpan{minimum, maximum};
}

void HeatmapSeries::onColorStopsChanged()
{
    const auto color = middleColor(colorStops());
    if (color_ == color) {
        return;
    }
    color_ = color;
    emit colorChanged();
}

bool HeatmapSeries::Extent::isFinite() const
{
    return std::isfinite(xMin) && std::isfinite(xMax) && std::isfinite(yMin) && std::isfinite(yMax);
}

HeatmapSeries::Extent HeatmapSeries::resolvedExtent() const
{
    const auto resolve = [](const qreal bound, const double fallback) { return std::isnan(bound) ? fallback : static_cast<double>(bound); };
    return {resolve(xMin_, 0.0), resolve(xMax_, columns_), resolve(yMin_, 0.0), resolve(yMax_, rows_)};
}

int HeatmapSeries::cellCount() const
{
    return columns_ * rows_;
}

bool HeatmapSeries::validateShape(const std::size_t valueCount, const int columns, const int rows) const
{
    if (columns < 0 || rows < 0) {
        qCWarning(lcQAccelPlot) << "HeatmapSeries columns and rows cannot be negative:" << columns << "x" << rows;
        return false;
    }
    const auto cells = static_cast<qint64>(columns) * rows;
    if (cells > std::numeric_limits<int>::max()) {
        qCWarning(lcQAccelPlot) << "HeatmapSeries grid of" << columns << "x" << rows << "cells exceeds" << std::numeric_limits<int>::max() << "cells";
        return false;
    }
    if (valueCount != static_cast<std::size_t>(cells)) {
        qCWarning(lcQAccelPlot) << "HeatmapSeries received" << valueCount << "values for" << columns << "x" << rows << "cells";
        return false;
    }
    return true;
}

void HeatmapSeries::storeValues(std::vector<float>&& values, const int columns)
{
    values_ = std::move(values);
    applyShape(columns);
}

void HeatmapSeries::applyShape(const int columns)
{
    const auto previousColumns = std::exchange(columns_, columns);
    const auto rows = columns_ > 0 ? static_cast<int>(values_.size() / static_cast<std::size_t>(columns_)) : 0;
    const auto previousRows = std::exchange(rows_, rows);
    ++valuesRevision_;
    if (columns_ != previousColumns) {
        emit columnsChanged();
    }
    if (rows_ != previousRows) {
        emit rowsChanged();
    }
    updateValueRange();
    invalidateDataRanges();
    inspectionDataChanged();
    update();
}

void HeatmapSeries::setBound(qreal& bound, const qreal value, void (HeatmapSeries::*changed)())
{
    if ((std::isnan(bound) && std::isnan(value)) || bound == value) {
        return;
    }
    bound = value;
    emit(this->*changed)();
    invalidateDataRanges();
    // The cell centers the records report moved.
    inspectionDataChanged();
    update();
}

std::optional<std::pair<int, int>> HeatmapSeries::cellAt(const QPointF& position) const
{
    if (cellCount() <= 0 || !xAxis() || !yAxis() || logScaleX() || logScaleY() || width() <= 0.0 || height() <= 0.0) {
        return std::nullopt;
    }
    const auto extent = resolvedExtent();
    if (!extent.isFinite()) {
        return std::nullopt;
    }
    const auto x = xAxis()->pixelToCoord(position.x(), width());
    const auto y = yAxis()->pixelToCoord(position.y(), height());
    const auto column = std::floor((x - extent.xMin) / (extent.xMax - extent.xMin) * columns_);
    const auto row = std::floor((y - extent.yMin) / (extent.yMax - extent.yMin) * rows_);
    // An empty extent gives NaN or infinity.
    if (!std::isfinite(column) || !std::isfinite(row) || column < 0.0 || column >= columns_ || row < 0.0 || row >= rows_) {
        return std::nullopt;
    }
    return std::pair<int, int>{static_cast<int>(row), static_cast<int>(column)};
}

bool HeatmapSeries::canDraw(QQuickWindow* window)
{
    if (logScaleX() || logScaleY()) {
        if (!warnedAboutLogAxis_) {
            qCWarning(lcQAccelPlot) << "HeatmapSeries does not support logarithmic axes and draws nothing on them.";
            warnedAboutLogAxis_ = true;
        }
        return false;
    }
    warnedAboutLogAxis_ = false;

    const auto capacity = Internal::dataTextureItemCapacity(Internal::maxTextureSize(window), 1);
    const auto overCapacity = cellCount() > capacity;
    if (overCapacity && !overCapacity_) {
        qCWarning(lcQAccelPlot) << "HeatmapSeries has" << cellCount() << "cells but draws at most" << capacity << "; the grid is not drawn.";
    }
    overCapacity_ = overCapacity;
    return !overCapacity;
}

void HeatmapSeries::updateMaterial(HeatmapMaterial& material, QQuickWindow* window, const bool newNode)
{
    if (uploadedRevision_ != valuesRevision_ || newNode) {
        material.values.upload(window, values_.data(), cellCount());
        uploadedRevision_ = valuesRevision_;
    }
    material.colorMap.upload(window, colorStops());

    // Item-local pixels map to cell coordinates through the axis viewports; computed in double so
    // the single-precision uniforms stay exact near the visible cells.
    const auto extent = resolvedExtent();
    const auto plotArea = resolvePlotRect();
    const auto columnsPerUnit = columns_ / (extent.xMax - extent.xMin);
    const auto rowsPerUnit = rows_ / (extent.yMax - extent.yMin);
    const auto xSpan = xAxis()->viewportMax() - xAxis()->viewportMin();
    const auto ySpan = yAxis()->viewportMax() - yAxis()->viewportMin();
    material.cellScale = QVector2D(static_cast<float>(xSpan / plotArea.width() * columnsPerUnit), static_cast<float>(-ySpan / plotArea.height() * rowsPerUnit));
    material.cellOffset = QVector2D(
        static_cast<float>((xAxis()->viewportMin() - extent.xMin) * columnsPerUnit), static_cast<float>((yAxis()->viewportMax() - extent.yMin) * rowsPerUnit));
    material.valueMin = static_cast<float>(dataValueMin());
    material.valueMax = static_cast<float>(dataValueMax());
    material.valueLogScale = colormap()->norm() == Colormap::Normalization::Log ? 1.0f : 0.0f;
    material.columns = columns_;
    material.rows = rows_;
}

} // namespace QAccelPlot
