//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
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
