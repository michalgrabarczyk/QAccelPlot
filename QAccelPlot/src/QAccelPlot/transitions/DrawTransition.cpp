//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/transitions/DrawTransition.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace QAccelPlot {

DrawTransition::DrawTransition(QObject* parent)
    : DataTransition(parent)
{
}

void DrawTransition::interpolate(const double easedProgress, const Dataset& /*from*/, const Dataset& to, Dataset& out)
{
    out.count = std::clamp(static_cast<int>(std::ceil(to.count * easedProgress)), std::min(to.count, 1), to.count);
    out.values.assign(to.values.begin(), to.values.begin() + static_cast<std::ptrdiff_t>(out.count) * to.stride);
}

} // namespace QAccelPlot
