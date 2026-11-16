//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/transitions/DrawTransition.hpp"

namespace QAccelPlot {

DrawTransition::DrawTransition(QObject* parent)
    : EnterExitTransition(parent)
{
    setStagger(1.0);
}

} // namespace QAccelPlot
