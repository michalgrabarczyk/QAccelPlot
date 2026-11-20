//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/effects/GradientFill.hpp"

#include "QAccelPlot/MathUtils.hpp"
#include "QAccelPlot/effects/internal/GradientCoordinates.hpp"
#include "QAccelPlot/effects/internal/GradientStops.hpp"

#include <QMetaMethod>
#include <QQmlProperty>
#include <QVariant>

#include <algorithm>
#include <limits>

namespace QAccelPlot {

namespace {

constexpr auto kNaN = std::numeric_limits<qreal>::quiet_NaN();

} // namespace

GradientFill::GradientFill(QObject* parent)
    : SeriesEffect(parent)
{
    // Connected first, so curves reacting to effectChanged() see the updated payload.
    connect(this, &SeriesEffect::effectChanged, this, &GradientFill::updatePayload);
    updatePayload();
}

Qt::Orientation GradientFill::direction() const
{
    return direction_;
}

void GradientFill::setDirection(Qt::Orientation direction)
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

qreal GradientFill::valueMin() const
{
    return valueMin_;
}

void GradientFill::setValueMin(const qreal value)
{
    if (nearlyEqualOrBothNaN(valueMin_, value)) {
        return;
    }

    valueMin_ = value;
    emit valueMinChanged();
    emit effectChanged();
}

void GradientFill::resetValueMin()
{
    setValueMin(kNaN);
}

qreal GradientFill::valueMax() const
{
    return valueMax_;
}

void GradientFill::setValueMax(const qreal value)
{
    if (nearlyEqualOrBothNaN(valueMax_, value)) {
        return;
    }

    valueMax_ = value;
    emit valueMaxChanged();
    emit effectChanged();
}

void GradientFill::resetValueMax()
{
    setValueMax(kNaN);
}

qreal GradientFill::baselineValue() const
{
    return baselineValue_;
}

void GradientFill::setBaselineValue(const qreal value)
{
    if (nearlyEqualOrBothNaN(baselineValue_, value)) {
        return;
    }

    baselineValue_ = value;
    emit baselineValueChanged();
    emit effectChanged();
}

void GradientFill::resetBaselineValue()
{
    setBaselineValue(kNaN);
}

qreal GradientFill::opacity() const
{
    return opacity_;
}

void GradientFill::setOpacity(const qreal value)
{
    const auto clamped = std::clamp(value, qreal{0.0}, qreal{1.0});
    if (nearlyEqual(opacity_, clamped)) {
        return;
    }

    opacity_ = clamped;
    emit opacityChanged();
    emit effectChanged();
}

const GradientFillPayload& GradientFill::payload() const
{
    return payload_;
}

void GradientFill::refresh()
{
    if (colormap_ || !gradient_ || !enabled()) {
        return;
    }
    if (Internal::readEffectStops(colormap_, gradient_) != payload_.stops) {
        emit effectChanged();
    }
}

GradientFillPayload GradientFill::readPayload() const
{
    auto payload = GradientFillPayload{};
    payload.enabled = enabled();
    payload.direction = direction_;
    payload.gradientValueMin = Internal::valueIfSet(valueMin_);
    payload.gradientValueMax = Internal::valueIfSet(valueMax_);
    payload.baselineValue = Internal::valueIfSet(baselineValue_);
    payload.opacity = static_cast<float>(opacity_);

    if (!payload.enabled) {
        return payload;
    }

    payload.stops = Internal::readEffectStops(colormap_, gradient_);
    return payload;
}

void GradientFill::updatePayload()
{
    payload_ = readPayload();
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

    colormapConnections_.push_back(connect(colormap_, &Colormap::colormapChanged, this, &SeriesEffect::effectChanged));
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
