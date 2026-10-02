//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/inspection/internal/InspectionTypes.hpp"

#include "QAccelPlot/MathUtils.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace QAccelPlot {

bool InspectionSource::supported() const noexcept
{
    return count >= 0;
}

double InspectionSource::x(const int index) const noexcept
{
    const auto i = static_cast<std::size_t>(index);
    return doubles ? doubles[i * 2] : static_cast<double>(floats[i * static_cast<std::size_t>(floatStride)]);
}

double InspectionSource::y(const int index) const noexcept
{
    const auto i = static_cast<std::size_t>(index);
    return doubles ? doubles[i * 2 + 1] : static_cast<double>(floats[i * static_cast<std::size_t>(floatStride) + 1]);
}

double InspectionSource::value(const int index) const noexcept
{
    return values ? static_cast<double>(values[static_cast<std::size_t>(index) * static_cast<std::size_t>(valueStride)])
                  : std::numeric_limits<double>::quiet_NaN();
}

bool InspectionSource::valid(const int index) const noexcept
{
    return isValidSample(x(index), logX) && isValidSample(y(index), logY);
}

double InspectionMetric::pixelX(const double value) const noexcept
{
    return x.toPixel(value, width);
}

double InspectionMetric::pixelY(const double value) const noexcept
{
    return y.toPixel(value, height);
}

double InspectionMetric::coordX(const double pixel) const noexcept
{
    return x.toCoord(pixel, width);
}

bool InspectionBounds::contains(const double x, const double y) const noexcept
{
    return x >= xMin && x <= xMax && y >= yMin && y <= yMax;
}

void InspectionHit::consider(const int candidate, const double candidateDistance) noexcept
{
    if (candidateDistance <= distance && (candidateDistance < distance || candidate > index)) {
        index = candidate;
        distance = candidateDistance;
    }
}

void SummaryAccumulator::add(const double y, const int index) noexcept
{
    if (count == 0 || y < minimum || (y == minimum && index > minimumIndex)) {
        minimum = y;
        minimumIndex = index;
    }
    if (count == 0 || y > maximum || (y == maximum && index > maximumIndex)) {
        maximum = y;
        maximumIndex = index;
    }
    ++count;
    const auto delta = y - mean;
    mean += delta / count;
    m2 += delta * (y - mean);
}

void SummaryAccumulator::merge(const SummaryAccumulator& other) noexcept
{
    if (other.count == 0) {
        return;
    }
    if (count == 0) {
        *this = other;
        return;
    }
    if (other.minimum < minimum || (other.minimum == minimum && other.minimumIndex > minimumIndex)) {
        minimum = other.minimum;
        minimumIndex = other.minimumIndex;
    }
    if (other.maximum > maximum || (other.maximum == maximum && other.maximumIndex > maximumIndex)) {
        maximum = other.maximum;
        maximumIndex = other.maximumIndex;
    }
    const auto combined = static_cast<double>(count) + static_cast<double>(other.count);
    const auto delta = other.mean - mean;
    m2 += other.m2 + delta * delta * static_cast<double>(count) * static_cast<double>(other.count) / combined;
    mean += delta * static_cast<double>(other.count) / combined;
    count += other.count;
}

double SummaryAccumulator::standardDeviation() const noexcept
{
    return count > 0 ? std::sqrt(std::max(0.0, m2 / count)) : std::numeric_limits<double>::quiet_NaN();
}

double distanceToInterval(const double position, const double a, const double b) noexcept
{
    return std::clamp(position, std::min(a, b), std::max(a, b)) - position;
}

} // namespace QAccelPlot
