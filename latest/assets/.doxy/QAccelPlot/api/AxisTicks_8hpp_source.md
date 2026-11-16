

# File AxisTicks.hpp

[**File List**](files.md) **>** [**axis**](dir_4047c0a16b95170c37806a99233d1784.md) **>** [**AxisTicks.hpp**](AxisTicks_8hpp.md)

[Go to the documentation of this file](AxisTicks_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QList>
#include <QString>

namespace QAccelPlot {

struct AxisTick {
    qreal value{0.0}; 
    QString label;    
};

struct AxisTicks {
    QList<qreal> subtickValues; 
    QList<AxisTick> majorTicks; 
};

struct AxisTickValues {
    QList<qreal> majorValues;   
    QList<qreal> subtickValues; 
    qreal step{0.0};            
    bool logScale{false};       
};

} // namespace QAccelPlot
```


