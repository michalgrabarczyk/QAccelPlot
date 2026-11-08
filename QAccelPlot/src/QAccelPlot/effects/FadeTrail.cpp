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

Internal::ItemTrail FadeTrail::trail(const int itemCount) const
{
    return Internal::ItemTrail{static_cast<float>(length_), static_cast<float>(minOpacity_), itemCount};
}

} // namespace QAccelPlot
