//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/internal/HeatmapGrid.hpp"

#include "QAccelPlot/axis/AxisMapping.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace QAccelPlot::Internal {

namespace {

constexpr auto kLn10 = 2.302585092994045684;

} // namespace

bool HeatmapGridDimension::isValid() const
{
    return std::isfinite(from) && std::isfinite(to) && (!logSpacing || (from > 0.0 && to > 0.0));
}

double HeatmapGridDimension::cellCoordinate(const double value) const
{
    if (!logSpacing) {
        return (value - from) / (to - from) * count;
    }
    if (!(value > 0.0)) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    const auto logFrom = std::log10(from);
    return (std::log10(value) - logFrom) / (std::log10(to) - logFrom) * count;
}

double HeatmapGridDimension::cellCenter(const int index) const
{
    if (!logSpacing) {
        return from + (index + 0.5) * ((to - from) / count);
    }
    const auto logFrom = std::log10(from);
    return std::pow(10.0, logFrom + (index + 0.5) * ((std::log10(to) - logFrom) / count));
}

std::optional<std::pair<double, double>> HeatmapGridDimension::dataExtent(const bool logAxis) const
{
    if (!isValid()) {
        return std::nullopt;
    }
    const auto low = std::min(from, to);
    const auto high = std::max(from, to);
    if (!logAxis || low > 0.0) {
        return std::pair{low, high};
    }
    if (high <= 0.0) {
        return std::nullopt;
    }
    const auto cellSize = (high - low) / count;
    const auto edgeIndex = std::floor(-low / cellSize) + 1.0;
    auto edge = low + edgeIndex * cellSize;
    if (edge <= 0.0) {
        edge = low + (edgeIndex + 1.0) * cellSize;
    }
    return std::pair{std::min(edge, high), high};
}

HeatmapShaderMapping HeatmapGridDimension::shaderMapping(const AxisMapping& axis, const double length) const
{
    using Mode = HeatmapShaderMapping::Mode;
    // The axis value at pixel 0 and its change per pixel, in log10 on a log axis.
    const auto mappedStart = axis.flipped ? axis.origin + axis.span : axis.origin;
    const auto mappedPerPixel = (axis.flipped ? -axis.span : axis.span) / length;
    if (axis.logarithmic == logSpacing) {
        const auto start = logSpacing ? std::log10(from) : from;
        const auto end = logSpacing ? std::log10(to) : to;
        const auto cellsPerUnit = count / (end - start);
        return {Mode::Affine, mappedPerPixel * cellsPerUnit, (mappedStart - start) * cellsPerUnit, 0.0, 0.0};
    }
    if (axis.logarithmic) {
        // value = startValue * exp(rate * p), so its cell is offset + scale * expm1(rate * p).
        const auto startValue = std::pow(10.0, mappedStart);
        const auto cellsPerUnit = count / (to - from);
        return {Mode::Exponential, startValue * cellsPerUnit, (startValue - from) * cellsPerUnit, mappedPerPixel * kLn10, 0.0};
    }
    // value = mappedStart + mappedPerPixel * p, so its cell is cellsPerUnit * ln(value / from). Taken
    // relative to the value at pixel 0 where it is positive, the logarithm stays small where cells are large.
    const auto cellsPerUnit = count / (std::log10(to) - std::log10(from)) / kLn10;
    const auto reference = mappedStart > 0.0 ? mappedStart : from;
    return {Mode::Logarithmic, cellsPerUnit, cellsPerUnit * std::log(reference / from), mappedPerPixel / reference, (mappedStart - reference) / reference};
}

} // namespace QAccelPlot::Internal
