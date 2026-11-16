//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/linestyles/NoLine.hpp"

namespace QAccelPlot {

NoLine::NoLine(QObject* parent)
    : LineStyle(parent)
{
}

bool NoLine::showLine() const
{
    return false;
}

} // namespace QAccelPlot
