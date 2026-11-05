

# File AxisLabelLayout.hpp

[**File List**](files.md) **>** [**axis**](dir_4047c0a16b95170c37806a99233d1784.md) **>** [**internal**](dir_3acc16ec162842b49991cd4d9cea8fc4.md) **>** [**AxisLabelLayout.hpp**](AxisLabelLayout_8hpp.md)

[Go to the documentation of this file](AxisLabelLayout_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <Qt>

namespace QAccelPlot::Internal {

struct AxisLabelLayoutParams {
    QRectF dataRect;                  
    qreal baseline{0.0};              
    Qt::Edge outward{Qt::BottomEdge}; 
    qreal alongFraction{0.5};         
    QSizeF textSize;                  
    qreal rotation{0.0};              
    qreal padding{0.0};               
    QPointF offset;                   
};

[[nodiscard]] QSizeF rotatedBoundingSize(const QSizeF& size, qreal degrees);

[[nodiscard]] QPointF axisLabelCenter(const AxisLabelLayoutParams& params);

} // namespace QAccelPlot::Internal
```


