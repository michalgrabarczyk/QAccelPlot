//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/axis/AxisLabel.hpp"

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
    return rotation_.value();
}

void AxisLabel::setRotation(const qreal degrees)
{
    if (!std::isfinite(degrees)) {
        resetRotation();
        return;
    }
    if (rotation_.set(degrees)) {
        emit rotationChanged();
    }
}

void AxisLabel::resetRotation()
{
    if (rotation_.reset()) {
        emit rotationChanged();
    }
}

void AxisLabel::setDefaultRotation(const qreal degrees)
{
    if (rotation_.setFallback(degrees)) {
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
