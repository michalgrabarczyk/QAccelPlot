

# File GradientCoordinateUtils.hpp

[**File List**](files.md) **>** [**effects**](dir_a4c3deeda37ae6198148ecdf2e43fc36.md) **>** [**GradientCoordinateUtils.hpp**](GradientCoordinateUtils_8hpp.md)

[Go to the documentation of this file](GradientCoordinateUtils_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include "effects/GradientColorTypes.hpp"

#include <algorithm>

namespace QAccelPlot {

inline float unboundedGradientCoordinate(
    const GradientDirection direction, const float value, const float minimum, const float maximum)
{
    constexpr auto minimumRange = float{1e-6f};
    const auto range = std::max(maximum - minimum, minimumRange);
    const auto normalized = (value - minimum) / range;
    return direction == GradientDirection::Vertical ? 1.0f - normalized : normalized;
}

} // namespace QAccelPlot
```


