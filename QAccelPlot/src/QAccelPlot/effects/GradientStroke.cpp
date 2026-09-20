//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/effects/GradientStroke.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/effects/GradientUtils.hpp"

#include <QMetaMethod>
#include <QQmlProperty>
#include <QVariant>

#include <algorithm>

namespace QAccelPlot {

GradientStroke::GradientStroke(QObject* parent)
    : LineCurveEffect(parent)
{
}

GradientDirection GradientStroke::direction() const
{
    return direction_;
}

void GradientStroke::setDirection(GradientDirection direction)
{
    if (direction_ == direction) {
        return;
    }

    direction_ = direction;
    emit directionChanged();
    emit effectChanged();
}

QObject* GradientStroke::gradient() const
{
    return gradient_;
}

void GradientStroke::setGradient(QObject* gradient)
{
    if (gradient_ == gradient) {
        return;
    }

    disconnectGradientSignals();
    gradient_ = gradient;
    reconnectGradientSignals();

    emit gradientChanged();
    emit effectChanged();
}

GradientValueSource GradientStroke::gradientValueMinSource() const
{
    return gradientValueMinSource_;
}

void GradientStroke::setGradientValueMinSource(GradientValueSource source)
{
    if (gradientValueMinSource_ == source) {
        return;
    }

    gradientValueMinSource_ = source;
    emit gradientValueMinSourceChanged();
    emit effectChanged();
}

qreal GradientStroke::gradientValueMin() const
{
    return gradientValueMin_;
}

void GradientStroke::setGradientValueMin(const qreal value)
{
    if (nearly_equal(gradientValueMin_, value)) {
        return;
    }
    gradientValueMin_ = value;
    emit gradientValueMinChanged();
    emit effectChanged();
}

GradientValueSource GradientStroke::gradientValueMaxSource() const
{
    return gradientValueMaxSource_;
}

void GradientStroke::setGradientValueMaxSource(GradientValueSource source)
{
    if (gradientValueMaxSource_ == source) {
        return;
    }

    gradientValueMaxSource_ = source;
    emit gradientValueMaxSourceChanged();
    emit effectChanged();
}

qreal GradientStroke::gradientValueMax() const
{
    return gradientValueMax_;
}

void GradientStroke::setGradientValueMax(const qreal value)
{
    if (nearly_equal(gradientValueMax_, value)) {
        return;
    }
    gradientValueMax_ = value;
    emit gradientValueMaxChanged();
    emit effectChanged();
}

GradientColorPayload GradientStroke::payload() const
{
    GradientColorPayload payload;
    payload.enabled = enabled();
    payload.direction = direction_;

    if (!payload.enabled || !gradient_) {
        return payload;
    }

    payload.stops = readGradientStops(gradient_);
    if (payload.stops.empty()) {
        return payload;
    }

    payload.gradientValueMin = (gradientValueMinSource_ == GradientValueSource::Fixed) ? std::optional<qreal>{gradientValueMin_} : std::nullopt;
    payload.gradientValueMax = (gradientValueMaxSource_ == GradientValueSource::Fixed) ? std::optional<qreal>{gradientValueMax_} : std::nullopt;

    return payload;
}

void GradientStroke::reconnectGradientSignals()
{
    if (!gradient_) {
        return;
    }

    const auto slotIndex = this->metaObject()->indexOfMethod("onGradientObjectChanged()");
    const auto slotMethod = (slotIndex >= 0) ? this->metaObject()->method(slotIndex) : QMetaMethod{};

    gradientConnections_.push_back(connect(gradient_, &QObject::destroyed, this, [this]() {
        gradient_ = nullptr;
        disconnectGradientSignals();
        emit gradientChanged();
        emit effectChanged();
    }));

    const char* gradientSignalNames[] = {
        "changed()",
        "updated()",
        "stopsChanged()",
    };

    for (const auto* signalName : gradientSignalNames) {
        const auto signalIndex = gradient_->metaObject()->indexOfSignal(signalName);
        if (signalIndex < 0) {
            continue;
        }
        if (!slotMethod.isValid()) {
            continue;
        }

        const auto signalMethod = gradient_->metaObject()->method(signalIndex);
        gradientConnections_.push_back(QObject::connect(gradient_, signalMethod, this, slotMethod));
    }

    const auto stopObjects = gradient_->children();
    for (auto* stopObject : stopObjects) {
        if (!stopObject) {
            continue;
        }

        const char* stopSignalNames[] = {
            "changed()",
            "positionChanged()",
            "colorChanged()",
        };

        for (const auto* signalName : stopSignalNames) {
            const auto signalIndex = stopObject->metaObject()->indexOfSignal(signalName);
            if (signalIndex < 0) {
                continue;
            }
            if (!slotMethod.isValid()) {
                continue;
            }

            const auto signalMethod = stopObject->metaObject()->method(signalIndex);
            gradientConnections_.push_back(QObject::connect(stopObject, signalMethod, this, slotMethod));
        }

        gradientConnections_.push_back(connect(stopObject, &QObject::destroyed, this, [this]() { emit effectChanged(); }));
    }
}

void GradientStroke::disconnectGradientSignals()
{
    for (const auto& connection : gradientConnections_) {
        disconnect(connection);
    }
    gradientConnections_.clear();
}

void GradientStroke::onGradientObjectChanged()
{
    emit effectChanged();
}

} // namespace QAccelPlot
