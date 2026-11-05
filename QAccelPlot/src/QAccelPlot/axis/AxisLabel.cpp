//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/AxisLabel.hpp"
#include "QAccelPlot/MathUtils.hpp"

#include <cmath>

namespace QAccelPlot {

namespace {

constexpr auto kDefaultAxisLabelPadding = qreal{30.0};

}

AxisLabel::AxisLabel(QObject* parent)
    : TextLabel({}, kDefaultAxisLabelPadding, parent)
{
}

Axis::LabelAlignment AxisLabel::alignment() const
{
    return alignment_;
}

void AxisLabel::setAlignment(const Axis::LabelAlignment alignment)
{
    if (alignment_ == alignment) {
        return;
    }
    alignment_ = alignment;
    emit alignmentChanged();
}

qreal AxisLabel::rotation() const
{
    return std::isfinite(rotation_) ? rotation_ : defaultRotation_;
}

void AxisLabel::setRotation(const qreal degrees)
{
    if (!std::isfinite(degrees)) {
        resetRotation();
        return;
    }
    if (std::isfinite(rotation_) && nearlyEqual(rotation_, degrees)) {
        return;
    }
    const auto previousRotation = rotation();
    rotation_ = degrees;
    if (!nearlyEqual(previousRotation, degrees)) {
        emit rotationChanged();
    }
}

void AxisLabel::resetRotation()
{
    if (!std::isfinite(rotation_)) {
        return;
    }
    const auto previousRotation = rotation_;
    rotation_ = std::numeric_limits<qreal>::quiet_NaN();
    if (!nearlyEqual(previousRotation, defaultRotation_)) {
        emit rotationChanged();
    }
}

void AxisLabel::setDefaultRotation(const qreal degrees)
{
    if (nearlyEqual(defaultRotation_, degrees)) {
        return;
    }
    defaultRotation_ = degrees;
    if (!std::isfinite(rotation_)) {
        emit rotationChanged();
    }
}

QPointF AxisLabel::offset() const
{
    return offset_;
}

void AxisLabel::setOffset(const QPointF& offset)
{
    if (offset_ == offset) {
        return;
    }
    offset_ = offset;
    emit offsetChanged();
}

QQmlComponent* AxisLabel::delegate() const
{
    return delegate_;
}

void AxisLabel::setDelegate(QQmlComponent* delegate)
{
    if (delegate_ == delegate) {
        return;
    }
    delegate_ = delegate;
    emit delegateChanged();
}

} // namespace QAccelPlot
