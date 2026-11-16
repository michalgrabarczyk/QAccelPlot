//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/theme/Colors.hpp"

namespace QAccelPlot {

Colors::Colors(QObject* parent)
    : QObject(parent)
{
}

// The palettes are exposed to QML as CONSTANT, read-only properties, so the const_cast
// cannot lead to mutation.
ColorPalette* Colors::dark() const
{
    return const_cast<ColorPalette*>(&ColorPalette::dark());
}

ColorPalette* Colors::light() const
{
    return const_cast<ColorPalette*>(&ColorPalette::light());
}

} // namespace QAccelPlot
