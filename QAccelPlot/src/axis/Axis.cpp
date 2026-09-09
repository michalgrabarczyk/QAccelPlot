//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "axis/Axis.hpp"
#include "axis/AxisTickPainter.hpp"

#include "MathUtils.hpp"
#include "QAccelPlotLogging.hpp"

#include <algorithm>
#include <limits>

#include <QMouseEvent>
#include <QPainter>
#include <QPen>

#include <cmath>

namespace QAccelPlot {

namespace {

constexpr auto kMinZoomScaleFactor = 0.0;
constexpr auto kMaxZoomScaleFactor = 1.0;
constexpr auto kLogScaleMinPositiveValue = 1e-10;
constexpr auto kHorizontalLabelOverflow = 25.0;
constexpr auto kVerticalLabelOverflow = 10.0;

}

Axis::Axis(QQuickItem* parent, Side side)
    : QQuickPaintedItem(parent)
    , ticker_(new AxisTicker(this))
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    connect(ticker_, &AxisTicker::tickColorChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::tickLabelColorChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::tickCountChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::subtickCountChanged, this, [this]() { update(); });
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
    connect(ticker_, &AxisTicker::tickLabelFormatterChanged, this, [this]() { update(); });
    connect(ticker_, &AxisTicker::tickLabelFormatChanged, this, [this]() { update(); });
    setSide(side);
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
    update();
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
    update();
}

qreal Axis::dataMin() const
{
    return dataMin_;
}

void Axis::setDataMin(const qreal m)
{
    if (nearly_equal(dataMin_, m)) {
        return;
    }
    dataMin_ = m;
    emit dataMinChanged();
}

qreal Axis::dataMax() const
{
    return dataMax_;
}

void Axis::setDataMax(const qreal m)
{
    if (nearly_equal(dataMax_, m)) {
        return;
    }
    dataMax_ = m;
    emit dataMaxChanged();
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
    emit logScaleChanged();
    emit rangeChanged(); // force curves to rebuild
    update();
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

void Axis::toggleLogScale()
{
    setLogScale(!logScale_);
}

void Axis::rescaleToData()
{
    if (dataMin_ < dataMax_) {
        if (logScale_) {
            // Clamp viewportMin to a small positive value for log scale
            setViewportMin(dataMin_ > 0 ? dataMin_ : kLogScaleMinPositiveValue);
        } else {
            setViewportMin(dataMin_);
        }
        setViewportMax(dataMax_);
    }
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
    tickPen.setWidth(ticker_->tickWidth());
    painter->setPen(tickPen);

    auto tickPainterParams = AxisTickPainter::Params{};
    tickPainterParams.viewportMin = viewportMin_;
    tickPainterParams.viewportMax = viewportMax_;
    tickPainterParams.orientation = orientation_;
    tickPainterParams.side = side_;
    tickPainterParams.logScale = logScale_;
    tickPainterParams.hovered = hovered_;
    tickPainterParams.ticker = ticker_;
    tickPainterParams.hoverColor = hoverColor_;
    tickPainterParams.defaultSubtickColor = ticker_->subtickColor();
    tickPainterParams.clampEdgeLabels = clampEdgeLabels_;
    tickPainterParams.labelOverflow = labelOv;

    AxisTickPainter::paintTicks(
        painter, dataRect, axisX, axisY, tickPainterParams, [this](const auto value, const auto length) { return coordToPixel(value, length); });

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

qreal Axis::coordToPixel(const qreal value, const qreal length) const
{
    auto ratio = qreal{};
    if (logScale_) {
        if (value <= 0 || viewportMin_ <= 0 || viewportMax_ <= 0) {
            qCDebug(lcQAccelPlot) << "non-positive value or viewport bounds (value=" << value << "viewportMin=" << viewportMin_
                                  << "viewportMax=" << viewportMax_ << "), returning 0";
            return 0;
        }
        const auto logMin = std::log10(viewportMin_);
        const auto logMax = std::log10(viewportMax_);
        const auto logRange = logMax - logMin;
        if (nearly_equal(logRange, 0.0)) {
            qCDebug(lcQAccelPlot) << "zero log range (viewportMin=" << viewportMin_ << "viewportMax=" << viewportMax_ << "), returning 0";
            return 0;
        }
        ratio = (std::log10(value) - logMin) / logRange;
    } else {
        const auto range = viewportMax_ - viewportMin_;
        if (nearly_equal(range, 0.0)) {
            qCDebug(lcQAccelPlot) << "zero range (viewportMin=" << viewportMin_ << "viewportMax=" << viewportMax_ << "), returning 0";
            return 0;
        }
        ratio = (value - viewportMin_) / range;
    }
    if (orientation_ == Horizontal) {
        return ratio * length;
    } else {
        return (1.0 - ratio) * length;
    }
}

qreal Axis::pixelToCoord(const qreal pos, const qreal length) const
{
    if (nearly_equal(length, 0.0)) {
        qCDebug(lcQAccelPlot) << "zero length, returning 0";
        return 0;
    }
    auto ratio = qreal{};
    if (orientation_ == Horizontal) {
        ratio = pos / length;
    } else {
        ratio = 1.0 - (pos / length);
    }
    if (logScale_) {
        if (viewportMin_ <= 0 || viewportMax_ <= 0) {
            qCDebug(lcQAccelPlot) << "non-positive viewport bounds (viewportMin=" << viewportMin_ << "viewportMax=" << viewportMax_ << "), returning 0";
            return 0;
        }
        const auto logMin = std::log10(viewportMin_);
        const auto logMax = std::log10(viewportMax_);
        const auto logRange = logMax - logMin;
        return std::pow(10.0, logMin + ratio * logRange);
    } else {
        const auto range = viewportMax_ - viewportMin_;
        return viewportMin_ + ratio * range;
    }
}

void Axis::updateDataRange(const qreal min, const qreal max)
{
    setDataRangeValues(min, max);
}

void Axis::setSourceDataRange(const QObject* source, const Orientation dimension, const qreal min, const qreal max)
{
    if (!source) {
        return;
    }

    auto& ranges = dimension == Horizontal ? horizontalDataRanges_ : verticalDataRanges_;
    ranges.insert(source, DataRange{min, max});

    if (!rangeSourceConnections_.contains(source)) {
        const auto connection = connect(source, &QObject::destroyed, this, [this, source]() {
            horizontalDataRanges_.remove(source);
            verticalDataRanges_.remove(source);
            rangeSourceConnections_.remove(source);
            recomputeSourceDataRange();
        });
        rangeSourceConnections_.insert(source, connection);
    }

    recomputeSourceDataRange();
}

void Axis::clearSourceDataRange(const QObject* source, const Orientation dimension)
{
    auto& ranges = dimension == Horizontal ? horizontalDataRanges_ : verticalDataRanges_;
    ranges.remove(source);

    if (!horizontalDataRanges_.contains(source) && !verticalDataRanges_.contains(source)) {
        disconnect(rangeSourceConnections_.take(source));
    }

    recomputeSourceDataRange();
}

void Axis::recomputeSourceDataRange()
{
    if (horizontalDataRanges_.isEmpty() && verticalDataRanges_.isEmpty()) {
        setDataRangeValues(0.0, 1.0);
        return;
    }

    auto min = std::numeric_limits<qreal>::max();
    auto max = std::numeric_limits<qreal>::lowest();
    const auto includeRanges = [&min, &max](const auto& ranges) {
        for (const auto& range : ranges) {
            min = std::min(min, range.min);
            max = std::max(max, range.max);
        }
    };
    includeRanges(horizontalDataRanges_);
    includeRanges(verticalDataRanges_);
    setDataRangeValues(min, max);
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

        if (logScale_ && viewportMin_ > 0.0 && viewportMax_ > 0.0) {
            const auto logMin = std::log10(viewportMin_);
            const auto logMax = std::log10(viewportMax_);
            auto logShift = qreal{};
            if (orientation_ == Horizontal) {
                logShift = -(delta.x() / dataLength) * (logMax - logMin);
            } else {
                logShift = (delta.y() / dataLength) * (logMax - logMin);
            }
            if (logShift != 0) {
                setViewportMin(std::pow(10.0, logMin + logShift));
                setViewportMax(std::pow(10.0, logMax + logShift));
            }
        } else {
            const auto range = viewportMax_ - viewportMin_;
            if (nearly_equal(range, 0.0)) {
                return;
            }
            auto shift = qreal{};
            if (orientation_ == Horizontal) {
                // Drag right -> shift view left (values decrease)
                shift = -(delta.x() / dataLength) * range;
            } else {
                // Drag down -> shift view up (values increase as Y is inverted)
                shift = (delta.y() / dataLength) * range;
            }
            if (shift != 0) {
                setViewportMin(viewportMin_ + shift);
                setViewportMax(viewportMax_ + shift);
            }
        }
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

} // namespace QAccelPlot
