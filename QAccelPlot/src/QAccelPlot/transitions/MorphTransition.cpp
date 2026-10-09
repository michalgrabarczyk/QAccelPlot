//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/transitions/MorphTransition.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace QAccelPlot {

MorphTransition::MorphTransition(QObject* parent)
    : DataTransition(parent)
{
}

double MorphTransition::interpolateCoordinate(const double from, const double to, const double easedProgress)
{
    // An invalid target appears immediately as a gap, and a valid target never
    // interpolates from an invalid source (which would stay NaN until the end).
    if (!std::isfinite(to) || !std::isfinite(from)) {
        return to;
    }
    return from + easedProgress * (to - from);
}

void MorphTransition::interpolate(const double easedProgress, const Dataset& from, const Dataset& to, Dataset& out)
{
    if (to.count <= 0 || from.count <= 0) {
        out.count = std::max(to.count, 0);
        out.values.assign(to.values.begin(), to.values.begin() + static_cast<std::ptrdiff_t>(out.count) * to.stride);
        return;
    }

    const auto stride = static_cast<std::size_t>(to.stride);
    out.count = std::max(from.count, to.count);
    out.values.resize(static_cast<std::size_t>(out.count) * stride);

    for (auto i = 0; i < out.count; ++i) {
        const auto* fromPoint = from.values.data() + static_cast<std::size_t>(std::min(i, from.count - 1)) * stride;
        const auto* toPoint = to.values.data() + static_cast<std::size_t>(std::min(i, to.count - 1)) * stride;
        auto* outPoint = out.values.data() + static_cast<std::size_t>(i) * stride;
        for (auto value = std::size_t{0}; value < stride; ++value) {
            outPoint[value] = interpolateCoordinate(fromPoint[value], toPoint[value], easedProgress);
        }
    }
}

} // namespace QAccelPlot
