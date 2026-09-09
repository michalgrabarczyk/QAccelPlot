//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "axis/AxisTickPainter.hpp"

#include "MathUtils.hpp"
#include "axis/Axis.hpp"

#include <QFontMetricsF>
#include <QPainter>
#include <QPen>
#include <QTransform>

#include <algorithm>
#include <cmath>

namespace QAccelPlot {

namespace {

constexpr auto kTickLabelMargin = qreal{1.0};
constexpr auto kMinimumTickLabelHeight = qreal{20.0};

QSizeF tickLabelSize(const QFontMetricsF& metrics, const QString& label)
{
    const auto bounds = metrics.boundingRect(label);
    const auto advance = metrics.horizontalAdvance(label);
    const auto leftOverhang = std::max(qreal{0.0}, -bounds.left());
    const auto rightOverhang = std::max(qreal{0.0}, bounds.right() - advance);
    const auto overhang = std::max(leftOverhang, rightOverhang);
    const auto width = std::ceil(advance + 2.0 * overhang + 2.0 * kTickLabelMargin);
    const auto measuredHeight = std::ceil(std::max(metrics.height(), bounds.height()) + 2.0 * kTickLabelMargin);
    return {width, std::max(kMinimumTickLabelHeight, measuredHeight)};
}

QRectF rotatedTickLabelBounds(const QRectF& rect, const qreal rotation)
{
    auto transform = QTransform{};
    transform.translate(rect.center().x(), rect.center().y());
    transform.rotate(rotation);
    transform.translate(-rect.center().x(), -rect.center().y());
    return transform.mapRect(rect);
}

QRectF clampTickLabelRect(const QRectF& rect, const qreal rotation, const Qt::Orientation orientation, const qreal minimum, const qreal maximum)
{
    auto clampedRect = rect;
    auto bounds = rotatedTickLabelBounds(clampedRect, rotation);

    const auto lowerBound = [&bounds, orientation]() { return orientation == Qt::Horizontal ? bounds.left() : bounds.top(); };
    const auto upperBound = [&bounds, orientation]() { return orientation == Qt::Horizontal ? bounds.right() : bounds.bottom(); };
    const auto translate = [&clampedRect, &bounds, orientation](const qreal offset) {
        const auto delta = orientation == Qt::Horizontal ? QPointF{offset, 0.0} : QPointF{0.0, offset};
        clampedRect.translate(delta);
        bounds.translate(delta);
    };

    if (lowerBound() < minimum) {
        translate(minimum - lowerBound());
    }
    if (upperBound() > maximum) {
        translate(maximum - upperBound());
    }
    return clampedRect;
}

}

void AxisTickPainter::paintTicks(
    QPainter* painter, const QRectF& rect, const qreal axisX, const qreal axisY, const Params& params, const MapToPosition& mapToPosition)
{
    if (params.ticker == nullptr) {
        return;
    }

    painter->save();
    painter->setFont(params.ticker->tickLabelFont());
    const auto ctx = PaintContext{painter, rect, axisX, axisY};
    if (params.logScale && params.viewportMin > 0 && params.viewportMax > 0) {
        paintLogScaleTicks(ctx, params, mapToPosition);
    } else {
        paintLinearTicks(ctx, params, mapToPosition);
    }
    painter->restore();
}

void AxisTickPainter::paintTick(const PaintContext& ctx, const qreal value, const Params& params, const MapToPosition& mapToPosition)
{
    constexpr static auto kVerticalLabelWidth = qreal{40.0};
    const auto label = params.ticker->tickLabelFormatter()->format(value, params.tickStep);

    if (params.orientation == Axis::Horizontal) {
        const auto x = ctx.rect.x() + mapToPosition(value, ctx.rect.width());
        const auto center = QPointF{x, ctx.axisY};
        const auto labelSize = tickLabelSize(QFontMetricsF{ctx.painter->font(), ctx.painter->device()}, label);
        auto labelRect = QRectF{};
        if (params.side == Axis::Bottom) {
            ctx.painter->drawLine(center + QPointF(0, params.ticker->tickLengthOut()), center + QPointF(0, -params.ticker->tickLengthIn()));
            labelRect = QRectF(x - labelSize.width() / 2.0, center.y() + params.ticker->tickLengthOut() + params.ticker->tickLabelPadding(), labelSize.width(),
                labelSize.height());
        } else {
            ctx.painter->drawLine(center + QPointF(0, -params.ticker->tickLengthOut()), center + QPointF(0, params.ticker->tickLengthIn()));
            labelRect = QRectF(x - labelSize.width() / 2.0,
                center.y() - params.ticker->tickLengthOut() - params.ticker->tickLabelPadding() - labelSize.height(), labelSize.width(), labelSize.height());
        }
        if (params.clampEdgeLabels) {
            const auto minX = ctx.rect.x() - params.labelOverflow;
            const auto maxX = ctx.rect.x() + ctx.rect.width() + params.labelOverflow;
            labelRect = clampTickLabelRect(labelRect, params.ticker->tickLabelRotation(), Qt::Horizontal, minX, maxX);
        }
        const auto linePen = ctx.painter->pen();
        auto labelPen = linePen;
        labelPen.setColor(
            params.hovered ? params.hoverColor : (params.ticker->tickLabelColor().isValid() ? params.ticker->tickLabelColor() : params.ticker->tickColor()));
        ctx.painter->setPen(labelPen);
        drawTickLabel(ctx.painter, labelRect, Qt::AlignCenter, label, params.ticker->tickLabelRotation());
        ctx.painter->setPen(linePen);
    } else {
        const auto y = ctx.rect.y() + mapToPosition(value, ctx.rect.height());
        const auto center = QPointF{ctx.axisX, y};
        auto labelRect = QRectF{};
        auto alignment = int{};
        if (params.side == Axis::Left) {
            ctx.painter->drawLine(center + QPointF(-params.ticker->tickLengthOut(), 0), center + QPointF(params.ticker->tickLengthIn(), 0));
            labelRect = QRectF(center.x() - params.ticker->tickLengthOut() - params.ticker->tickLabelPadding() - kVerticalLabelWidth,
                y - kMinimumTickLabelHeight / 2.0, kVerticalLabelWidth, kMinimumTickLabelHeight);
            alignment = Qt::AlignRight | Qt::AlignVCenter;
        } else {
            ctx.painter->drawLine(center + QPointF(params.ticker->tickLengthOut(), 0), center + QPointF(-params.ticker->tickLengthIn(), 0));
            labelRect = QRectF(center.x() + params.ticker->tickLengthOut() + params.ticker->tickLabelPadding(), y - kMinimumTickLabelHeight / 2.0,
                kVerticalLabelWidth, kMinimumTickLabelHeight);
            alignment = Qt::AlignLeft | Qt::AlignVCenter;
        }
        if (params.clampEdgeLabels) {
            const auto minY = ctx.rect.y() - params.labelOverflow;
            const auto maxY = ctx.rect.y() + ctx.rect.height() + params.labelOverflow;
            labelRect = clampTickLabelRect(labelRect, params.ticker->tickLabelRotation(), Qt::Vertical, minY, maxY);
        }
        const auto linePen = ctx.painter->pen();
        auto labelPen = linePen;
        labelPen.setColor(
            params.hovered ? params.hoverColor : (params.ticker->tickLabelColor().isValid() ? params.ticker->tickLabelColor() : params.ticker->tickColor()));
        ctx.painter->setPen(labelPen);
        drawTickLabel(ctx.painter, labelRect, alignment, label, params.ticker->tickLabelRotation());
        ctx.painter->setPen(linePen);
    }
}

void AxisTickPainter::paintSubtick(const PaintContext& ctx, const qreal value, const Params& params, const MapToPosition& mapToPosition)
{
    if (params.orientation == Axis::Horizontal) {
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

void AxisTickPainter::paintLogScaleTicks(const PaintContext& ctx, const Params& params, const MapToPosition& mapToPosition)
{
    const auto viewportLow = std::min(params.viewportMin, params.viewportMax);
    const auto viewportHigh = std::max(params.viewportMin, params.viewportMax);
    const auto logMin = static_cast<int>(std::floor(std::log10(viewportLow)));
    const auto logMax = static_cast<int>(std::ceil(std::log10(viewportHigh)));

    // Pass 1: subticks
    const auto savedPen = ctx.painter->pen();
    const auto effectiveSubtickColor = params.ticker->subtickColor().isValid() ? params.ticker->subtickColor() : params.defaultSubtickColor;
    auto subtickPen = QPen(params.hovered ? params.hoverColor : effectiveSubtickColor);
    subtickPen.setWidthF(params.ticker->subtickWidth());
    ctx.painter->setPen(subtickPen);

    for (auto exponent = logMin; exponent < logMax; ++exponent) {
        const auto value = std::pow(10.0, exponent);
        for (auto multiplier = 2; multiplier <= 9; ++multiplier) {
            const auto subValue = multiplier * value;
            if (subValue >= viewportLow && subValue <= viewportHigh) {
                paintSubtick(ctx, subValue, params, mapToPosition);
            }
        }
    }
    ctx.painter->setPen(savedPen);

    // Pass 2: major ticks
    for (auto exponent = logMin; exponent <= logMax; ++exponent) {
        const auto value = std::pow(10.0, exponent);
        if (value >= viewportLow && value <= viewportHigh) {
            paintTick(ctx, value, params, mapToPosition);
        }
    }
}

void AxisTickPainter::paintLinearTicks(const PaintContext& ctx, const Params& params, const MapToPosition& mapToPosition)
{
    if (params.ticker->tickCount() <= 0) {
        return;
    }

    const auto viewportLow = std::min(params.viewportMin, params.viewportMax);
    const auto viewportHigh = std::max(params.viewportMin, params.viewportMax);
    const auto range = viewportHigh - viewportLow;
    if (range <= 0.0) {
        return;
    }

    // Compute a "nice" world-aligned tick step from the requested tick count.
    const auto step = computeNiceStep(viewportLow, viewportHigh, params.ticker->tickCount());

    // Start at the first world-aligned tick >= viewportMin.
    const auto firstTick = std::ceil(viewportLow / step) * step;

    // Build a mutable copy so we can propagate tickStep into paintTick for label precision.
    auto p = params;
    p.tickStep = step;

    const auto effectiveSubtickColor = params.ticker->subtickColor().isValid() ? params.ticker->subtickColor() : params.defaultSubtickColor;

    // Begin one interval before firstTick so that subticks in [firstTick-step, firstTick]
    // that fall inside [viewportMin, viewportMax] are still painted even though their owning major tick is
    // outside the visible range.
    const auto loopStart = firstTick - step;

    // Small fraction of the step used as a boundary tolerance: prevents floating-point rounding
    // from excluding ticks that lie exactly on a viewport edge.
    constexpr static auto kRelativeTolerance = 1e-9;

    // Pass 1: all subticks
    if (params.ticker->subtickCount() > 0) {
        const auto savedPen = ctx.painter->pen();
        auto subtickPen = QPen(params.hovered ? params.hoverColor : effectiveSubtickColor);
        subtickPen.setWidthF(params.ticker->subtickWidth());
        ctx.painter->setPen(subtickPen);

        const auto subStep = step / (params.ticker->subtickCount() + 1);
        auto subTickIndex = 0;
        for (auto tickBase = loopStart; tickBase <= viewportHigh + step * kRelativeTolerance; tickBase = loopStart + (++subTickIndex) * step) {
            for (auto subIndex = 1; subIndex <= params.ticker->subtickCount(); ++subIndex) {
                const auto subValue = tickBase + subIndex * subStep;
                if (subValue < viewportLow - subStep * kRelativeTolerance) {
                    continue;
                }
                if (subValue > viewportHigh + subStep * kRelativeTolerance) {
                    break;
                }
                paintSubtick(ctx, subValue, p, mapToPosition);
            }
        }
        ctx.painter->setPen(savedPen);
    }

    // Pass 2: all major ticks (drawn on top of subticks)
    auto tickIndex = 0;
    for (auto tickBase = loopStart; tickBase <= viewportHigh + step * kRelativeTolerance; tickBase = loopStart + (++tickIndex) * step) {
        if (tickBase >= viewportLow - step * kRelativeTolerance && tickBase <= viewportHigh + step * kRelativeTolerance) {
            paintTick(ctx, tickBase, p, mapToPosition);
        }
    }
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

void AxisTickPainter::drawTickLabel(QPainter* painter, const QRectF& labelRect, const int alignment, const QString& label, const qreal rotation)
{
    if (nearly_equal(rotation, 0.0)) {
        painter->drawText(labelRect, alignment, label);
        return;
    }

    painter->save();
    painter->translate(labelRect.center());
    painter->rotate(rotation);

    const auto localRect = QRectF(-labelRect.width() / 2.0, -labelRect.height() / 2.0, labelRect.width(), labelRect.height());
    painter->drawText(localRect, alignment, label);
    painter->restore();
}

} // namespace QAccelPlot
