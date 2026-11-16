//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
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
