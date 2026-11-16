//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/transitions/MorphTransition.hpp"

#include <cmath>

namespace QAccelPlot {

MorphTransition::MorphTransition(QObject* parent)
    : DataTransition(parent)
{
}

double MorphTransition::interpolateCoordinate(const double from, const double to, const double easedProgress)
{
    // An invalid target appears immediately as a gap, and a valid target never
    // interpolates from an invalid source (which would stay NaN until the end).
    if (!std::isfinite(to) || !std::isfinite(from)) {
        return to;
    }
    return from + easedProgress * (to - from);
}

} // namespace QAccelPlot
