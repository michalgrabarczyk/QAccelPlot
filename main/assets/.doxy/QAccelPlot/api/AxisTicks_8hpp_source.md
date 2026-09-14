

# File AxisTicks.hpp

[**File List**](files.md) **>** [**axis**](dir_f07047c7c39e03c70b8bf3fe272880c9.md) **>** [**AxisTicks.hpp**](AxisTicks_8hpp.md)

[Go to the documentation of this file](AxisTicks_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
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

} // namespace QAccelPlot
```


