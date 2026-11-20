

# File GradientColorTypes.hpp

[**File List**](files.md) **>** [**effects**](dir_2cb1ffa6338b0172fd78b92843e0e53d.md) **>** [**GradientColorTypes.hpp**](GradientColorTypes_8hpp.md)

[Go to the documentation of this file](GradientColorTypes_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QColor>

#include <optional>
#include <vector>

namespace QAccelPlot {

struct GradientStopData {
    float position{0.0f};          
    QColor color{Qt::transparent}; 

    friend bool operator==(const GradientStopData& lhs, const GradientStopData& rhs)
    {
        return lhs.position == rhs.position && lhs.color == rhs.color;
    }

    friend bool operator!=(const GradientStopData& lhs, const GradientStopData& rhs)
    {
        return !(lhs == rhs);
    }
};

struct GradientColorPayload {
    bool isValid() const
    {
        return enabled && stops.size() >= 2;
    }

    bool enabled{false};                       
    Qt::Orientation direction{Qt::Horizontal}; 
    std::vector<GradientStopData> stops;       
    std::optional<qreal> gradientValueMin;
    std::optional<qreal> gradientValueMax;
};

struct GradientFillPayload {
    bool isValid() const
    {
        return enabled && opacity > 0.0f && stops.size() >= 2;
    }

    bool enabled{false};                       
    Qt::Orientation direction{Qt::Horizontal}; 
    std::vector<GradientStopData> stops;       
    std::optional<qreal> gradientValueMin;
    std::optional<qreal> gradientValueMax;
    std::optional<qreal> baselineValue;
    float opacity{1.0f}; 
};

} // namespace QAccelPlot
```


