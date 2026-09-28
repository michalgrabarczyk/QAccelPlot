

# File MathUtils.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**MathUtils.hpp**](MathUtils_8hpp.md)

[Go to the documentation of this file](MathUtils_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <algorithm>
#include <cmath>
#include <limits>

namespace QAccelPlot {

// Default relative tolerance for nearly_equal. Two machine-epsilon steps keep
// the comparison close to representable double precision at every magnitude.
// Can be overridden at compile time: -DQACCELPLOT_NEARLY_EQUAL_EPSILON=1e-9
#ifndef QACCELPLOT_NEARLY_EQUAL_EPSILON
inline constexpr double kNearlyEqualEpsilon = 2.0 * std::numeric_limits<double>::epsilon();
#else
inline constexpr double kNearlyEqualEpsilon = QACCELPLOT_NEARLY_EQUAL_EPSILON;
#endif

// Returns true if a and b are nearly equal using a combined relative + absolute
// tolerance.
//
//  - If a == b exactly (including both infinite with the same sign), returns true.
//  - The relative test scales with max(1, |a|, |b|) * eps_rel and therefore tracks
//    a small number of representable steps instead of using a broad tolerance.
//  - The absolute floor (eps_abs) prevents false negatives when both values are
//    near zero, where a purely relative test would require unrealistic precision.
[[nodiscard]] inline bool nearly_equal(double a, double b, double eps_rel = kNearlyEqualEpsilon, double eps_abs = kNearlyEqualEpsilon) noexcept
{
    if (a == b) {
        return true;
    }

    if (!std::isfinite(a) || !std::isfinite(b)) {
        return false;
    }

    const auto diff = std::abs(a - b);
    const auto scale = std::max({1.0, std::abs(a), std::abs(b)});
    return diff <= std::max(eps_abs, scale * eps_rel);
}

// Returns true when float coordinates stored relative to origin lose precision in the
// viewport [viewportMin, viewportMax]. Series upload double data to the GPU as floats
// relative to a render origin; far from it, the float spacing becomes visible. At 64
// viewport widths the rounding error stays below 1e-5 of the viewport.
[[nodiscard]] inline bool renderOriginTooFar(double origin, double viewportMin, double viewportMax) noexcept
{
    constexpr auto kMaxDistanceInViewports = 64.0;
    const auto span = viewportMax - viewportMin;
    if (!(span > 0.0) || !std::isfinite(span) || !std::isfinite(origin)) {
        return false;
    }
    const auto center = viewportMin + span * 0.5;
    return std::abs(center - origin) > kMaxDistanceInViewports * span;
}

// Returns origin, or the viewport center when origin is too far from the viewport.
[[nodiscard]] inline double renderOriginForViewport(double origin, double viewportMin, double viewportMax) noexcept
{
    return renderOriginTooFar(origin, viewportMin, viewportMax) ? viewportMin + (viewportMax - viewportMin) * 0.5 : origin;
}

// Returns true if a single sample coordinate can be plotted.
//
// A coordinate is invalid when it is non-finite (NaN or +/-Inf) or, on a
// logarithmic dimension, when it is not strictly positive. Invalid coordinates
// break line curves, are excluded from auto-ranging, and are never hit-tested.
//
// Implemented with plain comparisons (false for NaN and +/-Inf) rather than
// std::isfinite(), which some standard libraries do not inline. This check runs
// per coordinate on multi-million-point buffers.
[[nodiscard]] inline bool isValidSample(double value, bool logScale) noexcept
{
    return value >= std::numeric_limits<double>::lowest() && value <= std::numeric_limits<double>::max() && (!logScale || value > 0.0);
}

} // namespace QAccelPlot
```


