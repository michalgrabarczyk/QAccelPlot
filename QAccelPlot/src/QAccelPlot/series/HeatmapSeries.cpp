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
#include "QAccelPlot/axis/AxisMapping.hpp"
#include "QAccelPlot/materials/HeatmapMaterial.hpp"
#include "QAccelPlot/materials/internal/DataTextureLayout.hpp"
#include "QAccelPlot/series/internal/HeatmapGrid.hpp"
#include "QAccelPlot/series/internal/SeriesSupport.hpp"

#include <QLineF>
#include <QQuickWindow>
#include <QSGGeometryNode>

#include <algorithm>
#include <cmath>
#include <limits>

namespace QAccelPlot {

namespace {

constexpr auto kNaN = std::numeric_limits<qreal>::quiet_NaN();
constexpr auto kQuadVertexCount = 4;
// The fragment shader locates cells in single precision, which counts exactly up to 2^24.
constexpr auto kMaxShaderCellsPerDimension = 1 << 24;

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

std::optional<PlotSeries::DataExtent> dataExtent(const Internal::HeatmapGridDimension& dimension, const bool logAxis)
{
    const auto extent = dimension.dataExtent(logAxis);
    if (!extent) {
        return std::nullopt;
    }
    return PlotSeries::DataExtent{extent->first, extent->second};
}

} // namespace

HeatmapSeries::HeatmapSeries(QQuickItem* parent)
    : ColormappedSeries(DataPointStore::Layout{}, DataPointWording{"cell", "cells", "values", "values"}, parent)
    , edges_{kNaN, kNaN, kNaN, kNaN}
{
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::NoButton);
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

int HeatmapSeries::count() const
{
    return cellCount();
}

qreal HeatmapSeries::xMin() const
{
    return edges_.xMin;
}

void HeatmapSeries::setXMin(const qreal value)
{
    setBound(edges_.xMin, value, &HeatmapSeries::xMinChanged);
}

qreal HeatmapSeries::xMax() const
{
    return edges_.xMax;
}

void HeatmapSeries::setXMax(const qreal value)
{
    setBound(edges_.xMax, value, &HeatmapSeries::xMaxChanged);
}

qreal HeatmapSeries::yMin() const
{
    return edges_.yMin;
}

void HeatmapSeries::setYMin(const qreal value)
{
    setBound(edges_.yMin, value, &HeatmapSeries::yMinChanged);
}

qreal HeatmapSeries::yMax() const
{
    return edges_.yMax;
}

void HeatmapSeries::setYMax(const qreal value)
{
    setBound(edges_.yMax, value, &HeatmapSeries::yMaxChanged);
}

HeatmapSeries::Spacing HeatmapSeries::xSpacing() const
{
    return xSpacing_;
}

void HeatmapSeries::setXSpacing(const Spacing spacing)
{
    setSpacing(xSpacing_, spacing, &HeatmapSeries::xSpacingChanged);
}

HeatmapSeries::Spacing HeatmapSeries::ySpacing() const
{
    return ySpacing_;
}

void HeatmapSeries::setYSpacing(const Spacing spacing)
{
    setSpacing(ySpacing_, spacing, &HeatmapSeries::ySpacingChanged);
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
    const auto drawable = cellCount() > 0 && xAxis() && yAxis() && xAxis()->mapping().valid && yAxis()->mapping().valid && plotArea.width() > 0.0
        && plotArea.height() > 0.0 && colorStops().size() >= 2 && extent.isFinite() && extent.xMin != extent.xMax && extent.yMin != extent.yMax;
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

bool HeatmapSeries::recordInspectionSupported() const
{
    return true;
}

InspectionRecord HeatmapSeries::inspectionRecord(const int index) const
{
    auto result = InspectionRecord{};
    if (index < 0 || index >= cellCount()) {
        result.status = InspectionStatus::InvalidArgument;
        return result;
    }
    if (!xDimension().isValid() || !yDimension().isValid()) {
        result.status = InspectionStatus::Unavailable;
        return result;
    }
    const auto row = index / columns_;
    const auto column = index % columns_;
    const auto center = cellCenter(row, column);
    result.index = index;
    result.fields = QVariantMap{
        {QStringLiteral("x"), center.x()},
        {QStringLiteral("y"), center.y()},
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

bool HeatmapSeries::positionInspectionSupported() const
{
    return true;
}

InspectionDataPoint HeatmapSeries::inspectionDataPointAt(const QPointF& position) const
{
    auto result = InspectionDataPoint{};
    const auto cell = cellAt(position);
    if (!cell) {
        return result;
    }
    const auto [row, column] = *cell;
    const auto center = cellCenter(row, column);
    result.status = InspectionStatus::Ready;
    result.index = row * columns_ + column;
    result.position = center;
    result.value = valueAt(row, column);
    result.pixelPosition = QPointF{xAxis()->coordToPixel(center.x(), width()), yAxis()->coordToPixel(center.y(), height())};
    result.distance = QLineF{position, result.pixelPosition}.length();
    return result;
}

PlotSeries::DataRanges HeatmapSeries::computeDataRanges() const
{
    if (cellCount() <= 0) {
        return {};
    }
    auto ranges = DataRanges{};
    ranges.x = dataExtent(xDimension(), logScaleX());
    ranges.y = dataExtent(yDimension(), logScaleY());
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

bool HeatmapSeries::Extent::isFinite() const
{
    return std::isfinite(xMin) && std::isfinite(xMax) && std::isfinite(yMin) && std::isfinite(yMax);
}

HeatmapSeries::Extent HeatmapSeries::resolvedExtent() const
{
    const auto resolve = [](const qreal edge, const qreal fallback) { return std::isnan(edge) ? fallback : edge; };
    return {resolve(edges_.xMin, 0.0), resolve(edges_.xMax, columns_), resolve(edges_.yMin, 0.0), resolve(edges_.yMax, rows_)};
}

Internal::HeatmapGridDimension HeatmapSeries::xDimension() const
{
    const auto extent = resolvedExtent();
    return {extent.xMin, extent.xMax, columns_, xSpacing_ == Spacing::Log};
}

Internal::HeatmapGridDimension HeatmapSeries::yDimension() const
{
    const auto extent = resolvedExtent();
    return {extent.yMin, extent.yMax, rows_, ySpacing_ == Spacing::Log};
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
    const auto previousCount = cellCount();
    const auto previousColumns = std::exchange(columns_, columns);
    const auto rows = columns_ > 0 ? static_cast<int>(values_.size() / static_cast<std::size_t>(columns_)) : 0;
    const auto previousRows = std::exchange(rows_, rows);
    ++valuesRevision_;
    updateValueRange();
    invalidateDataRanges();
    inspectionDataChanged();
    if (columns_ != previousColumns) {
        emit columnsChanged();
    }
    if (rows_ != previousRows) {
        emit rowsChanged();
    }
    if (cellCount() != previousCount) {
        emit countChanged();
    }
    update();
}

void HeatmapSeries::setBound(qreal& bound, const qreal value, void (HeatmapSeries::*changed)())
{
    if ((std::isnan(bound) && std::isnan(value)) || bound == value) {
        return;
    }
    bound = value;
    invalidateDataRanges();
    // The cell centers the records report moved.
    inspectionDataChanged();
    emit(this->*changed)();
    update();
}

void HeatmapSeries::setSpacing(Spacing& current, const Spacing spacing, void (HeatmapSeries::*changed)())
{
    if (current == spacing) {
        return;
    }
    current = spacing;
    invalidateDataRanges();
    inspectionDataChanged();
    emit(this->*changed)();
    update();
}

std::optional<std::pair<int, int>> HeatmapSeries::cellAt(const QPointF& position) const
{
    if (cellCount() <= 0 || !xAxis() || !yAxis() || !xAxis()->mapping().valid || !yAxis()->mapping().valid || width() <= 0.0 || height() <= 0.0) {
        return std::nullopt;
    }
    const auto columns = xDimension();
    const auto rows = yDimension();
    if (!columns.isValid() || !rows.isValid()) {
        return std::nullopt;
    }
    const auto column = std::floor(columns.cellCoordinate(xAxis()->pixelToCoord(position.x(), width())));
    const auto row = std::floor(rows.cellCoordinate(yAxis()->pixelToCoord(position.y(), height())));
    // An empty extent gives NaN or infinity.
    if (!std::isfinite(column) || !std::isfinite(row) || column < 0.0 || column >= columns_ || row < 0.0 || row >= rows_) {
        return std::nullopt;
    }
    return std::pair<int, int>{static_cast<int>(row), static_cast<int>(column)};
}

QPointF HeatmapSeries::cellCenter(const int row, const int column) const
{
    return {xDimension().cellCenter(column), yDimension().cellCenter(row)};
}

bool HeatmapSeries::canDraw(QQuickWindow* window)
{
    const auto invalidLogSpacing = !xDimension().isValid() || !yDimension().isValid();
    if (invalidLogSpacing && !invalidLogSpacing_) {
        qCWarning(lcQAccelPlot) << "HeatmapSeries with Log spacing needs positive grid edges in that dimension; the heatmap is not drawn.";
    }
    invalidLogSpacing_ = invalidLogSpacing;
    if (invalidLogSpacing) {
        return false;
    }

    const auto overPrecision = columns_ > kMaxShaderCellsPerDimension || rows_ > kMaxShaderCellsPerDimension;
    if (overPrecision && !overPrecision_) {
        qCWarning(lcQAccelPlot) << "HeatmapSeries has" << columns_ << "x" << rows_ << "cells; grids of more than" << kMaxShaderCellsPerDimension
                                << "columns or rows are not drawn.";
    }
    overPrecision_ = overPrecision;
    if (overPrecision) {
        return false;
    }

    const auto capacity = std::min(Internal::dataTextureItemCapacity(Internal::maxTextureSize(window), 1), qint64{std::numeric_limits<int>::max()});
    updateRenderCapacity(static_cast<int>(capacity), cellCount(), CapacityOverflow::DropsAll);
    return !overRenderCapacity_;
}

void HeatmapSeries::updateMaterial(HeatmapMaterial& material, QQuickWindow* window, const bool newNode)
{
    if (uploadedRevision_ != valuesRevision_ || newNode) {
        material.values.upload(window, values_.data(), cellCount());
        uploadedRevision_ = valuesRevision_;
    }
    material.colorMap.upload(window, colorStops());

    const auto plotArea = resolvePlotRect();
    auto xMapping = xAxis()->mapping();
    xMapping.flipped = false;
    // Rows run upward whatever the orientation of the y axis.
    auto yMapping = yAxis()->mapping();
    yMapping.flipped = true;
    const auto columns = xDimension().shaderMapping(xMapping, plotArea.width());
    const auto rows = yDimension().shaderMapping(yMapping, plotArea.height());
    material.cellScale = QVector2D(static_cast<float>(columns.scale), static_cast<float>(rows.scale));
    material.cellOffset = QVector2D(static_cast<float>(columns.offset), static_cast<float>(rows.offset));
    material.cellRate = QVector2D(static_cast<float>(columns.rate), static_cast<float>(rows.rate));
    material.cellShift = QVector2D(static_cast<float>(columns.shift), static_cast<float>(rows.shift));
    material.columnMapping = static_cast<int>(columns.mode);
    material.rowMapping = static_cast<int>(rows.mode);
    material.valueMin = static_cast<float>(dataValueMin());
    material.valueMax = static_cast<float>(dataValueMax());
    material.valueLogScale = colormap()->norm() == Colormap::Normalization::Log ? 1.0f : 0.0f;
    material.columns = columns_;
    material.rows = rows_;
}

} // namespace QAccelPlot
