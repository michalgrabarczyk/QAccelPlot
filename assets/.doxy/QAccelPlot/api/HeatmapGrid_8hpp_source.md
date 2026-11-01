

# File HeatmapGrid.hpp

[**File List**](files.md) **>** [**internal**](dir_70e6e0d61970c92b37b608a046280901.md) **>** [**HeatmapGrid.hpp**](HeatmapGrid_8hpp.md)

[Go to the documentation of this file](HeatmapGrid_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <optional>
#include <utility>

namespace QAccelPlot {
struct AxisMapping;
}

namespace QAccelPlot::Internal {

struct HeatmapShaderMapping {
    enum class Mode {
        Affine = 0,      
        Exponential = 1, 
        Logarithmic = 2  
    };

    Mode mode{Mode::Affine}; 
    double scale{0.0};       
    double offset{0.0};      
    double rate{0.0};        
    double shift{0.0};       
};

struct HeatmapGridDimension {
    double from;     
    double to;       
    int count;       
    bool logSpacing; 

    bool isValid() const;
    double cellCoordinate(double value) const;
    double cellCenter(int index) const;
    std::optional<std::pair<double, double>> dataExtent(bool logAxis) const;
    HeatmapShaderMapping shaderMapping(const AxisMapping& axis, double length) const;
};

} // namespace QAccelPlot::Internal
```


