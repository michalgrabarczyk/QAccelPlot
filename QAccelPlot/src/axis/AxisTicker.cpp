//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "axis/AxisTicker.hpp"

#include "MathUtils.hpp"
#include "formatters/NumericTickLabelFormatter.hpp"

namespace QAccelPlot {

AxisTicker::AxisTicker(QObject* parent)
    : QObject(parent)
    , defaultTickLabelFormatter_(new NumericTickLabelFormatter(this))
{
    connect(defaultTickLabelFormatter_, &TickLabelFormatter::formatChanged, this, [this]() {
        if (!customTickLabelFormatter_) {
            emit tickLabelFormatChanged();
        }
    });
}

QColor AxisTicker::tickColor() const
{
    return tickColor_;
}

void AxisTicker::setTickColor(const QColor& c)
{
    if (tickColor_ == c) {
        return;
    }
    tickColor_ = c;
    emit tickColorChanged();
}

QColor AxisTicker::tickLabelColor() const
{
    return tickLabelColor_;
}

void AxisTicker::setTickLabelColor(const QColor& c)
{
    if (tickLabelColor_ == c) {
        return;
    }
    tickLabelColor_ = c;
    emit tickLabelColorChanged();
}

int AxisTicker::tickCount() const
{
    return tickCount_;
}

void AxisTicker::setTickCount(const int count)
{
    if (tickCount_ == count) {
        return;
    }
    tickCount_ = count;
    emit tickCountChanged();
}

int AxisTicker::subtickCount() const
{
    return subtickCount_;
}

void AxisTicker::setSubtickCount(const int count)
{
    if (subtickCount_ == count) {
        return;
    }
    subtickCount_ = count;
    emit subtickCountChanged();
}

qreal AxisTicker::tickLength() const
{
    return tickLengthIn_ + tickLengthOut_;
}

void AxisTicker::setTickLength(qreal length)
{
    const auto half = length / 2.0;
    if (nearly_equal(tickLengthIn_, half) && nearly_equal(tickLengthOut_, half)) {
        return;
    }
    tickLengthIn_ = half;
    tickLengthOut_ = half;
    emit tickLengthChanged();
    emit tickLengthInChanged();
    emit tickLengthOutChanged();
}

qreal AxisTicker::subtickLength() const
{
    return subtickLengthIn_ + subtickLengthOut_;
}

void AxisTicker::setSubtickLength(qreal length)
{
    const auto half = length / 2.0;
    if (nearly_equal(subtickLengthIn_, half) && nearly_equal(subtickLengthOut_, half)) {
        return;
    }
    subtickLengthIn_ = half;
    subtickLengthOut_ = half;
    emit subtickLengthChanged();
    emit subtickLengthInChanged();
    emit subtickLengthOutChanged();
}

qreal AxisTicker::tickLengthIn() const
{
    return tickLengthIn_;
}

void AxisTicker::setTickLengthIn(qreal length)
{
    if (nearly_equal(tickLengthIn_, length)) {
        return;
    }
    tickLengthIn_ = length;
    emit tickLengthInChanged();
}

qreal AxisTicker::tickLengthOut() const
{
    return tickLengthOut_;
}

void AxisTicker::setTickLengthOut(qreal length)
{
    if (nearly_equal(tickLengthOut_, length)) {
        return;
    }
    tickLengthOut_ = length;
    emit tickLengthOutChanged();
}

qreal AxisTicker::subtickLengthIn() const
{
    return subtickLengthIn_;
}

void AxisTicker::setSubtickLengthIn(qreal length)
{
    if (nearly_equal(subtickLengthIn_, length)) {
        return;
    }
    subtickLengthIn_ = length;
    emit subtickLengthInChanged();
}

qreal AxisTicker::subtickLengthOut() const
{
    return subtickLengthOut_;
}

void AxisTicker::setSubtickLengthOut(qreal length)
{
    if (nearly_equal(subtickLengthOut_, length)) {
        return;
    }
    subtickLengthOut_ = length;
    emit subtickLengthOutChanged();
}

QColor AxisTicker::subtickColor() const
{
    return subtickColor_;
}

void AxisTicker::setSubtickColor(const QColor& c)
{
    if (subtickColor_ == c) {
        return;
    }
    subtickColor_ = c;
    emit subtickColorChanged();
}

qreal AxisTicker::tickWidth() const
{
    return tickWidth_;
}

void AxisTicker::setTickWidth(qreal width)
{
    if (nearly_equal(tickWidth_, width)) {
        return;
    }
    tickWidth_ = width;
    emit tickWidthChanged();
}

qreal AxisTicker::subtickWidth() const
{
    return subtickWidth_;
}

void AxisTicker::setSubtickWidth(qreal width)
{
    if (nearly_equal(subtickWidth_, width)) {
        return;
    }
    subtickWidth_ = width;
    emit subtickWidthChanged();
}

qreal AxisTicker::tickLabelPadding() const
{
    return tickLabelPadding_;
}

void AxisTicker::setTickLabelPadding(qreal padding)
{
    if (nearly_equal(tickLabelPadding_, padding)) {
        return;
    }
    tickLabelPadding_ = padding;
    emit tickLabelPaddingChanged();
}

qreal AxisTicker::tickLabelRotation() const
{
    return tickLabelRotation_;
}

void AxisTicker::setTickLabelRotation(qreal rotation)
{
    if (nearly_equal(tickLabelRotation_, rotation)) {
        return;
    }
    tickLabelRotation_ = rotation;
    emit tickLabelRotationChanged();
}

QFont AxisTicker::tickLabelFont() const
{
    return tickLabelFont_;
}

void AxisTicker::setTickLabelFont(const QFont& f)
{
    if (tickLabelFont_ == f) {
        return;
    }
    tickLabelFont_ = f;
    emit tickLabelFontChanged();
}

TickLabelFormatter* AxisTicker::tickLabelFormatter() const
{
    return customTickLabelFormatter_ ? customTickLabelFormatter_.data() : defaultTickLabelFormatter_;
}

void AxisTicker::setTickLabelFormatter(TickLabelFormatter* formatter)
{
    auto* customFormatter = formatter == defaultTickLabelFormatter_ ? nullptr : formatter;
    if (customTickLabelFormatter_ == customFormatter) {
        return;
    }
    if (customTickLabelFormatter_) {
        disconnect(customTickLabelFormatter_, &TickLabelFormatter::formatChanged, this, &AxisTicker::tickLabelFormatChanged);
    }
    disconnect(customFormatterDestroyedConnection_);

    customTickLabelFormatter_ = customFormatter;
    if (customTickLabelFormatter_) {
        connect(customTickLabelFormatter_, &TickLabelFormatter::formatChanged, this, &AxisTicker::tickLabelFormatChanged);
        customFormatterDestroyedConnection_ = connect(customTickLabelFormatter_, &QObject::destroyed, this, [this]() {
            customTickLabelFormatter_.clear();
            customFormatterDestroyedConnection_ = {};
            emit tickLabelFormatterChanged();
        });
    }
    emit tickLabelFormatterChanged();
}

} // namespace QAccelPlot
