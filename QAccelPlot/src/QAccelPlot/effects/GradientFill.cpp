//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/effects/GradientFill.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/effects/GradientUtils.hpp"

#include <QMetaMethod>
#include <QQmlProperty>
#include <QVariant>

#include <algorithm>

namespace QAccelPlot {

GradientFill::GradientFill(QObject* parent)
    : LineCurveEffect(parent)
{
}

GradientDirection GradientFill::direction() const
{
    return direction_;
}

void GradientFill::setDirection(GradientDirection direction)
{
    if (direction_ == direction) {
        return;
    }

    direction_ = direction;
    emit directionChanged();
    emit effectChanged();
}

QObject* GradientFill::gradient() const
{
    return gradient_;
}

void GradientFill::setGradient(QObject* gradient)
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

Colormap* GradientFill::colormap() const
{
    return colormap_;
}

void GradientFill::setColormap(Colormap* colormap)
{
    if (colormap_ == colormap) {
        return;
    }

    disconnectColormapSignals();
    colormap_ = colormap;
    reconnectColormapSignals();

    emit colormapChanged();
    emit effectChanged();
}

GradientValueSource GradientFill::gradientValueMinSource() const
{
    return gradientValueMinSource_;
}

void GradientFill::setGradientValueMinSource(GradientValueSource source)
{
    if (gradientValueMinSource_ == source) {
        return;
    }

    gradientValueMinSource_ = source;
    emit gradientValueMinSourceChanged();
    emit effectChanged();
}

qreal GradientFill::gradientValueMin() const
{
    return gradientValueMin_;
}

void GradientFill::setGradientValueMin(const qreal value)
{
    if (nearly_equal(gradientValueMin_, value)) {
        return;
    }

    gradientValueMin_ = value;
    emit gradientValueMinChanged();
    emit effectChanged();
}

GradientValueSource GradientFill::gradientValueMaxSource() const
{
    return gradientValueMaxSource_;
}

void GradientFill::setGradientValueMaxSource(GradientValueSource source)
{
    if (gradientValueMaxSource_ == source) {
        return;
    }

    gradientValueMaxSource_ = source;
    emit gradientValueMaxSourceChanged();
    emit effectChanged();
}

qreal GradientFill::gradientValueMax() const
{
    return gradientValueMax_;
}

void GradientFill::setGradientValueMax(const qreal value)
{
    if (nearly_equal(gradientValueMax_, value)) {
        return;
    }

    gradientValueMax_ = value;
    emit gradientValueMaxChanged();
    emit effectChanged();
}

GradientFillBaseline GradientFill::baseline() const
{
    return baseline_;
}

void GradientFill::setBaseline(GradientFillBaseline baseline)
{
    if (baseline_ == baseline) {
        return;
    }

    baseline_ = baseline;
    emit baselineChanged();
    emit effectChanged();
}

qreal GradientFill::baselineValue() const
{
    return baselineValue_;
}

void GradientFill::setBaselineValue(const qreal value)
{
    if (nearly_equal(baselineValue_, value)) {
        return;
    }

    baselineValue_ = value;
    emit baselineValueChanged();
    emit effectChanged();
}

qreal GradientFill::opacity() const
{
    return opacity_;
}

void GradientFill::setOpacity(const qreal value)
{
    const auto clamped = std::clamp(value, qreal{0.0}, qreal{1.0});
    if (nearly_equal(opacity_, clamped)) {
        return;
    }

    opacity_ = clamped;
    emit opacityChanged();
    emit effectChanged();
}

GradientFillPayload GradientFill::payload() const
{
    auto payload = GradientFillPayload{};
    payload.enabled = enabled();
    payload.direction = direction_;
    payload.gradientValueMin = (gradientValueMinSource_ == GradientValueSource::Fixed) ? std::optional<qreal>{gradientValueMin_} : std::nullopt;
    payload.gradientValueMax = (gradientValueMaxSource_ == GradientValueSource::Fixed) ? std::optional<qreal>{gradientValueMax_} : std::nullopt;
    payload.baseline = baseline_;
    payload.baselineValue = baselineValue_;
    payload.opacity = static_cast<float>(opacity_);

    if (!payload.enabled) {
        return payload;
    }

    payload.stops = readEffectStops(colormap_, gradient_);
    return payload;
}

void GradientFill::reconnectGradientSignals()
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

void GradientFill::disconnectGradientSignals()
{
    for (const auto& connection : gradientConnections_) {
        disconnect(connection);
    }
    gradientConnections_.clear();
}

void GradientFill::onGradientObjectChanged()
{
    emit effectChanged();
}

void GradientFill::reconnectColormapSignals()
{
    if (!colormap_) {
        return;
    }

    colormapConnections_.push_back(connect(colormap_, &Colormap::colormapChanged, this, &LineCurveEffect::effectChanged));
    colormapConnections_.push_back(connect(colormap_, &QObject::destroyed, this, [this]() {
        colormap_ = nullptr;
        disconnectColormapSignals();
        emit colormapChanged();
        emit effectChanged();
    }));
}

void GradientFill::disconnectColormapSignals()
{
    for (const auto& connection : colormapConnections_) {
        disconnect(connection);
    }
    colormapConnections_.clear();
}

} // namespace QAccelPlot
