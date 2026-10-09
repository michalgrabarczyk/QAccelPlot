

# File RangeGesture.hpp

[**File List**](files.md) **>** [**axis**](dir_4047c0a16b95170c37806a99233d1784.md) **>** [**internal**](dir_3acc16ec162842b49991cd4d9cea8fc4.md) **>** [**RangeGesture.hpp**](RangeGesture_8hpp.md)

[Go to the documentation of this file](RangeGesture_8hpp.md)


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

namespace QAccelPlot::Internal {

struct ValueRange {
    qreal min{0.0}; 
    qreal max{1.0}; 
};

[[nodiscard]] ValueRange pannedRange(const ValueRange& range, qreal fraction, bool logarithmic);

[[nodiscard]] ValueRange zoomedRange(const ValueRange& range, qreal factor, qreal anchorRatio, bool logarithmic);

[[nodiscard]] qreal wheelZoomFactor(qreal zoomScaleFactor, bool zoomingIn);

} // namespace QAccelPlot::Internal
```


