

# File MathUtils.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**MathUtils.hpp**](MathUtils_8hpp.md)

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

// Default relative tolerance for nearly_equal.
// Can be overridden at compile time: -DQACCELPLOT_NEARLY_EQUAL_EPSILON=1e-9
#ifndef QACCELPLOT_NEARLY_EQUAL_EPSILON
inline constexpr double kNearlyEqualEpsilon = 1e-12;
#else
inline constexpr double kNearlyEqualEpsilon = QACCELPLOT_NEARLY_EQUAL_EPSILON;
#endif

// Returns true if a and b are nearly equal using a combined relative + absolute
// tolerance. Handles the full double range correctly, including values near zero
// and values at extreme magnitudes (e.g. 1e-20 or 1e21).
//
//  - If a == b exactly (including both infinite with the same sign), returns true.
//  - The relative test scales with max(|a|, |b|) * eps_rel.
//  - The absolute floor (eps_abs) prevents false negatives when both values are
//    near zero, where a purely relative test would require unrealistic precision.
[[nodiscard]] inline bool nearly_equal(double a, double b, double eps_rel = kNearlyEqualEpsilon, double eps_abs = std::numeric_limits<double>::min()) noexcept
{
    if (a == b) {
        return true;
    }
    const auto diff = std::abs(a - b);
    const auto mag = std::max(std::abs(a), std::abs(b));
    if (std::isinf(mag)) {
        return false;
    }
    return diff <= eps_abs || diff <= mag * eps_rel;
}

} // namespace QAccelPlot
```


