//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "linestyles/LineStyle.hpp"

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
