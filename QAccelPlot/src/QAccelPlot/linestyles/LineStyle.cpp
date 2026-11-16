//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/linestyles/LineStyle.hpp"

namespace QAccelPlot {

LineStyle::LineStyle(QObject* parent)
    : QObject(parent)
{
}

bool LineStyle::showLine() const
{
    return true;
}

DashParameters LineStyle::dashParameters() const
{
    return {};
}

} // namespace QAccelPlot
