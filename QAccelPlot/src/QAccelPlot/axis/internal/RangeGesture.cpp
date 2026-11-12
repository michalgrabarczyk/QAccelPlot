//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/axis/internal/RangeGesture.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace QAccelPlot::Internal {

namespace {

// Effective zoom factor for a factor of 0 or below: prevents a zero or negative range.
constexpr auto kMinEffectiveZoomFactor = qreal{0.01};
// Effective zoom factor for a factor of 1 or above: prevents a no-op zoom.
constexpr auto kMaxEffectiveZoomFactor = qreal{0.99};
// Smallest extent a zoom-in keeps, relative to the larger bound. Far above double precision, so a
// zoom-out step still moves the bounds.
constexpr auto kMinRelativeExtent = qreal{1e-12};

bool usesLogSpace(const ValueRange& range, const bool logarithmic)
{
    return logarithmic && range.min > 0.0 && range.max > 0.0;
}

// Returns \a result, or \a range when a bound of \a result left the range of a double: it is not
// finite, or in log space it reached zero. Also returns \a range when the bounds rounded together.
ValueRange usableOr(const ValueRange& result, const ValueRange& range, const bool logSpace)
{
    const auto finite = std::isfinite(result.min) && std::isfinite(result.max);
    const auto positive = result.min > 0.0 && result.max > 0.0;
    return finite && (positive || !logSpace) && result.min != result.max ? result : range;
}

// True when the extent between \a a and \a b is wide enough for a zoom-out step to move them.
bool wideEnough(const qreal a, const qreal b)
{
    const auto extent = std::abs(b - a);
    return extent >= std::numeric_limits<qreal>::min() && extent >= kMinRelativeExtent * std::max(std::abs(a), std::abs(b));
}

// True when a zoom-out step can move the bounds of \a range. In log space, this must hold for the
// log10 bounds as well.
bool zoomable(const ValueRange& range, const bool logSpace)
{
    return wideEnough(range.min, range.max) && (!logSpace || wideEnough(std::log10(range.min), std::log10(range.max)));
}

// Returns the zoom \a result, or \a range when \a result is unusable or a zoom-in made it too
// narrow to zoom out of again.
ValueRange zoomResultOr(const ValueRange& result, const ValueRange& range, const qreal factor, const bool logSpace)
{
    if (factor < 1.0 && !zoomable(result, logSpace)) {
        return range;
    }
    return usableOr(result, range, logSpace);
}

} // namespace

ValueRange pannedRange(const ValueRange& range, const qreal fraction, const bool logarithmic)
{
    // A round trip through log10 may not return the same bounds.
    if (fraction == 0.0) {
        return range;
    }
    if (usesLogSpace(range, logarithmic)) {
        const auto logMin = std::log10(range.min);
        const auto logMax = std::log10(range.max);
        const auto logShift = fraction * (logMax - logMin);
        return usableOr({std::pow(10.0, logMin + logShift), std::pow(10.0, logMax + logShift)}, range, true);
    }
    const auto shift = fraction * (range.max - range.min);
    return usableOr({range.min + shift, range.max + shift}, range, false);
}

ValueRange zoomedRange(const ValueRange& range, const qreal factor, const qreal anchorRatio, const bool logarithmic)
{
    if (usesLogSpace(range, logarithmic)) {
        const auto logMin = std::log10(range.min);
        const auto logExtent = std::log10(range.max) - logMin;
        const auto logAnchor = logMin + logExtent * anchorRatio;
        const auto newLogExtent = logExtent * factor;
        return zoomResultOr(
            {std::pow(10.0, logAnchor - newLogExtent * anchorRatio), std::pow(10.0, logAnchor + newLogExtent * (1.0 - anchorRatio))}, range, factor, true);
    }
    const auto extent = range.max - range.min;
    const auto anchor = range.min + extent * anchorRatio;
    const auto newExtent = extent * factor;
    return zoomResultOr({anchor - newExtent * anchorRatio, anchor + newExtent * (1.0 - anchorRatio)}, range, factor, false);
}

qreal wheelZoomFactor(const qreal zoomScaleFactor, const bool zoomingIn, const qreal steps)
{
    auto zoomInFactor = zoomScaleFactor;
    if (zoomScaleFactor <= 0.0) {
        zoomInFactor = kMinEffectiveZoomFactor;
    } else if (zoomScaleFactor >= 1.0) {
        zoomInFactor = kMaxEffectiveZoomFactor;
    }
    return std::pow(zoomingIn ? zoomInFactor : 1.0 / zoomInFactor, steps);
}

} // namespace QAccelPlot::Internal
