

# File GradientCoordinateUtils.hpp

[**File List**](files.md) **>** [**effects**](dir_2cb1ffa6338b0172fd78b92843e0e53d.md) **>** [**GradientCoordinateUtils.hpp**](GradientCoordinateUtils_8hpp.md)

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

#include "QAccelPlot/effects/GradientColorTypes.hpp"

#include <algorithm>

namespace QAccelPlot {

inline float unboundedGradientCoordinate(const GradientDirection direction, const qreal value, const qreal minimum, const qreal maximum)
{
    constexpr auto minimumRange = qreal{1e-12};
    const auto range = std::max(maximum - minimum, minimumRange);
    const auto normalized = (value - minimum) / range;
    return static_cast<float>(direction == GradientDirection::Vertical ? 1.0 - normalized : normalized);
}

} // namespace QAccelPlot
```


