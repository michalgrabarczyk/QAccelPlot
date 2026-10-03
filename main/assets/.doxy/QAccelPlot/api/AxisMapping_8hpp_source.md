

# File AxisMapping.hpp

[**File List**](files.md) **>** [**axis**](dir_4047c0a16b95170c37806a99233d1784.md) **>** [**AxisMapping.hpp**](AxisMapping_8hpp.md)

[Go to the documentation of this file](AxisMapping_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

namespace QAccelPlot {

struct AxisMapping {
    double origin{0.0};      
    double span{1.0};        
    bool logarithmic{false}; 
    bool flipped{false};     
    bool valid{false};       

    [[nodiscard]] double toMapped(double value) const noexcept;
    [[nodiscard]] double toPixel(double value, double length) const noexcept;
    [[nodiscard]] double toCoord(double pixel, double length) const noexcept;
};

} // namespace QAccelPlot
```


