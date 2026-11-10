//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/transitions/GrowTransition.hpp"

#include <cmath>
#include <limits>

namespace QAccelPlot {

GrowTransition::GrowTransition(QObject* parent)
    : EnterExitTransition(parent)
    , baseline_(std::numeric_limits<qreal>::quiet_NaN())
{
}

qreal GrowTransition::baseline() const
{
    return baseline_;
}

void GrowTransition::setBaseline(const qreal baseline)
{
    if (baseline_ == baseline || (std::isnan(baseline_) && std::isnan(baseline))) {
        return;
    }
    baseline_ = baseline;
    emit baselineChanged();
}

void GrowTransition::resetBaseline()
{
    setBaseline(std::numeric_limits<qreal>::quiet_NaN());
}

} // namespace QAccelPlot
