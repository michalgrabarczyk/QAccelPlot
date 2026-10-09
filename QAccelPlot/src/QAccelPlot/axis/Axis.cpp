//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/axis/AxisTickPainter.hpp"
#include "QAccelPlot/axis/internal/RangeGesture.hpp"
#include "QAccelPlot/series/PlotSeries.hpp"

#include <algorithm>
#include <limits>
#include <utility>

#include <QMouseEvent>
#include <QPainter>
#include <QPen>

#include <cmath>

namespace QAccelPlot {

namespace {

constexpr auto kMinZoomScaleFactor = 0.0;
constexpr auto kMaxZoomScaleFactor = 1.0;
constexpr auto kLogScaleMinPositiveValue = 1e-10;
// Multiplicative factor used to synthesize a valid log-scale range from a single positive value
// (e.g. a flat-data viewport centered on it, or a fallback max above a clamped min).
constexpr auto kLogScaleRangeFactor = 10.0;
// Fraction of the value's own magnitude used as the linear half-range around a single value.
constexpr auto kFlatLinearRangeFraction = 0.1;
// Half-range used to synthesize a linear viewport when the value is exactly zero, where a
// magnitude-relative range would degenerate to zero width.
constexpr auto kFlatLinearZeroHalfRange = 1.0;
// Data range of an axis whose series have no data.
constexpr auto kFallbackDataMin = 0.0;
constexpr auto kFallbackDataMax = 1.0;
constexpr auto kHorizontalLabelOverflow = 25.0;
constexpr auto kVerticalLabelOverflow = 10.0;

bool hoverEnabled()
{
    auto isInteger = false;
    const auto value = qEnvironmentVariableIntValue("QACCELPLOT_HOVER_ENABLED", &isInteger);
    return !isInteger || value != 0;
}

}

Axis::Axis(QQuickItem* parent, Side side)
    : QQuickPaintedItem(parent)
    , ticker_(new AxisTicker(this))
{
    setAcceptHoverEvents(hoverEnabled());
    setAcceptedMouseButtons(Qt::LeftButton);
    connect(ticker_, &AxisTicker::tickColorChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::tickLabelColorChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::tickCountChanged, this, &Axis::invalidateTicks);
    connect(ticker_, &AxisTicker::subtickCountChanged, this, &Axis::invalidateTicks);
    connect(ticker_, &AxisTicker::tickLengthChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::subtickLengthChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::tickLengthInChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::tickLengthOutChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::subtickLengthInChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::subtickLengthOutChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::subtickColorChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::tickWidthChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::subtickWidthChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::tickLabelPaddingChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::tickLabelRotationChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::tickLabelFontChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::tickLabelFormatterChanged, this, &Axis::invalidateTicks);
    connect(ticker_, &AxisTicker::tickLabelFormatChanged, this, &Axis::invalidateTicks);
    setSide(side);
    invalidateTicks();
}

qreal Axis::viewportMin() const
{
    return viewportMin_;
}

void Axis::setViewportMin(const qreal m)
{
    if (nearly_equal(viewportMin_, m)) {
        return;
    }
    viewportMin_ = m;
    emit viewportMinChanged();
    emit rangeChanged();
    invalidateTicks();
}

qreal Axis::viewportMax() const
{
    return viewportMax_;
}

void Axis::setViewportMax(const qreal m)
{
    if (nearly_equal(viewportMax_, m)) {
        return;
    }
    viewportMax_ = m;
    emit viewportMaxChanged();
    emit rangeChanged();
    invalidateTicks();
}

qreal Axis::dataMin() const
{
    ensureDataRange();
    return dataMin_;
}

void Axis::setDataMin(const qreal m)
{
    ensureDataRange();
    if (nearly_equal(dataMin_, m)) {
        return;
    }
    dataMin_ = m;
    emit dataMinChanged();
    autoRescaleToData();
}

qreal Axis::dataMax() const
{
    ensureDataRange();
    return dataMax_;
}

void Axis::setDataMax(const qreal m)
{
    ensureDataRange();
    if (nearly_equal(dataMax_, m)) {
        return;
    }
    dataMax_ = m;
    emit dataMaxChanged();
    autoRescaleToData();
}

void Axis::setDataRange(const qreal min, const qreal max)
{
    dataRangeStale_ = false;
    setDataRangeValues(min, max);
    autoRescaleToData();
}

Axis::Orientation Axis::orientation() const
{
    return orientation_;
}

void Axis::setOrientation(Orientation o)
{
    if (orientation_ == o) {
        return;
    }
    orientation_ = o;
    emit orientationChanged();
    update();
}

QString Axis::label() const
{
    return label_;
}

void Axis::setLabel(const QString& t)
{
    if (label_ == t) {
        return;
    }
    label_ = t;
    emit labelChanged();
    update();
}

QFont Axis::labelFont() const
{
    return labelFont_;
}

void Axis::setLabelFont(const QFont& f)
{
    if (labelFont_ == f) {
        return;
    }
    labelFont_ = f;
    emit labelFontChanged();
    update();
}

Axis::Side Axis::side() const
{
    return side_;
}

void Axis::setSide(Side s)
{
    if (side_ == s) {
        return;
    }
    side_ = s;
    if (s == Left || s == Right) {
        setOrientation(Vertical);
    } else {
        setOrientation(Horizontal);
    }
    emit sideChanged();
    update();
}

bool Axis::hovered() const
{
    return hovered_;
}

QColor Axis::baselineColor() const
{
    return baselineColor_;
}

void Axis::setBaselineColor(const QColor& c)
{
    if (baselineColor_ == c) {
        return;
    }
    baselineColor_ = c;
    emit baselineColorChanged();
    update();
}

QColor Axis::labelColor() const
{
    return labelColor_;
}

void Axis::setLabelColor(const QColor& c)
{
    if (labelColor_ == c) {
        return;
    }
    labelColor_ = c;
    emit labelColorChanged();
    update();
}

qreal Axis::baselineWidth() const
{
    return baselineWidth_;
}

void Axis::setBaselineWidth(const qreal width)
{
    const auto clampedWidth = std::max(qreal{0.0}, width);
    if (nearly_equal(baselineWidth_, clampedWidth)) {
        return;
    }
    baselineWidth_ = clampedWidth;
    emit baselineWidthChanged();
    update();
}

QColor Axis::hoverColor() const
{
    return hoverColor_;
}

void Axis::setHoverColor(const QColor& c)
{
    if (hoverColor_ == c) {
        return;
    }
    hoverColor_ = c;
    emit hoverColorChanged();
    update();
}

int Axis::axisTitlePadding() const
{
    return axisTitlePadding_;
}

void Axis::setAxisTitlePadding(const int padding)
{
    if (axisTitlePadding_ == padding) {
        return;
    }
    axisTitlePadding_ = padding;
    emit axisTitlePaddingChanged();
    update();
}

int Axis::axisLinePadding() const
{
    return axisLinePadding_;
}

void Axis::setAxisLinePadding(const int padding)
{
    if (axisLinePadding_ == padding) {
        return;
    }
    axisLinePadding_ = padding;
    emit axisLinePaddingChanged();
    update();
}

qreal Axis::layoutSize() const
{
    return layoutSize_;
}

void Axis::setLayoutSize(const qreal size)
{
    const auto clampedSize = std::max(qreal{0.0}, size);
    if (nearly_equal(layoutSize_, clampedSize)) {
        return;
    }
    layoutSize_ = clampedSize;
    emit layoutSizeChanged();
}

AxisTicker* Axis::ticker() const
{
    return ticker_;
}

bool Axis::logScale() const
{
    return logScale_;
}

void Axis::setLogScale(const bool on)
{
    if (logScale_ == on) {
        return;
    }
    logScale_ = on;
    if (logScale_ && (viewportMin_ <= 0.0 || viewportMax_ <= 0.0)) {
        // A non-positive viewport is invalid for log scale — coordToPixel() returns 0
        // for every value in that case, collapsing every curve to a single point.
        const auto newMin = viewportMin_ > 0.0 ? viewportMin_ : kLogScaleMinPositiveValue;
        const auto newMax = viewportMax_ > newMin ? viewportMax_ : newMin * kLogScaleRangeFactor;
        setViewportMin(newMin);
        setViewportMax(newMax);
    }
    emit logScaleChanged();
    emit rangeChanged(); // force curves to rebuild
    invalidateTicks();
}

double Axis::zoomScaleFactor() const
{
    return zoomScaleFactor_;
}

void Axis::setZoomScaleFactor(const double factor)
{
    const auto clampedFactor = std::clamp(factor, kMinZoomScaleFactor, kMaxZoomScaleFactor);
    if (nearly_equal(zoomScaleFactor_, clampedFactor)) {
        return;
    }
    zoomScaleFactor_ = clampedFactor;
    emit zoomScaleFactorChanged();
}

bool Axis::autoRescale() const
{
    return autoRescale_;
}

void Axis::setAutoRescale(const bool on)
{
    if (autoRescale_ == on) {
        return;
    }
    autoRescale_ = on;
    emit autoRescaleChanged();
    autoRescaleToData();
}

void Axis::toggleLogScale()
{
    setLogScale(!logScale_);
}

void Axis::rescaleToData()
{
    ensureDataRange();
    const auto range = dataFitRange();
    setViewportRange(range.min, range.max);
}

void Axis::paint(QPainter* painter)
{
    const auto r = contentsBoundingRect();
    const auto labelOv = labelOverflow();

    // Data rect: area corresponding to data/ticks (excluding label overflow margins)
    const auto labelOverflowBothEnds = 2.0 * labelOv;
    auto dataRect = r;
    if (orientation_ == Horizontal) {
        dataRect = QRectF(labelOv, r.y(), r.width() - labelOverflowBothEnds, r.height());
    } else {
        dataRect = QRectF(r.x(), labelOv, r.width(), r.height() - labelOverflowBothEnds);
    }

    // Ensure the axis line is far enough from the item edge to show inward ticks
    const auto maxIn = std::max(static_cast<qreal>(ticker_->tickLengthIn()), static_cast<qreal>(ticker_->subtickLengthIn()));
    const auto pad = std::max(static_cast<qreal>(axisLinePadding_), maxIn);
    const auto overlap = pad - static_cast<qreal>(axisLinePadding_);

    // Fill only the axis area, not the overlap or label overflow regions
    auto bgRect = dataRect;
    if (overlap > 0) {
        if (orientation_ == Horizontal) {
            if (side_ == Bottom) {
                bgRect.setTop(bgRect.top() + overlap);
            } else {
                bgRect.setBottom(bgRect.bottom() - overlap);
            }
        } else {
            if (side_ == Left) {
                bgRect.setRight(bgRect.right() - overlap);
            } else {
                bgRect.setLeft(bgRect.left() + overlap);
            }
        }
    }

    // Determine axis position based on side with padding inward
    auto axisX = qreal{0};
    auto axisY = qreal{0};
    if (orientation_ == Horizontal) {
        axisY = (side_ == Bottom) ? pad : dataRect.height() - pad;
    } else {
        axisX = (side_ == Right) ? pad : dataRect.width() - pad;
    }

    auto axisPen = QPen(hovered_ ? hoverColor_ : baselineColor_);
    axisPen.setWidthF(baselineWidth_);
    painter->setPen(axisPen);

    // Draw baseline — only across data area
    const auto start = (orientation_ == Horizontal) ? QPointF{dataRect.x(), axisY} : QPointF{axisX, dataRect.y()};
    const auto end = (orientation_ == Horizontal) ? QPointF{dataRect.x() + dataRect.width(), axisY} : QPointF{axisX, dataRect.y() + dataRect.height()};
    painter->drawLine(start, end);

    auto tickPen = QPen(hovered_ ? hoverColor_ : ticker_->tickColor());
    tickPen.setWidthF(ticker_->tickWidth());
    painter->setPen(tickPen);

    auto tickPainterParams = AxisTickPainter::Params{};
    tickPainterParams.orientation = orientation_;
    tickPainterParams.side = side_;
    tickPainterParams.hovered = hovered_;
    tickPainterParams.ticker = ticker_;
    tickPainterParams.hoverColor = hoverColor_;
    tickPainterParams.defaultSubtickColor = ticker_->subtickColor();
    tickPainterParams.clampEdgeLabels = clampEdgeLabels_;
    tickPainterParams.labelOverflow = labelOv;

    // Only draw the ticks computed in updatePolish(): this may run on the render thread, where the
    // formatter (and any tickLabel JS callback) must not be called.
    AxisTickPainter::paintTicks(
        painter, dataRect, axisX, axisY, tickPainterParams, ticks_, [this](const auto value, const auto length) { return coordToPixel(value, length); });

    paintLabel(painter, dataRect, axisX, axisY);
}

qreal Axis::inwardTickOverlap() const
{
    const auto maxIn = std::max(static_cast<qreal>(ticker_->tickLengthIn()), static_cast<qreal>(ticker_->subtickLengthIn()));
    return std::max(0.0, maxIn - static_cast<qreal>(axisLinePadding_));
}

qreal Axis::labelOverflow() const
{
    if (!extendWidgetForLabels_) {
        return 0.0;
    }
    return (orientation_ == Horizontal) ? kHorizontalLabelOverflow : kVerticalLabelOverflow;
}

AxisMapping Axis::mapping() const
{
    auto result = AxisMapping{};
    result.logarithmic = logScale_;
    result.flipped = orientation_ == Vertical;
    if (logScale_ && (viewportMin_ <= 0 || viewportMax_ <= 0)) {
        return result;
    }
    result.origin = result.toMapped(viewportMin_);
    result.span = result.toMapped(viewportMax_) - result.origin;
    result.valid = std::isfinite(result.origin) && std::isfinite(result.span) && !nearly_equal(result.span, 0.0);
    return result;
}

qreal Axis::coordToPixel(const qreal value, const qreal length) const
{
    if (logScale_ && (value <= 0 || viewportMin_ <= 0 || viewportMax_ <= 0)) {
        qCDebug(lcQAccelPlot) << "non-positive value or viewport bounds (value=" << value << "viewportMin=" << viewportMin_ << "viewportMax=" << viewportMax_
                              << "), returning 0";
        return 0;
    }
    const auto axisMapping = mapping();
    if (nearly_equal(axisMapping.span, 0.0)) {
        qCDebug(lcQAccelPlot) << "zero range (viewportMin=" << viewportMin_ << "viewportMax=" << viewportMax_ << "), returning 0";
        return 0;
    }
    return axisMapping.toPixel(value, length);
}

qreal Axis::pixelToCoord(const qreal pos, const qreal length) const
{
    if (nearly_equal(length, 0.0)) {
        qCDebug(lcQAccelPlot) << "zero length, returning 0";
        return 0;
    }
    if (logScale_ && (viewportMin_ <= 0 || viewportMax_ <= 0)) {
        qCDebug(lcQAccelPlot) << "non-positive viewport bounds (viewportMin=" << viewportMin_ << "viewportMax=" << viewportMax_ << "), returning 0";
        return 0;
    }
    return mapping().toCoord(pos, length);
}

QString Axis::formatValue(const qreal value, const qreal length) const
{
    if (!std::isfinite(value)) {
        return {};
    }
    return ticker_->tickLabelFormatter()->format(value, valueResolution(value, length));
}

void Axis::addDataRangeSource(const PlotSeries* series, const Orientation dimension)
{
    dataRangeSources_.append(DataRangeSource{series, dimension});
    invalidateDataRange();
}

void Axis::removeDataRangeSource(const PlotSeries* series, const Orientation dimension)
{
    const auto removed
        = dataRangeSources_.removeIf([series, dimension](const DataRangeSource& source) { return source.series == series && source.dimension == dimension; });
    if (removed > 0) {
        invalidateDataRange();
    }
}

void Axis::invalidateDataRange()
{
    if (!autoRescale_) {
        if (!std::exchange(dataRangeStale_, true)) {
            // The new range is not computed until it is read, so this only announces that it may differ.
            emit dataMinChanged();
            emit dataMaxChanged();
        }
        return;
    }

    const auto previousMin = dataMin_;
    const auto previousMax = dataMax_;
    dataRangeStale_ = true;
    ensureDataRange();
    if (!nearly_equal(previousMin, dataMin_)) {
        emit dataMinChanged();
    }
    if (!nearly_equal(previousMax, dataMax_)) {
        emit dataMaxChanged();
    }
    // An axis whose series were all cleared keeps its viewport instead of jumping to the fallback range.
    if (dataRangeFromSeries_) {
        rescaleToData();
    }
}

void Axis::ensureDataRange() const
{
    if (!std::exchange(dataRangeStale_, false)) {
        return;
    }

    auto min = std::numeric_limits<qreal>::max();
    auto max = std::numeric_limits<qreal>::lowest();
    for (const auto& source : dataRangeSources_) {
        const auto extent = source.dimension == Horizontal ? source.series->xDataRange() : source.series->yDataRange();
        if (extent) {
            min = std::min(min, extent->min);
            max = std::max(max, extent->max);
        }
    }

    const auto hasSeriesRange = min <= max;
    if (hasSeriesRange) {
        dataMin_ = min;
        dataMax_ = max;
    } else if (dataRangeFromSeries_) {
        // A series that never had data must not replace an application-set data range.
        dataMin_ = kFallbackDataMin;
        dataMax_ = kFallbackDataMax;
    }
    dataRangeFromSeries_ = hasSeriesRange;
}

void Axis::setDataRangeValues(const qreal min, const qreal max)
{
    const auto minChanged = !nearly_equal(dataMin_, min);
    const auto maxChanged = !nearly_equal(dataMax_, max);
    dataMin_ = min;
    dataMax_ = max;
    if (minChanged) {
        emit dataMinChanged();
    }
    if (maxChanged) {
        emit dataMaxChanged();
    }
}

Axis::DataRange Axis::dataFitRange() const
{
    if (dataMin_ < dataMax_) {
        if (logScale_) {
            const auto minimum = dataMin_ > 0 ? dataMin_ : kLogScaleMinPositiveValue;
            return {minimum, dataMax_ > minimum ? dataMax_ : minimum * kLogScaleRangeFactor};
        }
        return {dataMin_, dataMax_};
    }

    // Flat data (a constant series or a single point): dataMin_ == dataMax_ would
    // otherwise give a zero-width viewport. Synthesize a small range around the value instead.
    if (logScale_) {
        const auto center = dataMin_ > 0 ? dataMin_ : kLogScaleMinPositiveValue;
        return {center / kLogScaleRangeFactor, center * kLogScaleRangeFactor};
    }
    const auto half = dataMin_ != 0.0 ? std::abs(dataMin_) * kFlatLinearRangeFraction : kFlatLinearZeroHalfRange;
    return {dataMin_ - half, dataMin_ + half};
}

void Axis::setViewportRange(const qreal min, const qreal max)
{
    const auto minChanged = !nearly_equal(viewportMin_, min);
    const auto maxChanged = !nearly_equal(viewportMax_, max);
    if (!minChanged && !maxChanged) {
        return;
    }
    viewportMin_ = min;
    viewportMax_ = max;
    if (minChanged) {
        emit viewportMinChanged();
    }
    if (maxChanged) {
        emit viewportMaxChanged();
    }
    emit rangeChanged();
    invalidateTicks();
}

void Axis::autoRescaleToData()
{
    if (autoRescale_) {
        rescaleToData();
    }
}

void Axis::hoverEnterEvent(QHoverEvent* event)
{
    hovered_ = true;
    emit hoveredChanged();
    update();
    QQuickPaintedItem::hoverEnterEvent(event);
}

void Axis::hoverLeaveEvent(QHoverEvent* event)
{
    hovered_ = false;
    emit hoveredChanged();
    update();
    QQuickPaintedItem::hoverLeaveEvent(event);
}

void Axis::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        setFocus(true);
        isDragging_ = true;
        lastMousePos_ = event->position();
        event->accept();
    } else {
        QQuickPaintedItem::mousePressEvent(event);
    }
}

void Axis::mouseMoveEvent(QMouseEvent* event)
{
    if (isDragging_) {
        const auto delta = event->position() - lastMousePos_;
        lastMousePos_ = event->position();

        const auto dataLength = (orientation_ == Horizontal) ? width() - 2.0 * labelOverflow() : height() - 2.0 * labelOverflow();
        if (nearly_equal(dataLength, 0.0)) {
            return;
        }

        // The content follows the cursor. Dragging right brings lower values into view. Dragging down
        // brings higher ones, because values grow upward while pixel y grows downward.
        const auto fraction = (orientation_ == Horizontal) ? -delta.x() / dataLength : delta.y() / dataLength;
        const auto range = Internal::pannedRange({viewportMin_, viewportMax_}, fraction, logScale_);
        setViewportMin(range.min);
        setViewportMax(range.max);
        event->accept();
    } else {
        QQuickPaintedItem::mouseMoveEvent(event);
    }
}

void Axis::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isDragging_) {
        isDragging_ = false;
        event->accept();
    } else {
        QQuickPaintedItem::mouseReleaseEvent(event);
    }
}

void Axis::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked();
        event->accept();
    } else {
        QQuickPaintedItem::mouseDoubleClickEvent(event);
    }
}

void Axis::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_L) {
        toggleLogScale();
        event->accept();
    } else {
        QQuickPaintedItem::keyPressEvent(event);
    }
}

void Axis::updatePolish()
{
    // Runs on the GUI thread before the scene graph sync. Tick labels are formatted here rather than in
    // paint(), because a formatter's tickLabel JS callback must run on the QML engine's thread.
    ticks_ = AxisTickPainter::computeTicks(viewportMin_, viewportMax_, logScale_, ticker_);
}

void Axis::componentComplete()
{
    QQuickPaintedItem::componentComplete();
    // QML assigns autoRescale and the declared ranges in no guaranteed order; fit once all are set.
    autoRescaleToData();
}

qreal Axis::valueResolution(const qreal value, const qreal length) const
{
    // Assumed axis length when the caller has no geometry yet.
    constexpr static auto kFallbackLength = qreal{1000.0};
    const auto pixels = std::isfinite(length) && length > 0.0 ? length : kFallbackLength;
    const auto axisMapping = mapping();
    auto perPixel = std::abs(axisMapping.span) / pixels;
    if (logScale_) {
        // A logarithmic pixel covers a constant ratio, so the resolution scales with the value.
        perPixel = std::abs(value) * (std::pow(10.0, perPixel) - 1.0);
    }
    if (!axisMapping.valid || !std::isfinite(perPixel) || !(perPixel > 0.0)) {
        return 0.0;
    }
    // One decimal digit finer than a pixel, so neighboring samples in dense data stay distinguishable.
    return std::pow(10.0, std::floor(std::log10(perPixel)) - 1.0);
}

void Axis::paintLabel(QPainter* painter, const QRectF& r, const qreal axisX, const qreal axisY) const
{
    if (label_.isEmpty()) {
        return;
    }
    painter->save();
    painter->setFont(labelFont_);
    const auto labelColor = labelColor_.isValid() ? labelColor_ : baselineColor_;
    painter->setPen(hovered_ ? hoverColor_ : labelColor);
    if (orientation_ == Horizontal) {
        if (side_ == Bottom) {
            const auto titleRect = QRectF(r.x(), axisY + axisTitlePadding_, r.width(), r.height() - (axisY + axisTitlePadding_));
            painter->drawText(titleRect, Qt::AlignTop | Qt::AlignHCenter, label_);
        } else { // Top
            const auto titleRect = QRectF(r.x(), r.y(), r.width(), axisY - axisTitlePadding_);
            painter->drawText(titleRect, Qt::AlignBottom | Qt::AlignHCenter, label_);
        }
    } else {
        const auto fm = painter->fontMetrics();
        const auto titleLineHeight = static_cast<qreal>(fm.height());
        const auto titleLineHalfHeight = titleLineHeight / 2.0;
        if (side_ == Left) {
            painter->save();
            painter->translate(axisX - axisTitlePadding_ - titleLineHalfHeight, r.y() + r.height() / 2);
            painter->rotate(-90);
            const auto titleRect = QRectF(-r.height() / 2, -titleLineHalfHeight, r.height(), titleLineHeight);
            painter->drawText(titleRect, Qt::AlignCenter, label_);
            painter->restore();
        } else { // Right
            painter->save();
            painter->translate(axisX + axisTitlePadding_ + titleLineHalfHeight, r.y() + r.height() / 2);
            painter->rotate(90);
            const auto titleRect = QRectF(-r.height() / 2, -titleLineHalfHeight, r.height(), titleLineHeight);
            painter->drawText(titleRect, Qt::AlignCenter, label_);
            painter->restore();
        }
    }
    painter->restore();
}

void Axis::invalidateTicks()
{
    polish();
    update();
}

} // namespace QAccelPlot
