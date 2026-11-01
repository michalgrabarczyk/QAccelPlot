

# File StepMode.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**StepMode.hpp**](StepMode_8hpp.md)

[Go to the documentation of this file](StepMode_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

namespace StepModeNS {
Q_NAMESPACE
QML_NAMED_ELEMENT(StepMode)


enum class Mode {
    None,   
    Before, 
    After,  
    Center, 
};
Q_ENUM_NS(Mode)
} // namespace StepModeNS

using StepMode = StepModeNS::Mode;

} // namespace QAccelPlot
```


