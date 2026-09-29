

# File RectGeometry.hpp

[**File List**](files.md) **>** [**internal**](dir_70e6e0d61970c92b37b608a046280901.md) **>** [**RectGeometry.hpp**](RectGeometry_8hpp.md)

[Go to the documentation of this file](RectGeometry_8hpp.md)


```C++
//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QtGlobal>

#include <utility>

namespace QAccelPlot {
class Axis;
}

namespace QAccelPlot::Internal {

qreal edgePixel(double value, const Axis& axis, qreal length);

std::pair<qreal, qreal> widenedSpan(qreal a, qreal b, qreal minimumSize);

} // namespace QAccelPlot::Internal
```


