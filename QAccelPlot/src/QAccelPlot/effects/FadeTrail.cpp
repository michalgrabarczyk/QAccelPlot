//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/effects/FadeTrail.hpp"

#include "QAccelPlot/internal/ItemFade.hpp"

#include <algorithm>
#include <cstddef>

namespace QAccelPlot {

FadeTrail::FadeTrail(QObject* parent)
    : SeriesEffect(parent)
{
}

qreal FadeTrail::length() const
{
    return length_;
}

void FadeTrail::setLength(const qreal length)
{
    const auto clamped = std::clamp(length, 0.0, 1.0);
    if (length_ == clamped) {
        return;
    }
    length_ = clamped;
    emit lengthChanged();
    emit effectChanged();
}

qreal FadeTrail::minOpacity() const
{
    return minOpacity_;
}

void FadeTrail::setMinOpacity(const qreal opacity)
{
    const auto clamped = std::clamp(opacity, 0.0, 1.0);
    if (minOpacity_ == clamped) {
        return;
    }
    minOpacity_ = clamped;
    emit minOpacityChanged();
    emit effectChanged();
}

QEasingCurve FadeTrail::easing() const
{
    return easing_;
}

void FadeTrail::setEasing(const QEasingCurve& easing)
{
    if (easing_ == easing) {
        return;
    }
    easing_ = easing;
    emit easingChanged();
    emit effectChanged();
}

Internal::ItemTrail FadeTrail::trail(const int itemCount) const
{
    auto trail = Internal::ItemTrail{static_cast<float>(length_), static_cast<float>(minOpacity_), itemCount};
    for (auto index = std::size_t{0}; index < trail.easing.size(); ++index) {
        const auto progress = static_cast<qreal>(index) / static_cast<qreal>(trail.easing.size() - 1);
        trail.easing[index] = static_cast<float>(std::clamp(easing_.valueForProgress(progress), 0.0, 1.0));
    }
    return trail;
}

} // namespace QAccelPlot
