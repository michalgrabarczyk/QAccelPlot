//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/ColorBar.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/axis/Axis.hpp"
#include "QAccelPlot/axis/AxisTickPainter.hpp"

#include <QFontMetricsF>
#include <QLinearGradient>
#include <QPainter>
#include <QPen>

#include <algorithm>
#include <cmath>
#include <initializer_list>

namespace QAccelPlot {

namespace {

constexpr auto kDefaultLength = qreal{160.0};
// Stops sharing a position would collapse in QGradient, which keeps only the last color set at a
// position. Separating them by this much keeps a hard edge in the ramp.
constexpr auto kHardEdgeOffset = qreal{1e-6};

qreal rampPosition(const qreal value, const qreal minimum, const qreal maximum, const bool logScale)
{
    if (logScale) {
        if (value <= 0.0) {
            return 0.0;
        }
        const auto logMin = std::log10(minimum);
        const auto logRange = std::log10(maximum) - logMin;
        return nearly_equal(logRange, 0.0) ? 0.0 : (std::log10(value) - logMin) / logRange;
    }
    const auto range = maximum - minimum;
    return nearly_equal(range, 0.0) ? 0.0 : (value - minimum) / range;
}

} // namespace

ColorBar::ColorBar(QQuickItem* parent)
    : QQuickPaintedItem(parent)
    , ticker_(new AxisTicker(this))
{
    ticker_->setTickLengthIn(0.0);
    ticker_->setTickLengthOut(4.0);
    ticker_->setSubtickLengthIn(0.0);
    ticker_->setSubtickLengthOut(2.0);
    ticker_->setSubtickCount(0);
    ticker_->setTickWidth(1.0);

    const auto tickerSignals = {&AxisTicker::tickColorChanged, &AxisTicker::tickLabelColorChanged, &AxisTicker::tickCountChanged,
        &AxisTicker::subtickCountChanged, &AxisTicker::tickLengthChanged, &AxisTicker::subtickLengthChanged, &AxisTicker::tickLengthInChanged,
        &AxisTicker::tickLengthOutChanged, &AxisTicker::subtickLengthInChanged, &AxisTicker::subtickLengthOutChanged, &AxisTicker::subtickColorChanged,
        &AxisTicker::tickWidthChanged, &AxisTicker::subtickWidthChanged, &AxisTicker::tickLabelPaddingChanged, &AxisTicker::tickLabelRotationChanged,
        &AxisTicker::tickLabelFontChanged, &AxisTicker::tickLabelFormatterChanged, &AxisTicker::tickLabelFormatChanged};
    for (const auto signal : tickerSignals) {
        connect(ticker_, signal, this, &ColorBar::invalidate);
    }
    invalidate();
}

PointCloud* ColorBar::series() const
{
    return series_.data();
}

void ColorBar::setSeries(PointCloud* series)
{
    if (series_ == series) {
        return;
    }
    series_ = series;
    reconnectSeries();
    emit seriesChanged();
    invalidate();
}

ColorBar::Orientation ColorBar::orientation() const
{
    return orientation_;
}

void ColorBar::setOrientation(const Orientation orientation)
{
    if (orientation_ == orientation) {
        return;
    }
    orientation_ = orientation;
    emit orientationChanged();
    invalidate();
}

QString ColorBar::label() const
{
    return label_;
}

void ColorBar::setLabel(const QString& label)
{
    if (label_ == label) {
        return;
    }
    label_ = label;
    emit labelChanged();
    invalidate();
}

QFont ColorBar::labelFont() const
{
    return labelFont_;
}

void ColorBar::setLabelFont(const QFont& font)
{
    if (labelFont_ == font) {
        return;
    }
    labelFont_ = font;
    emit labelFontChanged();
    invalidate();
}

QColor ColorBar::labelColor() const
{
    return labelColor_;
}

void ColorBar::setLabelColor(const QColor& color)
{
    if (labelColor_ == color) {
        return;
    }
    labelColor_ = color;
    emit labelColorChanged();
    update();
}

qreal ColorBar::labelPadding() const
{
    return labelPadding_;
}

void ColorBar::setLabelPadding(const qreal padding)
{
    const auto clampedPadding = std::max(qreal{0.0}, padding);
    if (nearly_equal(labelPadding_, clampedPadding)) {
        return;
    }
    labelPadding_ = clampedPadding;
    emit labelPaddingChanged();
    invalidate();
}

qreal ColorBar::barThickness() const
{
    return barThickness_;
}

void ColorBar::setBarThickness(const qreal thickness)
{
    const auto clampedThickness = std::max(qreal{0.0}, thickness);
    if (nearly_equal(barThickness_, clampedThickness)) {
        return;
    }
    barThickness_ = clampedThickness;
    emit barThicknessChanged();
    invalidate();
}

QColor ColorBar::borderColor() const
{
    return borderColor_;
}

void ColorBar::setBorderColor(const QColor& color)
{
    if (borderColor_ == color) {
        return;
    }
    borderColor_ = color;
    emit borderColorChanged();
    update();
}

qreal ColorBar::borderWidth() const
{
    return borderWidth_;
}

void ColorBar::setBorderWidth(const qreal width)
{
    const auto clampedWidth = std::max(qreal{0.0}, width);
    if (nearly_equal(borderWidth_, clampedWidth)) {
        return;
    }
    borderWidth_ = clampedWidth;
    emit borderWidthChanged();
    invalidate();
}

AxisTicker* ColorBar::ticker() const
{
    return ticker_;
}

qreal ColorBar::valueToPixel(const qreal value, const qreal length) const
{
    const auto position = rampPosition(value, valueMin_, valueMax_, logScale_);
    return orientation_ == Horizontal ? position * length : (1.0 - position) * length;
}

void ColorBar::paint(QPainter* painter)
{
    if (stops_.size() < 2) {
        return;
    }
    paintStrip(painter);
    paintTicks(painter);
    paintTitle(painter);
}

void ColorBar::updatePolish()
{
    // Tick labels are formatted here rather than in paint(), because a formatter's tickLabel JS
    // callback must run on the QML engine's thread.
    captureColormap();
    ticks_ = stops_.empty() ? AxisTicks{} : AxisTickPainter::computeTicks(valueMin_, valueMax_, logScale_, ticker_);

    const auto metrics = QFontMetricsF{ticker_->tickLabelFont()};
    maxTickLabelWidth_ = 0.0;
    for (const auto& tick : ticks_.majorTicks) {
        maxTickLabelWidth_ = std::max(maxTickLabelWidth_, AxisTickPainter::tickLabelSize(metrics, tick.label).width());
    }

    updateImplicitSize();
    layout_ = computeLayout();
    update();
}

void ColorBar::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    if (newGeometry.size() != oldGeometry.size()) {
        polish();
    }
}

void ColorBar::reconnectSeries()
{
    for (const auto& connection : seriesConnections_) {
        disconnect(connection);
    }
    seriesConnections_.clear();

    if (series_) {
        seriesConnections_.append(connect(series_.data(), &PointCloud::colormapChanged, this, [this]() {
            reconnectColormap();
            invalidate();
        }));
        seriesConnections_.append(connect(series_.data(), &PointCloud::valueRangeChanged, this, &ColorBar::invalidate));
        seriesConnections_.append(connect(series_.data(), &QObject::destroyed, this, [this]() {
            reconnectSeries();
            emit seriesChanged();
            invalidate();
        }));
    }
    reconnectColormap();
}

void ColorBar::reconnectColormap()
{
    disconnect(colormapConnection_);
    auto* const colormap = series_ ? series_->colormap() : nullptr;
    if (colormap) {
        colormapConnection_ = connect(colormap, &Colormap::colormapChanged, this, &ColorBar::invalidate);
    }
}

void ColorBar::invalidate()
{
    polish();
    update();
}

void ColorBar::captureColormap()
{
    const auto* const colormap = series_ ? series_->colormap() : nullptr;
    if (!colormap) {
        stops_.clear();
        valueMin_ = 0.0;
        valueMax_ = 1.0;
        logScale_ = false;
        return;
    }
    stops_ = colormap->resolvedStops();
    valueMin_ = series_->dataValueMin();
    valueMax_ = series_->dataValueMax();
    // The series leaves every point unmapped when a log range is not positive; ticks fall back to linear.
    logScale_ = colormap->norm() == Colormap::Normalization::Log && valueMin_ > 0.0 && valueMax_ > 0.0;
}

void ColorBar::updateImplicitSize()
{
    const auto titleExtent = label_.isEmpty() ? qreal{0.0} : labelPadding_ + titleThickness();
    const auto thickness = std::ceil(titleOffset() - labelPadding_ + titleExtent);
    if (orientation_ == Vertical) {
        setImplicitSize(thickness, kDefaultLength);
    } else {
        setImplicitSize(kDefaultLength, thickness);
    }
}

ColorBar::Layout ColorBar::computeLayout() const
{
    auto layout = Layout{};
    layout.endInset = endInset();
    if (orientation_ == Vertical) {
        const auto length = std::max(qreal{0.0}, height() - 2.0 * layout.endInset);
        layout.strip = QRectF{0.0, layout.endInset, barThickness_, length};
        layout.tickArea = QRectF{0.0, layout.endInset, width(), length};
        layout.title = QRectF{titleOffset(), layout.endInset, titleThickness(), length};
    } else {
        const auto length = std::max(qreal{0.0}, width() - 2.0 * layout.endInset);
        layout.strip = QRectF{layout.endInset, 0.0, length, barThickness_};
        layout.tickArea = QRectF{layout.endInset, 0.0, length, height()};
        layout.title = QRectF{layout.endInset, titleOffset(), length, titleThickness()};
    }
    return layout;
}

qreal ColorBar::endInset() const
{
    const auto lineInset = std::max(borderWidth_, ticker_->tickWidth()) / 2.0;
    if (orientation_ == Vertical) {
        return std::max(lineInset, tickLabelHeight() / 2.0);
    }
    return lineInset;
}

qreal ColorBar::tickLabelHeight() const
{
    return AxisTickPainter::tickLabelSize(QFontMetricsF{ticker_->tickLabelFont()}, QStringLiteral("0")).height();
}

qreal ColorBar::tickLabelThickness() const
{
    return orientation_ == Vertical ? maxTickLabelWidth_ : tickLabelHeight();
}

qreal ColorBar::titleThickness() const
{
    return std::ceil(QFontMetricsF{labelFont_}.height());
}

qreal ColorBar::titleOffset() const
{
    return barThickness_ + ticker_->tickLengthOut() + ticker_->tickLabelPadding() + tickLabelThickness() + labelPadding_;
}

void ColorBar::paintStrip(QPainter* painter) const
{
    const auto& strip = layout_.strip;
    auto gradient = QLinearGradient{};
    if (orientation_ == Vertical) {
        gradient.setStart(strip.bottomLeft());
        gradient.setFinalStop(strip.topLeft());
    } else {
        gradient.setStart(strip.topLeft());
        gradient.setFinalStop(strip.topRight());
    }
    auto previous = qreal{-1.0};
    for (const auto& stop : stops_) {
        const auto position = std::min(qreal{1.0}, std::max(static_cast<qreal>(stop.position), previous + kHardEdgeOffset));
        gradient.setColorAt(position, stop.color);
        previous = position;
    }
    painter->fillRect(strip, gradient);

    if (borderWidth_ > 0.0) {
        auto pen = QPen{borderColor_};
        pen.setWidthF(borderWidth_);
        pen.setJoinStyle(Qt::MiterJoin);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        const auto half = borderWidth_ / 2.0;
        painter->drawRect(strip.adjusted(half, half, -half, -half));
    }
}

void ColorBar::paintTicks(QPainter* painter) const
{
    auto pen = QPen{ticker_->tickColor()};
    pen.setWidthF(ticker_->tickWidth());
    painter->setPen(pen);

    auto params = AxisTickPainter::Params{};
    params.orientation = orientation_ == Vertical ? Axis::Vertical : Axis::Horizontal;
    params.side = orientation_ == Vertical ? Axis::Right : Axis::Bottom;
    params.ticker = ticker_;
    params.hoverColor = ticker_->tickColor();
    params.defaultSubtickColor = ticker_->subtickColor();
    params.clampEdgeLabels = true;
    params.labelOverflow = layout_.endInset;

    AxisTickPainter::paintTicks(painter, layout_.tickArea, layout_.strip.right(), layout_.strip.bottom(), params, ticks_,
        [this](const auto value, const auto length) { return valueToPixel(value, length); });
}

void ColorBar::paintTitle(QPainter* painter) const
{
    if (label_.isEmpty()) {
        return;
    }
    const auto& title = layout_.title;
    painter->save();
    painter->setFont(labelFont_);
    painter->setPen(labelColor_);
    if (orientation_ == Vertical) {
        // Reads top to bottom, like a right-side axis title.
        painter->translate(title.center());
        painter->rotate(90.0);
        painter->drawText(QRectF{-title.height() / 2.0, -title.width() / 2.0, title.height(), title.width()}, Qt::AlignCenter, label_);
    } else {
        painter->drawText(title, Qt::AlignHCenter | Qt::AlignTop, label_);
    }
    painter->restore();
}

} // namespace QAccelPlot
