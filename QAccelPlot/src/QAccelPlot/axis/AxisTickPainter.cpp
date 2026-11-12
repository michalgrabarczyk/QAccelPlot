//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/AxisTickPainter.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/axis/internal/AxisLabelLayout.hpp"
#include "QAccelPlot/formatters/TickLabelFormatter.hpp"

#include <QFontMetricsF>
#include <QPainter>
#include <QPen>

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

namespace {

constexpr auto kTickLabelMargin = qreal{1.0};
constexpr auto kMinimumTickLabelHeight = qreal{20.0};

/// \brief Returns the size of \a label's text with margins, measured with \a metrics.
QSizeF measuredTickLabelSize(const QFontMetricsF& metrics, const QString& label)
{
    const auto bounds = metrics.boundingRect(label);
    const auto advance = metrics.horizontalAdvance(label);
    const auto leftOverhang = std::max(qreal{0.0}, -bounds.left());
    const auto rightOverhang = std::max(qreal{0.0}, bounds.right() - advance);
    const auto overhang = std::max(leftOverhang, rightOverhang);
    const auto width = std::ceil(advance + 2.0 * overhang + 2.0 * kTickLabelMargin);
    const auto height = std::ceil(std::max(metrics.height(), bounds.height()) + 2.0 * kTickLabelMargin);
    return {width, height};
}

/// \brief Returns the unit vector pointing from the axis line toward the tick labels on \a side.
QPointF outwardDirection(const Axis::Side side)
{
    switch (side) {
    case Axis::Bottom:
        return {0.0, 1.0};
    case Axis::Top:
        return {0.0, -1.0};
    case Axis::Left:
        return {-1.0, 0.0};
    case Axis::Right:
        return {1.0, 0.0};
    }
    return {};
}

/// \brief Returns the text rectangle of an unrotated label of \a labelSize for the tick at \a center.
QRectF unrotatedTickLabelRect(const QRectF& axisRect, const QPointF& center, const AxisTickPainter::Params& params, const QSizeF& labelSize)
{
    const auto& ticker = *params.ticker;
    switch (params.side) {
    case Axis::Bottom:
        return {center.x() - labelSize.width() / 2.0, center.y() + ticker.tickLengthOut() + ticker.tickLabelPadding(), labelSize.width(), labelSize.height()};
    case Axis::Top:
        return {center.x() - labelSize.width() / 2.0, center.y() - ticker.tickLengthOut() - ticker.tickLabelPadding() - labelSize.height(), labelSize.width(),
            labelSize.height()};
    case Axis::Left: {
        const auto labelRight = center.x() - ticker.tickLengthOut() - ticker.tickLabelPadding();
        return {axisRect.left(), center.y() - labelSize.height() / 2.0, std::max(qreal{0.0}, labelRight - axisRect.left()), labelSize.height()};
    }
    case Axis::Right: {
        const auto labelLeft = center.x() + ticker.tickLengthOut() + ticker.tickLabelPadding();
        return {labelLeft, center.y() - labelSize.height() / 2.0, std::max(qreal{0.0}, axisRect.right() - labelLeft), labelSize.height()};
    }
    }
    return {};
}

/// \brief Returns the text alignment of an unrotated label inside the rectangle from \c unrotatedTickLabelRect().
int unrotatedTickLabelAlignment(const Axis::Side side)
{
    switch (side) {
    case Axis::Left:
        return Qt::AlignRight | Qt::AlignVCenter;
    case Axis::Right:
        return Qt::AlignLeft | Qt::AlignVCenter;
    case Axis::Bottom:
    case Axis::Top:
        break;
    }
    return Qt::AlignCenter;
}

/// \brief Returns the bounds of a rotated label of \a boundsSize: outside \a anchor on \a side, centered on the tick.
QRectF rotatedTickLabelBounds(const QPointF& anchor, const Axis::Side side, const QSizeF& boundsSize)
{
    const auto width = boundsSize.width();
    const auto height = boundsSize.height();
    switch (side) {
    case Axis::Bottom:
        return {anchor.x() - width / 2.0, anchor.y(), width, height};
    case Axis::Top:
        return {anchor.x() - width / 2.0, anchor.y() - height, width, height};
    case Axis::Left:
        return {anchor.x() - width, anchor.y() - height / 2.0, width, height};
    case Axis::Right:
        return {anchor.x(), anchor.y() - height / 2.0, width, height};
    }
    return {};
}

/// \brief Moves label \a bounds along the axis so they stay within \a axisRect extended by the label overflow.
QRectF clampTickLabelRect(const QRectF& bounds, const QRectF& axisRect, const AxisTickPainter::Params& params)
{
    if (!params.clampEdgeLabels) {
        return bounds;
    }
    const auto horizontal = params.orientation == Qt::Horizontal;
    const auto minimum = horizontal ? axisRect.x() - params.labelOverflow : axisRect.y() - params.labelOverflow;
    const auto maximum = horizontal ? axisRect.x() + axisRect.width() + params.labelOverflow : axisRect.y() + axisRect.height() + params.labelOverflow;

    auto clamped = bounds;
    const auto lowerBound = [&clamped, horizontal]() { return horizontal ? clamped.left() : clamped.top(); };
    const auto upperBound = [&clamped, horizontal]() { return horizontal ? clamped.right() : clamped.bottom(); };
    const auto translate = [&clamped, horizontal](const qreal offset) { clamped.translate(horizontal ? QPointF{offset, 0.0} : QPointF{0.0, offset}); };

    if (lowerBound() < minimum) {
        translate(minimum - lowerBound());
    }
    if (upperBound() > maximum) {
        translate(maximum - upperBound());
    }
    return clamped;
}

}

AxisTicks AxisTickPainter::computeTicks(const qreal viewportMin, const qreal viewportMax, const bool logScale, const AxisTicker* ticker)
{
    const auto values = computeTickValues(viewportMin, viewportMax, logScale, ticker);
    auto ticks = AxisTicks{values.subtickValues, {}};
    if (values.majorValues.isEmpty()) {
        return ticks;
    }
    // Log-scale ticks have no single step (each decade is 10x the last), so the formatter gets a dedicated entry
    // point; the default numeric formatter labels them as powers of ten. The linear step lets the formatter choose a
    // matching label precision.
    const auto* formatter = ticker->tickLabelFormatter();
    for (const auto value : values.majorValues) {
        ticks.majorTicks.append(AxisTick{value, values.logScale ? formatter->formatLogTick(value) : formatter->format(value, values.step)});
    }
    return ticks;
}

AxisTickValues AxisTickPainter::computeTickValues(const qreal viewportMin, const qreal viewportMax, const bool logScale, const AxisTicker* ticker)
{
    if (ticker == nullptr || ticker->tickCount() <= 0) {
        return {};
    }
    const auto viewportLow = std::min(viewportMin, viewportMax);
    const auto viewportHigh = std::max(viewportMin, viewportMax);
    if (!std::isfinite(viewportLow) || !std::isfinite(viewportHigh) || viewportLow >= viewportHigh) {
        return {};
    }
    if (logScale && viewportLow > 0) {
        return computeLogScaleTickValues(viewportLow, viewportHigh, *ticker);
    }
    return computeLinearTickValues(viewportLow, viewportHigh, *ticker);
}

void AxisTickPainter::paintTicks(QPainter* painter, const QRectF& rect, const qreal axisX, const qreal axisY, const Params& params, const AxisTicks& ticks,
    const MapToPosition& mapToPosition)
{
    if (params.ticker == nullptr) {
        return;
    }

    painter->save();
    painter->setFont(params.ticker->tickLabelFont());
    const auto ctx = PaintContext{painter, rect, axisX, axisY};

    // Pass 1: subticks
    if (!ticks.subtickValues.isEmpty()) {
        const auto savedPen = painter->pen();
        const auto effectiveSubtickColor = params.ticker->subtickColor().isValid() ? params.ticker->subtickColor() : params.defaultSubtickColor;
        auto subtickPen = QPen(params.hovered ? params.hoverColor : effectiveSubtickColor);
        subtickPen.setWidthF(params.ticker->subtickWidth());
        painter->setPen(subtickPen);
        for (const auto value : ticks.subtickValues) {
            paintSubtick(ctx, value, params, mapToPosition);
        }
        painter->setPen(savedPen);
    }

    // Pass 2: major ticks (drawn on top of subticks)
    for (const auto& tick : ticks.majorTicks) {
        paintTick(ctx, tick, params, mapToPosition);
    }
    painter->restore();
}

qreal AxisTickPainter::computeNiceStep(const qreal viewportMin, const qreal viewportMax, const int tickCount)
{
    const auto range = viewportMax - viewportMin;
    if (range <= 0.0 || tickCount <= 0) {
        return 1.0;
    }
    const auto roughStep = range / tickCount;
    const auto magnitude = std::pow(10.0, std::floor(std::log10(roughStep)));
    const auto residual = roughStep / magnitude;
    // Fallback "nice" step residual: the top of the 1-2-5-10 sequence, representing the next decimal decade.
    constexpr static auto kNiceStepNextDecade = qreal{10.0};
    auto niceResidual = kNiceStepNextDecade;
    if (residual <= 1.0) {
        niceResidual = 1.0;
    } else if (residual <= 2.0) {
        niceResidual = 2.0;
    } else if (residual <= 5.0) {
        niceResidual = 5.0;
    }
    return niceResidual * magnitude;
}

QSizeF AxisTickPainter::tickLabelSize(const QFontMetricsF& metrics, const QString& label)
{
    const auto size = measuredTickLabelSize(metrics, label);
    return {size.width(), std::max(kMinimumTickLabelHeight, size.height())};
}

QSizeF AxisTickPainter::rotatedTickLabelSize(const QFontMetricsF& metrics, const QString& label, const qreal rotation)
{
    if (nearlyEqual(rotation, 0.0)) {
        return tickLabelSize(metrics, label);
    }
    return Internal::rotatedBoundingSize(measuredTickLabelSize(metrics, label), rotation);
}

AxisTickValues AxisTickPainter::computeLogScaleTickValues(const qreal viewportLow, const qreal viewportHigh, const AxisTicker& ticker)
{
    const auto logMin = static_cast<int>(std::floor(std::log10(viewportLow)));
    const auto logMax = static_cast<int>(std::ceil(std::log10(viewportHigh)));

    auto values = AxisTickValues{};
    values.logScale = true;
    for (auto exponent = logMin; exponent <= logMax; ++exponent) {
        const auto value = std::pow(10.0, exponent);
        if (value >= viewportLow && value <= viewportHigh) {
            values.majorValues.append(value);
        }
        if (ticker.subtickCount() <= 0 || exponent == logMax) {
            continue;
        }
        for (auto multiplier = 2; multiplier <= 9; ++multiplier) {
            const auto subValue = multiplier * value;
            if (subValue >= viewportLow && subValue <= viewportHigh) {
                values.subtickValues.append(subValue);
            }
        }
    }
    return values;
}

AxisTickValues AxisTickPainter::computeLinearTickValues(const qreal viewportLow, const qreal viewportHigh, const AxisTicker& ticker)
{
    // Compute a "nice" world-aligned tick step from the requested tick count, no finer than the formatter allows.
    const auto tickCount = std::min(ticker.tickCount(), kMaximumTickCount);
    const auto step = std::max(computeNiceStep(viewportLow, viewportHigh, tickCount), ticker.tickLabelFormatter()->minimumTickStep());
    if (!std::isfinite(step) || step <= 0.0) {
        return {};
    }

    // Start at the first world-aligned tick >= viewportLow.
    const auto firstTick = std::ceil(viewportLow / step) * step;

    // Begin one interval before firstTick so that subticks in [firstTick-step, firstTick]
    // that fall inside the viewport are still included even though their owning major tick is
    // outside the visible range.
    const auto loopStart = firstTick - step;

    // Small fraction of the step used as a boundary tolerance: prevents floating-point rounding
    // from excluding ticks that lie exactly on a viewport edge.
    constexpr static auto kRelativeTolerance = 1e-9;

    const auto subtickCount = std::clamp(ticker.subtickCount(), 0, kMaximumSubtickCount);
    const auto subStep = step / (subtickCount + 1);

    auto values = AxisTickValues{};
    values.step = step;
    auto tickIndex = 0;
    for (auto tickBase = loopStart; tickBase <= viewportHigh + step * kRelativeTolerance; tickBase = loopStart + (++tickIndex) * step) {
        if (tickBase >= viewportLow - step * kRelativeTolerance) {
            values.majorValues.append(tickBase);
        }
        for (auto subIndex = 1; subIndex <= subtickCount; ++subIndex) {
            const auto subValue = tickBase + subIndex * subStep;
            if (subValue < viewportLow - subStep * kRelativeTolerance) {
                continue;
            }
            if (subValue > viewportHigh + subStep * kRelativeTolerance) {
                break;
            }
            values.subtickValues.append(subValue);
        }
    }
    return values;
}

void AxisTickPainter::paintTick(const PaintContext& ctx, const AxisTick& tick, const Params& params, const MapToPosition& mapToPosition)
{
    const auto& ticker = *params.ticker;
    const auto horizontal = params.orientation == Qt::Horizontal;
    const auto position = horizontal ? ctx.rect.x() + mapToPosition(tick.value, ctx.rect.width()) : ctx.rect.y() + mapToPosition(tick.value, ctx.rect.height());
    const auto center = horizontal ? QPointF{position, ctx.axisY} : QPointF{ctx.axisX, position};
    const auto outward = outwardDirection(params.side);
    ctx.painter->drawLine(center + outward * ticker.tickLengthOut(), center - outward * ticker.tickLengthIn());

    const auto linePen = ctx.painter->pen();
    auto labelPen = linePen;
    labelPen.setColor(params.hovered ? params.hoverColor : (ticker.tickLabelColor().isValid() ? ticker.tickLabelColor() : ticker.tickColor()));
    ctx.painter->setPen(labelPen);

    const auto metrics = QFontMetricsF{ctx.painter->font(), ctx.painter->device()};
    const auto rotation = ticker.tickLabelRotation();
    if (nearlyEqual(rotation, 0.0)) {
        const auto labelRect = unrotatedTickLabelRect(ctx.rect, center, params, tickLabelSize(metrics, tick.label));
        drawTickLabel(ctx.painter, clampTickLabelRect(labelRect, ctx.rect, params), unrotatedTickLabelAlignment(params.side), tick.label);
    } else {
        // Placed by the rotated bounding box: the unrotated space beside the tick would clip a vertical axis label.
        const auto textSize = measuredTickLabelSize(metrics, tick.label);
        const auto anchor = center + outward * (ticker.tickLengthOut() + ticker.tickLabelPadding());
        const auto bounds = rotatedTickLabelBounds(anchor, params.side, Internal::rotatedBoundingSize(textSize, rotation));
        drawRotatedTickLabel(ctx.painter, clampTickLabelRect(bounds, ctx.rect, params), textSize, tick.label, rotation);
    }
    ctx.painter->setPen(linePen);
}

void AxisTickPainter::paintSubtick(const PaintContext& ctx, const qreal value, const Params& params, const MapToPosition& mapToPosition)
{
    if (params.orientation == Qt::Horizontal) {
        const auto x = ctx.rect.x() + mapToPosition(value, ctx.rect.width());
        const auto center = QPointF{x, ctx.axisY};
        if (params.side == Axis::Bottom) {
            ctx.painter->drawLine(center + QPointF(0, params.ticker->subtickLengthOut()), center + QPointF(0, -params.ticker->subtickLengthIn()));
        } else {
            ctx.painter->drawLine(center + QPointF(0, -params.ticker->subtickLengthOut()), center + QPointF(0, params.ticker->subtickLengthIn()));
        }
    } else {
        const auto y = ctx.rect.y() + mapToPosition(value, ctx.rect.height());
        const auto center = QPointF{ctx.axisX, y};
        if (params.side == Axis::Left) {
            ctx.painter->drawLine(center + QPointF(-params.ticker->subtickLengthOut(), 0), center + QPointF(params.ticker->subtickLengthIn(), 0));
        } else {
            ctx.painter->drawLine(center + QPointF(params.ticker->subtickLengthOut(), 0), center + QPointF(-params.ticker->subtickLengthIn(), 0));
        }
    }
}

void AxisTickPainter::drawTickLabel(QPainter* painter, const QRectF& labelRect, const int alignment, const QString& label)
{
    painter->drawText(labelRect, alignment, label);
}

void AxisTickPainter::drawRotatedTickLabel(QPainter* painter, const QRectF& bounds, const QSizeF& textSize, const QString& label, const qreal rotation)
{
    painter->save();
    painter->translate(bounds.center());
    painter->rotate(rotation);
    painter->drawText(QRectF{QPointF{-textSize.width() / 2.0, -textSize.height() / 2.0}, textSize}, Qt::AlignCenter, label);
    painter->restore();
}

} // namespace QAccelPlot
