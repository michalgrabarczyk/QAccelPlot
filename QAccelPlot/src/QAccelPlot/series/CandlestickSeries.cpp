//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/series/CandlestickSeries.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/effects/Glow.hpp"
#include "QAccelPlot/legend/LegendEntry.hpp"
#include "QAccelPlot/materials/CandlestickMaterial.hpp"
#include "QAccelPlot/series/internal/InterleavedData.hpp"
#include "QAccelPlot/series/internal/RecordHitIndex.hpp"
#include "QAccelPlot/series/internal/RecordNode.hpp"
#include "QAccelPlot/series/internal/RectGeometry.hpp"
#include "QAccelPlot/series/internal/SeriesEffectList.hpp"
#include "QAccelPlot/series/internal/SeriesSupport.hpp"
#include "QAccelPlot/series/internal/StoreTransition.hpp"
#include "QAccelPlot/theme/ColorPalette.hpp"

#include <QHoverEvent>
#include <QQuickWindow>

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <utility>

namespace QAccelPlot {

namespace {

// Values per data point: x, open, high, low, close.
constexpr auto kStride = 5;
constexpr auto kOpenColumn = 1;
constexpr auto kHighColumn = 2;
constexpr auto kLowColumn = 3;
constexpr auto kCloseColumn = 4;
constexpr auto kVertexBytesPerCandle = CandlestickMaterial::kVerticesPerCandle * 4;
constexpr auto kNaN = std::numeric_limits<double>::quiet_NaN();
constexpr auto kInf = std::numeric_limits<double>::infinity();

DataPointStore::Layout candleLayout()
{
    using Dimension = DataPointStore::Dimension;
    return DataPointStore::Layout{{Dimension::X, Dimension::Y, Dimension::Y, Dimension::Y, Dimension::Y}, DataPointStore::RangeRule::EachCoordinate, 0};
}

// Whether candle \a index of \a view is drawn: a valid x, and an open and a close that are not NaN.
bool candleDrawn(const DataPointStore::View& view, const int index, const bool logScaleX)
{
    return isValidCoordinate(view.value(index, 0), logScaleX) && !std::isnan(view.value(index, kOpenColumn)) && !std::isnan(view.value(index, kCloseColumn));
}

void widen(std::optional<DataPointStore::Extent>& extent, const double min, const double max)
{
    extent = extent ? DataPointStore::Extent{std::min(extent->min, min), std::max(extent->max, max)} : DataPointStore::Extent{min, max};
}

// The x span of a candle at \a x, \a halfWidth to each side; a left edge outside a log axis'
// domain moves to \a x. Matches candlestick.vert.
std::pair<double, double> candleSpan(const double x, const double halfWidth, const bool logScaleX)
{
    const auto left = x - halfWidth;
    return {isValidCoordinate(left, logScaleX) ? left : x, x + halfWidth};
}

// The extents of the valid coordinates of candle \a index, with x widened to its candleSpan().
DataPointStore::Extents candleExtents(const DataPointStore::View& view, const int index, const bool logScaleX, const bool logScaleY, const double halfWidth)
{
    auto extents = DataPointStore::Extents{};
    const auto x = view.value(index, 0);
    if (isValidCoordinate(x, logScaleX)) {
        const auto [left, right] = candleSpan(x, halfWidth, logScaleX);
        widen(extents.x, left, right);
    }
    for (const auto column : {kOpenColumn, kHighColumn, kLowColumn, kCloseColumn}) {
        const auto value = view.value(index, column);
        if (isValidCoordinate(value, logScaleY)) {
            widen(extents.y, value, value);
        }
    }
    return extents;
}

std::optional<PlotSeries::DataExtent> toDataExtent(const std::optional<DataPointStore::Extent>& extent)
{
    if (!extent) {
        return std::nullopt;
    }
    return PlotSeries::DataExtent{extent->min, extent->max};
}

} // namespace

CandlestickSeries::CandlestickSeries(QQuickItem* parent)
    : PlotSeries(candleLayout(), DataPointWording{"data point", "candles", "values", "values"}, parent)
    , upColor_(ColorPalette::dark().candleUp)
    , downColor_(ColorPalette::dark().candleDown)
    , transition_(std::make_unique<TransitionRunner>(this))
    , storeTransition_(std::make_unique<Internal::StoreTransition>(*transition_, dataPoints()))
    , hitIndex_(std::make_unique<Internal::RecordHitIndex>())
{
    setFlag(ItemHasContents, true);
    setAntialiasing(true);
    supportHover();
    setAcceptedMouseButtons(Qt::NoButton);
    setLegendSymbol(LegendSymbol::Candle);
    connect(transition_.get(), &TransitionRunner::frameDue, this, &CandlestickSeries::advanceTransition);
    connect(transition_.get(), &TransitionRunner::interrupted, this, &CandlestickSeries::endTransition);
    transition_->notifyTransitionChanges(this);
    connect(transition_.get(), &TransitionRunner::activeChanged, this, [this] { setAnimating(transition_->active()); });
    connect(this, &CandlestickSeries::upColorChanged, this, &PlotSeries::legendEntryChanged);
    connect(this, &CandlestickSeries::downColorChanged, this, &PlotSeries::legendEntryChanged);
    connect(this, &CandlestickSeries::lineWidthChanged, this, &PlotSeries::legendEntryChanged);
    connect(this, &QQuickItem::antialiasingChanged, this, &PlotSeries::legendEntryChanged);
}

CandlestickSeries::~CandlestickSeries()
{
    // runningChanged handlers can still read the series until QObject destruction.
    transition_->cancel();
}

CandlestickSeries::Style CandlestickSeries::style() const
{
    return style_;
}

void CandlestickSeries::setStyle(const Style style)
{
    if (style_ == style) {
        return;
    }
    style_ = style;
    emit styleChanged();
    // Marks the item dirty so the window re-delivers hover at the cursor, which may now miss the candle.
    update();
}

QColor CandlestickSeries::upColor() const
{
    return upColor_;
}

void CandlestickSeries::setUpColor(const QColor& color)
{
    if (upColor_ == color) {
        return;
    }
    upColor_ = color;
    emit upColorChanged();
    update();
}

QColor CandlestickSeries::downColor() const
{
    return downColor_;
}

void CandlestickSeries::setDownColor(const QColor& color)
{
    if (downColor_ == color) {
        return;
    }
    downColor_ = color;
    emit downColorChanged();
    update();
}

qreal CandlestickSeries::candleWidth() const
{
    return candleWidth_;
}

void CandlestickSeries::setCandleWidth(const qreal width)
{
    if (!Internal::assignNonNegative(candleWidth_, width)) {
        return;
    }
    onGeometryChanged();
    emit candleWidthChanged();
}

qreal CandlestickSeries::spacing() const
{
    if (!automaticSpacing()) {
        return pinnedSpacing_;
    }
    ensureAutoSpacing();
    return autoSpacing_.value();
}

void CandlestickSeries::setSpacing(const qreal spacing)
{
    if (!std::isfinite(spacing) || spacing <= 0.0) {
        resetSpacing();
        return;
    }
    if (!automaticSpacing() && nearlyEqual(pinnedSpacing_, spacing)) {
        return;
    }
    pinnedSpacing_ = spacing;
    onGeometryChanged();
    emit spacingChanged();
}

void CandlestickSeries::resetSpacing()
{
    if (automaticSpacing()) {
        return;
    }
    pinnedSpacing_ = kNaN;
    onGeometryChanged();
    emit spacingChanged();
}

qreal CandlestickSeries::lineWidth() const
{
    return lineWidth_;
}

void CandlestickSeries::setLineWidth(const qreal width)
{
    if (!Internal::assignNonNegative(lineWidth_, width)) {
        return;
    }
    emit lineWidthChanged();
    update();
}

qreal CandlestickSeries::minimumWidth() const
{
    return minimumWidth_;
}

void CandlestickSeries::setMinimumWidth(const qreal width)
{
    if (!Internal::assignNonNegative(minimumWidth_, width)) {
        return;
    }
    emit minimumWidthChanged();
    update();
}

QColor CandlestickSeries::hoverColor() const
{
    return hoverColor_;
}

void CandlestickSeries::setHoverColor(const QColor& color)
{
    if (hoverColor_ == color) {
        return;
    }
    hoverColor_ = color;
    emit hoverColorChanged();
    update();
}

qreal CandlestickSeries::hoverRadius() const
{
    return hoverRadius_;
}

void CandlestickSeries::setHoverRadius(const qreal radius)
{
    if (!Internal::assignNonNegative(hoverRadius_, radius)) {
        return;
    }
    emit hoverRadiusChanged();
    // Marks the item dirty so the window re-delivers hover at the cursor and updates hoveredIndex.
    update();
}

EnterExitTransition* CandlestickSeries::enterTransition() const
{
    return qobject_cast<EnterExitTransition*>(transition_->transition(TransitionRunner::Slot::Enter));
}

void CandlestickSeries::setEnterTransition(EnterExitTransition* transition)
{
    transition_->setTransition(TransitionRunner::Slot::Enter, transition);
}

DataTransition* CandlestickSeries::updateTransition() const
{
    return transition_->transition(TransitionRunner::Slot::Update);
}

void CandlestickSeries::setUpdateTransition(DataTransition* transition)
{
    transition_->setTransition(TransitionRunner::Slot::Update, transition);
}

EnterExitTransition* CandlestickSeries::exitTransition() const
{
    return qobject_cast<EnterExitTransition*>(transition_->transition(TransitionRunner::Slot::Exit));
}

void CandlestickSeries::setExitTransition(EnterExitTransition* transition)
{
    transition_->setTransition(TransitionRunner::Slot::Exit, transition);
}

int CandlestickSeries::hoveredIndex() const
{
    return hoveredIndex_;
}

std::optional<LegendEntry> CandlestickSeries::legendEntry() const
{
    auto entry = LegendEntry{};
    entry.symbol = legendSymbol();
    entry.color = upColor_;
    entry.secondaryColor = downColor_;
    entry.lineWidth = lineWidth_;
    entry.antialiasing = antialiasing();
    return entry;
}

void CandlestickSeries::appendData(const qreal x, const qreal open, const qreal high, const qreal low, const qreal close)
{
    endTransition();
    const auto dataPoint = std::array<double, kStride>{x, open, high, low, close};
    appendDataPoint(dataPoint.data());
}

void CandlestickSeries::replaceData(const int index, const qreal x, const qreal open, const qreal high, const qreal low, const qreal close)
{
    if (!validateDataPointIndex(index)) {
        return;
    }
    endTransition();
    const auto dataPoint = std::array<double, kStride>{x, open, high, low, close};
    replaceDataPoint(index, dataPoint.data());
}

void CandlestickSeries::setData(
    const QList<qreal>& xs, const QList<qreal>& opens, const QList<qreal>& highs, const QList<qreal>& lows, const QList<qreal>& closes)
{
    applyInterleavedData(Internal::interleave({xs, opens, highs, lows, closes}));
}

void CandlestickSeries::setData(const std::vector<double>& xs, const std::vector<double>& opens, const std::vector<double>& highs,
    const std::vector<double>& lows, const std::vector<double>& closes)
{
    applyInterleavedData(Internal::interleave({xs, opens, highs, lows, closes}));
}

void CandlestickSeries::clearData()
{
    if (transition_->animates(count(), 0)) {
        // Every candle is removed, so the exit transition animates them.
        applyDoubleData({}, 0);
        return;
    }
    discardTransition();
    PlotSeries::clearData();
}

QVariantMap CandlestickSeries::candleAt(const int index) const
{
    if (index < 0 || index >= count()) {
        return {};
    }
    const auto& points = dataPoints();
    return {{QStringLiteral("x"), points.value(index, 0)}, {QStringLiteral("open"), points.value(index, kOpenColumn)},
        {QStringLiteral("high"), points.value(index, kHighColumn)}, {QStringLiteral("low"), points.value(index, kLowColumn)},
        {QStringLiteral("close"), points.value(index, kCloseColumn)}};
}

int CandlestickSeries::candleIndexAt(const QPointF& position) const
{
    if (count() <= 0 || !xAxis() || !yAxis() || width() <= 0.0 || height() <= 0.0) {
        return -1;
    }
    ensureSpatialGrid();
    // The spans hold the candle width in data units, so only the pixel minimums reach past them.
    const auto overhang = 0.5 * std::max(lineWidth_, minimumWidth_);
    const auto drawnCount = std::min(dataPoints().drawnView().count, renderCapacity_);
    return hitIndex_->nearest({position, hoverRadius_, overhang, drawnCount}, *xAxis(), *yAxis(), size(),
        [this, &position](const int index) { return pixelDistance(index, position); });
}

bool CandlestickSeries::contains(const QPointF& point) const
{
    return boundingRect().contains(point) && candleIndexAt(point) >= 0;
}

bool CandlestickSeries::recordInspectionSupported() const
{
    return true;
}

InspectionRecord CandlestickSeries::inspectionRecord(const int index) const
{
    auto result = InspectionRecord{};
    if (index < 0 || index >= count()) {
        result.status = InspectionStatus::InvalidArgument;
        return result;
    }
    result.index = index;
    result.fields = candleAt(index);
    result.status = candleDrawn(dataPoints().view(), index, logScaleX()) ? InspectionStatus::Ready : InspectionStatus::NoMatch;
    return result;
}

InspectionRecord CandlestickSeries::inspectionRecordAt(const QPointF& position) const
{
    const auto index = candleIndexAt(position);
    return index < 0 ? InspectionRecord{} : inspectionRecord(index);
}

QSGNode* CandlestickSeries::updatePaintNode(QSGNode* oldNode, [[maybe_unused]] UpdatePaintNodeData* updatePaintNodeData)
{
    auto* window = this->window();
    updateRenderCapacity(Internal::recordCapacity(window, kStride, kVertexBytesPerCandle), count());
    const auto drawnCount = std::min(dataPoints().renderCount(), renderCapacity_);
    const auto drawable = drawnCount > 0 && xAxis() && yAxis() && !resolvePlotRect().isEmpty() && window;
    const auto frame = Internal::RecordFrame{window, drawnCount, drawable, kStride, CandlestickMaterial::kVerticesPerCandle, "CandlestickSeries"};
    return Internal::updateRecordNode<CandlestickMaterial>(
        oldNode, frame, dataPoints(), uploadedRevision_, [this, drawnCount](CandlestickMaterial& material) { updateMaterial(material, drawnCount); });
}

void CandlestickSeries::hoverEnterEvent(QHoverEvent* event)
{
    setHoveredIndex(candleIndexAt(event->position()));
    QQuickItem::hoverEnterEvent(event);
}

void CandlestickSeries::hoverMoveEvent(QHoverEvent* event)
{
    setHoveredIndex(candleIndexAt(event->position()));
    QQuickItem::hoverMoveEvent(event);
}

void CandlestickSeries::hoverLeaveEvent(QHoverEvent* event)
{
    setHoveredIndex(-1);
    QQuickItem::hoverLeaveEvent(event);
}

void CandlestickSeries::applyDataPoints(DataPointInput&& input)
{
    if (transition_->animates(count(), input.count)) {
        // Transitions animate doubles.
        const auto dataPointCount = input.count;
        applyDoubleData(std::move(input).toDoubles(), dataPointCount);
        return;
    }
    discardTransition();
    storeDataPoints(std::move(input));
}

bool CandlestickSeries::supportsEffect(const SeriesEffect& effect) const
{
    return qobject_cast<const Glow*>(&effect) != nullptr;
}

void CandlestickSeries::onAxisScaleChanged()
{
    // Log scale changes which data points are drawn.
    hitIndex_->invalidate();
    update();
}

bool CandlestickSeries::updateDataRangeInputs(const DataUpdate& dataUpdate)
{
    // Nothing read the default spacing since it was last discarded and announced.
    if (!autoSpacing_.valid) {
        return false;
    }
    const auto previousSpacing = autoSpacing_.value();
    updateAutoSpacing(dataUpdate);
    if (!automaticSpacing()) {
        return false;
    }
    if (dataUpdate.change == DataChange::Replaced) {
        // Discarded and scanned for only when read; PlotSeries rescans replaced data anyway.
        spacingChangePending_ = true;
        return false;
    }
    ensureAutoSpacing();
    spacingChangePending_ = autoSpacing_.value() != previousSpacing;
    // The data ranges hold half a candle of the previous spacing around every candle.
    return spacingChangePending_;
}

void CandlestickSeries::onDataPointsChanged([[maybe_unused]] const DataUpdate& dataUpdate)
{
    hitIndex_->invalidate();
    if (hoveredIndex_ >= count()) {
        setHoveredIndex(-1);
    }
    if (std::exchange(spacingChangePending_, false)) {
        emit spacingChanged();
    }
}

PlotSeries::DataRanges CandlestickSeries::computeDataRanges() const
{
    const auto view = dataPoints().view();
    const auto logX = logScaleX();
    const auto logY = logScaleY();
    const auto halfWidth = halfCandleWidth();
    auto extents = DataPointStore::Extents{};
    for (auto i = 0; i < view.count; ++i) {
        const auto candle = candleExtents(view, i, logX, logY, halfWidth);
        if (candle.x) {
            widen(extents.x, candle.x->min, candle.x->max);
        }
        if (candle.y) {
            widen(extents.y, candle.y->min, candle.y->max);
        }
    }
    return {toDataExtent(extents.x), toDataExtent(extents.y)};
}

DataPointStore::Extents CandlestickSeries::dataPointExtents(const int index) const
{
    return candleExtents(dataPoints().view(), index, logScaleX(), logScaleY(), halfCandleWidth());
}

void CandlestickSeries::AutoSpacing::include(const double x)
{
    if (!std::isfinite(x)) {
        return;
    }
    if (std::isfinite(lastX)) {
        const auto gap = std::abs(x - lastX);
        if (gap > 0.0) {
            minimumGap = std::min(minimumGap, gap);
        }
    }
    lastX = x;
}

double CandlestickSeries::AutoSpacing::value() const
{
    return std::isfinite(minimumGap) ? minimumGap : 1.0;
}

bool CandlestickSeries::automaticSpacing() const
{
    return std::isnan(pinnedSpacing_);
}

void CandlestickSeries::ensureAutoSpacing() const
{
    if (autoSpacing_.valid) {
        return;
    }
    const auto view = dataPoints().view();
    auto spacing = AutoSpacing{};
    for (auto i = 0; i < view.count; ++i) {
        spacing.include(view.value(i, 0));
    }
    spacing.count = view.count;
    spacing.valid = true;
    autoSpacing_ = spacing;
}

double CandlestickSeries::halfCandleWidth() const
{
    return 0.5 * candleWidth_ * spacing();
}

void CandlestickSeries::updateAutoSpacing(const DataUpdate& dataUpdate)
{
    switch (dataUpdate.change) {
    case DataChange::Replaced:
        autoSpacing_.valid = false;
        break;
    case DataChange::Appended:
        if (autoSpacing_.count == dataUpdate.previousCount) {
            autoSpacing_.include(dataPoints().value(dataUpdate.index, 0));
            autoSpacing_.count = count();
        } else {
            autoSpacing_.valid = false;
        }
        break;
    case DataChange::Modified:
        if (dataUpdate.previousCoordinates[0] != dataPoints().value(dataUpdate.index, 0)) {
            autoSpacing_.valid = false;
        }
        break;
    }
}

std::optional<CandlestickSeries::CandleBoxes> CandlestickSeries::pixelBoxes(const int index) const
{
    const auto view = dataPoints().drawnView();
    if (!candleDrawn(view, index, logScaleX())) {
        return std::nullopt;
    }
    const auto& xAxis = *this->xAxis();
    const auto& yAxis = *this->yAxis();
    const auto x = view.value(index, 0);
    const auto open = view.value(index, kOpenColumn);
    const auto close = view.value(index, kCloseColumn);
    const auto high = view.value(index, kHighColumn);
    const auto low = view.value(index, kLowColumn);
    const auto [left, right] = candleSpan(x, halfCandleWidth(), logScaleX());

    const auto center = xAxis.coordToPixel(x, width());
    const auto candle = Internal::widenedSpan(Internal::edgePixel(left, xAxis, width()), Internal::edgePixel(right, xAxis, width()), minimumWidth_);
    const auto openPixel = Internal::edgePixel(open, yAxis, height());
    const auto closePixel = Internal::edgePixel(close, yAxis, height());
    // A NaN high or low leaves out that half of the wick, which then ends at the body.
    const auto wickHigh = Internal::edgePixel(std::isnan(high) ? std::max(open, close) : high, yAxis, height());
    const auto wickLow = Internal::edgePixel(std::isnan(low) ? std::min(open, close) : low, yAxis, height());
    const auto halfLine = 0.5 * lineWidth_;

    auto result = CandleBoxes{};
    const auto add = [&result](const PixelBox& box) { result.boxes[static_cast<std::size_t>(result.count++)] = box; };
    if (lineWidth_ > 0.0) {
        add({center - halfLine, center + halfLine, std::min(wickHigh, wickLow), std::max(wickHigh, wickLow)});
    }
    if (style_ == Style::Candlestick) {
        const auto body = Internal::widenedSpan(openPixel, closePixel, lineWidth_);
        add({candle.first, candle.second, body.first, body.second});
    } else if (lineWidth_ > 0.0) {
        add({candle.first, center, openPixel - halfLine, openPixel + halfLine});
        add({center, candle.second, closePixel - halfLine, closePixel + halfLine});
    }
    return result;
}

std::optional<qreal> CandlestickSeries::pixelDistance(const int index, const QPointF& position) const
{
    const auto boxes = pixelBoxes(index);
    if (!boxes || boxes->count == 0) {
        return std::nullopt;
    }
    auto distance = kInf;
    for (auto i = 0; i < boxes->count; ++i) {
        const auto& box = boxes->boxes[static_cast<std::size_t>(i)];
        distance = std::min(distance, Internal::distanceToBox(position.x(), position.y(), {box.left, box.right}, {box.top, box.bottom}));
    }
    return distance;
}

void CandlestickSeries::applyInterleavedData(std::vector<double>&& data)
{
    const auto dataPointCount = static_cast<int>(data.size() / kStride);
    setData(std::move(data), dataPointCount);
}

void CandlestickSeries::applyDoubleData(std::vector<double>&& data, const int dataPointCount)
{
    const auto previousCount = count();
    if (storeTransition_->start(std::move(data), dataPointCount, transitionLayout())) {
        advanceTransition();
    } else {
        discardTransition();
    }
    notifyDataPointsReplaced(previousCount);
}

TransitionRunner::Layout CandlestickSeries::transitionLayout() const
{
    // The open, high, low, and close grow; without a GrowTransition baseline, from each candle's open.
    auto layout = TransitionRunner::Layout{kStride, {kOpenColumn, kHighColumn, kLowColumn, kCloseColumn}, Internal::growBaseline(0.0, yAxis()), false};
    layout.baselineColumn = kOpenColumn;
    layout.viewport = Internal::growViewport(yAxis());
    return layout;
}

void CandlestickSeries::advanceTransition()
{
    if (storeTransition_->advance()) {
        hitIndex_->invalidate();
        update();
    }
}

void CandlestickSeries::endTransition()
{
    if (storeTransition_->end()) {
        hitIndex_->invalidate();
        update();
    }
}

void CandlestickSeries::discardTransition()
{
    storeTransition_->discard();
    hitIndex_->invalidate();
}

void CandlestickSeries::setHoveredIndex(const int index)
{
    if (hoveredIndex_ == index) {
        return;
    }
    hoveredIndex_ = index;
    setHovered(index >= 0);
    emit hoveredIndexChanged();
    update();
}

void CandlestickSeries::onGeometryChanged()
{
    hitIndex_->invalidate();
    invalidateDataRanges();
    update();
}

void CandlestickSeries::ensureSpatialGrid() const
{
    if (hitIndex_->valid()) {
        return;
    }
    const auto drawn = dataPoints().drawnView();
    const auto logX = logScaleX();
    const auto halfWidth = halfCandleWidth();
    hitIndex_->build(drawn.count, Qt::Horizontal, [&](const int index) -> std::optional<std::pair<double, double>> {
        if (!candleDrawn(drawn, index, logX)) {
            return std::nullopt;
        }
        return candleSpan(drawn.value(index, 0), halfWidth, logX);
    });
}

void CandlestickSeries::updateMaterial(CandlestickMaterial& material, const int drawnCount) const
{
    const auto originX = dataPoints().renderOrigin(DataPointStore::Dimension::X);
    const auto originY = dataPoints().renderOrigin(DataPointStore::Dimension::Y);
    const auto rect = resolvePlotRect();
    material.color = upColor_;
    material.downColor = downColor_;
    material.domainMin = QVector2D(static_cast<float>(xAxis()->viewportMin() - originX), static_cast<float>(yAxis()->viewportMin() - originY));
    material.domainMax = QVector2D(static_cast<float>(xAxis()->viewportMax() - originX), static_cast<float>(yAxis()->viewportMax() - originY));
    material.viewportSize = QVector2D(static_cast<float>(rect.width()), static_cast<float>(rect.height()));
    material.logScaleX = logScaleX() ? 1.0f : 0.0f;
    material.logScaleY = logScaleY() ? 1.0f : 0.0f;
    material.hoverColor = hoverColor_;
    // An invalid hover color highlights no candle.
    material.hoveredIndex = hoverColor_.isValid() ? static_cast<float>(hoveredIndex_) : -1.0f;
    material.lineWidth = static_cast<float>(lineWidth_);
    material.halfWidth = static_cast<float>(halfCandleWidth());
    material.minimumWidth = static_cast<float>(minimumWidth_);
    material.ohlc = style_ == Style::Ohlc ? 1.0f : 0.0f;
    material.candleCount = static_cast<float>(drawnCount);
    material.itemFade = transition_->fade();
    material.glow = effectList().glowStyle(upColor_);
    material.glowDownColor = effectList().glowStyle(downColor_).color;
    material.antialiasing = antialiasing() ? 1.0f : 0.0f;
}

} // namespace QAccelPlot
