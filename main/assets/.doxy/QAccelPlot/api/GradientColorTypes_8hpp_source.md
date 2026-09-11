

# File GradientColorTypes.hpp

[**File List**](files.md) **>** [**effects**](dir_a4c3deeda37ae6198148ecdf2e43fc36.md) **>** [**GradientColorTypes.hpp**](GradientColorTypes_8hpp.md)

[Go to the documentation of this file](GradientColorTypes_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>
#include <QtQml/qqmlregistration.h>

#include <optional>
#include <vector>

namespace QAccelPlot {

namespace GradientDirectionNS {
Q_NAMESPACE
QML_NAMED_ELEMENT(GradientDirection)


enum class Direction {
    Horizontal, 
    Vertical,   
};
Q_ENUM_NS(Direction)
} // namespace GradientDirectionNS

using GradientDirection = GradientDirectionNS::Direction;

namespace GradientFillBaselineNS {
Q_NAMESPACE
QML_NAMED_ELEMENT(GradientFillBaseline)


enum class Mode {
    AxisMinimum, 
    Value,       
};
Q_ENUM_NS(Mode)
} // namespace GradientFillBaselineNS

using GradientFillBaseline = GradientFillBaselineNS::Mode;

namespace GradientValueSourceNS {
Q_NAMESPACE
QML_NAMED_ELEMENT(GradientValueSource)


enum class Source {
    DataRange, 
    Fixed,     
};
Q_ENUM_NS(Source)
} // namespace GradientValueSourceNS

using GradientValueSource = GradientValueSourceNS::Source;

struct GradientStopData {
    float position{0.0f};          
    QColor color{Qt::transparent}; 
};

struct GradientColorPayload {
    bool isValid() const
    {
        return enabled && stops.size() >= 2;
    }

    bool enabled{false};                                        
    GradientDirection direction{GradientDirection::Horizontal}; 
    std::vector<GradientStopData> stops;                        
    std::optional<float> gradientValueMin;
    std::optional<float> gradientValueMax;
};

struct GradientFillPayload {
    bool isValid() const
    {
        return enabled && opacity > 0.0f && stops.size() >= 2;
    }

    bool enabled{false};                                        
    GradientDirection direction{GradientDirection::Horizontal}; 
    std::vector<GradientStopData> stops;                        
    std::optional<float> gradientValueMin;
    std::optional<float> gradientValueMax;
    GradientFillBaseline baseline{GradientFillBaseline::AxisMinimum}; 
    float baselineValue{0.0f};                                        
    float opacity{1.0f};                                              
};

} // namespace QAccelPlot
```


