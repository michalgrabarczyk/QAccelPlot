//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/effects/SeriesEffect.hpp"

namespace QAccelPlot {

SeriesEffect::SeriesEffect(QObject* parent)
    : QObject(parent)
{
}

bool SeriesEffect::enabled() const
{
    return enabled_;
}

void SeriesEffect::setEnabled(const bool enabled)
{
    if (enabled_ == enabled) {
        return;
    }

    enabled_ = enabled;
    emit enabledChanged();
    emit effectChanged();
}

void SeriesEffect::refresh()
{
}

} // namespace QAccelPlot
