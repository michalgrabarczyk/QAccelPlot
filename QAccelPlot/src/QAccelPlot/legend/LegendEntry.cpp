//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/legend/LegendEntry.hpp"

namespace QAccelPlot {

QColor LegendEntry::downCandleColor() const
{
    return secondaryColor.isValid() ? secondaryColor : color;
}

} // namespace QAccelPlot
