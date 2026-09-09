//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "linestyles/NoLine.hpp"

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
