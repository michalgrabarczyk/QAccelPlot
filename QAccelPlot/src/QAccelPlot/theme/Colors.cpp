//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
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
