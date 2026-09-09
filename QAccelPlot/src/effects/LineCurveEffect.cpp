//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "effects/LineCurveEffect.hpp"

namespace QAccelPlot {

LineCurveEffect::LineCurveEffect(QObject* parent)
    : QObject(parent)
{
}

bool LineCurveEffect::enabled() const
{
    return enabled_;
}

void LineCurveEffect::setEnabled(const bool enabled)
{
    if (enabled_ == enabled) {
        return;
    }

    enabled_ = enabled;
    emit enabledChanged();
    emit effectChanged();
}

} // namespace QAccelPlot
