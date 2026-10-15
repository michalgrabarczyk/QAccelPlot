//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/DataPointStore.hpp"

#include "QAccelPlot/MathUtils.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <limits>
#include <utility>

namespace QAccelPlot {

namespace {

constexpr auto kMaxColumns = 8;

struct Bounds {
    double min{std::numeric_limits<double>::max()};
    double max{std::numeric_limits<double>::lowest()};

    void include(const double value)
    {
        min = std::min(min, value);
        max = std::max(max, value);
    }

    [[nodiscard]] std::optional<DataPointStore::Extent> extent() const
    {
        if (min > max) {
            return std::nullopt;
        }
        return DataPointStore::Extent{min, max};
    }
};

// True when an extent from the unchecked pass is empty, or its bounds are valid coordinates.
bool fastBoundsUsable(const Bounds& bounds, const bool logScale)
{
    return bounds.min > bounds.max || (isValidSample(bounds.min, logScale) && isValidSample(bounds.max, logScale));
}

// The first pass is a branch-free min/max scan in the buffer's own type. It is exact whenever every
// coordinate is valid, and NaN never wins a `value < min ? value : min` comparison, so NaN is
// ignored as required. +/-Inf and non-positive log-scale values show up in the resulting bounds,
// which sends the caller to the validity-checked scan.
template <typename T>
void scanUnchecked(const T* data, const int stride, const int first, const int last, const int columnCount, std::array<Bounds, kMaxColumns>& columns)
{
    auto mins = std::array<T, kMaxColumns>{};
    auto maxs = std::array<T, kMaxColumns>{};
    mins.fill(std::numeric_limits<T>::max());
    maxs.fill(std::numeric_limits<T>::lowest());
    const auto* end = data + static_cast<std::size_t>(last) * static_cast<std::size_t>(stride);
    for (const auto* point = data + static_cast<std::size_t>(first) * static_cast<std::size_t>(stride); point != end; point += stride) {
        for (auto column = 0; column < columnCount; ++column) {
            const auto value = point[column];
            mins[column] = value < mins[column] ? value : mins[column];
            maxs[column] = value > maxs[column] ? value : maxs[column];
        }
    }
    for (auto column = 0; column < columnCount; ++column) {
        if (first < last) {
            columns[column] = Bounds{static_cast<double>(mins[column]), static_cast<double>(maxs[column])};
        }
    }
}

} // namespace

double DataPointStore::View::value(const int index, const int column) const noexcept
{
    const auto offset = static_cast<std::size_t>(index) * static_cast<std::size_t>(stride) + static_cast<std::size_t>(column);
    return precise ? precise[offset] : static_cast<double>(floats[offset]);
}

DataPointStore::DataPointStore(Layout layout)
    : layout_(std::move(layout))
{
    Q_ASSERT(!layout_.columns.empty() && layout_.columns.size() <= kMaxColumns);
}

const DataPointStore::Layout& DataPointStore::layout() const noexcept
{
    return layout_;
}

void DataPointStore::setLayout(Layout layout)
{
    Q_ASSERT(!layout.columns.empty() && layout.columns.size() <= kMaxColumns);
    layout_ = std::move(layout);
    clear();
}

void DataPointStore::remapColumns(std::vector<Dimension> columns)
{
    Q_ASSERT(columns.size() == layout_.columns.size());
    layout_.columns = std::move(columns);
    compacted_.valid = false;
    invalidateRender();
}

int DataPointStore::columnCount() const noexcept
{
    return static_cast<int>(layout_.columns.size());
}

void DataPointStore::setPrecise(std::vector<double>&& coordinates, const int count, std::vector<float>&& attributes)
{
    Q_ASSERT(coordinates.size() == static_cast<std::size_t>(count) * static_cast<std::size_t>(columnCount()));
    Q_ASSERT(attributes.empty() || attributes.size() == static_cast<std::size_t>(count) * static_cast<std::size_t>(layout_.attributeCount));
    precise_ = std::move(coordinates);
    attributes_ = std::move(attributes);
    floats_.clear();
    count_ = count;
    isPrecise_ = true;
    hasAttributes_ = !attributes_.empty() && layout_.attributeCount > 0;
    categories_.clear();
    dataPointsChanged();
}

void DataPointStore::assignPrecise(const double* coordinates, const int count)
{
    const auto size = static_cast<std::size_t>(count) * static_cast<std::size_t>(columnCount());
    precise_.resize(size);
    if (size > 0) {
        std::memcpy(precise_.data(), coordinates, size * sizeof(double));
    }
    attributes_.clear();
    floats_.clear();
    count_ = count;
    isPrecise_ = true;
    hasAttributes_ = false;
    categories_.clear();
    dataPointsChanged();
}

void DataPointStore::setFloat(std::vector<float>&& coordinates, const int count, std::vector<float>&& attributes)
{
    Q_ASSERT(coordinates.size() == static_cast<std::size_t>(count) * static_cast<std::size_t>(columnCount()));
    Q_ASSERT(attributes.empty() || attributes.size() == static_cast<std::size_t>(count) * static_cast<std::size_t>(layout_.attributeCount));
    count_ = count;
    storeFloatCoordinates(std::move(coordinates), std::move(attributes));
    categories_.clear();
    dataPointsChanged();
}

void DataPointStore::assignFloat(const float* coordinates, const int count)
{
    const auto size = static_cast<std::size_t>(count) * static_cast<std::size_t>(columnCount());
    floats_.resize(size);
    if (size > 0) {
        std::memcpy(floats_.data(), coordinates, size * sizeof(float));
    }
    precise_.clear();
    attributes_.clear();
    count_ = count;
    isPrecise_ = false;
    hasAttributes_ = false;
    categories_.clear();
    dataPointsChanged();
}

void DataPointStore::setAttributes(std::vector<float>&& attributes)
{
    Q_ASSERT(attributes.empty() || attributes.size() == static_cast<std::size_t>(count_) * static_cast<std::size_t>(layout_.attributeCount));
    if (isPrecise_) {
        attributes_ = std::move(attributes);
        hasAttributes_ = !attributes_.empty() && layout_.attributeCount > 0;
        dataPointsChanged();
        return;
    }
    // Float data keeps its attributes interleaved, so the coordinates are moved out and back in.
    const auto columns = static_cast<std::size_t>(columnCount());
    const auto stride = static_cast<std::size_t>(renderStride());
    auto coordinates = std::vector<float>(static_cast<std::size_t>(count_) * columns);
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(count_); ++i) {
        std::copy_n(floats_.data() + i * stride, columns, coordinates.data() + i * columns);
    }
    storeFloatCoordinates(std::move(coordinates), std::move(attributes));
    dataPointsChanged();
}

void DataPointStore::setCategories(std::vector<int>&& categories)
{
    Q_ASSERT(categories.empty() || categories.size() == static_cast<std::size_t>(count_));
    categories_ = std::move(categories);
}

void DataPointStore::append(const double* coordinates)
{
    promoteToPrecise();
    const auto columns = columnCount();
    const auto extendInPlace = renderValid_ && !hasDrawn_ && !skipInvalid_ && !hasAttributes_ && originX_.settled && originY_.settled
        && renderLogScaleX_ == x_.logScale && renderLogScaleY_ == y_.logScale
        && renderData_.size() == static_cast<std::size_t>(count_) * static_cast<std::size_t>(columns);

    precise_.insert(precise_.end(), coordinates, coordinates + columns);
    if (hasAttributes_) {
        attributes_.insert(attributes_.end(), static_cast<std::size_t>(layout_.attributeCount), std::numeric_limits<float>::quiet_NaN());
    }
    if (!categories_.empty()) {
        categories_.push_back(-1);
    }
    ++count_;
    compacted_.valid = false;

    if (!extendInPlace) {
        invalidateRender();
        return;
    }
    for (auto column = 0; column < columns; ++column) {
        const auto dimension = layout_.columns[static_cast<std::size_t>(column)];
        renderData_.push_back(static_cast<float>(coordinates[column] - originValue(dimension)));
    }
    ++renderRevision_;
}

void DataPointStore::promoteToPrecise()
{
    if (isPrecise_) {
        return;
    }
    const auto columns = static_cast<std::size_t>(columnCount());
    const auto stride = static_cast<std::size_t>(renderStride());
    precise_.resize(static_cast<std::size_t>(count_) * columns);
    if (hasAttributes_) {
        attributes_.resize(static_cast<std::size_t>(count_) * static_cast<std::size_t>(layout_.attributeCount));
    }
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(count_); ++i) {
        const auto* point = floats_.data() + i * stride;
        std::copy_n(point, columns, precise_.data() + i * columns);
        if (hasAttributes_) {
            std::copy_n(point + columns, layout_.attributeCount, attributes_.data() + i * static_cast<std::size_t>(layout_.attributeCount));
        }
    }
    floats_.clear();
    isPrecise_ = true;
    dataPointsChanged();
}

void DataPointStore::clear()
{
    precise_.clear();
    floats_.clear();
    attributes_.clear();
    categories_.clear();
    count_ = 0;
    isPrecise_ = false;
    hasAttributes_ = false;
    dataPointsChanged();
}

int DataPointStore::count() const noexcept
{
    return count_;
}

bool DataPointStore::isPrecise() const noexcept
{
    return isPrecise_;
}

bool DataPointStore::hasAttributes() const noexcept
{
    return hasAttributes_;
}

bool DataPointStore::hasCategories() const noexcept
{
    return !categories_.empty();
}

DataPointStore::View DataPointStore::view() const noexcept
{
    if (isPrecise_) {
        return View{precise_.data(), nullptr, columnCount(), count_};
    }
    return View{nullptr, floats_.data(), renderStride(), count_};
}

double DataPointStore::value(const int index, const int column) const noexcept
{
    return view().value(index, column);
}

DataPointStore::AttributeView DataPointStore::attributes() const noexcept
{
    if (!hasAttributes_) {
        return {};
    }
    if (isPrecise_) {
        return AttributeView{attributes_.data(), layout_.attributeCount};
    }
    return AttributeView{floats_.data() + columnCount(), renderStride()};
}

float DataPointStore::attribute(const int index, const int which) const noexcept
{
    const auto view = attributes();
    if (!view.data || index < 0 || index >= count_ || which < 0 || which >= layout_.attributeCount) {
        return std::numeric_limits<float>::quiet_NaN();
    }
    return view.data[static_cast<std::size_t>(index) * static_cast<std::size_t>(view.stride) + static_cast<std::size_t>(which)];
}

const std::vector<int>& DataPointStore::categories() const noexcept
{
    return categories_;
}

DataPointStore::Extents DataPointStore::extents(const bool logScaleX, const bool logScaleY, const int first, const int last) const
{
    const auto data = view();
    const auto begin = std::clamp(first, 0, count_);
    const auto end = last < 0 ? count_ : std::clamp(last, begin, count_);
    const auto columns = columnCount();
    const auto logScaleOf = [&](const int column) { return layout_.columns[static_cast<std::size_t>(column)] == Dimension::X ? logScaleX : logScaleY; };

    auto columnBounds = std::array<Bounds, kMaxColumns>{};
    auto checked = true;
    if (layout_.rangeRule == RangeRule::EachCoordinate) {
        if (data.precise) {
            scanUnchecked(data.precise, data.stride, begin, end, columns, columnBounds);
        } else {
            scanUnchecked(data.floats, data.stride, begin, end, columns, columnBounds);
        }
        checked = false;
        for (auto column = 0; column < columns; ++column) {
            if (!fastBoundsUsable(columnBounds[column], logScaleOf(column))) {
                checked = true;
                break;
            }
        }
    }

    if (checked) {
        columnBounds.fill(Bounds{});
        for (auto i = begin; i < end; ++i) {
            auto hasNaN = false;
            auto allValid = true;
            for (auto column = 0; column < columns; ++column) {
                const auto value = data.value(i, column);
                hasNaN = hasNaN || std::isnan(value);
                allValid = allValid && isValidSample(value, logScaleOf(column));
            }
            if ((layout_.rangeRule == RangeRule::WholeDataPoint && !allValid) || (layout_.rangeRule == RangeRule::SkipOnNaN && hasNaN)) {
                continue;
            }
            for (auto column = 0; column < columns; ++column) {
                const auto value = data.value(i, column);
                if (isValidSample(value, logScaleOf(column))) {
                    columnBounds[column].include(value);
                }
            }
        }
    }

    auto x = Bounds{};
    auto y = Bounds{};
    for (auto column = 0; column < columns; ++column) {
        const auto& bounds = columnBounds[column];
        if (bounds.min > bounds.max) {
            continue;
        }
        auto& target = layout_.columns[static_cast<std::size_t>(column)] == Dimension::X ? x : y;
        target.include(bounds.min);
        target.include(bounds.max);
    }
    return Extents{x.extent(), y.extent()};
}

std::vector<double> DataPointStore::setDrawn(std::vector<double>&& dataPoints, const int count)
{
    Q_ASSERT(dataPoints.size() == static_cast<std::size_t>(count) * static_cast<std::size_t>(renderStride()));
    auto previous = std::exchange(drawn_, std::move(dataPoints));
    drawnCount_ = count;
    hasDrawn_ = true;
    compacted_.valid = false;
    invalidateRender();
    return previous;
}

void DataPointStore::clearDrawn()
{
    if (!hasDrawn_) {
        return;
    }
    drawn_.clear();
    drawnCount_ = 0;
    hasDrawn_ = false;
    compacted_.valid = false;
    invalidateRender();
}

bool DataPointStore::hasDrawn() const noexcept
{
    return hasDrawn_;
}

DataPointStore::View DataPointStore::drawnView() const
{
    ensureCompacted();
    if (!compacted_.active) {
        return sourceView();
    }
    const auto source = sourceView();
    if (source.precise) {
        return View{compacted_.precise.data(), nullptr, source.stride, compacted_.count};
    }
    return View{nullptr, compacted_.floats.data(), source.stride, compacted_.count};
}

void DataPointStore::setSkipInvalid(const bool skip)
{
    if (skipInvalid_ == skip) {
        return;
    }
    skipInvalid_ = skip;
    compacted_.valid = false;
    invalidateRender();
}

bool DataPointStore::skipInvalid() const noexcept
{
    return skipInvalid_;
}

void DataPointStore::setDimensionState(const Dimension dimension, const DimensionState& state)
{
    auto& current = dimension == Dimension::X ? x_ : y_;
    const auto scaleChanged = current.logScale != state.logScale;
    current = state;
    if (scaleChanged) {
        compacted_.valid = false;
        invalidateRender();
        return;
    }
    const auto& renderOrigin = dimension == Dimension::X ? originX_ : originY_;
    if (renderValid_ && usesOrigin() && !state.logScale && state.viewport && renderOriginTooFar(renderOrigin.value, state.viewport->min, state.viewport->max)) {
        invalidateRender();
    }
}

const DataPointStore::DimensionState& DataPointStore::dimensionState(const Dimension dimension) const noexcept
{
    return dimension == Dimension::X ? x_ : y_;
}

const std::vector<float>& DataPointStore::renderData() const
{
    ensureCompacted();
    if (!usesOrigin()) {
        return compacted_.active ? compacted_.floats : floats_;
    }
    ensureRenderData();
    return renderData_;
}

int DataPointStore::renderStride() const noexcept
{
    return columnCount() + (hasAttributes_ ? layout_.attributeCount : 0);
}

int DataPointStore::renderCount() const
{
    ensureCompacted();
    if (compacted_.active) {
        return compacted_.count;
    }
    return hasDrawn_ ? drawnCount_ : count_;
}

double DataPointStore::renderOrigin(const Dimension dimension) const
{
    if (!usesOrigin()) {
        return 0.0;
    }
    ensureCompacted();
    ensureRenderData();
    return originValue(dimension);
}

std::uint64_t DataPointStore::renderRevision() const noexcept
{
    return renderRevision_;
}

DataPointStore::View DataPointStore::sourceView() const noexcept
{
    if (hasDrawn_) {
        return View{drawn_.data(), nullptr, renderStride(), drawnCount_};
    }
    return view();
}

bool DataPointStore::usesOrigin() const noexcept
{
    return isPrecise_ || hasDrawn_;
}

bool DataPointStore::dataPointValid(const View& view, const int index) const noexcept
{
    for (auto column = 0; column < columnCount(); ++column) {
        const auto logScale = layout_.columns[static_cast<std::size_t>(column)] == Dimension::X ? x_.logScale : y_.logScale;
        if (!isValidSample(view.value(index, column), logScale)) {
            return false;
        }
    }
    return true;
}

DataPointStore::Origin& DataPointStore::origin(const Dimension dimension) const noexcept
{
    return dimension == Dimension::X ? originX_ : originY_;
}

double DataPointStore::originValue(const Dimension dimension) const noexcept
{
    return dimension == Dimension::X ? originX_.value : originY_.value;
}

void DataPointStore::storeFloatCoordinates(std::vector<float>&& coordinates, std::vector<float>&& attributes)
{
    precise_.clear();
    attributes_.clear();
    isPrecise_ = false;
    hasAttributes_ = !attributes.empty() && layout_.attributeCount > 0;
    if (!hasAttributes_) {
        floats_ = std::move(coordinates);
        return;
    }
    const auto columns = static_cast<std::size_t>(columnCount());
    const auto attributeCount = static_cast<std::size_t>(layout_.attributeCount);
    const auto stride = columns + attributeCount;
    // Interleaves into the existing buffer, whose capacity is reused across same-sized updates.
    floats_.resize(static_cast<std::size_t>(count_) * stride);
    const auto* coordinate = coordinates.data();
    const auto* attribute = attributes.data();
    auto* out = floats_.data();
    if (columns == 2 && attributeCount == 1) {
        // The layout of a point cloud with values, unrolled because it streams every frame.
        for (auto i = std::size_t{0}; i < static_cast<std::size_t>(count_); ++i) {
            out[i * 3] = coordinate[i * 2];
            out[i * 3 + 1] = coordinate[i * 2 + 1];
            out[i * 3 + 2] = attribute[i];
        }
        return;
    }
    for (auto i = std::size_t{0}; i < static_cast<std::size_t>(count_); ++i) {
        for (auto column = std::size_t{0}; column < columns; ++column) {
            *out++ = *coordinate++;
        }
        for (auto which = std::size_t{0}; which < attributeCount; ++which) {
            *out++ = *attribute++;
        }
    }
}

void DataPointStore::dataPointsChanged()
{
    if (categories_.size() != static_cast<std::size_t>(count_)) {
        categories_.clear();
    }
    if (hasDrawn_ && drawn_.size() != static_cast<std::size_t>(drawnCount_) * static_cast<std::size_t>(renderStride())) {
        drawn_.clear();
        drawnCount_ = 0;
        hasDrawn_ = false;
    }
    compacted_.valid = false;
    invalidateRender();
}

void DataPointStore::invalidateRender()
{
    renderValid_ = false;
    ++renderRevision_;
}

void DataPointStore::ensureCompacted() const
{
    if (!skipInvalid_) {
        compacted_.active = false;
        return;
    }
    if (compacted_.valid) {
        return;
    }
    compacted_.valid = true;
    const auto source = sourceView();
    auto invalidCount = 0;
    for (auto i = 0; i < source.count; ++i) {
        invalidCount += dataPointValid(source, i) ? 0 : 1;
    }
    compacted_.active = invalidCount > 0;
    compacted_.precise.clear();
    compacted_.floats.clear();
    compacted_.attributes.clear();
    compacted_.count = source.count - invalidCount;
    if (!compacted_.active) {
        return;
    }

    const auto stride = static_cast<std::size_t>(source.stride);
    const auto storedAttributes = source.precise && !hasDrawn_ && hasAttributes_;
    const auto attributeCount = static_cast<std::size_t>(layout_.attributeCount);
    if (source.precise) {
        compacted_.precise.reserve(static_cast<std::size_t>(compacted_.count) * stride);
    } else {
        compacted_.floats.reserve(static_cast<std::size_t>(compacted_.count) * stride);
    }
    for (auto i = 0; i < source.count; ++i) {
        if (!dataPointValid(source, i)) {
            continue;
        }
        const auto offset = static_cast<std::size_t>(i) * stride;
        if (source.precise) {
            compacted_.precise.insert(compacted_.precise.end(), source.precise + offset, source.precise + offset + stride);
        } else {
            compacted_.floats.insert(compacted_.floats.end(), source.floats + offset, source.floats + offset + stride);
        }
        if (storedAttributes) {
            const auto* first = attributes_.data() + static_cast<std::size_t>(i) * attributeCount;
            compacted_.attributes.insert(compacted_.attributes.end(), first, first + attributeCount);
        }
    }
}

void DataPointStore::ensureRenderData() const
{
    if (renderValid_) {
        return;
    }
    renderValid_ = true;
    const auto source = sourceView();
    chooseOrigins(source);
    const auto storedAttributes = !hasDrawn_ && hasAttributes_;
    if (compacted_.active) {
        const auto compacted = View{compacted_.precise.data(), nullptr, source.stride, compacted_.count};
        writeRenderData(compacted, storedAttributes ? compacted_.attributes.data() : nullptr, layout_.attributeCount);
        return;
    }
    writeRenderData(source, storedAttributes ? attributes_.data() : nullptr, layout_.attributeCount);
}

void DataPointStore::chooseOrigins(const View& source) const
{
    renderLogScaleX_ = x_.logScale;
    renderLogScaleY_ = y_.logScale;
    // Log-scale values aren't translation-invariant, so a log-scale dimension is never shifted.
    originX_ = Origin{0.0, x_.logScale};
    originY_ = Origin{0.0, y_.logScale};
    for (auto i = 0; i < source.count && !(originX_.settled && originY_.settled); ++i) {
        for (auto column = 0; column < columnCount(); ++column) {
            auto& candidate = origin(layout_.columns[static_cast<std::size_t>(column)]);
            const auto value = source.value(i, column);
            if (!candidate.settled && std::isfinite(value)) {
                candidate = Origin{value, true};
            }
        }
    }
    if (!x_.logScale && x_.viewport) {
        originX_.value = renderOriginForViewport(originX_.value, x_.viewport->min, x_.viewport->max);
    }
    if (!y_.logScale && y_.viewport) {
        originY_.value = renderOriginForViewport(originY_.value, y_.viewport->min, y_.viewport->max);
    }
}

void DataPointStore::writeRenderData(const View& source, const float* attributes, const int attributeStride) const
{
    const auto columns = columnCount();
    const auto stride = renderStride();
    auto columnOrigins = std::array<double, kMaxColumns>{};
    for (auto column = 0; column < columns; ++column) {
        columnOrigins[column] = originValue(layout_.columns[static_cast<std::size_t>(column)]);
    }
    renderData_.resize(static_cast<std::size_t>(source.count) * static_cast<std::size_t>(stride));
    auto* out = renderData_.data();
    for (auto i = 0; i < source.count; ++i) {
        for (auto column = 0; column < columns; ++column) {
            *out++ = static_cast<float>(source.value(i, column) - columnOrigins[column]);
        }
        if (attributes) {
            const auto* first = attributes + static_cast<std::size_t>(i) * static_cast<std::size_t>(attributeStride);
            out = std::copy_n(first, stride - columns, out);
        } else {
            // Drawn data points carry their attributes after the coordinates.
            for (auto extra = columns; extra < stride; ++extra) {
                *out++ = static_cast<float>(source.value(i, extra));
            }
        }
    }
}

} // namespace QAccelPlot
