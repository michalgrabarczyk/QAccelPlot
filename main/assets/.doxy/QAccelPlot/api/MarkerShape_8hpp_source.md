

# File MarkerShape.hpp

[**File List**](files.md) **>** [**QAccelPlot**](dir_84505bf06e96cd50072ae15b96eb466a.md) **>** [**src**](dir_3588d0448386bbe164b4703bb7530415.md) **>** [**QAccelPlot**](dir_0cbea278626d30118177d562182e643b.md) **>** [**series**](dir_70064bc2bead69da871bd372e93dce80.md) **>** [**MarkerShape.hpp**](MarkerShape_8hpp.md)

[Go to the documentation of this file](MarkerShape_8hpp.md)


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

namespace MarkerShapeNS {
Q_NAMESPACE
QML_NAMED_ELEMENT(MarkerShape)


enum class Shape {
    None,          
    Circle,        
    Square,        
    Diamond,       
    TriangleUp,    
    TriangleDown,  
    TriangleLeft,  
    TriangleRight, 
    Cross,         
    XCross,        
    HLine,         
    VLine,         
    Star,          
    Asterisk,      
    Pixel,         
    Hexagon,       
    Pentagon       
};
Q_ENUM_NS(Shape)
} // namespace MarkerShapeNS

using MarkerShape = MarkerShapeNS::Shape;

} // namespace QAccelPlot
```


