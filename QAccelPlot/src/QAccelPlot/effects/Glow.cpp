//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/effects/Glow.hpp"

#include "QAccelPlot/internal/GlowStyle.hpp"

#include <algorithm>

namespace QAccelPlot {

Glow::Glow(QObject* parent)
    : SeriesEffect(parent)
{
}

QColor Glow::color() const
{
    return color_;
}

void Glow::setColor(const QColor& color)
{
    if (color_ == color) {
        return;
    }
    color_ = color;
    emit colorChanged();
    emit effectChanged();
}

void Glow::resetColor()
{
    setColor(QColor{});
}

qreal Glow::radius() const
{
    return radius_;
}

void Glow::setRadius(const qreal radius)
{
    const auto clamped = std::max(radius, 0.0);
    if (radius_ == clamped) {
        return;
    }
    radius_ = clamped;
    emit radiusChanged();
    emit effectChanged();
}

qreal Glow::intensity() const
{
    return intensity_;
}

void Glow::setIntensity(const qreal intensity)
{
    const auto clamped = std::clamp(intensity, 0.0, 1.0);
    if (intensity_ == clamped) {
        return;
    }
    intensity_ = clamped;
    emit intensityChanged();
    emit effectChanged();
}

Internal::GlowStyle Glow::style(const QColor& seriesColor) const
{
    return Internal::GlowStyle{color_.isValid() ? color_ : seriesColor, static_cast<float>(radius_), static_cast<float>(intensity_)};
}

} // namespace QAccelPlot
