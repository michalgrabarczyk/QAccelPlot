

# File GradientUtils.hpp

[**File List**](files.md) **>** [**effects**](dir_a4c3deeda37ae6198148ecdf2e43fc36.md) **>** [**GradientUtils.hpp**](GradientUtils_8hpp.md)

[Go to the documentation of this file](GradientUtils_8hpp.md)


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

#include <QColor>
#include <QObject>
#include <QQmlProperty>

#include <algorithm>
#include <vector>

namespace QAccelPlot {

inline void appendStopFromObject(std::vector<GradientStopData>& outStops, QObject* stopObject)
{
    if (!stopObject) {
        return;
    }

    const auto positionVariant = QQmlProperty::read(stopObject, QStringLiteral("position"));
    const auto colorVariant = QQmlProperty::read(stopObject, QStringLiteral("color"));

    if (!positionVariant.isValid() || !colorVariant.isValid()) {
        return;
    }

    const auto color = colorVariant.value<QColor>();
    if (!color.isValid()) {
        return;
    }

    const auto unclampedPosition = static_cast<float>(positionVariant.toReal());
    auto stopData = GradientStopData{};
    stopData.position = std::clamp(unclampedPosition, 0.0f, 1.0f);
    stopData.color = color;
    outStops.push_back(stopData);
}

} // namespace QAccelPlot
```


