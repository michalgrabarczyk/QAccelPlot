//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/AxisMapping.hpp"

#include <cmath>

namespace QAccelPlot {

double AxisMapping::toMapped(const double value) const noexcept
{
    return logarithmic ? std::log10(value) : value;
}

double AxisMapping::toPixel(const double value, const double length) const noexcept
{
    const auto ratio = (toMapped(value) - origin) / span;
    return (flipped ? 1.0 - ratio : ratio) * length;
}

double AxisMapping::toCoord(const double pixel, const double length) const noexcept
{
    const auto ratio = flipped ? 1.0 - (pixel / length) : pixel / length;
    const auto mapped = origin + ratio * span;
    return logarithmic ? std::pow(10.0, mapped) : mapped;
}

} // namespace QAccelPlot
